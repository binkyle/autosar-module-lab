#include "CanCommCoordinator_App.h"
#include "CanCommCoordinator_Types.h"
#include "CanCommCoordinator_Adapter.h"
#include "CanCommCoordinator_State.h"
#include "CanCommCoordinator_Policy.h"
#include "CanCommCoordinator_Debug.h"
#include "CanCommCoordinator_SourceMask.h"

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
        /* Snapshot incomplete -> safe fallback: empty source mask */
        CanComm_SourceMask_Clear(&input.activeSourceMask);
    }

    /* 3. Merge runtime state sources (e.g. hold valid) into snapshot */
    CanComm_SourceMask_Or(&input.activeSourceMask, CanComm_State_GetActiveSourceMask());

    /* 4. Evaluate policy */
    (void)CanComm_Policy_Evaluate(&input, &decision);

    /* 5. Apply decision via adapters */
    (void)CanComm_Adapter_ApplyDecision(&decision);

    /* 6. Update debug / trace */
    CanComm_Debug_Update(&input, &decision);
}
