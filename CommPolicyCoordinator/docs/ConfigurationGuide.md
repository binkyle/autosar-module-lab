# CommPolicyCoordinator Configuration Guide

## 通道配置（ChannelConfig）原则

`CommPolicy_ChannelConfigType` **只描述通道身份、分组信息和总线类型**，禁止加入具体业务场景字段（如 `keepInAdasOnly`、`keepInDiagOnly`、`releasableInParkingOnly` 等）。

业务策略应通过 `CommPolicy_OutputActionSetType` 的 `ChannelMask` 表达。

## 如何配置 Channel

编辑项目配置层中的 `CommPolicy_ChannelConfigTable`：

```c
const CommPolicy_ChannelConfigType CommPolicy_ChannelConfigTable[COMM_POLICY_CH_NUM] =
{
    {
        .logicalChannel = 0u,
        .busType        = COMM_POLICY_BUS_CAN,
        .commUser       = 0u,
        .networkHandle  = 0u,
        .channelMask    = { .word = { CH_MASK(0u), 0u } },
        .groupMask      = { .word = { CH_MASK(0u), 0u } }
    },
    /* ... */
};
```

| 字段 | 说明 |
|------|------|
| `logicalChannel` | 逻辑通道 ID |
| `busType` | 总线类型（CAN / LIN / ETH / FR / USER） |
| `commUser` | ComM User Handle |
| `networkHandle` | Network Handle |
| `channelMask` | 通道唯一 `ChannelMask`（单 bit） |
| `groupMask` | 通道分组 mask（可用于批量策略） |

禁止在 Policy 或 State 中硬编码通道索引。

## 如何配置 ActionSet

`CommPolicy_OutputActionSetType` 使用 `ChannelMask` 描述策略输出，与具体通道解耦：

```c
static const CommPolicy_OutputActionSetType CommPolicy_ActionPartialOnly =
{
    .fullComMask    = { .word = { CH_MASK(0u), 0u } },
    .noComMask      = { .word = { 0x3Eu, 0u } },
    .txUnblockMask  = { .word = { CH_MASK(0u), 0u } },
    .txBlockMask    = { .word = { 0x3Eu, 0u } },
    .modeRequestId  = EXAMPLE_MODE_REQ_ALL_FULL,
    .sleepCandidate = FALSE
};
```

Policy 引擎在运行时根据 `ActionSet` 中的 `ChannelMask` 逐通道判断，生成 `OutputDecision`。

## SourceMask 配置

### 配置 Source 数量

编辑 `include/CommPolicy_Cfg.h`：

```c
#define COMM_POLICY_SOURCE_WORD_NUM          2u   /* 2 words = 64 sources */
#define COMM_POLICY_SOURCE_BITS_PER_WORD     32u
#define COMM_POLICY_MAX_SOURCE_NUM           (COMM_POLICY_SOURCE_WORD_NUM * COMM_POLICY_SOURCE_BITS_PER_WORD)
```

如需支持超过 64 个 source，增加 `COMM_POLICY_SOURCE_WORD_NUM` 即可，核心代码无需修改。

### 可选功能开关

```c
#define COMM_POLICY_ENABLE_SOURCE_CONFIG_TABLE  STD_OFF
#define COMM_POLICY_ENABLE_SOURCE_DEBUG_NAME    STD_OFF
```

默认 `STD_OFF`，以节省 ROM。如需在运行时查看 source 名称或配置属性，可设为 `STD_ON`。

### 在项目层定义 Source ID

核心框架**不定义**具体 source 语义。项目层在配置 `.c` 文件中自行分配 ID：

```c
#define EXAMPLE_SRC_ID_POWER_RUN        0u
#define EXAMPLE_SRC_ID_NETWORK_GLOBAL   1u
#define EXAMPLE_SRC_ID_NETWORK_PARTIAL  2u
#define EXAMPLE_SRC_ID_DIAG             3u
#define EXAMPLE_SRC_ID_OTA              4u
#define EXAMPLE_SRC_ID_LOCAL_KEEP       5u
#define EXAMPLE_SRC_ID_HOLD_VALID       6u
```

## ChannelMask 配置

### 配置 Channel 数量

编辑 `include/CommPolicy_Cfg.h`：

```c
#define COMM_POLICY_CHANNEL_WORD_NUM          2u   /* 2 words = 64 channels */
#define COMM_POLICY_CHANNEL_BITS_PER_WORD     32u
#define COMM_POLICY_MAX_CHANNEL_NUM           (COMM_POLICY_CHANNEL_WORD_NUM * COMM_POLICY_CHANNEL_BITS_PER_WORD)
```

如需支持超过 64 个通道，增加 `COMM_POLICY_CHANNEL_WORD_NUM` 即可，核心代码无需修改。

## 如何配置 PolicyRule

规则表使用 **priority + mask** 模型。引擎选择 **priority 最高**的匹配规则；priority 相同则表中靠前的优先。

每个规则包含：

| 字段 | 说明 |
|------|------|
| `priority` | 优先级，数值越大越优先 |
| `requiredAllMask` | active source mask 必须包含的全部 bit |
| `requiredAnyMask` | active source mask 至少包含一个 bit（空则自动满足） |
| `forbiddenMask` | active source mask 不能包含的任何 bit |
| `actionSet` | 指向 `CommPolicy_OutputActionSetType` |

示例：

```c
const CommPolicy_PolicyRuleType CommPolicy_PolicyRuleTable[] =
{
    /* POWER_RUN -> all FULL (priority 100) */
    {
        .priority       = 100u,
        .requiredAllMask.word = { SRC_MASK(EXAMPLE_SRC_ID_POWER_RUN), 0u },
        .requiredAnyMask.word = { 0u, 0u },
        .forbiddenMask.word   = { 0u, 0u },
        .actionSet      = &CommPolicy_ActionAllFull
    },
    /* NETWORK_GLOBAL + HOLD_VALID -> all FULL (priority 90) */
    {
        .priority       = 90u,
        .requiredAllMask.word = { SRC_MASK(EXAMPLE_SRC_ID_NETWORK_GLOBAL)
                                  | SRC_MASK(EXAMPLE_SRC_ID_HOLD_VALID), 0u },
        .requiredAnyMask.word = { 0u, 0u },
        .forbiddenMask.word   = { 0u, 0u },
        .actionSet      = &CommPolicy_ActionAllFull
    },
    /* catch-all: no active sources -> all NO (priority 0) */
    {
        .priority       = 0u,
        .requiredAllMask.word = { 0u, 0u },
        .requiredAnyMask.word = { 0u, 0u },
        .forbiddenMask.word   = { 0xFFFFFFFFu, 0xFFFFFFFFu },
        .actionSet      = &CommPolicy_ActionAllNo
    }
};
```

## 如何配置 Fallback

Fallback 是一个 `CommPolicy_OutputActionSetType` 指针：

```c
const CommPolicy_OutputActionSetType* const CommPolicy_FallbackActionSet = &CommPolicy_ActionAllFull;
```

当没有规则匹配时（例如输入包含未在任何规则中定义的 source 组合），Policy 引擎使用此 ActionSet 生成 OutputDecision。Fallback 是最终安全网，建议配置为保守状态（如全部 FULL）。

## 如何接入 BSW

1. 实现 `project/vector/CommPolicy_VectorAdapter.c` 中的 TODO
2. 将 `Com_IpduGroupStart/Stop`、`Nm_Enable/DisableCommunication` 迁移到 BswM ActionList
3. 在 BswM Rule 中引用 `CommPolicyCoordinator` 输出的 `modeRequestId`（如果需要）
