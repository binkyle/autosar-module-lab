# CanCommCoordinator Design Document

## 模块定位

`CanCommCoordinator` 是项目通信策略协调模块，位于应用层与标准 BSW 之间。

它**不是**以下标准模块的替代品：
- CanNm、Nm、ComM、BswM、CanSM、CanIf、CanTrcv、Com、EcuM

它应遵循：
```text
标准 BSW 能做的，优先交给 BSW；
BSW 不能完整表达的项目组合策略，才放到 CanCommCoordinator；
核心策略不直接依赖 Vector/DaVinci 生成符号；
项目绑定放 Adapter/Cfg；
模块必须可单元测试、可 host 编译、可迁移。
```

### 禁止长期存在的内容

以下行为只能作为**迁移期兼容模式**存在，不能成为长期架构：
- 直接解析 NM PDU 原始字节（PNI/PNC bit）
- 直接调用 `Com_IpduGroupStart/Stop`
- 直接调用 `Nm_EnableCommunication/Nm_DisableCommunication`
- 在核心 Policy 中引用 Vector/DaVinci 生成符号
- 硬编码通道索引或字节偏移

### 标准模式 vs 兼容模式

| 功能 | 标准模式（目标） | 迁移期兼容模式 |
|------|----------------|--------------|
| PNC 识别 | CanNm / ComM PNC | Legacy NM parse（`CANCOMM_ENABLE_LEGACY_NM_PARSE = STD_ON`） |
| I-PDU Group 控制 | BswM ActionList | 模块直接调用 Com_IpduGroupStart/Stop（不推荐） |
| Nm Enable/Disable | BswM ActionList | 模块直接调用 Nm_Enable/DisableCommunication（不推荐） |
| 通道模式请求 | ComM_RequestComMode via Adapter | 相同，但允许临时硬编码 |

迁移完成后，应将 `CANCOMM_ENABLE_LEGACY_NM_PARSE` 设置为 `STD_OFF` 并删除相关代码。

## 分层架构

```text
┌─────────────────────────────────────────┐
│  CanCommCoordinator.c (External API)    │
│  Init / MainFunction / OnNmRxIndication │
├─────────────────────────────────────────┤
│  CanCommCoordinator_App.c               │
│  State_Tick → BuildSnapshot → Policy    │
│  → ApplyDecision → Debug_Update         │
├─────────────────────────────────────────┤
│  CanCommCoordinator_Policy.c            │
│  Pure table-driven policy engine        │
│  Match ActionSet → Build OutputDecision │
│  Zero BSW dependency                    │
├─────────────────────────────────────────┤
│  CanCommCoordinator_State.c             │
│  Hold timer, runtime state, exclusive   │
├─────────────────────────────────────────┤
│  CanCommCoordinator_Adapter.c           │
│  Abstract interface (exclusive area)    │
│  BuildSnapshot / ApplyDecision skeleton │
├─────────────────────────────────────────┤
│  CanCommCoordinator_VectorAdapter.c     │
│  ComM_RequestComMode, BswM_xxx, SchM    │
├─────────────────────────────────────────┤
│  BSW (ComM, Nm, CanNm, BswM, ...)       │
└─────────────────────────────────────────┘
```

**关键设计约束**：
- `ChannelConfig` 只描述通道身份/分组，不包含业务策略。
- `ActionSet` 使用 bit mask 描述策略输出，与通道索引解耦。
- `Policy` 引擎将 `ActionSet` 翻译为 `OutputDecision`（per-channel 数组）。

## 核心数据流

1. **Input Gathering**: Adapter 从外部获取各通信保持源（如 KL15、PNC、Diag、OTA 等）的激活状态
2. **Snapshot Build**: App 层将输入组装为 `CanComm_InputSnapshotType`，内部使用 `CanComm_SourceMaskType`（multi-word bitset）表示激活源集合
3. **Policy Evaluate**: 查表引擎根据 `priority + requiredAllMask + requiredAnyMask + forbiddenMask` 匹配最佳规则，得到 `CanComm_OutputActionSetType`
4. **Decision Build**: Policy 引擎根据 `ActionSet.mask` + `ChannelConfig.channelMask` 生成 `CanComm_OutputDecisionType`
5. **Decision Apply**: Adapter 将决策转换为 ComM_RequestComMode、BswM_RequestMode 等调用
6. **Debug Update**: 记录最近一次输入与决策，用于诊断

