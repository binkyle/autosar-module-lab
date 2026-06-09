#ifndef COMMPOLICY_DEBUG_H
#define COMMPOLICY_DEBUG_H

#include "Std_Types.h"
#include "CommPolicy_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void CommPolicy_Debug_Init(void);
extern void CommPolicy_Debug_Update(
    const CommPolicy_InputSnapshotType* input,
    const CommPolicy_OutputDecisionType* decision);

#ifdef __cplusplus
}
#endif

#endif /* COMMPOLICY_DEBUG_H */
