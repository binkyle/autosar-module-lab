#include "CanCommCoordinator_Adapter.h"
#include "CanCommCoordinator_Cfg.h"
#include "CanCommCoordinator_Types.h"

/* --- Private state for de-duplication ----------------------------------- */
static ComM_ModeType lastRequestedMode[CANCOMM_CH_NUM];
static CanComm_BswMRequestType lastBswMRequest = CANCOMM_BSWM_NO_REQUEST;
static boolean initialized = FALSE;

/* --- Reset API (called by Init or test setup) --------------------------- */

void CanComm_Adapter_Reset(void)
{
    CanComm_ChannelIdType ch;
    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        lastRequestedMode[ch] = COMM_NO_COMMUNICATION;
    }
    lastBswMRequest = CANCOMM_BSWM_NO_REQUEST;
    initialized = TRUE;
}

/* --- Decision Application (core implementation with de-duplication) ----- */
/*
 * This file provides the canonical ApplyDecision implementation.
 * It is NOT overridden by project layers; instead, project layers provide
 * the underlying primitives (RequestComMode, RequestBswMMode, BuildInputSnapshot,
 * Enter/ExitExclusiveArea) via GenericMock or VectorAdapter.
 *
 * De-duplication: only calls ComM_RequestComMode when target mode changes,
 * reducing BSW overhead and NM message storms.
 */

Std_ReturnType CanComm_Adapter_ApplyDecision(
    const CanComm_OutputDecisionType* decision)
{
    Std_ReturnType ret = E_OK;
    CanComm_ChannelIdType ch;

    if (decision == NULL_PTR)
    {
        return E_NOT_OK;
    }

    /* Lazy init: if Reset was never called, initialize now */
    if (initialized == FALSE)
    {
        CanComm_Adapter_Reset();
    }

    /* Apply per-channel ComM mode requests (de-duplicated) */
    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        if (decision->targetComMode[ch] != lastRequestedMode[ch])
        {
            if (CanComm_Adapter_RequestComMode(ch, decision->targetComMode[ch]) == E_OK)
            {
                lastRequestedMode[ch] = decision->targetComMode[ch];
            }
            else
            {
                ret = E_NOT_OK;
            }
        }
    }

    /* Optional: BswM request (de-duplicated) */
#if (CANCOMM_USE_BSWM_ACTION == STD_ON)
    if (decision->bswmRequest != lastBswMRequest)
    {
        if (CanComm_Adapter_RequestBswMMode(decision->bswmRequest) == E_OK)
        {
            lastBswMRequest = decision->bswmRequest;
        }
        else
        {
            ret = E_NOT_OK;
        }
    }
#endif

    return ret;
}
