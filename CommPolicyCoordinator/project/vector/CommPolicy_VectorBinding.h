#ifndef COMMPOLICY_VECTORBINDING_H
#define COMMPOLICY_VECTORBINDING_H

/*
 * Vector binding layer only. Do not include from core include/src or project/generic.
 * This header contains Vector/MICROSAR-specific types and bindings that must NOT
 * leak into the core framework or generic project layers.
 */

#include "Std_Types.h"
#include "CommPolicy_Types.h"
#include "ComM.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- BswM abstraction types (project can map to Vector/BswM generated types) */
typedef uint16 CommPolicy_VectorBswMUserType;
typedef uint16 CommPolicy_VectorBswMModeType;

/* --- Vector Channel Binding (logicalChannel -> ComM User/NetworkHandle) - */
typedef struct
{
    CommPolicy_LogicalChannelIdType logicalChannel;
    ComM_UserHandleType             commUser;
    NetworkHandleType               networkHandle;
} CommPolicy_VectorChannelBindingType;

extern const CommPolicy_VectorChannelBindingType CommPolicy_VectorChannelBindingTable[];

/* --- Vector Mode Request Binding (modeRequestId -> BswM user/mode) ------ */
typedef struct
{
    CommPolicy_ModeRequestIdType modeRequestId;
    CommPolicy_VectorBswMUserType bswmRequestingModule;
    CommPolicy_VectorBswMModeType bswmRequestedMode;
} CommPolicy_VectorModeRequestBindingType;

extern const CommPolicy_VectorModeRequestBindingType CommPolicy_VectorModeRequestBindingTable[];
extern const uint8 CommPolicy_VectorModeRequestBindingCount;

#ifdef __cplusplus
}
#endif

#endif /* COMMPOLICY_VECTORBINDING_H */
