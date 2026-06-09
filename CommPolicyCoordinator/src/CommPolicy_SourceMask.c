#include "Compiler.h"
#include "CommPolicy_SourceMask.h"
#include "CommPolicy_Cfg.h"

/* --- Implementation ----------------------------------------------------- */

void CommPolicy_SourceMask_Clear(CommPolicy_SourceMaskType* mask)
{
    uint8 i;

    if (mask != NULL_PTR)
    {
        for (i = 0u; i < COMM_POLICY_SOURCE_WORD_NUM; i++)
        {
            mask->word[i] = 0u;
        }
    }
}

Std_ReturnType CommPolicy_SourceMask_Set(
    CommPolicy_SourceMaskType* mask,
    CommPolicy_SourceIdType sourceId)
{
    if (mask == NULL_PTR)
    {
        return E_NOT_OK;
    }

    if (sourceId >= COMM_POLICY_MAX_SOURCE_NUM)
    {
        return E_NOT_OK;
    }

    mask->word[COMM_POLICY_SOURCE_WORD_INDEX(sourceId)] |= COMM_POLICY_SOURCE_BIT_MASK(sourceId);
    return E_OK;
}

Std_ReturnType CommPolicy_SourceMask_Reset(
    CommPolicy_SourceMaskType* mask,
    CommPolicy_SourceIdType sourceId)
{
    if (mask == NULL_PTR)
    {
        return E_NOT_OK;
    }

    if (sourceId >= COMM_POLICY_MAX_SOURCE_NUM)
    {
        return E_NOT_OK;
    }

    mask->word[COMM_POLICY_SOURCE_WORD_INDEX(sourceId)] &= ~COMM_POLICY_SOURCE_BIT_MASK(sourceId);
    return E_OK;
}

boolean CommPolicy_SourceMask_IsSet(
    const CommPolicy_SourceMaskType* mask,
    CommPolicy_SourceIdType sourceId)
{
    if ((mask == NULL_PTR) || (sourceId >= COMM_POLICY_MAX_SOURCE_NUM))
    {
        return FALSE;
    }

    return ((mask->word[COMM_POLICY_SOURCE_WORD_INDEX(sourceId)] & COMM_POLICY_SOURCE_BIT_MASK(sourceId)) != 0u)
        ? TRUE : FALSE;
}

boolean CommPolicy_SourceMask_IsEmpty(
    const CommPolicy_SourceMaskType* mask)
{
    uint8 i;

    if (mask == NULL_PTR)
    {
        return FALSE;
    }

    for (i = 0u; i < COMM_POLICY_SOURCE_WORD_NUM; i++)
    {
        if (mask->word[i] != 0u)
        {
            return FALSE;
        }
    }

    return TRUE;
}

boolean CommPolicy_SourceMask_IsAllSet(
    const CommPolicy_SourceMaskType* active,
    const CommPolicy_SourceMaskType* requiredAll)
{
    uint8 i;

    if ((active == NULL_PTR) || (requiredAll == NULL_PTR))
    {
        return FALSE;
    }

    for (i = 0u; i < COMM_POLICY_SOURCE_WORD_NUM; i++)
    {
        if ((active->word[i] & requiredAll->word[i]) != requiredAll->word[i])
        {
            return FALSE;
        }
    }

    return TRUE;
}

boolean CommPolicy_SourceMask_IsAnySet(
    const CommPolicy_SourceMaskType* active,
    const CommPolicy_SourceMaskType* requiredAny)
{
    uint8 i;
    boolean hasRequired = FALSE;

    if ((active == NULL_PTR) || (requiredAny == NULL_PTR))
    {
        return FALSE;
    }

    for (i = 0u; i < COMM_POLICY_SOURCE_WORD_NUM; i++)
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

boolean CommPolicy_SourceMask_IsNoneSet(
    const CommPolicy_SourceMaskType* active,
    const CommPolicy_SourceMaskType* forbidden)
{
    uint8 i;

    if ((active == NULL_PTR) || (forbidden == NULL_PTR))
    {
        return FALSE;
    }

    for (i = 0u; i < COMM_POLICY_SOURCE_WORD_NUM; i++)
    {
        if ((active->word[i] & forbidden->word[i]) != 0u)
        {
            return FALSE;
        }
    }

    return TRUE;
}

void CommPolicy_SourceMask_Copy(
    CommPolicy_SourceMaskType* dest,
    const CommPolicy_SourceMaskType* src)
{
    uint8 i;

    if ((dest != NULL_PTR) && (src != NULL_PTR))
    {
        for (i = 0u; i < COMM_POLICY_SOURCE_WORD_NUM; i++)
        {
            dest->word[i] = src->word[i];
        }
    }
}

void CommPolicy_SourceMask_Or(
    CommPolicy_SourceMaskType* dest,
    const CommPolicy_SourceMaskType* src)
{
    uint8 i;

    if ((dest != NULL_PTR) && (src != NULL_PTR))
    {
        for (i = 0u; i < COMM_POLICY_SOURCE_WORD_NUM; i++)
        {
            dest->word[i] |= src->word[i];
        }
    }
}
