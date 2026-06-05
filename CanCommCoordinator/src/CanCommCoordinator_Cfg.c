#include "CanCommCoordinator_Cfg.h"
#include "CanCommCoordinator_Types.h"
#include "ComM.h"

/* --- Output Action Set Templates ---------------------------------------- */
/* Action: All channels FULL_COMMUNICATION */
static const CanComm_OutputActionSetType CanComm_ActionAllFull =
{
    .fullComMask    = CANCOMM_MASK_ALL,
    .noComMask      = 0u,
    .txUnblockMask  = CANCOMM_MASK_ALL,
    .txBlockMask    = 0u,
    .bswmRequest    = CANCOMM_BSWM_REQUEST_FULL,
    .sleepCandidate = FALSE
};

/* Action: All channels NO_COMMUNICATION, sleep candidate */
static const CanComm_OutputActionSetType CanComm_ActionAllNo =
{
    .fullComMask    = 0u,
    .noComMask      = CANCOMM_MASK_ALL,
    .txUnblockMask  = 0u,
    .txBlockMask    = CANCOMM_MASK_ALL,
    .bswmRequest    = CANCOMM_BSWM_REQUEST_SLEEP_CANDIDATE,
    .sleepCandidate = TRUE
};

/* Action: ADAS_ONLY -> ADAS1 FULL, others NO */
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

/* --- Policy Rule Table -------------------------------------------------- */
/*
 * Rule matching order: first match wins.
 * Recommend placing most specific rules first, catch-all last.
 */
const CanComm_PolicyRuleType CanComm_PolicyRuleTable[] =
{
    /* 1. KL15 ON -> all FULL */
    {
        .ignition = CANCOMM_IGN_ON,
        .pncRequest = CANCOMM_PNC_NONE,
        .holdState = CANCOMM_HOLD_INVALID,
        .localKeepAwake = CANCOMM_LOCAL_KEEP_NONE,
        .actionSet = &CanComm_ActionAllFull
    },
    /* 2. KL15 OFF + PNC_GLOBAL valid -> all FULL */
    {
        .ignition = CANCOMM_IGN_OFF,
        .pncRequest = CANCOMM_PNC_GLOBAL,
        .holdState = CANCOMM_HOLD_VALID,
        .localKeepAwake = CANCOMM_LOCAL_KEEP_NONE,
        .actionSet = &CanComm_ActionAllFull
    },
    /* 3. KL15 OFF + PNC_ADAS_ONLY valid -> ADAS1 only */
    {
        .ignition = CANCOMM_IGN_OFF,
        .pncRequest = CANCOMM_PNC_ADAS_ONLY,
        .holdState = CANCOMM_HOLD_VALID,
        .localKeepAwake = CANCOMM_LOCAL_KEEP_NONE,
        .actionSet = &CanComm_ActionAdasOnly
    },
    /* 4. KL15 OFF + no PNC + no local keep -> all NO, sleep candidate */
    {
        .ignition = CANCOMM_IGN_OFF,
        .pncRequest = CANCOMM_PNC_NONE,
        .holdState = CANCOMM_HOLD_INVALID,
        .localKeepAwake = CANCOMM_LOCAL_KEEP_NONE,
        .actionSet = &CanComm_ActionAllNo
    }
    /* Fallback is handled by Policy engine when no rule matches */
};

#define CANCOMM_POLICY_RULE_COUNT \
    (sizeof(CanComm_PolicyRuleTable) / sizeof(CanComm_PolicyRuleTable[0]))

const uint8 CanComm_PolicyRuleCount = (uint8)CANCOMM_POLICY_RULE_COUNT;

/* --- Fallback Action Set ------------------------------------------------ */
const CanComm_OutputActionSetType* const CanComm_FallbackActionSet = &CanComm_ActionAllFull;

/* --- Channel Configuration Table ---------------------------------------- */
/*
 * Maps logical channels to project-specific BSW handles.
 * Vector/DaVinci generated symbols are referenced ONLY here.
 */
const CanComm_ChannelConfigType CanComm_ChannelConfigTable[CANCOMM_CH_NUM] =
{
    /* ADAS1 */
    {
        .logicalCh = CANCOMM_CH_ADAS1,
        .commUser = ComMConf_ComMUser_CN_Backbone2_4e435fd4,
        .nmChannel = 0u,
        .nmRxPduId = 0u,
        .channelMask = CANCOMM_MASK_ADAS1,
        .groupMask = CANCOMM_MASK_ADAS1   /* ADAS group */
    },
    /* ADAS2 */
    {
        .logicalCh = CANCOMM_CH_ADAS2,
        .commUser = ComMConf_ComMUser_CN_Backbone2_4e435fd4_RX,
        .nmChannel = 1u,
        .nmRxPduId = 1u,
        .channelMask = CANCOMM_MASK_ADAS2,
        .groupMask = CANCOMM_MASK_ADAS2
    },
    /* ADAS_SUB */
    {
        .logicalCh = CANCOMM_CH_ADAS_SUB,
        .commUser = ComMConf_ComMUser_CN_LIN00_06e61e65,
        .nmChannel = 2u,
        .nmRxPduId = 2u,
        .channelMask = CANCOMM_MASK_ADAS_SUB,
        .groupMask = CANCOMM_MASK_ADAS_SUB
    },
    /* PDC */
    {
        .logicalCh = CANCOMM_CH_PDC,
        .commUser = ComMConf_ComMUser_CN_LIN01_3ff1080c,
        .nmChannel = 3u,
        .nmRxPduId = 3u,
        .channelMask = CANCOMM_MASK_PDC,
        .groupMask = CANCOMM_MASK_PDC
    },
    /* CAN5 */
    {
        .logicalCh = CANCOMM_CH_CAN5,
        .commUser = ComMConf_ComMUser_CN_LIN02_0c2d96c8,
        .nmChannel = 4u,
        .nmRxPduId = 4u,
        .channelMask = CANCOMM_MASK_CAN5,
        .groupMask = CANCOMM_MASK_CAN5
    },
    /* CAN9 */
    {
        .logicalCh = CANCOMM_CH_CAN9,
        .commUser = ComMConf_ComMUser_CN_LIN03_400ext_1856f4be,
        .nmChannel = 5u,
        .nmRxPduId = 5u,
        .channelMask = CANCOMM_MASK_CAN9,
        .groupMask = CANCOMM_MASK_CAN9
    }
};

/* --- Legacy NM Parse Configuration -------------------------------------- */
const CanComm_LegacyNmParseConfigType CanComm_LegacyNmParseConfig =
{
    .pniByteOffset = 5u,
    .pniBitMask = 0x40u,         /* bit 6 (0-indexed: 0x40) */
    .pncGlobalByteOffset = 5u,
    .pncGlobalBitMask = 0x08u,   /* bit 3 */
    .pncAdasByteOffset = 6u,
    .pncAdasBitMask = 0x01u      /* bit 0 */
};
