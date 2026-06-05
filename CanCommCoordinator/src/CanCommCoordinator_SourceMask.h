#ifndef CANCOMMCOORDINATOR_SOURCEMASK_H
#define CANCOMMCOORDINATOR_SOURCEMASK_H

#include "Std_Types.h"
#include "CanCommCoordinator_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- Source Mask Bit Helpers -------------------------------------------- */
#define CANCOMM_SOURCE_WORD_INDEX(id)   ((id) / CANCOMM_SOURCE_BITS_PER_WORD)
#define CANCOMM_SOURCE_BIT_INDEX(id)    ((id) % CANCOMM_SOURCE_BITS_PER_WORD)
#define CANCOMM_SOURCE_BIT_MASK(id)     ((uint32)1u << CANCOMM_SOURCE_BIT_INDEX(id))

/* --- Source Mask Tool Functions ----------------------------------------- */

extern void CanComm_SourceMask_Clear(CanComm_SourceMaskType* mask);

extern Std_ReturnType CanComm_SourceMask_Set(
    CanComm_SourceMaskType* mask,
    CanComm_SourceIdType sourceId);

extern Std_ReturnType CanComm_SourceMask_Reset(
    CanComm_SourceMaskType* mask,
    CanComm_SourceIdType sourceId);

extern boolean CanComm_SourceMask_IsSet(
    const CanComm_SourceMaskType* mask,
    CanComm_SourceIdType sourceId);

extern boolean CanComm_SourceMask_IsEmpty(
    const CanComm_SourceMaskType* mask);

extern boolean CanComm_SourceMask_IsAllSet(
    const CanComm_SourceMaskType* active,
    const CanComm_SourceMaskType* requiredAll);

extern boolean CanComm_SourceMask_IsAnySet(
    const CanComm_SourceMaskType* active,
    const CanComm_SourceMaskType* requiredAny);

extern boolean CanComm_SourceMask_IsNoneSet(
    const CanComm_SourceMaskType* active,
    const CanComm_SourceMaskType* forbidden);

/* --- Set Operations ----------------------------------------------------- */

extern void CanComm_SourceMask_Copy(
    CanComm_SourceMaskType* dest,
    const CanComm_SourceMaskType* src);

extern void CanComm_SourceMask_Or(
    CanComm_SourceMaskType* dest,
    const CanComm_SourceMaskType* src);

#ifdef __cplusplus
}
#endif

#endif /* CANCOMMCOORDINATOR_SOURCEMASK_H */
