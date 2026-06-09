#ifndef CANNM_H
#define CANNM_H

#include "Std_Types.h"
#include "Nm.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8 Nm_StateType;
typedef uint8 Nm_ModeType;

#define NM_STATE_BUS_SLEEP            0x01u
#define NM_STATE_PREPARE_BUS_SLEEP    0x02u
#define NM_STATE_READY_SLEEP          0x03u
#define NM_STATE_NORMAL_OPERATION     0x04u
#define NM_STATE_REPEAT_MESSAGE       0x05u
#define NM_STATE_SYNCHRONIZE          0x06u

extern Std_ReturnType CanNm_GetState(NetworkHandleType nmChannelHandle, Nm_StateType* nmStatePtr, Nm_ModeType* nmModePtr);

#ifdef __cplusplus
}
#endif

#endif /* CANNM_H */