### 为什么引入 ActionSet

将策略输出从 `OutputDecision`（per-channel 数组）抽象为 `ActionSet`（bit mask）有以下好处：
- **解耦业务与通道**: 新增/删除通道时，只需修改 `ChannelConfigTable`，无需修改 ActionSet。
- **禁止特化字段**: 防止在 `ChannelConfig` 中加入 `keepInAdasOnly`、`releasableInParkingOnly` 等具体场景字段。
- **可测试性**: ActionSet 是纯数据，可在 host 环境直接验证 mask 组合。

## 与 BSW 的关系

| 功能 | 负责模块 | CanCommCoordinator 角色 |
|------|---------|------------------------|
| PNI/PNC bit 识别 | CanNm / Nm | 不解析（Legacy 模式除外） |
| PNC 状态机 | ComM | 读取 ComM 模式，不替代 |
| 通道模式请求 | ComM | 通过 Adapter 调用 ComM_RequestComMode |
| I-PDU Group 控制 | Com + BswM ActionList | 建议迁移到 BswM，不直接调用 Com_IpduGroupStart/Stop |
| Nm Enable/Disable | Nm | 建议迁移到 BswM ActionList |
| Sleep/ShutDown | EcuM + BswM | 只输出 sleepCandidate，不直接操作 EcuM |

## 为什么不能直接复刻 CanServer

- `CanServer` 通常紧耦合 Vector 生成符号、硬编码通道索引、直接解析 NM 原始字节
- 无法在不同项目间迁移
- 无法在 host 环境进行单元测试
- 策略逻辑与 BSW 调用混杂，难以维护和审计
- 长期直接持有 `Com_IpduGroupStart/Stop`、`Nm_Enable/DisableCommunication` 等本应由 BswM 管理的调用

## SourceMask 框架设计

### 为什么引入 SourceMask

旧设计使用显式枚举字段（`ignition`、`pncRequest`、`holdState` 等），导致：
- 每新增一个 source 都要修改核心类型定义
- 核心代码耦合项目具体的 source 语义（IGNITION、PNC_ADAS、DIAG 等）
- 最多只能支持 32 个 source（单个 `uint32`）

新设计使用 **multi-word bitset**：
```c
typedef struct {
    uint32 word[CANCOMM_SOURCE_WORD_NUM];
} CanComm_SourceMaskType;
```

- **核心框架零 source 语义**：IGNITION/PNC/DIAG/OTA 等名称只出现在项目配置层（`Cfg.c`）
- **可扩展**：通过 `CANCOMM_SOURCE_WORD_NUM`（默认 2 words = 64 sources）自由配置 source 数量
- **统一接口**：所有 source 通过 `CanComm_SourceMask_Set/Reset/IsSet` 操作，无特化字段

### SourceMask 与 Policy 的关系

Policy 规则使用三个 mask 描述匹配条件：

| Mask | 语义 | 空 mask 行为 |
|------|------|-------------|
| `requiredAllMask` | active 必须包含全部 bit | 空 = 自动满足 |
| `requiredAnyMask` | active 至少包含一个 bit | 空 = 自动满足 |
| `forbiddenMask` | active 不能包含任何 bit | 空 = 自动满足 |

引擎选择 **priority 最高**的匹配规则；priority 相同则表中靠前的优先。无匹配时触发 `FallbackActionSet`。

### 可选配置项

通过条件编译控制：

```c
#define CANCOMM_ENABLE_SOURCE_CONFIG_TABLE  STD_OFF  /* SourceConfigTable 是否编译进镜像 */
#define CANCOMM_ENABLE_SOURCE_DEBUG_NAME    STD_OFF  /* debugName 字符串是否进入结构体 */
```

默认均为 `STD_OFF`，以节省 ROM。

## 状态机设计

```text
[Init] --init--> [Idle]
[Idle] --OnSourceActive(src)--> 设置 SourceMask bit
[Idle] --OnSourceInactive(src)--> 清除 SourceMask bit
[Hold Valid] --tick/timeout--> 清除 HOLD_VALID bit
[SourceMask empty] + no rule match --> Fallback (safe FULL)
[SourceMask empty] --> catch-all rule (All No / Sleep Candidate)
```

State 层管理 `HOLD_VALID` source（id 由项目配置），PNC source 激活时启动 timer，超时后自动清除对应的 source bit。
