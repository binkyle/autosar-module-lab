#include "CommPolicy_Adapter.h"
#include "CommPolicy_Cfg.h"
#include "CommPolicy_Types.h"

/* --- Private state for de-duplication ----------------------------------- */
static CommPolicy_CommModeType lastRequestedMode[COMM_POLICY_CH_NUM];
static CommPolicy_ModeRequestIdType lastModeRequestId = COMM_POLICY_MODE_REQUEST_NONE;
static boolean initialized = FALSE;

/* --- Reset API (called by Init or test setup) --------------------------- */

void CommPolicy_Adapter_Reset(void)
{
    CommPolicy_LogicalChannelIdType ch;
    for (ch = 0u; ch < COMM_POLICY_CH_NUM; ch++)
    {
        lastRequestedMode[ch] = COMM_POLICY_NO_COMMUNICATION;
    }
    lastModeRequestId = COMM_POLICY_MODE_REQUEST_NONE;
    initialized = TRUE;
}

/* --- Decision Application (core implementation with de-duplication) ----- */
/*
 * This file provides the canonical ApplyDecision implementation.
 * It is NOT overridden by project layers; instead, project layers provide
 * the underlying primitives (RequestComMode, RequestMode, BuildInputSnapshot,
 * Enter/ExitExclusiveArea) via GenericMock or VectorAdapter.
 *
 * De-duplication: only calls ComM_RequestComMode when target mode changes,
 * reducing BSW overhead and NM message storms.
 */

Std_ReturnType CommPolicy_Adapter_ApplyDecision(
    const CommPolicy_OutputDecisionType* decision)
{
    Std_ReturnType ret = E_OK;
    CommPolicy_LogicalChannelIdType ch;

    if (decision == NULL_PTR)
    {
        return E_NOT_OK;
    }

    /* Lazy init: if Reset was never called, initialize now */
    if (initialized == FALSE)
    {
        CommPolicy_Adapter_Reset();
    }

    /* Apply per-channel ComM mode requests (de-duplicated) */
    for (ch = 0u; ch < COMM_POLICY_CH_NUM; ch++)
    {
        if (decision->targetComMode[ch] != lastRequestedMode[ch])
        {
            if (CommPolicy_Adapter_RequestComMode(ch, decision->targetComMode[ch]) == E_OK)
            {
                lastRequestedMode[ch] = decision->targetComMode[ch];
            }
            else
            {
                ret = E_NOT_OK;
            }
        }
    }

    /* Mode request (de-duplicated) */
    if (decision->modeRequestId != lastModeRequestId)
    {
        if (CommPolicy_Adapter_RequestMode(decision->modeRequestId) == E_OK)
        {
            lastModeRequestId = decision->modeRequestId;
        }
        else
        {
            ret = E_NOT_OK;
        }
    }

    return ret;
}
