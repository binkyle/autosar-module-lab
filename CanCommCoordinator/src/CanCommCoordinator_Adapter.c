#include "CanCommCoordinator_Adapter.h"
#include "CanCommCoordinator_Cfg.h"
#include "CanCommCoordinator_Types.h"
#include "SchM_CanCommCoordinator.h"

/* --- Default Stubs (intentionally fail at link-time if not overridden) -- */
/*
 * This file provides the default exclusive-area helpers ONLY.
 * All other Adapter functions MUST be provided by either:
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

/* --- Snapshot Builder (default implementation, calls concrete adapters) - */

Std_ReturnType CanComm_Adapter_BuildInputSnapshot(
    CanComm_InputSnapshotType* snapshot)
{
    Std_ReturnType ret = E_OK;

    if (snapshot == NULL_PTR)
    {
        return E_NOT_OK;
    }

    snapshot->ignition = CANCOMM_IGN_UNKNOWN;
    snapshot->pncRequest = CANCOMM_PNC_UNKNOWN;
    snapshot->holdState = CANCOMM_HOLD_INVALID;
    snapshot->localKeepAwake = CANCOMM_LOCAL_KEEP_NONE;
    snapshot->diagKeepAwake = FALSE;
    snapshot->otaKeepAwake = FALSE;

    if (CanComm_Adapter_GetIgnition(&snapshot->ignition) != E_OK)
    {
        ret = E_NOT_OK;
    }

    if (CanComm_Adapter_GetPncRequest(&snapshot->pncRequest) != E_OK)
    {
        ret = E_NOT_OK;
    }

    if (CanComm_Adapter_GetLocalKeepAwake(&snapshot->localKeepAwake) != E_OK)
    {
        ret = E_NOT_OK;
    }

    (void)CanComm_Adapter_GetDiagKeepAwake(&snapshot->diagKeepAwake);
    (void)CanComm_Adapter_GetOtaKeepAwake(&snapshot->otaKeepAwake);

    return ret;
}

/* --- Decision Application (default implementation, calls concrete adapters) */

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
