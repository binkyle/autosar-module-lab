#include "unity.h"
#include "CanCommCoordinator_State.h"
#include "CanCommCoordinator_Cfg.h"
#include "CanCommCoordinator_Types.h"

/* GenericMock control API */
extern void CanComm_Mock_Reset(void);

static void test_state_setup(void)
{
    CanComm_Mock_Reset();
    CanComm_State_Init();
}

/* --- State Tests -------------------------------------------------------- */

void test_state_init_default(void)
{
    test_state_setup();
    const CanComm_RuntimeStateType* state = CanComm_State_Get();

    TEST_ASSERT_EQUAL_HEX8(CANCOMM_PNC_NONE, state->lastPncRequest);
    TEST_ASSERT_EQUAL_HEX8(CANCOMM_HOLD_INVALID, state->holdState);
    TEST_ASSERT_EQUAL_UINT16(0u, state->holdTimerTicks);
}

void test_state_on_pnc_request_reload_hold_timer(void)
{
    test_state_setup();
    CanComm_State_OnPncRequest(CANCOMM_PNC_ADAS_ONLY);

    const CanComm_RuntimeStateType* state = CanComm_State_Get();

    TEST_ASSERT_EQUAL_HEX8(CANCOMM_PNC_ADAS_ONLY, state->lastPncRequest);
    TEST_ASSERT_EQUAL_HEX8(CANCOMM_HOLD_VALID, state->holdState);
    TEST_ASSERT_EQUAL_UINT16(CANCOMM_PNC_HOLD_TICKS, state->holdTimerTicks);
}

void test_state_tick_to_timeout(void)
{
    uint16 i;

    test_state_setup();
    CanComm_State_OnPncRequest(CANCOMM_PNC_GLOBAL);
    TEST_ASSERT_EQUAL_HEX8(CANCOMM_HOLD_VALID, CanComm_State_GetHoldState());

    /* Tick until timeout */
    for (i = 0u; i < CANCOMM_PNC_HOLD_TICKS; i++)
    {
        CanComm_State_Tick();
    }

    TEST_ASSERT_EQUAL_HEX8(CANCOMM_HOLD_INVALID, CanComm_State_GetHoldState());
}

void test_state_hold_valid_before_timeout(void)
{
    test_state_setup();
    CanComm_State_OnPncRequest(CANCOMM_PNC_ADAS_ONLY);

    /* Tick one less than timeout */
    CanComm_State_Tick();

    TEST_ASSERT_EQUAL_HEX8(CANCOMM_HOLD_VALID, CanComm_State_GetHoldState());
}

void test_state_hold_invalid_after_timeout(void)
{
    uint16 i;

    test_state_setup();
    CanComm_State_OnPncRequest(CANCOMM_PNC_GLOBAL);

    for (i = 0u; i <= CANCOMM_PNC_HOLD_TICKS; i++)
    {
        CanComm_State_Tick();
    }

    TEST_ASSERT_EQUAL_HEX8(CANCOMM_HOLD_INVALID, CanComm_State_GetHoldState());
}

void test_state_exclusive_area_called(void)
{
    extern boolean CanComm_Mock_GetExclusiveAreaEntered(void);
    extern boolean CanComm_Mock_GetExclusiveAreaExited(void);

    test_state_setup();

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
    test_state_setup();

    CanComm_State_OnPncRequest(CANCOMM_PNC_INVALID);

    const CanComm_RuntimeStateType* state = CanComm_State_Get();
    TEST_ASSERT_EQUAL_HEX8(CANCOMM_HOLD_INVALID, state->holdState);
    TEST_ASSERT_EQUAL_UINT16(0u, state->holdTimerTicks);
}

void test_state_on_pnc_unknown_does_not_reload(void)
{
    test_state_setup();

    CanComm_State_OnPncRequest(CANCOMM_PNC_UNKNOWN);

    const CanComm_RuntimeStateType* state = CanComm_State_Get();
    TEST_ASSERT_EQUAL_HEX8(CANCOMM_HOLD_INVALID, state->holdState);
    TEST_ASSERT_EQUAL_UINT16(0u, state->holdTimerTicks);
}
