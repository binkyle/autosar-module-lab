#include "unity.h"
#include "CanCommCoordinator.h"
#include "CanCommCoordinator_Cfg.h"
#include "CanCommCoordinator_Types.h"
#include "CanCommCoordinator_SourceMask.h"

/* GenericMock control API */
extern void CanComm_Mock_Reset(void);
extern void CanComm_Mock_SetSourceActive(CanComm_SourceIdType sourceId);
extern uint8 CanComm_Mock_GetRequestComModeCallCount(CanComm_ChannelIdType ch);

void test_mainfunction_repeated_call_no_duplicate_request(void)
{
    CanComm_ChannelIdType ch;

    CanComm_Mock_Reset();
    CanCommCoordinator_Init();

    /* First MainFunction call with IGNITION ON -> should request FULL */
    CanComm_Mock_SetSourceActive(0u);
    CanCommCoordinator_MainFunction();

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_UINT8(1u, CanComm_Mock_GetRequestComModeCallCount(ch));
    }

    /* Second MainFunction call with same input -> should NOT request again
     * because ApplyDecision de-duplicates unchanged target modes. */
    CanCommCoordinator_MainFunction();

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_UINT8(1u, CanComm_Mock_GetRequestComModeCallCount(ch));
    }
}

void test_mainfunction_init_then_run(void)
{
    CanComm_Mock_Reset();
    CanCommCoordinator_Init();

    /* After Init, running MainFunction should not crash */
    CanCommCoordinator_MainFunction();

    TEST_ASSERT_TRUE(1); /* If we get here, no crash occurred */
}
