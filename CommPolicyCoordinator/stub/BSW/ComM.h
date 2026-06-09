#ifndef COMM_H
#define COMM_H

#include "Std_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8 ComM_ModeType;
#define COMM_NO_COMMUNICATION       0x00u
#define COMM_SILENT_COMMUNICATION   0x01u
#define COMM_FULL_COMMUNICATION     0x02u

typedef uint8 ComM_UserHandleType;
typedef uint8 NetworkHandleType;

#define ComMConf_ComMUser_CN_Backbone2_4e435fd4                         0
#define ComMConf_ComMUser_CN_Backbone2_4e435fd4_RX                      1
#define ComMConf_ComMUser_CN_LIN00_06e61e65                             2
#define ComMConf_ComMUser_CN_LIN01_3ff1080c                             3
#define ComMConf_ComMUser_CN_LIN02_0c2d96c8                             4
#define ComMConf_ComMUser_CN_LIN03_400ext_1856f4be                      5

extern Std_ReturnType ComM_RequestComMode(ComM_UserHandleType User, ComM_ModeType ComMode);
extern Std_ReturnType ComM_GetCurrentComMode(ComM_UserHandleType User, ComM_ModeType* ComMode);

#ifdef __cplusplus
}
#endif

#endif /* COMM_H */
