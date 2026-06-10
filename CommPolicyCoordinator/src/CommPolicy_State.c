#include "CommPolicy_State.h"
#include "CommPolicy_Cfg.h"
#include "CommPolicy_Adapter.h"
#include "CommPolicy_SourceMask.h"

/* --- External Config ---------------------------------------------------- */
extern const CommPolicy_SourceTimerConfigType CommPolicy_SourceTimerConfigTable[];
extern const uint8 CommPolicy_SourceTimerConfigCount;

/* --- Private Data ------------------------------------------------------- */
static CommPolicy_RuntimeStateType CommPolicy_State;

/* --- Implementation ----------------------------------------------------- */

void CommPolicy_State_Init(void)
{
    CommPolicy_LogicalChannelIdType ch;

    CommPolicy_Adapter_EnterExclusiveArea();

    CommPolicy_SourceMask_Clear(&CommPolicy_State.activeSourceMask);
    CommPolicy_State.holdTimerTicks = 0u;

    for (ch = 0u; ch < COMM_POLICY_CH_NUM; ch++)
    {
        CommPolicy_State.lastDecision.targetComMode[ch] = COMM_POLICY_NO_COMMUNICATION;
        CommPolicy_State.lastDecision.targetTxGate[ch] = COMM_POLICY_TX_BLOCK;
        CommPolicy_State.lastAppliedMode[ch] = COMM_POLICY_NO_COMMUNICATION;
    }

    CommPolicy_State.lastDecision.modeRequestId = COMM_POLICY_MODE_REQUEST_NONE;
    CommPolicy_State.lastDecision.sleepCandidate = FALSE;

    CommPolicy_Adapter_ExitExclusiveArea();
}

void CommPolicy_State_Tick(void)
{
    uint8 i;

    CommPolicy_Adapter_EnterExclusiveArea();

    if (CommPolicy_State.holdTimerTicks > 0u)
    {
        CommPolicy_State.holdTimerTicks--;

        if (CommPolicy_State.holdTimerTicks == 0u)
        {
            /* Hold timer expired -> clear all hold sources from config table */
            for (i = 0u; i < CommPolicy_SourceTimerConfigCount; i++)
            {
                (void)CommPolicy_SourceMask_Reset(
                    &CommPolicy_State.activeSourceMask,
                    CommPolicy_SourceTimerConfigTable[i].holdSourceId);
            }
        }
    }

    CommPolicy_Adapter_ExitExclusiveArea();
}

void CommPolicy_State_OnSourceActive(CommPolicy_SourceIdType sourceId)
{
    uint8 i;

    CommPolicy_Adapter_EnterExclusiveArea();

    (void)CommPolicy_SourceMask_Set(&CommPolicy_State.activeSourceMask, sourceId);

    /* Check timer config table: if this source triggers a hold timer, reload it */
    for (i = 0u; i < CommPolicy_SourceTimerConfigCount; i++)
    {
        if (sourceId == CommPolicy_SourceTimerConfigTable[i].triggerSourceId)
        {
            CommPolicy_State.holdTimerTicks = CommPolicy_SourceTimerConfigTable[i].holdTicks;
            (void)CommPolicy_SourceMask_Set(
                &CommPolicy_State.activeSourceMask,
                CommPolicy_SourceTimerConfigTable[i].holdSourceId);
        }
    }

    CommPolicy_Adapter_ExitExclusiveArea();
}

void CommPolicy_State_OnSourceInactive(CommPolicy_SourceIdType sourceId)
{
    CommPolicy_Adapter_EnterExclusiveArea();

    (void)CommPolicy_SourceMask_Reset(&CommPolicy_State.activeSourceMask, sourceId);

    CommPolicy_Adapter_ExitExclusiveArea();
}

const CommPolicy_SourceMaskType* CommPolicy_State_GetActiveSourceMask(void)
{
    return &CommPolicy_State.activeSourceMask;
}

const CommPolicy_RuntimeStateType* CommPolicy_State_Get(void)
{
    return &CommPolicy_State;
}
