#ifndef CANCOMMCOORDINATOR_ADAPTER_H
#define CANCOMMCOORDINATOR_ADAPTER_H

#include "Std_Types.h"
#include "CanCommCoordinator_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- Abstract Adapter Interface ----------------------------------------- */

extern Std_ReturnType CanComm_Adapter_GetIgnition(
    CanComm_IgnitionModeType* ignition);

extern Std_ReturnType CanComm_Adapter_GetPncRequest(
    CanComm_PncRequestType* pncRequest);

extern Std_ReturnType CanComm_Adapter_GetComMode(
    CanComm_ChannelIdType ch,
    ComM_ModeType* mode);

extern Std_ReturnType CanComm_Adapter_RequestComMode(
    CanComm_ChannelIdType ch,
    ComM_ModeType mode);

extern Std_ReturnType CanComm_Adapter_RequestBswMMode(
    CanComm_BswMRequestType mode);

extern Std_ReturnType CanComm_Adapter_GetLocalKeepAwake(
    CanComm_LocalKeepAwakeType* keepAwake);

extern Std_ReturnType CanComm_Adapter_GetDiagKeepAwake(
    boolean* diagKeepAwake);

extern Std_ReturnType CanComm_Adapter_GetOtaKeepAwake(
    boolean* otaKeepAwake);

extern void CanComm_Adapter_EnterExclusiveArea(void);
extern void CanComm_Adapter_ExitExclusiveArea(void);

/* --- Snapshot & Decision Application ------------------------------------ */

extern Std_ReturnType CanComm_Adapter_BuildInputSnapshot(
    CanComm_InputSnapshotType* snapshot);

extern Std_ReturnType CanComm_Adapter_ApplyDecision(
    const CanComm_OutputDecisionType* decision);

#ifdef __cplusplus
}
#endif

#endif /* CANCOMMCOORDINATOR_ADAPTER_H */
