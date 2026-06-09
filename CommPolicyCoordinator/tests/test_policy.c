#include "unity.h"
#include "CommPolicy_Policy.h"
#include "CommPolicy_Types.h"
#include "CommPolicy_Cfg.h"
#include "CommPolicy_SourceMask.h"

/* Generic example source ids (must match GenericCfg.c) */
#define SRC_POWER_RUN       0u
#define SRC_NETWORK_GLOBAL  1u
#define SRC_NETWORK_PARTIAL 2u
#define SRC_DIAG            3u
#define SRC_OTA             4u
#define SRC_LOCAL_KEEP      5u
#define SRC_HOLD_VALID      6u

/* stubs for config externs */
extern const CommPolicy_PolicyRuleType CommPolicy_PolicyRuleTable[];
extern const uint8 CommPolicy_PolicyRuleCount;
extern const CommPolicy_OutputActionSetType* const CommPolicy_FallbackActionSet;

/* --- Policy Tests ------------------------------------------------------- */

void test_policy_power_run_all_full(void)
{
    CommPolicy_InputSnapshotType input;
    CommPolicy_OutputDecisionType decision;
    CommPolicy_LogicalChannelIdType ch;

    CommPolicy_SourceMask_Clear(&input.activeSourceMask);
    CommPolicy_SourceMask_Set(&input.activeSourceMask, SRC_POWER_RUN);

    TEST_ASSERT_EQUAL(E_OK, CommPolicy_Policy_Evaluate(&input, &decision));

    for (ch = 0u; ch < COMM_POLICY_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_HEX8(COMM_FULL_COMMUNICATION, decision.targetComMode[ch]);
        TEST_ASSERT_EQUAL_HEX8(COMM_POLICY_TX_UNBLOCK, decision.targetTxGate[ch]);
    }
    TEST_ASSERT_FALSE(decision.sleepCandidate);
}

void test_policy_network_global_all_full(void)
{
    CommPolicy_InputSnapshotType input;
    CommPolicy_OutputDecisionType decision;
    CommPolicy_LogicalChannelIdType ch;

    CommPolicy_SourceMask_Clear(&input.activeSourceMask);
    CommPolicy_SourceMask_Set(&input.activeSourceMask, SRC_NETWORK_GLOBAL);
    CommPolicy_SourceMask_Set(&input.activeSourceMask, SRC_HOLD_VALID);

    TEST_ASSERT_EQUAL(E_OK, CommPolicy_Policy_Evaluate(&input, &decision));

    for (ch = 0u; ch < COMM_POLICY_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_HEX8(COMM_FULL_COMMUNICATION, decision.targetComMode[ch]);
    }
    TEST_ASSERT_FALSE(decision.sleepCandidate);
}

void test_policy_network_partial_partial_only(void)
{
    CommPolicy_InputSnapshotType input;
    CommPolicy_OutputDecisionType decision;
    CommPolicy_LogicalChannelIdType ch;

    CommPolicy_SourceMask_Clear(&input.activeSourceMask);
    CommPolicy_SourceMask_Set(&input.activeSourceMask, SRC_NETWORK_PARTIAL);
    CommPolicy_SourceMask_Set(&input.activeSourceMask, SRC_HOLD_VALID);

    TEST_ASSERT_EQUAL(E_OK, CommPolicy_Policy_Evaluate(&input, &decision));

    /* Partial rule: only logical channel 0 gets FULL, others NO */
    TEST_ASSERT_EQUAL_HEX8(COMM_FULL_COMMUNICATION, decision.targetComMode[0u]);
    TEST_ASSERT_EQUAL_HEX8(COMM_POLICY_TX_UNBLOCK, decision.targetTxGate[0u]);

    for (ch = 1u; ch < COMM_POLICY_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_HEX8(COMM_NO_COMMUNICATION, decision.targetComMode[ch]);
        TEST_ASSERT_EQUAL_HEX8(COMM_POLICY_TX_BLOCK, decision.targetTxGate[ch]);
    }
    TEST_ASSERT_FALSE(decision.sleepCandidate);
}

void test_policy_diag_keep_all_full(void)
{
    CommPolicy_InputSnapshotType input;
    CommPolicy_OutputDecisionType decision;
    CommPolicy_LogicalChannelIdType ch;

    CommPolicy_SourceMask_Clear(&input.activeSourceMask);
    CommPolicy_SourceMask_Set(&input.activeSourceMask, SRC_DIAG);

    TEST_ASSERT_EQUAL(E_OK, CommPolicy_Policy_Evaluate(&input, &decision));

    for (ch = 0u; ch < COMM_POLICY_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_HEX8(COMM_FULL_COMMUNICATION, decision.targetComMode[ch]);
    }
    TEST_ASSERT_FALSE(decision.sleepCandidate);
}

