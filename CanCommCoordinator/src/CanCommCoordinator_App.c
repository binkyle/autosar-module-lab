#include "CanCommCoordinator_App.h"
#include "CanCommCoordinator_Types.h"
#include "CanCommCoordinator_Adapter.h"
#include "CanCommCoordinator_State.h"
#include "CanCommCoordinator_Policy.h"
#include "CanCommCoordinator_Debug.h"

void CanCommCoordinator_App_Init(void)
{
    CanComm_State_Init();
    CanComm_Debug_Init();
}

void CanCommCoordinator_App_MainFunction(void)
{
    CanComm_InputSnapshotType input;
    CanComm_OutputDecisionType decision;
    Std_ReturnType ret;

    /* 1. Tick state machines (timers, hold logic) */
    CanComm_State_Tick();

    /* 2. Build input snapshot from adapters */
    ret = CanComm_Adapter_BuildInputSnapshot(&input);
    if (ret != E_OK)
    {
        /* Snapshot incomplete -> safe fallback via Policy */
        input.ignition = CANCOMM_IGN_UNKNOWN;
        input.pncRequest = CANCOMM_PNC_UNKNOWN;
        input.holdState = CANCOMM_HOLD_INVALID;
        input.localKeepAwake = CANCOMM_LOCAL_KEEP_NONE;
        input.diagKeepAwake = FALSE;
        input.otaKeepAwake = FALSE;
    }

    /* Enrich snapshot with runtime state */
    if (input.ignition == CANCOMM_IGN_OFF)
    {
        input.holdState = CanComm_State_GetHoldState();
    }

    /* 3. Evaluate policy */
    (void)CanComm_Policy_Evaluate(&input, &decision);

    /* 4. Apply decision via adapters */
    (void)CanComm_Adapter_ApplyDecision(&decision);

    /* 5. Update debug / trace */
    CanComm_Debug_Update(&input, &decision);
}
