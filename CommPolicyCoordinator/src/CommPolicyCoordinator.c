#include "CommPolicyCoordinator.h"
#include "CommPolicy_Cfg.h"
#include "CommPolicy_Types.h"
#include "CommPolicyCoordinator_App.h"
#include "CommPolicy_State.h"
#include "CommPolicy_Adapter.h"

/* --- Public API --------------------------------------------------------- */

void CommPolicyCoordinator_Init(void)
{
    CommPolicy_Adapter_Reset();
    CommPolicyCoordinator_App_Init();
}

void CommPolicyCoordinator_MainFunction(void)
{
    CommPolicyCoordinator_App_MainFunction();
}
