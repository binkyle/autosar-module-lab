#ifndef COMMPOLICY_ADAPTER_H
#define COMMPOLICY_ADAPTER_H

#include "Std_Types.h"
#include "CommPolicy_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- Abstract Adapter Interface (project must implement) ---------------- */

/* Per-channel communication mode operations (used by ApplyDecision implementation) */
extern Std_ReturnType CommPolicy_Adapter_GetComMode(
    CommPolicy_LogicalChannelIdType ch,
    CommPolicy_CommModeType* mode);

extern Std_ReturnType CommPolicy_Adapter_RequestComMode(
    CommPolicy_LogicalChannelIdType ch,
    CommPolicy_CommModeType mode);

extern Std_ReturnType CommPolicy_Adapter_RequestMode(
    CommPolicy_ModeRequestIdType modeRequestId);

/* Exclusive area protection */
extern void CommPolicy_Adapter_EnterExclusiveArea(void);
extern void CommPolicy_Adapter_ExitExclusiveArea(void);

/* --- Snapshot & Decision Application ------------------------------------ */

extern Std_ReturnType CommPolicy_Adapter_BuildInputSnapshot(
    CommPolicy_InputSnapshotType* snapshot);

/* Core canonical ApplyDecision with de-duplication */
extern Std_ReturnType CommPolicy_Adapter_ApplyDecision(
    const CommPolicy_OutputDecisionType* decision);

/* --- Reset -------------------------------------------------------------- */
extern void CommPolicy_Adapter_Reset(void);

#ifdef __cplusplus
}
#endif

#endif /* COMMPOLICY_ADAPTER_H */
