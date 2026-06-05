#include "unity.h"
#include "CanCommCoordinator_Policy.h"
#include "CanCommCoordinator_Types.h"
#include "CanCommCoordinator_Cfg.h"
#include "CanCommCoordinator_SourceMask.h"

/* Project-specific source ids (must match Cfg.c) */
#define SRC_IGNITION        0u
#define SRC_PNC_GLOBAL      1u
#define SRC_PNC_ADAS        2u
#define SRC_DIAG_KEEP       3u
#define SRC_OTA_KEEP        4u
#define SRC_LOCAL_KEEP      5u
#define SRC_HOLD_VALID      6u

/* stubs for config externs */
extern const CanComm_PolicyRuleType CanComm_PolicyRuleTable[];
extern const uint8 CanComm_PolicyRuleCount;
extern const CanComm_OutputActionSetType* const CanComm_FallbackActionSet;

/* --- Policy Tests ------------------------------------------------------- */

void test_policy_ignition_on_all_full(void)
{
    CanComm_InputSnapshotType input;
    CanComm_OutputDecisionType decision;
    CanComm_ChannelIdType ch;

    CanComm_SourceMask_Clear(&input.activeSourceMask);
    CanComm_SourceMask_Set(&input.activeSourceMask, SRC_IGNITION);

    TEST_ASSERT_EQUAL(E_OK, CanComm_Policy_Evaluate(&input, &decision));

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_HEX8(COMM_FULL_COMMUNICATION, decision.targetComMode[ch]);
        TEST_ASSERT_EQUAL_HEX8(CANCOMM_TX_UNBLOCK, decision.targetTxGate[ch]);
    }
    TEST_ASSERT_FALSE(decision.sleepCandidate);
}

void test_policy_pnc_global_all_full(void)
{
    CanComm_InputSnapshotType input;
    CanComm_OutputDecisionType decision;
    CanComm_ChannelIdType ch;

    CanComm_SourceMask_Clear(&input.activeSourceMask);
    CanComm_SourceMask_Set(&input.activeSourceMask, SRC_PNC_GLOBAL);
    CanComm_SourceMask_Set(&input.activeSourceMask, SRC_HOLD_VALID);

    TEST_ASSERT_EQUAL(E_OK, CanComm_Policy_Evaluate(&input, &decision));

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_HEX8(COMM_FULL_COMMUNICATION, decision.targetComMode[ch]);
    }
    TEST_ASSERT_FALSE(decision.sleepCandidate);
}

void test_policy_pnc_adas_adas_only(void)
{
    CanComm_InputSnapshotType input;
    CanComm_OutputDecisionType decision;

    CanComm_SourceMask_Clear(&input.activeSourceMask);
    CanComm_SourceMask_Set(&input.activeSourceMask, SRC_PNC_ADAS);
    CanComm_SourceMask_Set(&input.activeSourceMask, SRC_HOLD_VALID);

    TEST_ASSERT_EQUAL(E_OK, CanComm_Policy_Evaluate(&input, &decision));

    TEST_ASSERT_EQUAL_HEX8(COMM_FULL_COMMUNICATION, decision.targetComMode[CANCOMM_CH_ADAS1]);
    TEST_ASSERT_EQUAL_HEX8(CANCOMM_TX_UNBLOCK, decision.targetTxGate[CANCOMM_CH_ADAS1]);

    TEST_ASSERT_EQUAL_HEX8(COMM_NO_COMMUNICATION, decision.targetComMode[CANCOMM_CH_ADAS2]);
    TEST_ASSERT_EQUAL_HEX8(COMM_NO_COMMUNICATION, decision.targetComMode[CANCOMM_CH_ADAS_SUB]);
    TEST_ASSERT_EQUAL_HEX8(COMM_NO_COMMUNICATION, decision.targetComMode[CANCOMM_CH_PDC]);
    TEST_ASSERT_EQUAL_HEX8(COMM_NO_COMMUNICATION, decision.targetComMode[CANCOMM_CH_CAN5]);
    TEST_ASSERT_EQUAL_HEX8(COMM_NO_COMMUNICATION, decision.targetComMode[CANCOMM_CH_CAN9]);

    TEST_ASSERT_FALSE(decision.sleepCandidate);
}

void test_policy_diag_keep_all_full(void)
{
    CanComm_InputSnapshotType input;
    CanComm_OutputDecisionType decision;
    CanComm_ChannelIdType ch;

    CanComm_SourceMask_Clear(&input.activeSourceMask);
    CanComm_SourceMask_Set(&input.activeSourceMask, SRC_DIAG_KEEP);

    TEST_ASSERT_EQUAL(E_OK, CanComm_Policy_Evaluate(&input, &decision));

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_HEX8(COMM_FULL_COMMUNICATION, decision.targetComMode[ch]);
    }
    TEST_ASSERT_FALSE(decision.sleepCandidate);
}

