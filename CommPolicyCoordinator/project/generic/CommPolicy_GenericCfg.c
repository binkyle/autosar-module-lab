#include "CommPolicy_Cfg.h"
#include "CommPolicy_Types.h"
#include "CommPolicy_SourceMask.h"
#include "CommPolicy_ChannelMask.h"

/* --- Generic Example Source IDs ----------------------------------------- */
/* These are EXAMPLE definitions for host testing. Project-specific IDs
 * should be defined in project configuration layers, NOT in core framework. */
#define EXAMPLE_SRC_ID_POWER_RUN        0u
#define EXAMPLE_SRC_ID_NETWORK_GLOBAL   1u
#define EXAMPLE_SRC_ID_NETWORK_PARTIAL  2u
#define EXAMPLE_SRC_ID_DIAG             3u
#define EXAMPLE_SRC_ID_OTA              4u
#define EXAMPLE_SRC_ID_LOCAL_KEEP       5u
#define EXAMPLE_SRC_ID_HOLD_VALID       6u

#define EXAMPLE_MODE_REQ_ALL_FULL       0u
#define EXAMPLE_MODE_REQ_ALL_NO         1u

#define SRC_MASK(id)    COMM_POLICY_SOURCE_BIT_MASK(id)

/* --- Helper to build a single-channel mask ------------------------------ */
#define CH_MASK(ch)     COMM_POLICY_CHANNEL_BIT_MASK(ch)

/* --- Source Timer Configuration ----------------------------------------- */
#define EXAMPLE_HOLD_TICKS  10u

const CommPolicy_SourceTimerConfigType CommPolicy_SourceTimerConfigTable[] =
{
    { EXAMPLE_SRC_ID_NETWORK_GLOBAL, EXAMPLE_SRC_ID_HOLD_VALID, EXAMPLE_HOLD_TICKS },
    { EXAMPLE_SRC_ID_NETWORK_PARTIAL, EXAMPLE_SRC_ID_HOLD_VALID, EXAMPLE_HOLD_TICKS }
};

#define COMM_POLICY_SOURCE_TIMER_CONFIG_COUNT \
    (sizeof(CommPolicy_SourceTimerConfigTable) / sizeof(CommPolicy_SourceTimerConfigTable[0]))

const uint8 CommPolicy_SourceTimerConfigCount = (uint8)COMM_POLICY_SOURCE_TIMER_CONFIG_COUNT;

/* --- Output Action Set Templates ---------------------------------------- */
/* Action: All channels FULL_COMMUNICATION */
static const CommPolicy_OutputActionSetType CommPolicy_ActionAllFull =
{
    .fullComMask    = { .word = { 0x3Fu, 0u } },
    .noComMask      = { .word = { 0u, 0u } },
    .txUnblockMask  = { .word = { 0x3Fu, 0u } },
    .txBlockMask    = { .word = { 0u, 0u } },
    .modeRequestId  = EXAMPLE_MODE_REQ_ALL_FULL,
    .sleepCandidate = FALSE
};

/* Action: All channels NO_COMMUNICATION, sleep candidate */
static const CommPolicy_OutputActionSetType CommPolicy_ActionAllNo =
{
    .fullComMask    = { .word = { 0u, 0u } },
    .noComMask      = { .word = { 0x3Fu, 0u } },
    .txUnblockMask  = { .word = { 0u, 0u } },
    .txBlockMask    = { .word = { 0x3Fu, 0u } },
    .modeRequestId  = EXAMPLE_MODE_REQ_ALL_NO,
    .sleepCandidate = TRUE
};

/* Action: Partial network -> only logical channel 0 FULL */
static const CommPolicy_OutputActionSetType CommPolicy_ActionPartialOnly =
{
    .fullComMask    = { .word = { CH_MASK(0u), 0u } },
    .noComMask      = { .word = { 0x3Eu, 0u } },
    .txUnblockMask  = { .word = { CH_MASK(0u), 0u } },
    .txBlockMask    = { .word = { 0x3Eu, 0u } },
    .modeRequestId  = EXAMPLE_MODE_REQ_ALL_FULL,
    .sleepCandidate = FALSE
};

/* --- Policy Rule Table -------------------------------------------------- */
/*
 * Generic example rules for host testing.
 * Project-specific rules should be defined in project configuration.
 */
