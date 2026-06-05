#include "CanCommCoordinator_Adapter.h"
#include "CanCommCoordinator_Cfg.h"
#include "CanCommCoordinator_Types.h"
#include "CanCommCoordinator_SourceMask.h"
#include "SchM_CanCommCoordinator.h"

/* --- Default Stubs (intentionally minimal, project layer overrides) ----- */
/*
 * This file provides default exclusive-area helpers and skeleton Adapter logic.
 * BuildInputSnapshot / ApplyDecision MAY be overridden by:
 *   - project/vector/CanCommCoordinator_VectorAdapter.c  (Vector target)
 *   - project/generic/CanCommCoordinator_GenericMock.c   (host test)
 *
 * Weak symbols are intentionally NOT used, because they are not portable
 * across all embedded compilers (e.g. MSVC, some ARM/GHS variants).
 */

void CanComm_Adapter_EnterExclusiveArea(void)
{
    SchM_Enter_CanCommCoordinator_CanCommExclusiveArea();
}

void CanComm_Adapter_ExitExclusiveArea(void)
{
    SchM_Exit_CanCommCoordinator_CanCommExclusiveArea();
}

/* --- Snapshot Builder (default: empty source mask, safe fallback) ------- */

Std_ReturnType CanComm_Adapter_BuildInputSnapshot(
    CanComm_InputSnapshotType* snapshot)
{
    if (snapshot == NULL_PTR)
    {
        return E_NOT_OK;
    }

    CanComm_SourceMask_Clear(&snapshot->activeSourceMask);
    return E_OK;
}

/* --- Decision Application (default implementation) ---------------------- */

Std_ReturnType CanComm_Adapter_ApplyDecision(
    const CanComm_OutputDecisionType* decision)
{
    Std_ReturnType ret = E_OK;
    CanComm_ChannelIdType ch;

    if (decision == NULL_PTR)
    {
        return E_NOT_OK;
    }

    /* Apply per-channel ComM mode requests */
    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        if (CanComm_Adapter_RequestComMode(ch, decision->targetComMode[ch]) != E_OK)
        {
            ret = E_NOT_OK;
        }
    }

    /* Optional: BswM request */
#if (CANCOMM_USE_BSWM_ACTION == STD_ON)
    if (CanComm_Adapter_RequestBswMMode(decision->bswmRequest) != E_OK)
    {
        ret = E_NOT_OK;
    }
#endif

    return ret;
}
