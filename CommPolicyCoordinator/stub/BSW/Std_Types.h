#ifndef STD_TYPES_H
#define STD_TYPES_H

#include "Platform_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8 Std_ReturnType;

#ifndef E_OK
#define E_OK 0x00u
#endif

#ifndef E_NOT_OK
#define E_NOT_OK 0x01u
#endif

#define STD_ON  0x01u
#define STD_OFF 0x00u
#define STD_ACTIVE 0x01u
#define STD_IDLE   0x00u

#ifndef NULL_PTR
#define NULL_PTR ((void*)0)
#endif

typedef struct
{
    uint16 vendorID;
    uint16 moduleID;
    uint8  sw_major_version;
    uint8  sw_minor_version;
    uint8  sw_patch_version;
} Std_VersionInfoType;

#ifdef __cplusplus
}
#endif

#endif /* STD_TYPES_H */
