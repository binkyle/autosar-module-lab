#include "Compiler.h"
#include "CommPolicy_ChannelMask.h"
#include "CommPolicy_Cfg.h"

/* --- Implementation ----------------------------------------------------- */

void CommPolicy_ChannelMask_Clear(CommPolicy_ChannelMaskType* mask)
{
    uint8 i;

    if (mask != NULL_PTR)
    {
        for (i = 0u; i < COMM_POLICY_CHANNEL_WORD_NUM; i++)
        {
            mask->word[i] = 0u;
        }
    }
}

Std_ReturnType CommPolicy_ChannelMask_Set(
    CommPolicy_ChannelMaskType* mask,
    CommPolicy_LogicalChannelIdType ch)
{
    if (mask == NULL_PTR)
    {
        return E_NOT_OK;
    }

    if (ch >= COMM_POLICY_MAX_CHANNEL_NUM)
    {
        return E_NOT_OK;
    }

    mask->word[COMM_POLICY_CHANNEL_WORD_INDEX(ch)] |= COMM_POLICY_CHANNEL_BIT_MASK(ch);
    return E_OK;
}

Std_ReturnType CommPolicy_ChannelMask_Reset(
    CommPolicy_ChannelMaskType* mask,
    CommPolicy_LogicalChannelIdType ch)
{
    if (mask == NULL_PTR)
    {
        return E_NOT_OK;
    }

    if (ch >= COMM_POLICY_MAX_CHANNEL_NUM)
    {
        return E_NOT_OK;
    }

    mask->word[COMM_POLICY_CHANNEL_WORD_INDEX(ch)] &= ~COMM_POLICY_CHANNEL_BIT_MASK(ch);
    return E_OK;
}

boolean CommPolicy_ChannelMask_IsSet(
    const CommPolicy_ChannelMaskType* mask,
    CommPolicy_LogicalChannelIdType ch)
{
    if ((mask == NULL_PTR) || (ch >= COMM_POLICY_MAX_CHANNEL_NUM))
    {
        return FALSE;
    }

    return ((mask->word[COMM_POLICY_CHANNEL_WORD_INDEX(ch)] & COMM_POLICY_CHANNEL_BIT_MASK(ch)) != 0u)
        ? TRUE : FALSE;
}

boolean CommPolicy_ChannelMask_IsEmpty(
    const CommPolicy_ChannelMaskType* mask)
{
    uint8 i;

    if (mask == NULL_PTR)
    {
        return FALSE;
    }

    for (i = 0u; i < COMM_POLICY_CHANNEL_WORD_NUM; i++)
    {
        if (mask->word[i] != 0u)
        {
            return FALSE;
        }
    }

    return TRUE;
}

boolean CommPolicy_ChannelMask_IsAllSet(
    const CommPolicy_ChannelMaskType* active,
    const CommPolicy_ChannelMaskType* requiredAll)
{
    uint8 i;

    if ((active == NULL_PTR) || (requiredAll == NULL_PTR))
    {
        return FALSE;
    }

    for (i = 0u; i < COMM_POLICY_CHANNEL_WORD_NUM; i++)
    {
        if ((active->word[i] & requiredAll->word[i]) != requiredAll->word[i])
        {
            return FALSE;
        }
    }

    return TRUE;
}

boolean CommPolicy_ChannelMask_IsAnySet(
    const CommPolicy_ChannelMaskType* active,
    const CommPolicy_ChannelMaskType* requiredAny)
{
    uint8 i;
    boolean hasRequired = FALSE;

    if ((active == NULL_PTR) || (requiredAny == NULL_PTR))
    {
        return FALSE;
    }

    for (i = 0u; i < COMM_POLICY_CHANNEL_WORD_NUM; i++)
    {
        if (requiredAny->word[i] != 0u)
        {
            hasRequired = TRUE;
            if ((active->word[i] & requiredAny->word[i]) != 0u)
            {
                return TRUE;
            }
        }
    }

    /* If requiredAny is empty (all zeros), condition is vacuously true */
    return !hasRequired;
}

boolean CommPolicy_ChannelMask_IsNoneSet(
    const CommPolicy_ChannelMaskType* active,
    const CommPolicy_ChannelMaskType* forbidden)
{
    uint8 i;

    if ((active == NULL_PTR) || (forbidden == NULL_PTR))
    {
        return FALSE;
    }

    for (i = 0u; i < COMM_POLICY_CHANNEL_WORD_NUM; i++)
    {
        if ((active->word[i] & forbidden->word[i]) != 0u)
        {
            return FALSE;
        }
    }

    return TRUE;
}

void CommPolicy_ChannelMask_Copy(
    CommPolicy_ChannelMaskType* dest,
    const CommPolicy_ChannelMaskType* src)
{
    uint8 i;

    if ((dest != NULL_PTR) && (src != NULL_PTR))
    {
        for (i = 0u; i < COMM_POLICY_CHANNEL_WORD_NUM; i++)
        {
            dest->word[i] = src->word[i];
        }
    }
}

void CommPolicy_ChannelMask_Or(
    CommPolicy_ChannelMaskType* dest,
    const CommPolicy_ChannelMaskType* src)
{
    uint8 i;

    if ((dest != NULL_PTR) && (src != NULL_PTR))
    {
        for (i = 0u; i < COMM_POLICY_CHANNEL_WORD_NUM; i++)
        {
            dest->word[i] |= src->word[i];
        }
    }
}
