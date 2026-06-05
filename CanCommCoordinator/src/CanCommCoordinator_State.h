#ifndef CANCOMMCOORDINATOR_STATE_H
#define CANCOMMCOORDINATOR_STATE_H

#include "Std_Types.h"
#include "CanCommCoordinator_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- Runtime State ------------------------------------------------------ */
typedef struct
{
    CanComm_PncRequestType lastPncRequest;
    CanComm_HoldStateType holdState;
    uint16 holdTimerTicks;
    CanComm_OutputDecisionType lastDecision;
    ComM_ModeType lastAppliedMode[CANCOMM_CH_NUM];
} CanComm_RuntimeStateType;

/* --- API ---------------------------------------------------------------- */

extern void CanComm_State_Init(void);
extern void CanComm_State_Tick(void);
extern void CanComm_State_OnPncRequest(CanComm_PncRequestType pncRequest);
extern CanComm_HoldStateType CanComm_State_GetHoldState(void);
extern const CanComm_RuntimeStateType* CanComm_State_Get(void);

#ifdef __cplusplus
}
#endif

#endif /* CANCOMMCOORDINATOR_STATE_H */
