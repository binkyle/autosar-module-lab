#include "CanCommCoordinator.h"
#include "CanCommCoordinator_Cfg.h"
#include "CanCommCoordinator_Types.h"
#include "CanCommCoordinator_App.h"
#include "CanCommCoordinator_State.h"
#include "CanCommCoordinator_Adapter.h"

/* --- External Config ---------------------------------------------------- */
extern const CanComm_ChannelConfigType CanComm_ChannelConfigTable[CANCOMM_CH_NUM];

/* --- Public API --------------------------------------------------------- */

void CanCommCoordinator_Init(void)
{
    CanComm_Adapter_Reset();
    CanCommCoordinator_App_Init();
}

void CanCommCoordinator_MainFunction(void)
{
    CanCommCoordinator_App_MainFunction();
}
