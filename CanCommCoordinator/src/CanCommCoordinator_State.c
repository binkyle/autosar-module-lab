#include "CanCommCoordinator_State.h"
#include "CanCommCoordinator_Cfg.h"
#include "CanCommCoordinator_Adapter.h"

/* --- Private Data ------------------------------------------------------- */
static CanComm_RuntimeStateType CanComm_State;

/* --- Implementation ----------------------------------------------------- */

void CanComm_State_Init(void)
{
    CanComm_ChannelIdType ch;

    CanComm_Adapter_EnterExclusiveArea();

    CanComm_State.lastPncRequest = CANCOMM_PNC_NONE;
    CanComm_State.holdState = CANCOMM_HOLD_INVALID;
    CanComm_State.holdTimerTicks = 0u;

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        CanComm_State.lastDecision.targetComMode[ch] = COMM_NO_COMMUNICATION;
        CanComm_State.lastDecision.targetTxGate[ch] = CANCOMM_TX_BLOCK;
        CanComm_State.lastAppliedMode[ch] = COMM_NO_COMMUNICATION;
    }

    CanComm_State.lastDecision.bswmRequest = CANCOMM_BSWM_NO_REQUEST;
    CanComm_State.lastDecision.sleepCandidate = FALSE;

    CanComm_Adapter_ExitExclusiveArea();
}

void CanComm_State_Tick(void)
{
    CanComm_Adapter_EnterExclusiveArea();

    if (CanComm_State.holdTimerTicks > 0u)
    {
        CanComm_State.holdTimerTicks--;

        if (CanComm_State.holdTimerTicks == 0u)
        {
            CanComm_State.holdState = CANCOMM_HOLD_INVALID;
        }
    }

    CanComm_Adapter_ExitExclusiveArea();
}

void CanComm_State_OnPncRequest(CanComm_PncRequestType pncRequest)
{
    CanComm_Adapter_EnterExclusiveArea();

    CanComm_State.lastPncRequest = pncRequest;

    if ((pncRequest == CANCOMM_PNC_GLOBAL) || (pncRequest == CANCOMM_PNC_ADAS_ONLY))
    {
        CanComm_State.holdTimerTicks = CANCOMM_PNC_HOLD_TICKS;
        CanComm_State.holdState = CANCOMM_HOLD_VALID;
    }
    else if (pncRequest == CANCOMM_PNC_NONE)
    {
        /* Allow timer to run down naturally */
    }
    else
    {
        /* INVALID / UNKNOWN: do not reload timer */
    }

    CanComm_Adapter_ExitExclusiveArea();
}

CanComm_HoldStateType CanComm_State_GetHoldState(void)
{
    CanComm_HoldStateType state;

    CanComm_Adapter_EnterExclusiveArea();
    state = CanComm_State.holdState;
    CanComm_Adapter_ExitExclusiveArea();

    return state;
}

const CanComm_RuntimeStateType* CanComm_State_Get(void)
{
    return &CanComm_State;
}
