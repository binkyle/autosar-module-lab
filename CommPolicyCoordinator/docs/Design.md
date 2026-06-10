# CommPolicyCoordinator Design Document

## 模块定位

`CommPolicyCoordinator` 是项目级 **跨总线通用通信策略协调框架**，位于应用层与标准 BSW 之间。

它**不是**以下标准模块的替代品：
- Nm、ComM、BswM、CanSM、CanIf、CanTrcv、Com、EcuM、LinSM、EthSM

它应遵循：
```text
标准 BSW 能做的，优先交给 BSW；
BSW 不能完整表达的项目组合策略，才放到 CommPolicyCoordinator；
核心策略不直接依赖 Vector/DaVinci 生成符号；
项目绑定放 Adapter/Cfg；
框架必须可单元测试、可 host 编译、可迁移。
```

### 禁止长期存在的内容

以下行为不能成为长期架构：
- 直接解析 NM PDU 原始字节（PNI/PNC bit）
- 直接调用 `Com_IpduGroupStart/Stop`
- 直接调用 `Nm_EnableCommunication/Nm_DisableCommunication`
- 在核心 Policy 中引用 Vector/DaVinci 生成符号
- 硬编码通道索引或字节偏移
- 在核心框架中嵌入项目业务语义（ADAS/PNC 等）

## 分层架构

```text
┌─────────────────────────────────────────┐
│  CommPolicyCoordinator.c (External API) │
│  Init / MainFunction                    │
├─────────────────────────────────────────┤
│  CommPolicyCoordinator_App.c            │
│  State_Tick → BuildSnapshot → Policy    │
│  → ApplyDecision → Debug_Update         │
├─────────────────────────────────────────┤
│  CommPolicy_Policy.c                    │
│  Pure table-driven policy engine        │
│  Match ActionSet → Build OutputDecision │
│  Zero BSW dependency                    │
├─────────────────────────────────────────┤
│  CommPolicy_State.c                     │
│  Hold timer, runtime state, exclusive   │
├─────────────────────────────────────────┤
│  CommPolicy_Adapter.c                   │
│  Abstract interface (exclusive area)    │
│  BuildSnapshot / ApplyDecision skeleton │
├─────────────────────────────────────────┤
│  CommPolicy_VectorAdapter.c             │
│  ComM_RequestComMode, BswM_xxx, SchM    │
├─────────────────────────────────────────┤
│  BSW (ComM, Nm, BswM, ...)              │
└─────────────────────────────────────────┘
```

**关键设计约束**：
- `ChannelConfig` 只描述通道身份/分组/总线类型，不包含业务策略。
- `ActionSet` 使用 `ChannelMask` 描述策略输出，与通道索引解耦。
- `Policy` 引擎将 `ActionSet` 翻译为 `OutputDecision`（per-channel 数组）。

## 核心数据流

1. **Input Gathering**: Adapter 从外部获取各通信保持源（如 PowerRun、Network、Diag、OTA 等）的激活状态
2. **Snapshot Build**: App 层将输入组装为 `CommPolicy_InputSnapshotType`，内部使用 `CommPolicy_SourceMaskType`（multi-word bitset）表示激活源集合
3. **Policy Evaluate**: 查表引擎根据 `priority + requiredAllMask + requiredAnyMask + forbiddenMask` 匹配最佳规则，得到 `CommPolicy_OutputActionSetType`
4. **Decision Build**: Policy 引擎根据 `ActionSet` 中的 `ChannelMask` 生成 `CommPolicy_OutputDecisionType`
5. **Decision Apply**: `CommPolicy_VectorAdapter.c` 通过 `CommPolicy_VectorChannelBindingTable` 将逻辑通道映射到 ComM User，并将核心 `CommPolicy_CommModeType` 映射为 `ComM_ModeType` 后调用 `ComM_RequestComMode`；`BswM_RequestMode` 等通过项目配置输出
6. **Debug Update**: 记录最近一次输入与决策，用于诊断

### 为什么引入 ActionSet

将策略输出从 `OutputDecision`（per-channel 数组）抽象为 `ActionSet`（ChannelMask）有以下好处：
- **解耦业务与通道**: 新增/删除通道时，只需修改 `ChannelConfigTable`，无需修改 ActionSet。
- **禁止特化字段**: 防止在 `ChannelConfig` 中加入 `keepInAdasOnly`、`releasableInParkingOnly` 等具体场景字段。
- **可测试性**: ActionSet 是纯数据，可在 host 环境直接验证 mask 组合。

## 与 BSW 的关系

