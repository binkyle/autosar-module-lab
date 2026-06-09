#include "unity.h"
#include "CommPolicy_State.h"
#include "CommPolicy_SourceMask.h"
#include "CommPolicy_Cfg.h"

/* GenericCfg hold ticks */
#define EXAMPLE_HOLD_TICKS  10u

void test_state_init_default(void)
{
    CommPolicy_LogicalChannelIdType ch;
    const CommPolicy_RuntimeStateType* state;

    CommPolicy_State_Init();
    state = CommPolicy_State_Get();

    TEST_ASSERT_TRUE(CommPolicy_SourceMask_IsEmpty(&state->activeSourceMask));
    TEST_ASSERT_EQUAL_UINT16(0u, state->holdTimerTicks);

    for (ch = 0u; ch < COMM_POLICY_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_HEX8(COMM_NO_COMMUNICATION, state->lastDecision.targetComMode[ch]);
        TEST_ASSERT_EQUAL_HEX8(COMM_POLICY_TX_BLOCK, state->lastDecision.targetTxGate[ch]);
        TEST_ASSERT_EQUAL_HEX8(COMM_NO_COMMUNICATION, state->lastAppliedMode[ch]);
    }
}

void test_state_on_source_active_sets_bit(void)
{
    CommPolicy_State_Init();
    CommPolicy_State_OnSourceActive(5u);

    const CommPolicy_SourceMaskType* mask = CommPolicy_State_GetActiveSourceMask();
    TEST_ASSERT_TRUE(CommPolicy_SourceMask_IsSet(mask, 5u));
}

void test_state_on_source_inactive_clears_bit(void)
{
    CommPolicy_State_Init();
    CommPolicy_State_OnSourceActive(5u);
    CommPolicy_State_OnSourceInactive(5u);

    const CommPolicy_SourceMaskType* mask = CommPolicy_State_GetActiveSourceMask();
    TEST_ASSERT_FALSE(CommPolicy_SourceMask_IsSet(mask, 5u));
}

void test_state_tick_to_timeout(void)
{
    CommPolicy_State_Init();

    /* Activate a trigger source (id 1 = NETWORK_GLOBAL) which reloads hold timer */
    CommPolicy_State_OnSourceActive(1u);
    TEST_ASSERT_TRUE(CommPolicy_SourceMask_IsSet(CommPolicy_State_GetActiveSourceMask(), 6u));

    /* Tick until timeout */
    uint16 i;
    for (i = 0u; i < EXAMPLE_HOLD_TICKS; i++)
    {
        CommPolicy_State_Tick();
    }

    TEST_ASSERT_FALSE(CommPolicy_SourceMask_IsSet(CommPolicy_State_GetActiveSourceMask(), 6u));
}

void test_state_exclusive_area_called(void)
{
    extern boolean CommPolicy_Mock_GetExclusiveAreaEntered(void);
    extern boolean CommPolicy_Mock_GetExclusiveAreaExited(void);

    CommPolicy_State_Init();

    TEST_ASSERT_TRUE(CommPolicy_Mock_GetExclusiveAreaEntered());
    TEST_ASSERT_TRUE(CommPolicy_Mock_GetExclusiveAreaExited());
}

void test_state_null_pointer_get(void)
{
    /* CommPolicy_State_Get returns a pointer to static data; it never returns NULL */
    const CommPolicy_RuntimeStateType* state = CommPolicy_State_Get();
    TEST_ASSERT_NOT_NULL(state);
}

void test_state_on_unrelated_source_does_not_reload(void)
{
    CommPolicy_State_Init();

    /* Activate an unrelated source (not a trigger source) */
    CommPolicy_State_OnSourceActive(10u);

    const CommPolicy_RuntimeStateType* state = CommPolicy_State_Get();
    TEST_ASSERT_EQUAL_UINT16(0u, state->holdTimerTicks);
}
