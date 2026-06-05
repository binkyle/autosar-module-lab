# CanCommCoordinator Configuration Guide

## 通道配置（ChannelConfig）原则

`CanComm_ChannelConfigType` **只描述通道身份和分组信息**，禁止加入具体业务场景字段（如 `keepInAdasOnly`、`keepInDiagOnly`、`releasableInParkingOnly` 等）。

业务策略应通过 `CanComm_OutputActionSetType` 的 bit mask 表达。

## 如何配置 Channel

编辑 `src/CanCommCoordinator_Cfg.c` 中的 `CanComm_ChannelConfigTable`：

```c
const CanComm_ChannelConfigType CanComm_ChannelConfigTable[CANCOMM_CH_NUM] =
{
    {
        .logicalCh   = CANCOMM_CH_ADAS1,
        .commUser    = ComMConf_ComMUser_CN_Backbone2_4e435fd4,
        .nmChannel   = 0u,
        .nmRxPduId   = 0u,
        .channelMask = CANCOMM_MASK_ADAS1,
        .groupMask   = CANCOMM_MASK_ADAS1   /* ADAS 分组 */
    },
    /* ... */
};
```

| 字段 | 说明 |
|------|------|
| `logicalCh` | 逻辑通道 ID（项目无关枚举） |
| `commUser` | ComM User Handle（Vector 生成符号，仅在此引用） |
| `nmChannel` | Nm Network Handle |
| `nmRxPduId` | NM RX PDU ID（Legacy parse 用） |
| `channelMask` | 通道唯一 bit mask（必须是 2 的幂） |
| `groupMask` | 通道分组 mask（可用于批量策略） |

禁止在 Policy 或 State 中硬编码通道索引。

## 如何配置 ActionSet

`CanComm_OutputActionSetType` 使用 bit mask 描述策略输出，与具体通道解耦：

```c
static const CanComm_OutputActionSetType CanComm_ActionAdasOnly =
{
    .fullComMask    = CANCOMM_MASK_ADAS1,
    .noComMask      = (CANCOMM_MASK_ADAS2 | CANCOMM_MASK_ADAS_SUB |
                       CANCOMM_MASK_PDC | CANCOMM_MASK_CAN5 | CANCOMM_MASK_CAN9),
    .txUnblockMask  = CANCOMM_MASK_ADAS1,
    .txBlockMask    = (CANCOMM_MASK_ADAS2 | CANCOMM_MASK_ADAS_SUB |
                       CANCOMM_MASK_PDC | CANCOMM_MASK_CAN5 | CANCOMM_MASK_CAN9),
    .bswmRequest    = CANCOMM_BSWM_REQUEST_FULL,
    .sleepCandidate = FALSE
};
```

Policy 引擎在运行时根据 `ActionSet.mask` + `ChannelConfig.channelMask` 自动生成每个通道的 `OutputDecision`。

## SourceMask 配置

### 配置 Source 数量

编辑 `include/CanCommCoordinator_Cfg.h`：

```c
#define CANCOMM_SOURCE_WORD_NUM          2u   /* 2 words = 64 sources */
#define CANCOMM_SOURCE_BITS_PER_WORD     32u
#define CANCOMM_MAX_SOURCE_NUM           (CANCOMM_SOURCE_WORD_NUM * CANCOMM_SOURCE_BITS_PER_WORD)
```

如需支持超过 64 个 source，增加 `CANCOMM_SOURCE_WORD_NUM` 即可，核心代码无需修改。

### 可选功能开关

```c
#define CANCOMM_ENABLE_SOURCE_CONFIG_TABLE  STD_OFF  /* SourceConfigTable（调试/诊断用） */
#define CANCOMM_ENABLE_SOURCE_DEBUG_NAME    STD_OFF  /* source 名称字符串（调试/日志用） */
```

默认 `STD_OFF`，以节省 ROM。如需在运行时查看 source 名称或配置属性，可设为 `STD_ON`。

### 在项目层定义 Source ID

核心框架**不定义**具体 source 语义。项目层在 `src/CanCommCoordinator_Cfg.c` 中自行分配 ID：