| 功能 | 负责模块 | CommPolicyCoordinator 角色 |
|------|---------|---------------------------|
| PNI/PNC bit 识别 | CanNm / Nm / ComM | 不解析，由 BSW 上报 source 状态 |
| PNC 状态机 | ComM | 读取 ComM 模式，不替代 |
| 通道模式请求 | ComM | `VectorAdapter` 通过 `VectorChannelBindingTable` 调用 `ComM_RequestComMode`；核心框架只操作 `CommPolicy_CommModeType` |
| I-PDU Group 控制 | Com + BswM ActionList | 建议迁移到 BswM，不直接调用 Com_IpduGroupStart/Stop |
| Nm Enable/Disable | Nm | 建议迁移到 BswM ActionList |
| Sleep/ShutDown | EcuM + BswM | 只输出 sleepCandidate，不直接操作 EcuM |

## 核心类型与 ComM 解耦

核心框架定义独立的 `CommPolicy_CommModeType`：

```c
typedef enum
{
    COMM_POLICY_NO_COMMUNICATION = 0,
    COMM_POLICY_SILENT_COMMUNICATION,
    COMM_POLICY_FULL_COMMUNICATION
} CommPolicy_CommModeType;
```

- **核心 `include/` 和 `src/` 不引用 `ComM.h`**，也不使用 `ComM_ModeType`、`ComM_UserHandleType` 或 `NetworkHandleType`
- `CommPolicy_VectorAdapter.c` 负责双向映射：`CommPolicy_CommModeType` ↔ `ComM_ModeType`
- `CommPolicy_VectorChannelBindingTable` 保存逻辑通道到 ComM User / Network Handle 的映射，使 `ChannelConfigType` 保持零 BSW 依赖
- Host 测试和 generic 项目无需 ComM stub 即可编译和运行

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
    uint32 word[COMM_POLICY_SOURCE_WORD_NUM];
} CommPolicy_SourceMaskType;
```

- **核心框架零 source 语义**：POWER_RUN/NETWORK/DIAG/OTA 等名称只出现在项目配置层
- **可扩展**：通过 `COMM_POLICY_SOURCE_WORD_NUM`（默认 2 words = 64 sources）自由配置 source 数量
- **统一接口**：所有 source 通过 `CommPolicy_SourceMask_Set/Reset/IsSet` 操作，无特化字段

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
#define COMM_POLICY_ENABLE_SOURCE_CONFIG_TABLE  STD_OFF
#define COMM_POLICY_ENABLE_SOURCE_DEBUG_NAME    STD_OFF
```

默认均为 `STD_OFF`，以节省 ROM。

## ChannelMask 框架设计

### 为什么引入 ChannelMask

旧设计使用单个 `uint32` 表示通道掩码，导致：
- 最多只能支持 32 个通道
- 无法表达复杂的多通道分组策略

新设计使用 **multi-word bitset**：
```c
typedef struct {
    uint32 word[COMM_POLICY_CHANNEL_WORD_NUM];
} CommPolicy_ChannelMaskType;
```

- **可扩展**：通过 `COMM_POLICY_CHANNEL_WORD_NUM`（默认 2 words = 64 channels）自由配置通道数量
- **与 SourceMask 同构**：使用相同的工具函数设计模式（Clear, Set, Reset, IsSet, IsEmpty, IsAllSet, IsAnySet, IsNoneSet, Copy, Or）
- **解耦策略与通道索引**：`ActionSet` 使用 `ChannelMask` 而非直接与通道索引绑定

### ChannelMask 与 ActionSet 的关系

`CommPolicy_OutputActionSetType` 使用四个 `ChannelMask` 描述策略输出：

| Mask | 语义 |
|------|------|
| `fullComMask` | 需要设置为 FULL_COMMUNICATION 的通道 |
| `noComMask` | 需要设置为 NO_COMMUNICATION 的通道 |
| `txUnblockMask` | 需要 TX_UNBLOCK 的通道 |
| `txBlockMask` | 需要 TX_BLOCK 的通道 |

Policy 引擎通过 `CommPolicy_ChannelMask_IsSet` 逐通道判断，生成 `OutputDecision`。

## 状态机设计

```text
[Init] --init--> [Idle]
[Idle] --OnSourceActive(src)--> 设置 SourceMask bit
[Idle] --OnSourceInactive(src)--> 清除 SourceMask bit
[Hold Valid] --tick/timeout--> 清除 HOLD_VALID bit
[SourceMask empty] + no rule match --> Fallback (safe FULL)
[SourceMask empty] --> catch-all rule (All No / Sleep Candidate)
```

State 层管理 `HOLD_VALID` source（id 由项目配置），trigger source 激活时启动 timer，超时后自动清除对应的 hold source bit。
