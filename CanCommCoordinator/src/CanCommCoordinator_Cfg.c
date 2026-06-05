#include "CanCommCoordinator_Cfg.h"
#include "CanCommCoordinator_Types.h"
#include "CanCommCoordinator_SourceMask.h"
#include "ComM.h"

/* --- Project-Specific Source IDs ---------------------------------------- */
/* These are project configuration, NOT core framework definitions. */
#define CANCOMM_SRC_ID_IGNITION         0u
#define CANCOMM_SRC_ID_PNC_GLOBAL       1u
#define CANCOMM_SRC_ID_PNC_ADAS         2u
#define CANCOMM_SRC_ID_DIAG_KEEP        3u
#define CANCOMM_SRC_ID_OTA_KEEP         4u
#define CANCOMM_SRC_ID_LOCAL_KEEP       5u
#define CANCOMM_SRC_ID_HOLD_VALID       6u

#define SRC_MASK(id)    CANCOMM_SOURCE_BIT_MASK(id)

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
 * Rule matching: highest priority wins. If priority equal, first in table wins.
 *
 * Source semantics (project-specific):
 *   IGNITION_ON  -> all channels FULL (highest priority)
 *   PNC_GLOBAL + HOLD_VALID -> all channels FULL
 *   PNC_ADAS + HOLD_VALID   -> ADAS1 only
 *   DIAG / OTA / LOCAL keep -> all channels FULL
 *   No active sources       -> all NO, sleep candidate
 */
const CanComm_PolicyRuleType CanComm_PolicyRuleTable[] =
{
    /* 1. IGNITION ON -> all FULL (priority 100) */
    {
        .priority       = 100u,
        .requiredAllMask.word    = { SRC_MASK(CANCOMM_SRC_ID_IGNITION), 0u },
        .requiredAnyMask.word    = { 0u, 0u },
        .forbiddenMask.word      = { 0u, 0u },
        .actionSet      = &CanComm_ActionAllFull
    },
    /* 2. PNC_GLOBAL + HOLD_VALID -> all FULL (priority 90) */
    {
        .priority       = 90u,
        .requiredAllMask.word    = { SRC_MASK(CANCOMM_SRC_ID_PNC_GLOBAL) | SRC_MASK(CANCOMM_SRC_ID_HOLD_VALID), 0u },
        .requiredAnyMask.word    = { 0u, 0u },
        .forbiddenMask.word      = { 0u, 0u },
        .actionSet      = &CanComm_ActionAllFull
    },
    /* 3. PNC_ADAS + HOLD_VALID -> ADAS1 only (priority 80) */
    {
        .priority       = 80u,
        .requiredAllMask.word    = { SRC_MASK(CANCOMM_SRC_ID_PNC_ADAS) | SRC_MASK(CANCOMM_SRC_ID_HOLD_VALID), 0u },
        .requiredAnyMask.word    = { 0u, 0u },
        .forbiddenMask.word      = { SRC_MASK(CANCOMM_SRC_ID_PNC_GLOBAL), 0u },
        .actionSet      = &CanComm_ActionAdasOnly
    },
    /* 4. DIAG / OTA / LOCAL keep awake -> all FULL (priority 85) */
    {
        .priority       = 85u,
        .requiredAllMask.word    = { 0u, 0u },
        .requiredAnyMask.word    = { SRC_MASK(CANCOMM_SRC_ID_DIAG_KEEP) | SRC_MASK(CANCOMM_SRC_ID_OTA_KEEP) | SRC_MASK(CANCOMM_SRC_ID_LOCAL_KEEP), 0u },
        .forbiddenMask.word      = { 0u, 0u },
        .actionSet      = &CanComm_ActionAllFull
    },
    /* 5. Catch-all: no active sources -> all NO, sleep candidate (priority 0)
     *    forbiddenMask = ALL_SOURCES ensures this only matches when mask is empty.
     *    Non-empty inputs that miss rules 1-4 will hit fallback (safe FULL).
     */
    {
        .priority       = 0u,
        .requiredAllMask.word    = { 0u, 0u },
        .requiredAnyMask.word    = { 0u, 0u },
        .forbiddenMask.word      = { 0xFFFFFFFFu, 0xFFFFFFFFu },
        .actionSet      = &CanComm_ActionAllNo
    }
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
