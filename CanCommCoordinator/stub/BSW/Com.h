#ifndef COM_H
#define COM_H

#include "Std_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Minimal Com stub for compilation */
extern void Com_IpduGroupStart(uint16 ipduGroupId, boolean initialize);
extern void Com_IpduGroupStop(uint16 ipduGroupId);

#ifdef __cplusplus
}
#endif

#endif /* COM_H */
