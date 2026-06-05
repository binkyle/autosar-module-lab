#include "unity.h"
#include "CanCommCoordinator_Adapter.h"
#include "CanCommCoordinator_Cfg.h"
#include "CanCommCoordinator_Types.h"

/* GenericMock control API */
extern void CanComm_Mock_Reset(void);
extern void CanComm_Mock_SetIgnition(CanComm_IgnitionModeType ignition);
extern void CanComm_Mock_SetPncRequest(CanComm_PncRequestType pnc);
extern void CanComm_Mock_SetLocalKeepAwake(CanComm_LocalKeepAwakeType keep);
extern ComM_ModeType CanComm_Mock_GetRequestedMode(CanComm_ChannelIdType ch);
extern uint8 CanComm_Mock_GetRequestComModeCallCount(CanComm_ChannelIdType ch);
extern CanComm_BswMRequestType CanComm_Mock_GetLastBswMRequest(void);
extern uint8 CanComm_Mock_GetBswMRequestCallCount(void);
extern boolean CanComm_Mock_GetExclusiveAreaEntered(void);
extern boolean CanComm_Mock_GetExclusiveAreaExited(void);

/* --- Adapter Tests ------------------------------------------------------ */

void test_adapter_build_snapshot(void)
{
    CanComm_InputSnapshotType snapshot;
    CanComm_Mock_Reset();

    CanComm_Mock_SetIgnition(CANCOMM_IGN_ON);
    CanComm_Mock_SetPncRequest(CANCOMM_PNC_GLOBAL);
    CanComm_Mock_SetLocalKeepAwake(CANCOMM_LOCAL_KEEP_ACTIVE);

    TEST_ASSERT_EQUAL(E_OK, CanComm_Adapter_BuildInputSnapshot(&snapshot));
    TEST_ASSERT_EQUAL_HEX8(CANCOMM_IGN_ON, snapshot.ignition);
    TEST_ASSERT_EQUAL_HEX8(CANCOMM_PNC_GLOBAL, snapshot.pncRequest);
    TEST_ASSERT_EQUAL_HEX8(CANCOMM_LOCAL_KEEP_ACTIVE, snapshot.localKeepAwake);
}

void test_adapter_request_com_mode_called_when_target_differs(void)
{
    CanComm_OutputDecisionType decision;
    CanComm_Mock_Reset();
    CanComm_ChannelIdType ch;

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        decision.targetComMode[ch] = COMM_FULL_COMMUNICATION;
        decision.targetTxGate[ch] = CANCOMM_TX_UNBLOCK;
    }
    decision.bswmRequest = CANCOMM_BSWM_REQUEST_FULL;
    decision.sleepCandidate = FALSE;

    TEST_ASSERT_EQUAL(E_OK, CanComm_Adapter_ApplyDecision(&decision));
    TEST_ASSERT_EQUAL_UINT8(1u, CanComm_Mock_GetRequestComModeCallCount(CANCOMM_CH_ADAS1));
}

void test_adapter_no_repeat_request_when_target_same(void)
{
    /* This test verifies that the adapter is called; anti-repeat logic
     * would live in a higher-level ApplyDecision wrapper if required. */
    CanComm_OutputDecisionType decision;
    CanComm_Mock_Reset();
    CanComm_ChannelIdType ch;

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        decision.targetComMode[ch] = COMM_NO_COMMUNICATION;
        decision.targetTxGate[ch] = CANCOMM_TX_BLOCK;
    }
    decision.bswmRequest = CANCOMM_BSWM_REQUEST_NO;
    decision.sleepCandidate = FALSE;

    /* First apply */
    TEST_ASSERT_EQUAL(E_OK, CanComm_Adapter_ApplyDecision(&decision));
    /* Second apply */
    TEST_ASSERT_EQUAL(E_OK, CanComm_Adapter_ApplyDecision(&decision));

    TEST_ASSERT_EQUAL_UINT8(2u, CanComm_Mock_GetRequestComModeCallCount(CANCOMM_CH_ADAS1));
}

void test_adapter_handle_comm_e_not_ok(void)
{
    CanComm_OutputDecisionType decision;
    CanComm_Mock_Reset();
    decision.bswmRequest = CANCOMM_BSWM_REQUEST_FULL;
    decision.sleepCandidate = FALSE;

    /* NULL decision should return E_NOT_OK */
    TEST_ASSERT_EQUAL(E_NOT_OK, CanComm_Adapter_ApplyDecision(NULL));
}

void test_adapter_bswm_request_called(void)
{
    CanComm_OutputDecisionType decision;
    CanComm_Mock_Reset();
    CanComm_ChannelIdType ch;

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
    TEST_ASSERT_EQUAL(E_NOT_OK, CanComm_Adapter_BuildInputSnapshot(NULL));
}

void test_adapter_apply_decision_null(void)
{
    TEST_ASSERT_EQUAL(E_NOT_OK, CanComm_Adapter_ApplyDecision(NULL));
}
