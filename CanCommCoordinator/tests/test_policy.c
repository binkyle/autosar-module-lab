#include "unity.h"
#include "CanCommCoordinator_Policy.h"
#include "CanCommCoordinator_Types.h"
#include "CanCommCoordinator_Cfg.h"
#include "CanCommCoordinator_State.h"

/* stubs for config externs */
extern const CanComm_PolicyRuleType CanComm_PolicyRuleTable[];
extern const uint8 CanComm_PolicyRuleCount;
extern const CanComm_OutputActionSetType* const CanComm_FallbackActionSet;

/* --- Policy Tests ------------------------------------------------------- */

void test_policy_kl15_on_all_full(void)
{
    CanComm_InputSnapshotType input;
    CanComm_OutputDecisionType decision;
    CanComm_ChannelIdType ch;

    input.ignition = CANCOMM_IGN_ON;
    input.pncRequest = CANCOMM_PNC_NONE;
    input.holdState = CANCOMM_HOLD_INVALID;
    input.localKeepAwake = CANCOMM_LOCAL_KEEP_NONE;
    input.diagKeepAwake = FALSE;
    input.otaKeepAwake = FALSE;

    TEST_ASSERT_EQUAL(E_OK, CanComm_Policy_Evaluate(&input, &decision));

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_HEX8(COMM_FULL_COMMUNICATION, decision.targetComMode[ch]);
        TEST_ASSERT_EQUAL_HEX8(CANCOMM_TX_UNBLOCK, decision.targetTxGate[ch]);
    }
    TEST_ASSERT_FALSE(decision.sleepCandidate);
}

void test_policy_global_all_full(void)
{
    CanComm_InputSnapshotType input;
    CanComm_OutputDecisionType decision;
    CanComm_ChannelIdType ch;

    input.ignition = CANCOMM_IGN_OFF;
    input.pncRequest = CANCOMM_PNC_GLOBAL;
    input.holdState = CANCOMM_HOLD_VALID;
    input.localKeepAwake = CANCOMM_LOCAL_KEEP_NONE;
    input.diagKeepAwake = FALSE;
    input.otaKeepAwake = FALSE;

    TEST_ASSERT_EQUAL(E_OK, CanComm_Policy_Evaluate(&input, &decision));

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_HEX8(COMM_FULL_COMMUNICATION, decision.targetComMode[ch]);
    }
    TEST_ASSERT_FALSE(decision.sleepCandidate);
}

void test_policy_adas_only_only_adas1(void)
{
    CanComm_InputSnapshotType input;
    CanComm_OutputDecisionType decision;

    input.ignition = CANCOMM_IGN_OFF;
    input.pncRequest = CANCOMM_PNC_ADAS_ONLY;
    input.holdState = CANCOMM_HOLD_VALID;
    input.localKeepAwake = CANCOMM_LOCAL_KEEP_NONE;
    input.diagKeepAwake = FALSE;
    input.otaKeepAwake = FALSE;

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

void test_policy_no_pnc_all_no_sleep_candidate(void)
{
    CanComm_InputSnapshotType input;
    CanComm_OutputDecisionType decision;
    CanComm_ChannelIdType ch;

    input.ignition = CANCOMM_IGN_OFF;
    input.pncRequest = CANCOMM_PNC_NONE;
    input.holdState = CANCOMM_HOLD_INVALID;
    input.localKeepAwake = CANCOMM_LOCAL_KEEP_NONE;
    input.diagKeepAwake = FALSE;
    input.otaKeepAwake = FALSE;

    TEST_ASSERT_EQUAL(E_OK, CanComm_Policy_Evaluate(&input, &decision));

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_HEX8(COMM_NO_COMMUNICATION, decision.targetComMode[ch]);
        TEST_ASSERT_EQUAL_HEX8(CANCOMM_TX_BLOCK, decision.targetTxGate[ch]);
    }
    TEST_ASSERT_TRUE(decision.sleepCandidate);
}

