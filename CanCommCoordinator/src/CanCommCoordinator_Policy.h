#ifndef CANCOMMCOORDINATOR_POLICY_H
#define CANCOMMCOORDINATOR_POLICY_H

#include "Std_Types.h"
#include "CanCommCoordinator_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- Pure Policy Engine ------------------------------------------------- */

extern Std_ReturnType CanComm_Policy_Evaluate(
    const CanComm_InputSnapshotType* input,
    CanComm_OutputDecisionType* decision);

#ifdef __cplusplus
}
#endif

#endif /* CANCOMMCOORDINATOR_POLICY_H */
