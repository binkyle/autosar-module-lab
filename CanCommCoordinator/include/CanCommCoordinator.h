#ifndef CANCOMMCOORDINATOR_H
#define CANCOMMCOORDINATOR_H

#include "Std_Types.h"
#include "PduR.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- Module API --------------------------------------------------------- */

extern void CanCommCoordinator_Init(void);
extern void CanCommCoordinator_MainFunction(void);

#if (CANCOMM_ENABLE_LEGACY_NM_PARSE == STD_ON)
extern void CanCommCoordinator_OnNmRxIndication(
    PduIdType rxPduId,
    const PduInfoType* pduInfo);
#endif

#ifdef __cplusplus
}
#endif

#endif /* CANCOMMCOORDINATOR_H */
