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

## 如何配置 PolicyRule

规则表按 **first match wins** 顺序排列。建议：
1. 最具体的规则放前面
2. catch-all / fallback 由引擎处理

每个规则指向一个 `CanComm_OutputActionSetType`：

```c
const CanComm_PolicyRuleType CanComm_PolicyRuleTable[] =
{
    {
        .ignition = CANCOMM_IGN_OFF,
        .pncRequest = CANCOMM_PNC_ADAS_ONLY,
        .holdState = CANCOMM_HOLD_VALID,
        .localKeepAwake = CANCOMM_LOCAL_KEEP_NONE,
        .actionSet = &CanComm_ActionAdasOnly
    }
};
```

## 如何配置 Fallback

Fallback 是一个 `CanComm_OutputActionSetType` 指针：

```c
const CanComm_OutputActionSetType* const CanComm_FallbackActionSet = &CanComm_ActionAllFull;
```

当没有规则匹配时，Policy 引擎使用此 ActionSet 生成 OutputDecision。

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
