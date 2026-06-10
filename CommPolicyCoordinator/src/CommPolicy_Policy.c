#include "Compiler.h"
#include "CommPolicy_Policy.h"
#include "CommPolicy_Cfg.h"
#include "CommPolicy_Types.h"
#include "CommPolicy_SourceMask.h"
#include "CommPolicy_ChannelMask.h"

/* --- External Config ---------------------------------------------------- */
extern const CommPolicy_PolicyRuleType CommPolicy_PolicyRuleTable[];
extern const uint8 CommPolicy_PolicyRuleCount;
extern const CommPolicy_OutputActionSetType* const CommPolicy_FallbackActionSet;

/* --- Private Helpers ---------------------------------------------------- */

STATIC boolean Policy_MatchRule(
    const CommPolicy_PolicyRuleType* rule,
    const CommPolicy_InputSnapshotType* input)
{
    if ((rule == NULL_PTR) || (input == NULL_PTR))
    {
        return FALSE;
    }

    return (boolean)(
        (CommPolicy_SourceMask_IsAllSet(&input->activeSourceMask, &rule->requiredAllMask) == TRUE) &&
        (CommPolicy_SourceMask_IsAnySet(&input->activeSourceMask, &rule->requiredAnyMask) == TRUE) &&
        (CommPolicy_SourceMask_IsNoneSet(&input->activeSourceMask, &rule->forbiddenMask) == TRUE));
}

STATIC void Policy_BuildDecisionFromActionSet(
    const CommPolicy_OutputActionSetType* actionSet,
    CommPolicy_OutputDecisionType* decision)
{
    CommPolicy_LogicalChannelIdType ch;

    for (ch = 0u; ch < COMM_POLICY_CH_NUM; ch++)
    {
        /* Evaluate ComM mode: fullComMask takes priority over noComMask */
        if (CommPolicy_ChannelMask_IsSet(&actionSet->fullComMask, ch) == TRUE)
        {
            decision->targetComMode[ch] = COMM_POLICY_FULL_COMMUNICATION;
        }
        else if (CommPolicy_ChannelMask_IsSet(&actionSet->noComMask, ch) == TRUE)
        {
            decision->targetComMode[ch] = COMM_POLICY_NO_COMMUNICATION;
        }
        else
        {
            /* Default safe state if neither mask is set: NO_COMMUNICATION */
            decision->targetComMode[ch] = COMM_POLICY_NO_COMMUNICATION;
        }

        /* Evaluate Tx gate: txUnblockMask takes priority over txBlockMask */
        if (CommPolicy_ChannelMask_IsSet(&actionSet->txUnblockMask, ch) == TRUE)
        {
            decision->targetTxGate[ch] = COMM_POLICY_TX_UNBLOCK;
        }
        else if (CommPolicy_ChannelMask_IsSet(&actionSet->txBlockMask, ch) == TRUE)
        {
            decision->targetTxGate[ch] = COMM_POLICY_TX_BLOCK;
        }
        else
        {
            /* Default safe state: BLOCK */
            decision->targetTxGate[ch] = COMM_POLICY_TX_BLOCK;
        }
    }

    decision->modeRequestId = actionSet->modeRequestId;
    decision->sleepCandidate = actionSet->sleepCandidate;
}

STATIC void Policy_ApplyFallback(CommPolicy_OutputDecisionType* decision)
{
    if (CommPolicy_FallbackActionSet != NULL_PTR)
    {
        Policy_BuildDecisionFromActionSet(CommPolicy_FallbackActionSet, decision);
    }
    else
    {
        CommPolicy_LogicalChannelIdType ch;
        /* Ultimate safe fallback if config is broken: all FULL */
        for (ch = 0u; ch < COMM_POLICY_CH_NUM; ch++)
        {
            decision->targetComMode[ch] = COMM_POLICY_FULL_COMMUNICATION;
            decision->targetTxGate[ch] = COMM_POLICY_TX_UNBLOCK;
        }
        decision->modeRequestId = COMM_POLICY_MODE_REQUEST_NONE;
        decision->sleepCandidate = FALSE;
    }
}

/* --- Public API --------------------------------------------------------- */

Std_ReturnType CommPolicy_Policy_Evaluate(
    const CommPolicy_InputSnapshotType* input,
    CommPolicy_OutputDecisionType* decision)
{
    uint8 i;
    const CommPolicy_PolicyRuleType* bestRule = NULL_PTR;
    uint16 bestPriority = 0u;
    boolean firstMatch = TRUE;

    if ((input == NULL_PTR) || (decision == NULL_PTR))
    {
        return E_NOT_OK;
    }

    /* Iterate all rules, find highest-priority match */
    for (i = 0u; i < CommPolicy_PolicyRuleCount; i++)
    {
        if (Policy_MatchRule(&CommPolicy_PolicyRuleTable[i], input) == TRUE)
        {
            if ((firstMatch == TRUE) ||
                (CommPolicy_PolicyRuleTable[i].priority > bestPriority))
            {
                bestRule = &CommPolicy_PolicyRuleTable[i];
                bestPriority = CommPolicy_PolicyRuleTable[i].priority;
                firstMatch = FALSE;
            }
        }
    }

    if (bestRule != NULL_PTR)
    {
        const CommPolicy_OutputActionSetType* actionSet = bestRule->actionSet;

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

    /* No rule matched -> apply configured fallback */
    Policy_ApplyFallback(decision);
    return E_OK;
}