void test_policy_invalid_input_fallback(void)
{
    CanComm_InputSnapshotType input;
    CanComm_OutputDecisionType decision;
    CanComm_ChannelIdType ch;

    input.ignition = CANCOMM_IGN_OFF;
    input.pncRequest = CANCOMM_PNC_INVALID;
    input.holdState = CANCOMM_HOLD_INVALID;
    input.localKeepAwake = CANCOMM_LOCAL_KEEP_NONE;
    input.diagKeepAwake = FALSE;
    input.otaKeepAwake = FALSE;

    TEST_ASSERT_EQUAL(E_OK, CanComm_Policy_Evaluate(&input, &decision));

    /* Fallback is ALL_FULL per configuration */
    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_HEX8(COMM_FULL_COMMUNICATION, decision.targetComMode[ch]);
    }
}

void test_policy_null_pointer(void)
{
    CanComm_InputSnapshotType input;
    CanComm_OutputDecisionType decision;

    input.ignition = CANCOMM_IGN_ON;

    TEST_ASSERT_EQUAL(E_NOT_OK, CanComm_Policy_Evaluate(NULL, &decision));
    TEST_ASSERT_EQUAL(E_NOT_OK, CanComm_Policy_Evaluate(&input, NULL));
}

void test_policy_first_match_priority(void)
{
    /*
     * Rule table order: KL15_ON (first) beats everything else.
     * If we supply ignition=ON but also valid ADAS_ONLY PNC fields,
     * the first rule (KL15 ON -> ALL FULL) must win.
     */
    CanComm_InputSnapshotType input;
    CanComm_OutputDecisionType decision;
    CanComm_ChannelIdType ch;

    input.ignition = CANCOMM_IGN_ON;
    input.pncRequest = CANCOMM_PNC_ADAS_ONLY;
    input.holdState = CANCOMM_HOLD_VALID;
    input.localKeepAwake = CANCOMM_LOCAL_KEEP_NONE;
    input.diagKeepAwake = FALSE;
    input.otaKeepAwake = FALSE;

    TEST_ASSERT_EQUAL(E_OK, CanComm_Policy_Evaluate(&input, &decision));

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_HEX8(COMM_FULL_COMMUNICATION, decision.targetComMode[ch]);
    }
    TEST_ASSERT_FALSE(decision.sleepCandidate);
}

void test_policy_pnc_unknown_failsafe(void)
{
    CanComm_InputSnapshotType input;
    CanComm_OutputDecisionType decision;
    CanComm_ChannelIdType ch;

    input.ignition = CANCOMM_IGN_OFF;
    input.pncRequest = CANCOMM_PNC_UNKNOWN;
    input.holdState = CANCOMM_HOLD_INVALID;
    input.localKeepAwake = CANCOMM_LOCAL_KEEP_NONE;
    input.diagKeepAwake = FALSE;
    input.otaKeepAwake = FALSE;

    TEST_ASSERT_EQUAL(E_OK, CanComm_Policy_Evaluate(&input, &decision));

    /* Fallback is ALL_FULL per configuration */
    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_HEX8(COMM_FULL_COMMUNICATION, decision.targetComMode[ch]);
    }
}

void test_policy_pnc_invalid_failsafe(void)
{
    CanComm_InputSnapshotType input;
    CanComm_OutputDecisionType decision;
    CanComm_ChannelIdType ch;

    input.ignition = CANCOMM_IGN_OFF;
    input.pncRequest = CANCOMM_PNC_INVALID;
    input.holdState = CANCOMM_HOLD_INVALID;
    input.localKeepAwake = CANCOMM_LOCAL_KEEP_NONE;
    input.diagKeepAwake = FALSE;
    input.otaKeepAwake = FALSE;

    TEST_ASSERT_EQUAL(E_OK, CanComm_Policy_Evaluate(&input, &decision));

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_HEX8(COMM_FULL_COMMUNICATION, decision.targetComMode[ch]);
    }
}

