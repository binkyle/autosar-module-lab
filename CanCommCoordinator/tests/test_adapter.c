#include "unity.h"
#include "CanCommCoordinator_Adapter.h"
#include "CanCommCoordinator_Types.h"
#include "CanCommCoordinator_Cfg.h"
#include "CanCommCoordinator_SourceMask.h"

/* GenericMock control API */
extern void CanComm_Mock_Reset(void);
extern void CanComm_Mock_SetSourceActive(CanComm_SourceIdType sourceId);
extern void CanComm_Mock_SetSourceInactive(CanComm_SourceIdType sourceId);
extern void CanComm_Mock_ClearAllSources(void);
extern const CanComm_SourceMaskType* CanComm_Mock_GetActiveSourceMask(void);
extern uint8 CanComm_Mock_GetRequestComModeCallCount(CanComm_ChannelIdType ch);
extern uint8 CanComm_Mock_GetBswMRequestCallCount(void);
extern CanComm_BswMRequestType CanComm_Mock_GetLastBswMRequest(void);

/* --- Adapter Tests ------------------------------------------------------ */

void test_adapter_build_snapshot(void)
{
    CanComm_InputSnapshotType snapshot;

    CanComm_Mock_Reset();
    CanComm_Mock_SetSourceActive(0u);
    CanComm_Mock_SetSourceActive(5u);

    TEST_ASSERT_EQUAL(E_OK, CanComm_Adapter_BuildInputSnapshot(&snapshot));
    TEST_ASSERT_TRUE(CanComm_SourceMask_IsSet(&snapshot.activeSourceMask, 0u));
    TEST_ASSERT_TRUE(CanComm_SourceMask_IsSet(&snapshot.activeSourceMask, 5u));
    TEST_ASSERT_FALSE(CanComm_SourceMask_IsSet(&snapshot.activeSourceMask, 1u));
}

void test_adapter_request_com_mode_called_when_target_differs(void)
{
    CanComm_OutputDecisionType decision;
    CanComm_ChannelIdType ch;

    CanComm_Mock_Reset();

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        decision.targetComMode[ch] = COMM_FULL_COMMUNICATION;
        decision.targetTxGate[ch] = CANCOMM_TX_UNBLOCK;
    }
    decision.bswmRequest = CANCOMM_BSWM_REQUEST_ALL_FULL;
    decision.sleepCandidate = FALSE;

    TEST_ASSERT_EQUAL(E_OK, CanComm_Adapter_ApplyDecision(&decision));

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_UINT8(1u, CanComm_Mock_GetRequestComModeCallCount(ch));
    }
}

void test_adapter_no_repeat_request_when_target_same(void)
{
    /* ApplyDecision de-duplicates: same target mode should not trigger
     * another ComM_RequestComMode call. */
    CanComm_OutputDecisionType decision;
    CanComm_ChannelIdType ch;

    CanComm_Mock_Reset();

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        decision.targetComMode[ch] = COMM_FULL_COMMUNICATION;
        decision.targetTxGate[ch] = CANCOMM_TX_UNBLOCK;
    }
    decision.bswmRequest = CANCOMM_BSWM_REQUEST_ALL_FULL;
    decision.sleepCandidate = FALSE;

    (void)CanComm_Adapter_ApplyDecision(&decision);
    (void)CanComm_Adapter_ApplyDecision(&decision);

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_UINT8(1u, CanComm_Mock_GetRequestComModeCallCount(ch));
    }
}

void test_adapter_handle_comm_e_not_ok(void)
{
    ComM_ModeType mode;

    TEST_ASSERT_EQUAL(E_NOT_OK, CanComm_Adapter_GetComMode(CANCOMM_CH_NUM, &mode));
    TEST_ASSERT_EQUAL(E_NOT_OK, CanComm_Adapter_RequestComMode(CANCOMM_CH_NUM, COMM_FULL_COMMUNICATION));
}

void test_adapter_bswm_request_called(void)
{
    CanComm_OutputDecisionType decision;
    CanComm_ChannelIdType ch;

    CanComm_Mock_Reset();

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        decision.targetComMode[ch] = COMM_FULL_COMMUNICATION;
        decision.targetTxGate[ch] = CANCOMM_TX_UNBLOCK;
    }
    decision.bswmRequest = CANCOMM_BSWM_REQUEST_SLEEP_CANDIDATE;
    decision.sleepCandidate = TRUE;

    TEST_ASSERT_EQUAL(E_OK, CanComm_Adapter_ApplyDecision(&decision));
    TEST_ASSERT_EQUAL_UINT8(1u, CanComm_Mock_GetBswMRequestCallCount());
    TEST_ASSERT_EQUAL_HEX8(CANCOMM_BSWM_REQUEST_SLEEP_CANDIDATE, CanComm_Mock_GetLastBswMRequest());
}

void test_adapter_request_com_mode_channel_bounds(void)
{
    ComM_ModeType mode;

    /* Channel out of bounds must return E_NOT_OK */
    TEST_ASSERT_EQUAL(E_NOT_OK, CanComm_Adapter_RequestComMode(CANCOMM_CH_NUM, COMM_FULL_COMMUNICATION));
    TEST_ASSERT_EQUAL(E_NOT_OK, CanComm_Adapter_GetComMode(CANCOMM_CH_NUM, &mode));
}

void test_adapter_build_snapshot_null(void)
{
    TEST_ASSERT_EQUAL(E_NOT_OK, CanComm_Adapter_BuildInputSnapshot(NULL_PTR));
}

void test_adapter_apply_decision_null(void)
{
    TEST_ASSERT_EQUAL(E_NOT_OK, CanComm_Adapter_ApplyDecision(NULL_PTR));
}
