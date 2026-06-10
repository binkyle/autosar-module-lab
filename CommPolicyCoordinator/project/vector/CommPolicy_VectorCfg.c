#include "CommPolicy_Cfg.h"
#include "CommPolicy_Types.h"
#include "CommPolicy_SourceMask.h"
#include "CommPolicy_ChannelMask.h"
#include "CommPolicy_VectorBinding.h"

/* --- Project-Specific Source IDs (Vector project) ----------------------- */
/* These are project configuration, NOT core framework definitions. */
#define VECTOR_SRC_ID_IGNITION         0u
#define VECTOR_SRC_ID_PNC_GLOBAL       1u
#define VECTOR_SRC_ID_PNC_ADAS         2u
#define VECTOR_SRC_ID_DIAG_KEEP        3u
#define VECTOR_SRC_ID_OTA_KEEP         4u
#define VECTOR_SRC_ID_LOCAL_KEEP       5u
#define VECTOR_SRC_ID_HOLD_VALID       6u

#define VECTOR_MODE_REQ_ALL_FULL       0u
#define VECTOR_MODE_REQ_ADAS_ONLY      1u
#define VECTOR_MODE_REQ_ALL_NO         2u
#define VECTOR_MODE_REQ_SLEEP          3u

#define SRC_MASK(id)    COMM_POLICY_SOURCE_BIT_MASK(id)
#define CH_MASK(ch)     COMM_POLICY_CHANNEL_BIT_MASK(ch)

/* --- Source Timer Configuration ----------------------------------------- */
#define VECTOR_PNC_HOLD_TICKS  150u

const CommPolicy_SourceTimerConfigType CommPolicy_SourceTimerConfigTable[] =
{
    { VECTOR_SRC_ID_PNC_GLOBAL, VECTOR_SRC_ID_HOLD_VALID, VECTOR_PNC_HOLD_TICKS },
    { VECTOR_SRC_ID_PNC_ADAS,   VECTOR_SRC_ID_HOLD_VALID, VECTOR_PNC_HOLD_TICKS }
};

#define COMM_POLICY_SOURCE_TIMER_CONFIG_COUNT \
    (sizeof(CommPolicy_SourceTimerConfigTable) / sizeof(CommPolicy_SourceTimerConfigTable[0]))

const uint8 CommPolicy_SourceTimerConfigCount = (uint8)COMM_POLICY_SOURCE_TIMER_CONFIG_COUNT;

/* --- Output Action Set Templates ---------------------------------------- */
static const CommPolicy_OutputActionSetType CommPolicy_ActionAllFull =
{
    .fullComMask    = { .word = { 0x3Fu, 0u } },
    .noComMask      = { .word = { 0u, 0u } },
    .txUnblockMask  = { .word = { 0x3Fu, 0u } },
    .txBlockMask    = { .word = { 0u, 0u } },
    .modeRequestId  = VECTOR_MODE_REQ_ALL_FULL,
    .sleepCandidate = FALSE
};

static const CommPolicy_OutputActionSetType CommPolicy_ActionAllNo =
{
    .fullComMask    = { .word = { 0u, 0u } },
    .noComMask      = { .word = { 0x3Fu, 0u } },
    .txUnblockMask  = { .word = { 0u, 0u } },
    .txBlockMask    = { .word = { 0x3Fu, 0u } },
    .modeRequestId  = VECTOR_MODE_REQ_SLEEP,
    .sleepCandidate = TRUE
};

static const CommPolicy_OutputActionSetType CommPolicy_ActionAdasOnly =
{
    .fullComMask    = { .word = { CH_MASK(0u), 0u } },
    .noComMask      = { .word = { 0x3Eu, 0u } },
    .txUnblockMask  = { .word = { CH_MASK(0u), 0u } },
    .txBlockMask    = { .word = { 0x3Eu, 0u } },
    .modeRequestId  = VECTOR_MODE_REQ_ADAS_ONLY,
    .sleepCandidate = FALSE
};

/* --- Policy Rule Table -------------------------------------------------- */
const CommPolicy_PolicyRuleType CommPolicy_PolicyRuleTable[] =
{
    {
        .priority       = 100u,
        .requiredAllMask.word    = { SRC_MASK(VECTOR_SRC_ID_IGNITION), 0u },
        .requiredAnyMask.word    = { 0u, 0u },
        .forbiddenMask.word      = { 0u, 0u },
        .actionSet      = &CommPolicy_ActionAllFull
    },
    {
        .priority       = 90u,
        .requiredAllMask.word    = { SRC_MASK(VECTOR_SRC_ID_PNC_GLOBAL) | SRC_MASK(VECTOR_SRC_ID_HOLD_VALID), 0u },
        .requiredAnyMask.word    = { 0u, 0u },
        .forbiddenMask.word      = { 0u, 0u },
        .actionSet      = &CommPolicy_ActionAllFull
    },
    {
        .priority       = 80u,
        .requiredAllMask.word    = { SRC_MASK(VECTOR_SRC_ID_PNC_ADAS) | SRC_MASK(VECTOR_SRC_ID_HOLD_VALID), 0u },
        .requiredAnyMask.word    = { 0u, 0u },
        .forbiddenMask.word      = { SRC_MASK(VECTOR_SRC_ID_PNC_GLOBAL), 0u },
        .actionSet      = &CommPolicy_ActionAdasOnly
    },
    {
        .priority       = 85u,
        .requiredAllMask.word    = { 0u, 0u },
        .requiredAnyMask.word    = { SRC_MASK(VECTOR_SRC_ID_DIAG_KEEP) | SRC_MASK(VECTOR_SRC_ID_OTA_KEEP) | SRC_MASK(VECTOR_SRC_ID_LOCAL_KEEP), 0u },
        .forbiddenMask.word      = { 0u, 0u },
        .actionSet      = &CommPolicy_ActionAllFull
    },
    {
        .priority       = 0u,
        .requiredAllMask.word    = { 0u, 0u },
        .requiredAnyMask.word    = { 0u, 0u },
        .forbiddenMask.word      = { 0xFFFFFFFFu, 0xFFFFFFFFu },
        .actionSet      = &CommPolicy_ActionAllNo
    }
};

