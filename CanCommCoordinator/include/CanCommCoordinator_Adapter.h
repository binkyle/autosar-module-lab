#ifndef CANCOMMCOORDINATOR_ADAPTER_H
#define CANCOMMCOORDINATOR_ADAPTER_H

#include "Std_Types.h"
#include "CanCommCoordinator_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- Abstract Adapter Interface ----------------------------------------- */

/* Per-channel ComM operations (used by ApplyDecision implementation) */
extern Std_ReturnType CanComm_Adapter_GetComMode(
    CanComm_ChannelIdType ch,
    ComM_ModeType* mode);

extern Std_ReturnType CanComm_Adapter_RequestComMode(
    CanComm_ChannelIdType ch,
    ComM_ModeType mode);

extern Std_ReturnType CanComm_Adapter_RequestBswMMode(
    CanComm_BswMRequestType mode);

/* Exclusive area protection */
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
