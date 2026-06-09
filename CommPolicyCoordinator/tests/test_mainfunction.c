#include "unity.h"
#include "CommPolicyCoordinator.h"
#include "CommPolicy_Cfg.h"
#include "CommPolicy_Types.h"
#include "CommPolicy_SourceMask.h"

/* GenericMock control API */
extern void CommPolicy_Mock_Reset(void);
extern void CommPolicy_Mock_SetSourceActive(CommPolicy_SourceIdType sourceId);
extern uint8 CommPolicy_Mock_GetRequestComModeCallCount(CommPolicy_LogicalChannelIdType ch);

void test_mainfunction_repeated_call_no_duplicate_request(void)
{
    CommPolicy_LogicalChannelIdType ch;

    CommPolicy_Mock_Reset();
    CommPolicyCoordinator_Init();

    /* First MainFunction call with POWER_RUN ON -> should request FULL */
    CommPolicy_Mock_SetSourceActive(0u);
    CommPolicyCoordinator_MainFunction();

    for (ch = 0u; ch < COMM_POLICY_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_UINT8(1u, CommPolicy_Mock_GetRequestComModeCallCount(ch));
    }

    /* Second MainFunction call with same input -> should NOT request again
     * because ApplyDecision de-duplicates unchanged target modes. */
    CommPolicyCoordinator_MainFunction();

    for (ch = 0u; ch < COMM_POLICY_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_UINT8(1u, CommPolicy_Mock_GetRequestComModeCallCount(ch));
    }
}

void test_mainfunction_init_then_run(void)
{
    CommPolicy_Mock_Reset();
    CommPolicyCoordinator_Init();

    /* After Init, running MainFunction should not crash */
    CommPolicyCoordinator_MainFunction();

    TEST_ASSERT_TRUE(1); /* If we get here, no crash occurred */
}
