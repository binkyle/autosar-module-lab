#include "CommPolicyCoordinator_App.h"
#include "CommPolicy_Types.h"
#include "CommPolicy_Adapter.h"
#include "CommPolicy_State.h"
#include "CommPolicy_Policy.h"
#include "CommPolicy_Debug.h"
#include "CommPolicy_SourceMask.h"

void CommPolicyCoordinator_App_Init(void)
{
    CommPolicy_State_Init();
    CommPolicy_Debug_Init();
}

void CommPolicyCoordinator_App_MainFunction(void)
{
    CommPolicy_InputSnapshotType input;
    CommPolicy_OutputDecisionType decision;
    Std_ReturnType ret;

    /* 1. Tick state machines (timers, hold logic) */
    CommPolicy_State_Tick();

    /* 2. Build input snapshot from adapters */
    ret = CommPolicy_Adapter_BuildInputSnapshot(&input);
    if (ret != E_OK)
    {
        /* Snapshot incomplete -> safe fallback: empty source mask */
        CommPolicy_SourceMask_Clear(&input.activeSourceMask);
    }

    /* 3. Merge runtime state sources (e.g. hold valid) into snapshot */
    CommPolicy_SourceMask_Or(&input.activeSourceMask, CommPolicy_State_GetActiveSourceMask());

    /* 4. Evaluate policy */
    (void)CommPolicy_Policy_Evaluate(&input, &decision);

    /* 5. Apply decision via adapters */
    (void)CommPolicy_Adapter_ApplyDecision(&decision);

    /* 6. Update debug / trace */
    CommPolicy_Debug_Update(&input, &decision);
}
