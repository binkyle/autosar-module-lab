#include "unity.h"
#include "CommPolicy_Adapter.h"
#include "CommPolicy_Types.h"
#include "CommPolicy_Cfg.h"
#include "CommPolicy_SourceMask.h"

/* GenericMock control API */
extern void CommPolicy_Mock_Reset(void);
extern void CommPolicy_Mock_SetSourceActive(CommPolicy_SourceIdType sourceId);
extern void CommPolicy_Mock_SetSourceInactive(CommPolicy_SourceIdType sourceId);
extern void CommPolicy_Mock_ClearAllSources(void);
extern const CommPolicy_SourceMaskType* CommPolicy_Mock_GetActiveSourceMask(void);
extern uint8 CommPolicy_Mock_GetRequestComModeCallCount(CommPolicy_LogicalChannelIdType ch);
extern uint8 CommPolicy_Mock_GetModeRequestCallCount(void);
extern CommPolicy_ModeRequestIdType CommPolicy_Mock_GetLastModeRequestId(void);

/* --- Adapter Tests ------------------------------------------------------ */

void test_adapter_build_snapshot(void)
{
    CommPolicy_InputSnapshotType snapshot;

    CommPolicy_Mock_Reset();
    CommPolicy_Mock_SetSourceActive(0u);
    CommPolicy_Mock_SetSourceActive(5u);

    TEST_ASSERT_EQUAL(E_OK, CommPolicy_Adapter_BuildInputSnapshot(&snapshot));
    TEST_ASSERT_TRUE(CommPolicy_SourceMask_IsSet(&snapshot.activeSourceMask, 0u));
    TEST_ASSERT_TRUE(CommPolicy_SourceMask_IsSet(&snapshot.activeSourceMask, 5u));
    TEST_ASSERT_FALSE(CommPolicy_SourceMask_IsSet(&snapshot.activeSourceMask, 1u));
}

void test_adapter_request_com_mode_called_when_target_differs(void)
{
    CommPolicy_OutputDecisionType decision;
    CommPolicy_LogicalChannelIdType ch;

    CommPolicy_Mock_Reset();

    for (ch = 0u; ch < COMM_POLICY_CH_NUM; ch++)
    {
        decision.targetComMode[ch] = COMM_POLICY_FULL_COMMUNICATION;
        decision.targetTxGate[ch] = COMM_POLICY_TX_UNBLOCK;
    }
    decision.modeRequestId = 0u; /* EXAMPLE_MODE_REQ_ALL_FULL */
    decision.sleepCandidate = FALSE;

    TEST_ASSERT_EQUAL(E_OK, CommPolicy_Adapter_ApplyDecision(&decision));

    for (ch = 0u; ch < COMM_POLICY_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_UINT8(1u, CommPolicy_Mock_GetRequestComModeCallCount(ch));
    }
}

void test_adapter_no_repeat_request_when_target_same(void)
{
    /* ApplyDecision de-duplicates: same target mode should not trigger
     * another ComM_RequestComMode call. */
    CommPolicy_OutputDecisionType decision;
    CommPolicy_LogicalChannelIdType ch;

    CommPolicy_Mock_Reset();

    for (ch = 0u; ch < COMM_POLICY_CH_NUM; ch++)
    {
        decision.targetComMode[ch] = COMM_POLICY_FULL_COMMUNICATION;
        decision.targetTxGate[ch] = COMM_POLICY_TX_UNBLOCK;
    }
    decision.modeRequestId = 0u; /* EXAMPLE_MODE_REQ_ALL_FULL */
    decision.sleepCandidate = FALSE;

    (void)CommPolicy_Adapter_ApplyDecision(&decision);
    (void)CommPolicy_Adapter_ApplyDecision(&decision);

    for (ch = 0u; ch < COMM_POLICY_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_UINT8(1u, CommPolicy_Mock_GetRequestComModeCallCount(ch));
    }
}

void test_adapter_handle_comm_e_not_ok(void)
{
    CommPolicy_CommModeType mode;

    TEST_ASSERT_EQUAL(E_NOT_OK, CommPolicy_Adapter_GetComMode(COMM_POLICY_CH_NUM, &mode));
    TEST_ASSERT_EQUAL(E_NOT_OK, CommPolicy_Adapter_RequestComMode(COMM_POLICY_CH_NUM, COMM_POLICY_FULL_COMMUNICATION));
}

void test_adapter_mode_request_called(void)
{
    CommPolicy_OutputDecisionType decision;
    CommPolicy_LogicalChannelIdType ch;

    CommPolicy_Mock_Reset();

    for (ch = 0u; ch < COMM_POLICY_CH_NUM; ch++)
    {
        decision.targetComMode[ch] = COMM_POLICY_FULL_COMMUNICATION;
        decision.targetTxGate[ch] = COMM_POLICY_TX_UNBLOCK;
    }
    decision.modeRequestId = 1u; /* EXAMPLE_MODE_REQ_ALL_NO */
    decision.sleepCandidate = TRUE;

    TEST_ASSERT_EQUAL(E_OK, CommPolicy_Adapter_ApplyDecision(&decision));
    TEST_ASSERT_EQUAL_UINT8(1u, CommPolicy_Mock_GetModeRequestCallCount());
    TEST_ASSERT_EQUAL_UINT16(1u, CommPolicy_Mock_GetLastModeRequestId());
}

void test_adapter_request_com_mode_channel_bounds(void)
{
    CommPolicy_CommModeType mode;

    /* Channel out of bounds must return E_NOT_OK */
    TEST_ASSERT_EQUAL(E_NOT_OK, CommPolicy_Adapter_RequestComMode(COMM_POLICY_CH_NUM, COMM_POLICY_FULL_COMMUNICATION));
    TEST_ASSERT_EQUAL(E_NOT_OK, CommPolicy_Adapter_GetComMode(COMM_POLICY_CH_NUM, &mode));
}

void test_adapter_build_snapshot_null(void)
{
    TEST_ASSERT_EQUAL(E_NOT_OK, CommPolicy_Adapter_BuildInputSnapshot(NULL_PTR));
}

void test_adapter_apply_decision_null(void)
{
    TEST_ASSERT_EQUAL(E_NOT_OK, CommPolicy_Adapter_ApplyDecision(NULL_PTR));
}