#define COMM_POLICY_POLICY_RULE_COUNT \
    (sizeof(CommPolicy_PolicyRuleTable) / sizeof(CommPolicy_PolicyRuleTable[0]))

const uint8 CommPolicy_PolicyRuleCount = (uint8)COMM_POLICY_POLICY_RULE_COUNT;

/* --- Fallback Action Set ------------------------------------------------ */
#if (COMM_POLICY_FALLBACK_STRATEGY == COMM_POLICY_FALLBACK_ALL_NO)
const CommPolicy_OutputActionSetType* const CommPolicy_FallbackActionSet = &CommPolicy_ActionAllNo;
#else
const CommPolicy_OutputActionSetType* const CommPolicy_FallbackActionSet = &CommPolicy_ActionAllFull;
#endif

/* --- Channel Configuration Table ---------------------------------------- */
const CommPolicy_ChannelConfigType CommPolicy_ChannelConfigTable[COMM_POLICY_CH_NUM] =
{
    {
        .logicalChannel = 0u,
        .busType        = COMM_POLICY_BUS_CAN,
        .channelMask    = { .word = { CH_MASK(0u), 0u } },
        .groupMask      = { .word = { CH_MASK(0u), 0u } }
    },
    {
        .logicalChannel = 1u,
        .busType        = COMM_POLICY_BUS_CAN,
        .channelMask    = { .word = { CH_MASK(1u), 0u } },
        .groupMask      = { .word = { CH_MASK(1u), 0u } }
    },
    {
        .logicalChannel = 2u,
        .busType        = COMM_POLICY_BUS_LIN,
        .channelMask    = { .word = { CH_MASK(2u), 0u } },
        .groupMask      = { .word = { CH_MASK(2u), 0u } }
    },
    {
        .logicalChannel = 3u,
        .busType        = COMM_POLICY_BUS_LIN,
        .channelMask    = { .word = { CH_MASK(3u), 0u } },
        .groupMask      = { .word = { CH_MASK(3u), 0u } }
    },
    {
        .logicalChannel = 4u,
        .busType        = COMM_POLICY_BUS_CAN,
        .channelMask    = { .word = { CH_MASK(4u), 0u } },
        .groupMask      = { .word = { CH_MASK(4u), 0u } }
    },
    {
        .logicalChannel = 5u,
        .busType        = COMM_POLICY_BUS_CAN,
        .channelMask    = { .word = { CH_MASK(5u), 0u } },
        .groupMask      = { .word = { CH_MASK(5u), 0u } }
    }
};

/* --- Vector Channel Binding Table --------------------------------------- */
const CommPolicy_VectorChannelBindingType CommPolicy_VectorChannelBindingTable[COMM_POLICY_CH_NUM] =
{
    { 0u, ComMConf_ComMUser_CN_Backbone2_4e435fd4,      0u },
    { 1u, ComMConf_ComMUser_CN_Backbone2_4e435fd4_RX,   1u },
    { 2u, ComMConf_ComMUser_CN_LIN00_06e61e65,          2u },
    { 3u, ComMConf_ComMUser_CN_LIN01_3ff1080c,          3u },
    { 4u, ComMConf_ComMUser_CN_LIN02_0c2d96c8,          4u },
    { 5u, ComMConf_ComMUser_CN_LIN03_400ext_1856f4be,   5u }
};

/* --- Vector Mode Request Binding Table ---------------------------------- */
const CommPolicy_VectorModeRequestBindingType CommPolicy_VectorModeRequestBindingTable[] =
{
    { VECTOR_MODE_REQ_ALL_FULL,  0u, 0u },  /* TODO: replace with generated BswM symbols */
    { VECTOR_MODE_REQ_ADAS_ONLY, 0u, 0u },
    { VECTOR_MODE_REQ_ALL_NO,    0u, 0u },
    { VECTOR_MODE_REQ_SLEEP,     0u, 0u }
};

#define COMM_POLICY_VECTOR_MODE_REQ_BINDING_COUNT \
    (sizeof(CommPolicy_VectorModeRequestBindingTable) / sizeof(CommPolicy_VectorModeRequestBindingTable[0]))

const uint8 CommPolicy_VectorModeRequestBindingCount = (uint8)COMM_POLICY_VECTOR_MODE_REQ_BINDING_COUNT;
