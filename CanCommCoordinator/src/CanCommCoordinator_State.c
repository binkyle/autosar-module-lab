#include "CanCommCoordinator_State.h"
#include "CanCommCoordinator_Cfg.h"
#include "CanCommCoordinator_Adapter.h"
#include "CanCommCoordinator_SourceMask.h"

/* --- External Config ---------------------------------------------------- */
extern const CanComm_SourceTimerConfigType CanComm_SourceTimerConfigTable[];
extern const uint8 CanComm_SourceTimerConfigCount;

/* --- Private Data ------------------------------------------------------- */
static CanComm_RuntimeStateType CanComm_State;

/* --- Implementation ----------------------------------------------------- */

void CanComm_State_Init(void)
{
    CanComm_ChannelIdType ch;

    CanComm_Adapter_EnterExclusiveArea();

    CanComm_SourceMask_Clear(&CanComm_State.activeSourceMask);
    CanComm_State.holdTimerTicks = 0u;

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        CanComm_State.lastDecision.targetComMode[ch] = COMM_NO_COMMUNICATION;
        CanComm_State.lastDecision.targetTxGate[ch] = CANCOMM_TX_BLOCK;
        CanComm_State.lastAppliedMode[ch] = COMM_NO_COMMUNICATION;
    }

    CanComm_State.lastDecision.bswmRequest = CANCOMM_BSWM_NO_REQUEST;
    CanComm_State.lastDecision.sleepCandidate = FALSE;

    CanComm_Adapter_ExitExclusiveArea();
}

void CanComm_State_Tick(void)
{
    uint8 i;

    CanComm_Adapter_EnterExclusiveArea();

    if (CanComm_State.holdTimerTicks > 0u)
    {
        CanComm_State.holdTimerTicks--;

        if (CanComm_State.holdTimerTicks == 0u)
        {
            /* Hold timer expired -> clear all hold sources from config table */
            for (i = 0u; i < CanComm_SourceTimerConfigCount; i++)
            {
                (void)CanComm_SourceMask_Reset(
                    &CanComm_State.activeSourceMask,
                    CanComm_SourceTimerConfigTable[i].holdSourceId);
            }
        }
    }

    CanComm_Adapter_ExitExclusiveArea();
}

void CanComm_State_OnSourceActive(CanComm_SourceIdType sourceId)
{
    uint8 i;

    CanComm_Adapter_EnterExclusiveArea();

    (void)CanComm_SourceMask_Set(&CanComm_State.activeSourceMask, sourceId);

    /* Check timer config table: if this source triggers a hold timer, reload it */
    for (i = 0u; i < CanComm_SourceTimerConfigCount; i++)
    {
        if (sourceId == CanComm_SourceTimerConfigTable[i].triggerSourceId)
        {
            CanComm_State.holdTimerTicks = CanComm_SourceTimerConfigTable[i].holdTicks;
            (void)CanComm_SourceMask_Set(
                &CanComm_State.activeSourceMask,
                CanComm_SourceTimerConfigTable[i].holdSourceId);
        }
    }

    CanComm_Adapter_ExitExclusiveArea();
}

void CanComm_State_OnSourceInactive(CanComm_SourceIdType sourceId)
{
    CanComm_Adapter_EnterExclusiveArea();

    (void)CanComm_SourceMask_Reset(&CanComm_State.activeSourceMask, sourceId);

    CanComm_Adapter_ExitExclusiveArea();
}

const CanComm_SourceMaskType* CanComm_State_GetActiveSourceMask(void)
{
    return &CanComm_State.activeSourceMask;
}

const CanComm_RuntimeStateType* CanComm_State_Get(void)
{
    return &CanComm_State;
}
