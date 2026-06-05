#include "CanCommCoordinator_Debug.h"
#include "CanCommCoordinator_Cfg.h"
#include "CanCommCoordinator_SourceMask.h"

/* --- Private Debug State ------------------------------------------------ */
typedef struct
{
    CanComm_InputSnapshotType lastInput;
    CanComm_OutputDecisionType lastDecision;
    uint32 cycleCount;
} CanComm_DebugStateType;

static CanComm_DebugStateType CanComm_DebugState;

/* --- Implementation ----------------------------------------------------- */

void CanComm_Debug_Init(void)
{
    CanComm_ChannelIdType ch;

    CanComm_SourceMask_Clear(&CanComm_DebugState.lastInput.activeSourceMask);

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        CanComm_DebugState.lastDecision.targetComMode[ch] = COMM_NO_COMMUNICATION;
        CanComm_DebugState.lastDecision.targetTxGate[ch] = CANCOMM_TX_BLOCK;
    }
    CanComm_DebugState.lastDecision.bswmRequest = CANCOMM_BSWM_NO_REQUEST;
    CanComm_DebugState.lastDecision.sleepCandidate = FALSE;

    CanComm_DebugState.cycleCount = 0u;
}

void CanComm_Debug_Update(
    const CanComm_InputSnapshotType* input,
    const CanComm_OutputDecisionType* decision)
{
    CanComm_ChannelIdType ch;

    if ((input == NULL_PTR) || (decision == NULL_PTR))
    {
        return;
    }

    CanComm_SourceMask_Copy(&CanComm_DebugState.lastInput.activeSourceMask, &input->activeSourceMask);

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        CanComm_DebugState.lastDecision.targetComMode[ch] = decision->targetComMode[ch];
        CanComm_DebugState.lastDecision.targetTxGate[ch] = decision->targetTxGate[ch];
    }
    CanComm_DebugState.lastDecision.bswmRequest = decision->bswmRequest;
    CanComm_DebugState.lastDecision.sleepCandidate = decision->sleepCandidate;

    CanComm_DebugState.cycleCount++;
}
