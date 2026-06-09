#ifndef COMMPOLICY_POLICY_H
#define COMMPOLICY_POLICY_H

#include "Std_Types.h"
#include "CommPolicy_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- Pure Policy Engine ------------------------------------------------- */

extern Std_ReturnType CommPolicy_Policy_Evaluate(
    const CommPolicy_InputSnapshotType* input,
    CommPolicy_OutputDecisionType* decision);

#ifdef __cplusplus
}
#endif

#endif /* COMMPOLICY_POLICY_H */
