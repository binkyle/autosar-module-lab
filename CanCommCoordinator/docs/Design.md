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

1. **Input Gathering**: Adapter 从外部获取 KL15、PNC、LocalKeepAwake 等信号
2. **Snapshot Build**: App 层将输入组装为 `CanComm_InputSnapshotType`
3. **Policy Evaluate**: 查表引擎根据输入匹配规则，得到 `CanComm_OutputActionSetType`
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

## 状态机设计

```text
[Init] --init--> [Idle]
[Idle] --OnNmRxIndication--> [Hold Valid] (reload timer)
[Hold Valid] --tick/timeout--> [Hold Invalid]
[Hold Valid] --KL15 ON--> [All Full]
[Hold Invalid] + no local keep --> [All No / Sleep Candidate]
```
