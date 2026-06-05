#include "Compiler.h"
#include "CanCommCoordinator_SourceMask.h"
#include "CanCommCoordinator_Cfg.h"

/* --- Implementation ----------------------------------------------------- */

void CanComm_SourceMask_Clear(CanComm_SourceMaskType* mask)
{
    uint8 i;

    if (mask != NULL_PTR)
    {
        for (i = 0u; i < CANCOMM_SOURCE_WORD_NUM; i++)
        {
            mask->word[i] = 0u;
        }
    }
}

Std_ReturnType CanComm_SourceMask_Set(
    CanComm_SourceMaskType* mask,
    CanComm_SourceIdType sourceId)
{
    if (mask == NULL_PTR)
    {
        return E_NOT_OK;
    }

    if (sourceId >= CANCOMM_MAX_SOURCE_NUM)
    {
        return E_NOT_OK;
    }

    mask->word[CANCOMM_SOURCE_WORD_INDEX(sourceId)] |= CANCOMM_SOURCE_BIT_MASK(sourceId);
    return E_OK;
}

Std_ReturnType CanComm_SourceMask_Reset(
    CanComm_SourceMaskType* mask,
    CanComm_SourceIdType sourceId)
{
    if (mask == NULL_PTR)
    {
        return E_NOT_OK;
    }

    if (sourceId >= CANCOMM_MAX_SOURCE_NUM)
    {
        return E_NOT_OK;
    }

    mask->word[CANCOMM_SOURCE_WORD_INDEX(sourceId)] &= ~CANCOMM_SOURCE_BIT_MASK(sourceId);
    return E_OK;
}

boolean CanComm_SourceMask_IsSet(
    const CanComm_SourceMaskType* mask,
    CanComm_SourceIdType sourceId)
{
    if ((mask == NULL_PTR) || (sourceId >= CANCOMM_MAX_SOURCE_NUM))
    {
        return FALSE;
    }

    return ((mask->word[CANCOMM_SOURCE_WORD_INDEX(sourceId)] & CANCOMM_SOURCE_BIT_MASK(sourceId)) != 0u)
        ? TRUE : FALSE;
}

boolean CanComm_SourceMask_IsEmpty(
    const CanComm_SourceMaskType* mask)
{
    uint8 i;

    if (mask == NULL_PTR)
    {
        return FALSE;
    }

    for (i = 0u; i < CANCOMM_SOURCE_WORD_NUM; i++)
    {
        if (mask->word[i] != 0u)
        {
            return FALSE;
        }
    }

    return TRUE;
}

boolean CanComm_SourceMask_IsAllSet(
    const CanComm_SourceMaskType* active,
    const CanComm_SourceMaskType* requiredAll)
{
    uint8 i;

    if ((active == NULL_PTR) || (requiredAll == NULL_PTR))
    {
        return FALSE;
    }

    for (i = 0u; i < CANCOMM_SOURCE_WORD_NUM; i++)
    {
        if ((active->word[i] & requiredAll->word[i]) != requiredAll->word[i])
        {
            return FALSE;
        }
    }

    return TRUE;
}

boolean CanComm_SourceMask_IsAnySet(
    const CanComm_SourceMaskType* active,
    const CanComm_SourceMaskType* requiredAny)
{
    uint8 i;
    boolean hasRequired = FALSE;

    if ((active == NULL_PTR) || (requiredAny == NULL_PTR))
    {
        return FALSE;
    }

    for (i = 0u; i < CANCOMM_SOURCE_WORD_NUM; i++)
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

boolean CanComm_SourceMask_IsNoneSet(
    const CanComm_SourceMaskType* active,
    const CanComm_SourceMaskType* forbidden)
{
    uint8 i;

    if ((active == NULL_PTR) || (forbidden == NULL_PTR))
    {
        return FALSE;
    }

    for (i = 0u; i < CANCOMM_SOURCE_WORD_NUM; i++)
    {
        if ((active->word[i] & forbidden->word[i]) != 0u)
        {
            return FALSE;
        }
    }

    return TRUE;
}

void CanComm_SourceMask_Copy(
    CanComm_SourceMaskType* dest,
    const CanComm_SourceMaskType* src)
{
    uint8 i;

    if ((dest != NULL_PTR) && (src != NULL_PTR))
    {
        for (i = 0u; i < CANCOMM_SOURCE_WORD_NUM; i++)
        {
            dest->word[i] = src->word[i];
        }
    }
}

void CanComm_SourceMask_Or(
    CanComm_SourceMaskType* dest,
    const CanComm_SourceMaskType* src)
{
    uint8 i;

    if ((dest != NULL_PTR) && (src != NULL_PTR))
    {
        for (i = 0u; i < CANCOMM_SOURCE_WORD_NUM; i++)
        {
            dest->word[i] |= src->word[i];
        }
    }
}
