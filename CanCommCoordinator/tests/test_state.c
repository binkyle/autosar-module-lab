#include "unity.h"
#include "CanCommCoordinator_State.h"
#include "CanCommCoordinator_SourceMask.h"
#include "CanCommCoordinator_Cfg.h"

void test_state_init_default(void)
{
    CanComm_ChannelIdType ch;
    const CanComm_RuntimeStateType* state;

    CanComm_State_Init();
    state = CanComm_State_Get();

    TEST_ASSERT_TRUE(CanComm_SourceMask_IsEmpty(&state->activeSourceMask));
    TEST_ASSERT_EQUAL_UINT16(0u, state->holdTimerTicks);

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        TEST_ASSERT_EQUAL_HEX8(COMM_NO_COMMUNICATION, state->lastDecision.targetComMode[ch]);
        TEST_ASSERT_EQUAL_HEX8(CANCOMM_TX_BLOCK, state->lastDecision.targetTxGate[ch]);
        TEST_ASSERT_EQUAL_HEX8(COMM_NO_COMMUNICATION, state->lastAppliedMode[ch]);
    }
}

void test_state_on_source_active_sets_bit(void)
{
    CanComm_State_Init();
    CanComm_State_OnSourceActive(5u);

    const CanComm_SourceMaskType* mask = CanComm_State_GetActiveSourceMask();
    TEST_ASSERT_TRUE(CanComm_SourceMask_IsSet(mask, 5u));
}

void test_state_on_source_inactive_clears_bit(void)
{
    CanComm_State_Init();
    CanComm_State_OnSourceActive(5u);
    CanComm_State_OnSourceInactive(5u);

    const CanComm_SourceMaskType* mask = CanComm_State_GetActiveSourceMask();
    TEST_ASSERT_FALSE(CanComm_SourceMask_IsSet(mask, 5u));
}

void test_state_tick_to_timeout(void)
{
    CanComm_State_Init();

    /* Activate a PNC source (id 1) which reloads hold timer */
    CanComm_State_OnSourceActive(1u);
    TEST_ASSERT_TRUE(CanComm_SourceMask_IsSet(CanComm_State_GetActiveSourceMask(), 6u));

    /* Tick until timeout */
    uint16 i;
    for (i = 0u; i < CANCOMM_PNC_HOLD_TICKS; i++)
    {
        CanComm_State_Tick();
    }

    TEST_ASSERT_FALSE(CanComm_SourceMask_IsSet(CanComm_State_GetActiveSourceMask(), 6u));
}

void test_state_exclusive_area_called(void)
{
    extern boolean CanComm_Mock_GetExclusiveAreaEntered(void);
    extern boolean CanComm_Mock_GetExclusiveAreaExited(void);

    CanComm_State_Init();

    TEST_ASSERT_TRUE(CanComm_Mock_GetExclusiveAreaEntered());
    TEST_ASSERT_TRUE(CanComm_Mock_GetExclusiveAreaExited());
}

void test_state_null_pointer_get(void)
{
    /* CanComm_State_Get returns a pointer to static data; it never returns NULL */
    const CanComm_RuntimeStateType* state = CanComm_State_Get();
    TEST_ASSERT_NOT_NULL(state);
}

void test_state_on_pnc_invalid_does_not_reload(void)
{
    CanComm_State_Init();

    /* Activate an unrelated source (not PNC) */
    CanComm_State_OnSourceActive(10u);

    const CanComm_RuntimeStateType* state = CanComm_State_Get();
    TEST_ASSERT_EQUAL_UINT16(0u, state->holdTimerTicks);
}
