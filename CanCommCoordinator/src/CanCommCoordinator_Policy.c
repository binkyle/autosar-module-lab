#include "Compiler.h"
#include "CanCommCoordinator_Policy.h"
#include "CanCommCoordinator_Cfg.h"
#include "CanCommCoordinator_Types.h"

/* --- External Config ---------------------------------------------------- */
extern const CanComm_PolicyRuleType CanComm_PolicyRuleTable[];
extern const uint8 CanComm_PolicyRuleCount;
extern const CanComm_OutputActionSetType* const CanComm_FallbackActionSet;
extern const CanComm_ChannelConfigType CanComm_ChannelConfigTable[CANCOMM_CH_NUM];

/* --- Private Helpers ---------------------------------------------------- */

STATIC boolean Policy_MatchRule(
    const CanComm_PolicyRuleType* rule,
    const CanComm_InputSnapshotType* input)
{
    /*
     * Wildcard logic: for some fields, a specific enum value in the rule
     * means "don't care". Here we use exact match for clarity,
     * but ignition=ON ignores PNC/hold/local because KL15 ON is highest priority.
     */
    if (rule->ignition != input->ignition)
    {
        return FALSE;
    }

    /* For KL15 ON, we don't care about other fields (already covered by rule ordering) */
    if (input->ignition == CANCOMM_IGN_ON)
    {
        return TRUE;
    }

    /* KL15 OFF cases: exact match on PNC, hold, localKeepAwake */
    if (rule->pncRequest != input->pncRequest)
    {
        return FALSE;
    }

    if (rule->holdState != input->holdState)
    {
        return FALSE;
    }

    if (rule->localKeepAwake != input->localKeepAwake)
    {
        return FALSE;
    }

    return TRUE;
}

STATIC void Policy_BuildDecisionFromActionSet(
    const CanComm_OutputActionSetType* actionSet,
    CanComm_OutputDecisionType* decision)
{
    CanComm_ChannelIdType ch;

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        uint32 mask = CanComm_ChannelConfigTable[ch].channelMask;

        /* Evaluate ComM mode: fullComMask takes priority over noComMask */
        if ((actionSet->fullComMask & mask) != 0u)
        {
            decision->targetComMode[ch] = COMM_FULL_COMMUNICATION;
        }
        else if ((actionSet->noComMask & mask) != 0u)
        {
            decision->targetComMode[ch] = COMM_NO_COMMUNICATION;
        }
        else
        {
            /* Default safe state if neither mask is set: NO_COMMUNICATION */
            decision->targetComMode[ch] = COMM_NO_COMMUNICATION;
        }

        /* Evaluate Tx gate: txUnblockMask takes priority over txBlockMask */
        if ((actionSet->txUnblockMask & mask) != 0u)
        {
            decision->targetTxGate[ch] = CANCOMM_TX_UNBLOCK;
        }
        else if ((actionSet->txBlockMask & mask) != 0u)
        {
            decision->targetTxGate[ch] = CANCOMM_TX_BLOCK;
        }
        else
        {
            /* Default safe state: BLOCK */
            decision->targetTxGate[ch] = CANCOMM_TX_BLOCK;
        }
    }

    decision->bswmRequest = actionSet->bswmRequest;
    decision->sleepCandidate = actionSet->sleepCandidate;
}

STATIC void Policy_ApplyFallback(CanComm_OutputDecisionType* decision)
{
    if (CanComm_FallbackActionSet != NULL_PTR)
    {
        Policy_BuildDecisionFromActionSet(CanComm_FallbackActionSet, decision);
    }
    else
    {
        CanComm_ChannelIdType ch;
        /* Ultimate safe fallback if config is broken: all FULL */
        for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
        {
            decision->targetComMode[ch] = COMM_FULL_COMMUNICATION;
            decision->targetTxGate[ch] = CANCOMM_TX_UNBLOCK;
        }
        decision->bswmRequest = CANCOMM_BSWM_REQUEST_FULL;
        decision->sleepCandidate = FALSE;
    }
}

/* --- Public API --------------------------------------------------------- */

Std_ReturnType CanComm_Policy_Evaluate(
    const CanComm_InputSnapshotType* input,
    CanComm_OutputDecisionType* decision)
{
    uint8 i;

    if ((input == NULL_PTR) || (decision == NULL_PTR))
    {
        return E_NOT_OK;
    }

    /* Iterate policy rule table: first match wins */
    for (i = 0u; i < CanComm_PolicyRuleCount; i++)
    {
        if (Policy_MatchRule(&CanComm_PolicyRuleTable[i], input) == TRUE)
        {
            const CanComm_OutputActionSetType* actionSet = CanComm_PolicyRuleTable[i].actionSet;

            if (actionSet != NULL_PTR)
            {
                Policy_BuildDecisionFromActionSet(actionSet, decision);
            }
            else
            {
                /* Broken config: rule matched but has no actionSet */
                Policy_ApplyFallback(decision);
            }

            return E_OK;
        }
    }

    /* No rule matched -> apply configured fallback */
    Policy_ApplyFallback(decision);
    return E_OK;
}