void test_policy_action_set_adas_only_mask(void)
{
    /*
     * Verify that the ADAS_ONLY ActionSet correctly maps to per-channel
     * OutputDecision via the Policy_BuildDecisionFromActionSet logic.
     */
    CanComm_InputSnapshotType input;
    CanComm_OutputDecisionType decision;

    input.ignition = CANCOMM_IGN_OFF;
    input.pncRequest = CANCOMM_PNC_ADAS_ONLY;
    input.holdState = CANCOMM_HOLD_VALID;
    input.localKeepAwake = CANCOMM_LOCAL_KEEP_NONE;
    input.diagKeepAwake = FALSE;
    input.otaKeepAwake = FALSE;

    TEST_ASSERT_EQUAL(E_OK, CanComm_Policy_Evaluate(&input, &decision));

    /* ADAS1 must be FULL + UNBLOCK */
    TEST_ASSERT_EQUAL_HEX8(COMM_FULL_COMMUNICATION, decision.targetComMode[CANCOMM_CH_ADAS1]);
    TEST_ASSERT_EQUAL_HEX8(CANCOMM_TX_UNBLOCK, decision.targetTxGate[CANCOMM_CH_ADAS1]);

    /* All other channels must be NO + BLOCK */
    TEST_ASSERT_EQUAL_HEX8(COMM_NO_COMMUNICATION, decision.targetComMode[CANCOMM_CH_ADAS2]);
    TEST_ASSERT_EQUAL_HEX8(COMM_NO_COMMUNICATION, decision.targetComMode[CANCOMM_CH_ADAS_SUB]);
    TEST_ASSERT_EQUAL_HEX8(COMM_NO_COMMUNICATION, decision.targetComMode[CANCOMM_CH_PDC]);
    TEST_ASSERT_EQUAL_HEX8(COMM_NO_COMMUNICATION, decision.targetComMode[CANCOMM_CH_CAN5]);
    TEST_ASSERT_EQUAL_HEX8(COMM_NO_COMMUNICATION, decision.targetComMode[CANCOMM_CH_CAN9]);

    TEST_ASSERT_EQUAL_HEX8(CANCOMM_TX_BLOCK, decision.targetTxGate[CANCOMM_CH_ADAS2]);
    TEST_ASSERT_EQUAL_HEX8(CANCOMM_TX_BLOCK, decision.targetTxGate[CANCOMM_CH_ADAS_SUB]);
    TEST_ASSERT_EQUAL_HEX8(CANCOMM_TX_BLOCK, decision.targetTxGate[CANCOMM_CH_PDC]);
    TEST_ASSERT_EQUAL_HEX8(CANCOMM_TX_BLOCK, decision.targetTxGate[CANCOMM_CH_CAN5]);
    TEST_ASSERT_EQUAL_HEX8(CANCOMM_TX_BLOCK, decision.targetTxGate[CANCOMM_CH_CAN9]);

    TEST_ASSERT_FALSE(decision.sleepCandidate);
}

void test_policy_action_set_all_no_mask(void)
{
    CanComm_InputSnapshotType input;
    CanComm_OutputDecisionType decision;
    CanComm_ChannelIdType ch;

    input.ignition = CANCOMM_IGN_OFF;
    input.pncRequest = CANCOMM_PNC_NONE;
    input.holdState = CANCOMM_HOLD_INVALID;
    input.localKeepAwake = CANCOMM_LOCAL_KEEP_NONE;
    input.diagKeepAwake = FALSE;
    input.otaKeepAwake = FALSE;

    TEST_ASSERT_EQUAL(E_OK, CanComm_Policy_Evaluate(&input, &decision));

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_HEX8(COMM_NO_COMMUNICATION, decision.targetComMode[ch]);
        TEST_ASSERT_EQUAL_HEX8(CANCOMM_TX_BLOCK, decision.targetTxGate[ch]);
    }
    TEST_ASSERT_TRUE(decision.sleepCandidate);
}

void test_policy_action_set_all_full_mask(void)
{
    CanComm_InputSnapshotType input;
    CanComm_OutputDecisionType decision;
    CanComm_ChannelIdType ch;

    input.ignition = CANCOMM_IGN_ON;
    input.pncRequest = CANCOMM_PNC_NONE;
    input.holdState = CANCOMM_HOLD_INVALID;
    input.localKeepAwake = CANCOMM_LOCAL_KEEP_NONE;
    input.diagKeepAwake = FALSE;
    input.otaKeepAwake = FALSE;

    TEST_ASSERT_EQUAL(E_OK, CanComm_Policy_Evaluate(&input, &decision));

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_HEX8(COMM_FULL_COMMUNICATION, decision.targetComMode[ch]);
        TEST_ASSERT_EQUAL_HEX8(CANCOMM_TX_UNBLOCK, decision.targetTxGate[ch]);
    }
    TEST_ASSERT_FALSE(decision.sleepCandidate);
}