void test_policy_no_source_all_no(void)
{
    CanComm_InputSnapshotType input;
    CanComm_OutputDecisionType decision;
    CanComm_ChannelIdType ch;

    CanComm_SourceMask_Clear(&input.activeSourceMask);

    TEST_ASSERT_EQUAL(E_OK, CanComm_Policy_Evaluate(&input, &decision));

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_HEX8(COMM_NO_COMMUNICATION, decision.targetComMode[ch]);
        TEST_ASSERT_EQUAL_HEX8(CANCOMM_TX_BLOCK, decision.targetTxGate[ch]);
    }
    TEST_ASSERT_TRUE(decision.sleepCandidate);
}

void test_policy_null_pointer(void)
{
    CanComm_InputSnapshotType input;
    CanComm_OutputDecisionType decision;

    CanComm_SourceMask_Clear(&input.activeSourceMask);

    TEST_ASSERT_EQUAL(E_NOT_OK, CanComm_Policy_Evaluate(NULL_PTR, &decision));
    TEST_ASSERT_EQUAL(E_NOT_OK, CanComm_Policy_Evaluate(&input, NULL_PTR));
}

void test_policy_priority_ignition_beats_pnc(void)
{
    /*
     * If both IGNITION and PNC_GLOBAL are active, IGNITION rule (priority 100)
     * must win over PNC_GLOBAL rule (priority 90).
     */
    CanComm_InputSnapshotType input;
    CanComm_OutputDecisionType decision;
    CanComm_ChannelIdType ch;

    CanComm_SourceMask_Clear(&input.activeSourceMask);
    CanComm_SourceMask_Set(&input.activeSourceMask, SRC_IGNITION);
    CanComm_SourceMask_Set(&input.activeSourceMask, SRC_PNC_GLOBAL);
    CanComm_SourceMask_Set(&input.activeSourceMask, SRC_HOLD_VALID);

    TEST_ASSERT_EQUAL(E_OK, CanComm_Policy_Evaluate(&input, &decision));

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_HEX8(COMM_FULL_COMMUNICATION, decision.targetComMode[ch]);
    }
    TEST_ASSERT_FALSE(decision.sleepCandidate);
}

void test_policy_multi_source_any_match(void)
{
    /*
     * DIAG + OTA both active -> ANY keep-awake rule (priority 85) matches.
     */
    CanComm_InputSnapshotType input;
    CanComm_OutputDecisionType decision;
    CanComm_ChannelIdType ch;

    CanComm_SourceMask_Clear(&input.activeSourceMask);
    CanComm_SourceMask_Set(&input.activeSourceMask, SRC_DIAG_KEEP);
    CanComm_SourceMask_Set(&input.activeSourceMask, SRC_OTA_KEEP);

    TEST_ASSERT_EQUAL(E_OK, CanComm_Policy_Evaluate(&input, &decision));

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_HEX8(COMM_FULL_COMMUNICATION, decision.targetComMode[ch]);
    }
    TEST_ASSERT_FALSE(decision.sleepCandidate);
}

void test_policy_priority_same_first_match_wins(void)
{
    /*
     * If two rules had the same priority, the first in table wins.
     * Current table has unique priorities, so we verify the ordering
     * by checking that PNC_GLOBAL (priority 90) wins over PNC_ADAS (priority 80)
     * when both have HOLD_VALID but IGNITION is off.
     */
    CanComm_InputSnapshotType input;
    CanComm_OutputDecisionType decision;

    CanComm_SourceMask_Clear(&input.activeSourceMask);
    CanComm_SourceMask_Set(&input.activeSourceMask, SRC_PNC_GLOBAL);
    CanComm_SourceMask_Set(&input.activeSourceMask, SRC_PNC_ADAS);
    CanComm_SourceMask_Set(&input.activeSourceMask, SRC_HOLD_VALID);

    TEST_ASSERT_EQUAL(E_OK, CanComm_Policy_Evaluate(&input, &decision));

    /* PNC_GLOBAL rule (priority 90) should win over PNC_ADAS (priority 80) */
    CanComm_ChannelIdType ch;
    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_HEX8(COMM_FULL_COMMUNICATION, decision.targetComMode[ch]);
    }
}

void test_policy_fallback_when_no_rule_matches(void)
{
    CanComm_InputSnapshotType input;
    CanComm_OutputDecisionType decision;
    CanComm_ChannelIdType ch;

    /* Set a source that no rule explicitly requires, with forbidden combinations */
    CanComm_SourceMask_Clear(&input.activeSourceMask);
    CanComm_SourceMask_Set(&input.activeSourceMask, SRC_PNC_GLOBAL);
    /* Note: without HOLD_VALID, PNC_GLOBAL rule does not match */

    TEST_ASSERT_EQUAL(E_OK, CanComm_Policy_Evaluate(&input, &decision));

    /* Fallback is ALL_FULL per configuration */
    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_HEX8(COMM_FULL_COMMUNICATION, decision.targetComMode[ch]);
    }
}
