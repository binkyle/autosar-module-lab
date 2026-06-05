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
    CanComm_SourceMaskType activeSourceMask;  /* Sources managed by State layer */
    uint16 holdTimerTicks;
    CanComm_OutputDecisionType lastDecision;
    ComM_ModeType lastAppliedMode[CANCOMM_CH_NUM];
} CanComm_RuntimeStateType;

/* --- API ---------------------------------------------------------------- */

extern void CanComm_State_Init(void);
extern void CanComm_State_Tick(void);
extern void CanComm_State_OnSourceActive(CanComm_SourceIdType sourceId);
extern void CanComm_State_OnSourceInactive(CanComm_SourceIdType sourceId);
extern const CanComm_SourceMaskType* CanComm_State_GetActiveSourceMask(void);
extern const CanComm_RuntimeStateType* CanComm_State_Get(void);

#ifdef __cplusplus
}
#endif

#endif /* CANCOMMCOORDINATOR_STATE_H */
