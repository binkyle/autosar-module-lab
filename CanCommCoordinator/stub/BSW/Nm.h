#ifndef NM_H
#define NM_H

#include "Std_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16 NetworkHandleType;

extern Std_ReturnType Nm_EnableCommunication(NetworkHandleType nmNetworkHandle);
extern Std_ReturnType Nm_DisableCommunication(NetworkHandleType nmNetworkHandle);

#ifdef __cplusplus
}
#endif

#endif /* NM_H */
