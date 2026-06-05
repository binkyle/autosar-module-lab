#ifndef CANCOMMCOORDINATOR_DEBUG_H
#define CANCOMMCOORDINATOR_DEBUG_H

#include "Std_Types.h"
#include "CanCommCoordinator_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void CanComm_Debug_Init(void);
extern void CanComm_Debug_Update(
    const CanComm_InputSnapshotType* input,
    const CanComm_OutputDecisionType* decision);

#ifdef __cplusplus
}
#endif

#endif /* CANCOMMCOORDINATOR_DEBUG_H */