const CommPolicy_PolicyRuleType CommPolicy_PolicyRuleTable[] =
{
    /* 1. POWER_RUN -> all FULL (priority 100) */
    {
        .priority       = 100u,
        .requiredAllMask.word    = { SRC_MASK(EXAMPLE_SRC_ID_POWER_RUN), 0u },
        .requiredAnyMask.word    = { 0u, 0u },
        .forbiddenMask.word      = { 0u, 0u },
        .actionSet      = &CommPolicy_ActionAllFull
    },
    /* 2. NETWORK_GLOBAL + HOLD_VALID -> all FULL (priority 90) */
    {
        .priority       = 90u,
        .requiredAllMask.word    = { SRC_MASK(EXAMPLE_SRC_ID_NETWORK_GLOBAL) | SRC_MASK(EXAMPLE_SRC_ID_HOLD_VALID), 0u },
        .requiredAnyMask.word    = { 0u, 0u },
        .forbiddenMask.word      = { 0u, 0u },
        .actionSet      = &CommPolicy_ActionAllFull
    },
    /* 3. NETWORK_PARTIAL + HOLD_VALID -> partial only (priority 80) */
    {
        .priority       = 80u,
        .requiredAllMask.word    = { SRC_MASK(EXAMPLE_SRC_ID_NETWORK_PARTIAL) | SRC_MASK(EXAMPLE_SRC_ID_HOLD_VALID), 0u },
        .requiredAnyMask.word    = { 0u, 0u },
        .forbiddenMask.word      = { SRC_MASK(EXAMPLE_SRC_ID_NETWORK_GLOBAL), 0u },
        .actionSet      = &CommPolicy_ActionPartialOnly
    },
    /* 4. DIAG / OTA / LOCAL keep -> all FULL (priority 85) */
    {
        .priority       = 85u,
        .requiredAllMask.word    = { 0u, 0u },
        .requiredAnyMask.word    = { SRC_MASK(EXAMPLE_SRC_ID_DIAG) | SRC_MASK(EXAMPLE_SRC_ID_OTA) | SRC_MASK(EXAMPLE_SRC_ID_LOCAL_KEEP), 0u },
        .forbiddenMask.word      = { 0u, 0u },
        .actionSet      = &CommPolicy_ActionAllFull
    },
    /* 5. Catch-all: no active sources -> all NO, sleep candidate (priority 0) */
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
/* Generic example: mixed CAN / LIN / ETH channels for cross-bus demo */
const CommPolicy_ChannelConfigType CommPolicy_ChannelConfigTable[COMM_POLICY_CH_NUM] =
{
    /* Logical 0: CAN backbone */
    {
        .logicalChannel = 0u,
        .busType        = COMM_POLICY_BUS_CAN,
        .channelMask    = { .word = { CH_MASK(0u), 0u } },
        .groupMask      = { .word = { CH_MASK(0u), 0u } }
    },
    /* Logical 1: CAN secondary */
    {
        .logicalChannel = 1u,
        .busType        = COMM_POLICY_BUS_CAN,
        .channelMask    = { .word = { CH_MASK(1u), 0u } },
        .groupMask      = { .word = { CH_MASK(1u), 0u } }
    },
    /* Logical 2: LIN body */
    {
        .logicalChannel = 2u,
        .busType        = COMM_POLICY_BUS_LIN,
        .channelMask    = { .word = { CH_MASK(2u), 0u } },
        .groupMask      = { .word = { CH_MASK(2u), 0u } }
    },
    /* Logical 3: LIN comfort */
    {
        .logicalChannel = 3u,
        .busType        = COMM_POLICY_BUS_LIN,
        .channelMask    = { .word = { CH_MASK(3u), 0u } },
        .groupMask      = { .word = { CH_MASK(3u), 0u } }
    },
    /* Logical 4: ETH backbone */
    {
        .logicalChannel = 4u,
        .busType        = COMM_POLICY_BUS_ETH,
        .channelMask    = { .word = { CH_MASK(4u), 0u } },
        .groupMask      = { .word = { CH_MASK(4u), 0u } }
    },
    /* Logical 5: User-defined */
    {
        .logicalChannel = 5u,
        .busType        = COMM_POLICY_BUS_USER,
        .channelMask    = { .word = { CH_MASK(5u), 0u } },
        .groupMask      = { .word = { CH_MASK(5u), 0u } }
    }
};
