#ifndef COMMPOLICY_CHANNELMASK_H
#define COMMPOLICY_CHANNELMASK_H

#include "Std_Types.h"
#include "CommPolicy_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- Channel Mask Bit Helpers ------------------------------------------- */
#define COMM_POLICY_CHANNEL_WORD_INDEX(id)   ((id) / COMM_POLICY_CHANNEL_BITS_PER_WORD)
#define COMM_POLICY_CHANNEL_BIT_INDEX(id)    ((id) % COMM_POLICY_CHANNEL_BITS_PER_WORD)
#define COMM_POLICY_CHANNEL_BIT_MASK(id)     ((uint32)1u << COMM_POLICY_CHANNEL_BIT_INDEX(id))

/* --- Channel Mask Tool Functions ---------------------------------------- */

extern void CommPolicy_ChannelMask_Clear(CommPolicy_ChannelMaskType* mask);

extern Std_ReturnType CommPolicy_ChannelMask_Set(
    CommPolicy_ChannelMaskType* mask,
    CommPolicy_LogicalChannelIdType ch);

extern Std_ReturnType CommPolicy_ChannelMask_Reset(
    CommPolicy_ChannelMaskType* mask,
    CommPolicy_LogicalChannelIdType ch);

extern boolean CommPolicy_ChannelMask_IsSet(
    const CommPolicy_ChannelMaskType* mask,
    CommPolicy_LogicalChannelIdType ch);

extern boolean CommPolicy_ChannelMask_IsEmpty(
    const CommPolicy_ChannelMaskType* mask);

extern boolean CommPolicy_ChannelMask_IsAllSet(
    const CommPolicy_ChannelMaskType* active,
    const CommPolicy_ChannelMaskType* requiredAll);

extern boolean CommPolicy_ChannelMask_IsAnySet(
    const CommPolicy_ChannelMaskType* active,
    const CommPolicy_ChannelMaskType* requiredAny);

extern boolean CommPolicy_ChannelMask_IsNoneSet(
    const CommPolicy_ChannelMaskType* active,
    const CommPolicy_ChannelMaskType* forbidden);

/* --- Set Operations ----------------------------------------------------- */

extern void CommPolicy_ChannelMask_Copy(
    CommPolicy_ChannelMaskType* dest,
    const CommPolicy_ChannelMaskType* src);

extern void CommPolicy_ChannelMask_Or(
    CommPolicy_ChannelMaskType* dest,
    const CommPolicy_ChannelMaskType* src);

#ifdef __cplusplus
}
#endif

#endif /* COMMPOLICY_CHANNELMASK_H */
