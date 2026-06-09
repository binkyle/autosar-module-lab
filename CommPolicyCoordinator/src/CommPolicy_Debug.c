#include "CommPolicy_Debug.h"
#include "CommPolicy_Cfg.h"
#include "CommPolicy_SourceMask.h"

/* --- Private Debug State ------------------------------------------------ */
typedef struct
{
    CommPolicy_InputSnapshotType lastInput;
    CommPolicy_OutputDecisionType lastDecision;
    uint32 cycleCount;
} CommPolicy_DebugStateType;

static CommPolicy_DebugStateType CommPolicy_DebugState;

/* --- Implementation ----------------------------------------------------- */

void CommPolicy_Debug_Init(void)
{
    CommPolicy_LogicalChannelIdType ch;

    CommPolicy_SourceMask_Clear(&CommPolicy_DebugState.lastInput.activeSourceMask);

    for (ch = 0u; ch < COMM_POLICY_CH_NUM; ch++)
    {
        CommPolicy_DebugState.lastDecision.targetComMode[ch] = COMM_NO_COMMUNICATION;
        CommPolicy_DebugState.lastDecision.targetTxGate[ch] = COMM_POLICY_TX_BLOCK;
    }
    CommPolicy_DebugState.lastDecision.modeRequestId = COMM_POLICY_MODE_REQUEST_NONE;
    CommPolicy_DebugState.lastDecision.sleepCandidate = FALSE;

    CommPolicy_DebugState.cycleCount = 0u;
}

void CommPolicy_Debug_Update(
    const CommPolicy_InputSnapshotType* input,
    const CommPolicy_OutputDecisionType* decision)
{
    CommPolicy_LogicalChannelIdType ch;

    if ((input == NULL_PTR) || (decision == NULL_PTR))
    {
        return;
    }

    CommPolicy_SourceMask_Copy(&CommPolicy_DebugState.lastInput.activeSourceMask, &input->activeSourceMask);

    for (ch = 0u; ch < COMM_POLICY_CH_NUM; ch++)
    {
        CommPolicy_DebugState.lastDecision.targetComMode[ch] = decision->targetComMode[ch];
        CommPolicy_DebugState.lastDecision.targetTxGate[ch] = decision->targetTxGate[ch];
    }
    CommPolicy_DebugState.lastDecision.modeRequestId = decision->modeRequestId;
    CommPolicy_DebugState.lastDecision.sleepCandidate = decision->sleepCandidate;

    CommPolicy_DebugState.cycleCount++;
}