void test_policy_no_source_all_no(void)
{
    CommPolicy_InputSnapshotType input;
    CommPolicy_OutputDecisionType decision;
    CommPolicy_LogicalChannelIdType ch;

    CommPolicy_SourceMask_Clear(&input.activeSourceMask);

    TEST_ASSERT_EQUAL(E_OK, CommPolicy_Policy_Evaluate(&input, &decision));

    for (ch = 0u; ch < COMM_POLICY_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_HEX8(COMM_NO_COMMUNICATION, decision.targetComMode[ch]);
        TEST_ASSERT_EQUAL_HEX8(COMM_POLICY_TX_BLOCK, decision.targetTxGate[ch]);
    }
    TEST_ASSERT_TRUE(decision.sleepCandidate);
}

void test_policy_null_pointer(void)
{
    CommPolicy_InputSnapshotType input;
    CommPolicy_OutputDecisionType decision;

    CommPolicy_SourceMask_Clear(&input.activeSourceMask);

    TEST_ASSERT_EQUAL(E_NOT_OK, CommPolicy_Policy_Evaluate(NULL_PTR, &decision));
    TEST_ASSERT_EQUAL(E_NOT_OK, CommPolicy_Policy_Evaluate(&input, NULL_PTR));
}

void test_policy_priority_power_beats_network(void)
{
    /*
     * If both POWER_RUN and NETWORK_GLOBAL are active, POWER_RUN rule (priority 100)
     * must win over NETWORK_GLOBAL rule (priority 90).
     */
    CommPolicy_InputSnapshotType input;
    CommPolicy_OutputDecisionType decision;
    CommPolicy_LogicalChannelIdType ch;

    CommPolicy_SourceMask_Clear(&input.activeSourceMask);
    CommPolicy_SourceMask_Set(&input.activeSourceMask, SRC_POWER_RUN);
    CommPolicy_SourceMask_Set(&input.activeSourceMask, SRC_NETWORK_GLOBAL);
    CommPolicy_SourceMask_Set(&input.activeSourceMask, SRC_HOLD_VALID);

    TEST_ASSERT_EQUAL(E_OK, CommPolicy_Policy_Evaluate(&input, &decision));

    for (ch = 0u; ch < COMM_POLICY_CH_NUM; ch++)
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
    CommPolicy_InputSnapshotType input;
    CommPolicy_OutputDecisionType decision;
    CommPolicy_LogicalChannelIdType ch;

    CommPolicy_SourceMask_Clear(&input.activeSourceMask);
    CommPolicy_SourceMask_Set(&input.activeSourceMask, SRC_DIAG);
    CommPolicy_SourceMask_Set(&input.activeSourceMask, SRC_OTA);

    TEST_ASSERT_EQUAL(E_OK, CommPolicy_Policy_Evaluate(&input, &decision));

    for (ch = 0u; ch < COMM_POLICY_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_HEX8(COMM_FULL_COMMUNICATION, decision.targetComMode[ch]);
    }
    TEST_ASSERT_FALSE(decision.sleepCandidate);
}

void test_policy_same_priority_first_match_wins(void)
{
    /*
     * If two rules had the same priority, the first in table wins.
     * Current table has unique priorities, so we verify the ordering
     * by checking that NETWORK_GLOBAL (priority 90) wins over NETWORK_PARTIAL (priority 80)
     * when both have HOLD_VALID but POWER_RUN is off.
     */
    CommPolicy_InputSnapshotType input;
    CommPolicy_OutputDecisionType decision;
    CommPolicy_LogicalChannelIdType ch;

    CommPolicy_SourceMask_Clear(&input.activeSourceMask);
    CommPolicy_SourceMask_Set(&input.activeSourceMask, SRC_NETWORK_GLOBAL);
    CommPolicy_SourceMask_Set(&input.activeSourceMask, SRC_NETWORK_PARTIAL);
    CommPolicy_SourceMask_Set(&input.activeSourceMask, SRC_HOLD_VALID);

    TEST_ASSERT_EQUAL(E_OK, CommPolicy_Policy_Evaluate(&input, &decision));

    /* NETWORK_GLOBAL rule (priority 90) should win over NETWORK_PARTIAL (priority 80) */
    for (ch = 0u; ch < COMM_POLICY_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_HEX8(COMM_FULL_COMMUNICATION, decision.targetComMode[ch]);
    }
}

void test_policy_fallback_when_no_rule_matches(void)
{
    CommPolicy_InputSnapshotType input;
    CommPolicy_OutputDecisionType decision;
    CommPolicy_LogicalChannelIdType ch;

    /* Set a source that no rule explicitly requires, with forbidden combinations */
    CommPolicy_SourceMask_Clear(&input.activeSourceMask);
    CommPolicy_SourceMask_Set(&input.activeSourceMask, SRC_NETWORK_GLOBAL);
    /* Note: without HOLD_VALID, NETWORK_GLOBAL rule does not match */

    TEST_ASSERT_EQUAL(E_OK, CommPolicy_Policy_Evaluate(&input, &decision));

    /* Fallback is ALL_FULL per configuration */
    for (ch = 0u; ch < COMM_POLICY_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_HEX8(COMM_FULL_COMMUNICATION, decision.targetComMode[ch]);
    }
}
