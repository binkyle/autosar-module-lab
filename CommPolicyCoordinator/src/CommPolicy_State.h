#ifndef COMMPOLICY_STATE_H
#define COMMPOLICY_STATE_H

#include "Std_Types.h"
#include "CommPolicy_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- Runtime State ------------------------------------------------------ */
typedef struct
{
    CommPolicy_SourceMaskType activeSourceMask;  /* Sources managed by State layer */
    uint16 holdTimerTicks;
    CommPolicy_OutputDecisionType lastDecision;
    ComM_ModeType lastAppliedMode[COMM_POLICY_CH_NUM];
} CommPolicy_RuntimeStateType;

/* --- API ---------------------------------------------------------------- */

extern void CommPolicy_State_Init(void);
extern void CommPolicy_State_Tick(void);
extern void CommPolicy_State_OnSourceActive(CommPolicy_SourceIdType sourceId);
extern void CommPolicy_State_OnSourceInactive(CommPolicy_SourceIdType sourceId);
extern const CommPolicy_SourceMaskType* CommPolicy_State_GetActiveSourceMask(void);
extern const CommPolicy_RuntimeStateType* CommPolicy_State_Get(void);

#ifdef __cplusplus
}
#endif

#endif /* COMMPOLICY_STATE_H */
