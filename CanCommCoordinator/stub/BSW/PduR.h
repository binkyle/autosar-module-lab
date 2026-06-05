#ifndef PDUR_H
#define PDUR_H

#include "Std_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16 PduIdType;

typedef struct
{
    uint8* SduDataPtr;
    uint8* MetaDataPtr;
    uint16 SduLength;
} PduInfoType;

#ifdef __cplusplus
}
#endif

#endif /* PDUR_H */
