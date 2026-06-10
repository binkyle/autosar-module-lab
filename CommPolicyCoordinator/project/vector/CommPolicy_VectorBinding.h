#ifndef COMMPOLICY_VECTORBINDING_H
#define COMMPOLICY_VECTORBINDING_H

#include "Std_Types.h"
#include "CommPolicy_Types.h"
#include "ComM.h"

#ifdef __cplusplus
extern "C" {
#endif

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
    uint8                        bswmRequestingModule;
    uint8                        bswmRequestedMode;
} CommPolicy_VectorModeRequestBindingType;

extern const CommPolicy_VectorModeRequestBindingType CommPolicy_VectorModeRequestBindingTable[];
extern const uint8 CommPolicy_VectorModeRequestBindingCount;

#ifdef __cplusplus
}
#endif

#endif /* COMMPOLICY_VECTORBINDING_H */