```c
#define CANCOMM_SRC_ID_IGNITION         0u
#define CANCOMM_SRC_ID_PNC_GLOBAL       1u
#define CANCOMM_SRC_ID_PNC_ADAS         2u
#define CANCOMM_SRC_ID_DIAG_KEEP        3u
#define CANCOMM_SRC_ID_OTA_KEEP         4u
#define CANCOMM_SRC_ID_LOCAL_KEEP       5u
#define CANCOMM_SRC_ID_HOLD_VALID       6u
```

## 如何配置 PolicyRule

规则表使用 **priority + mask** 模型。引擎选择 **priority 最高**的匹配规则；priority 相同则表中靠前的优先。

每个规则包含：

| 字段 | 说明 |
|------|------|
| `priority` | 优先级，数值越大越优先 |
| `requiredAllMask` | active source mask 必须包含的全部 bit |
| `requiredAnyMask` | active source mask 至少包含一个 bit（空则自动满足） |
| `forbiddenMask` | active source mask 不能包含的任何 bit |
| `actionSet` | 指向 `CanComm_OutputActionSetType` |

示例：

```c
const CanComm_PolicyRuleType CanComm_PolicyRuleTable[] =
{
    /* IGNITION ON -> all FULL (priority 100) */
    {
        .priority       = 100u,
        .requiredAllMask.word = { SRC_MASK(CANCOMM_SRC_ID_IGNITION), 0u },
        .requiredAnyMask.word = { 0u, 0u },
        .forbiddenMask.word   = { 0u, 0u },
        .actionSet      = &CanComm_ActionAllFull
    },
    /* PNC_GLOBAL + HOLD_VALID -> all FULL (priority 90) */
    {
        .priority       = 90u,
        .requiredAllMask.word = { SRC_MASK(CANCOMM_SRC_ID_PNC_GLOBAL)
                                  | SRC_MASK(CANCOMM_SRC_ID_HOLD_VALID), 0u },
        .requiredAnyMask.word = { 0u, 0u },
        .forbiddenMask.word   = { 0u, 0u },
        .actionSet      = &CanComm_ActionAllFull
    },
    /* catch-all: no active sources -> all NO (priority 0) */
    {
        .priority       = 0u,
        .requiredAllMask.word = { 0u, 0u },
        .requiredAnyMask.word = { 0u, 0u },
        .forbiddenMask.word   = { 0xFFFFFFFFu, 0xFFFFFFFFu }, /* 只在空 mask 时匹配 */
        .actionSet      = &CanComm_ActionAllNo
    }
};
```

## 如何配置 Fallback

Fallback 是一个 `CanComm_OutputActionSetType` 指针：

```c
const CanComm_OutputActionSetType* const CanComm_FallbackActionSet = &CanComm_ActionAllFull;
```

当没有规则匹配时（例如输入包含未在任何规则中定义的 source 组合），Policy 引擎使用此 ActionSet 生成 OutputDecision。Fallback 是最终安全网，建议配置为保守状态（如全部 FULL）。

## 如何接入 BSW

1. 实现 `project/vector/CanCommCoordinator_VectorAdapter.c` 中的 TODO
2. 将 `Com_IpduGroupStart/Stop`、`Nm_Enable/DisableCommunication` 迁移到 BswM ActionList
3. 在 BswM Rule 中引用 `CanCommCoordinator` 输出的模式请求（如果需要）

## Legacy NM Parse 配置

```c
#define CANCOMM_ENABLE_LEGACY_NM_PARSE STD_ON
```

在 `CanCommCoordinator_Cfg.c` 中配置字节偏移和掩码：

```c
const CanComm_LegacyNmParseConfigType CanComm_LegacyNmParseConfig =
{
    .pniByteOffset = 5u,
    .pniBitMask = 0x40u,
    .pncGlobalByteOffset = 5u,
    .pncGlobalBitMask = 0x08u,
    .pncAdasByteOffset = 6u,
    .pncAdasBitMask = 0x01u
};
```
