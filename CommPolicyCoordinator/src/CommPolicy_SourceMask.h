#ifndef COMMPOLICY_SOURCEMASK_H
#define COMMPOLICY_SOURCEMASK_H

#include "Std_Types.h"
#include "CommPolicy_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- Source Mask Bit Helpers -------------------------------------------- */
#define COMM_POLICY_SOURCE_WORD_INDEX(id)   ((id) / COMM_POLICY_SOURCE_BITS_PER_WORD)
#define COMM_POLICY_SOURCE_BIT_INDEX(id)    ((id) % COMM_POLICY_SOURCE_BITS_PER_WORD)
#define COMM_POLICY_SOURCE_BIT_MASK(id)     ((uint32)1u << COMM_POLICY_SOURCE_BIT_INDEX(id))

/* --- Source Mask Tool Functions ----------------------------------------- */

extern void CommPolicy_SourceMask_Clear(CommPolicy_SourceMaskType* mask);

extern Std_ReturnType CommPolicy_SourceMask_Set(
    CommPolicy_SourceMaskType* mask,
    CommPolicy_SourceIdType sourceId);

extern Std_ReturnType CommPolicy_SourceMask_Reset(
    CommPolicy_SourceMaskType* mask,
    CommPolicy_SourceIdType sourceId);

extern boolean CommPolicy_SourceMask_IsSet(
    const CommPolicy_SourceMaskType* mask,
    CommPolicy_SourceIdType sourceId);

extern boolean CommPolicy_SourceMask_IsEmpty(
    const CommPolicy_SourceMaskType* mask);

extern boolean CommPolicy_SourceMask_IsAllSet(
    const CommPolicy_SourceMaskType* active,
    const CommPolicy_SourceMaskType* requiredAll);

extern boolean CommPolicy_SourceMask_IsAnySet(
    const CommPolicy_SourceMaskType* active,
    const CommPolicy_SourceMaskType* requiredAny);

extern boolean CommPolicy_SourceMask_IsNoneSet(
    const CommPolicy_SourceMaskType* active,
    const CommPolicy_SourceMaskType* forbidden);

/* --- Set Operations ----------------------------------------------------- */

extern void CommPolicy_SourceMask_Copy(
    CommPolicy_SourceMaskType* dest,
    const CommPolicy_SourceMaskType* src);

extern void CommPolicy_SourceMask_Or(
    CommPolicy_SourceMaskType* dest,
    const CommPolicy_SourceMaskType* src);

#ifdef __cplusplus
}
#endif

#endif /* COMMPOLICY_SOURCEMASK_H */
