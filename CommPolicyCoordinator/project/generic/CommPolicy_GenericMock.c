#include "CommPolicy_Adapter.h"
#include "CommPolicy_Cfg.h"
#include "CommPolicy_Types.h"
#include "CommPolicy_SourceMask.h"

/*
 * Generic Mock Adapter for Host Unit Testing
 * ------------------------------------------
 * No dependencies on Vector/MICROSAR.
 * All outputs are controllable via test-settable variables.
 */

/* --- Test-Settable Mock State ------------------------------------------- */
typedef struct
{
    CommPolicy_SourceMaskType activeSourceMask;

    CommPolicy_CommModeType requestedMode[COMM_POLICY_CH_NUM];
    uint8 requestComModeCallCount[COMM_POLICY_CH_NUM];
    CommPolicy_ModeRequestIdType lastModeRequestId;
    uint8 modeRequestCallCount;

    boolean exclusiveAreaEntered;
    boolean exclusiveAreaExited;
} CommPolicy_MockStateType;

static CommPolicy_MockStateType CommPolicy_MockState;

/* --- Mock Control API --------------------------------------------------- */

void CommPolicy_Mock_Reset(void)
{
    CommPolicy_LogicalChannelIdType ch;

    CommPolicy_SourceMask_Clear(&CommPolicy_MockState.activeSourceMask);

    for (ch = 0u; ch < COMM_POLICY_CH_NUM; ch++)
    {
        CommPolicy_MockState.requestedMode[ch] = COMM_POLICY_NO_COMMUNICATION;
        CommPolicy_MockState.requestComModeCallCount[ch] = 0u;
    }

    CommPolicy_MockState.lastModeRequestId = COMM_POLICY_MODE_REQUEST_NONE;
    CommPolicy_MockState.modeRequestCallCount = 0u;
    CommPolicy_MockState.exclusiveAreaEntered = FALSE;
    CommPolicy_MockState.exclusiveAreaExited = FALSE;

    CommPolicy_Adapter_Reset();
}

void CommPolicy_Mock_SetSourceActive(CommPolicy_SourceIdType sourceId)
{
    (void)CommPolicy_SourceMask_Set(&CommPolicy_MockState.activeSourceMask, sourceId);
}

void CommPolicy_Mock_SetSourceInactive(CommPolicy_SourceIdType sourceId)
{
    (void)CommPolicy_SourceMask_Reset(&CommPolicy_MockState.activeSourceMask, sourceId);
}

void CommPolicy_Mock_ClearAllSources(void)
{
    CommPolicy_SourceMask_Clear(&CommPolicy_MockState.activeSourceMask);
}

const CommPolicy_SourceMaskType* CommPolicy_Mock_GetActiveSourceMask(void)
{
    return &CommPolicy_MockState.activeSourceMask;
}

CommPolicy_CommModeType CommPolicy_Mock_GetRequestedMode(CommPolicy_LogicalChannelIdType ch)
{
    if (ch < COMM_POLICY_CH_NUM)
    {
        return CommPolicy_MockState.requestedMode[ch];
    }
    return COMM_POLICY_NO_COMMUNICATION;
}

uint8 CommPolicy_Mock_GetRequestComModeCallCount(CommPolicy_LogicalChannelIdType ch)
{
    if (ch < COMM_POLICY_CH_NUM)
    {
        return CommPolicy_MockState.requestComModeCallCount[ch];
    }
    return 0u;
}

CommPolicy_ModeRequestIdType CommPolicy_Mock_GetLastModeRequestId(void)
{
    return CommPolicy_MockState.lastModeRequestId;
}

uint8 CommPolicy_Mock_GetModeRequestCallCount(void)
{
    return CommPolicy_MockState.modeRequestCallCount;
}

boolean CommPolicy_Mock_GetExclusiveAreaEntered(void)
{
    return CommPolicy_MockState.exclusiveAreaEntered;
}

boolean CommPolicy_Mock_GetExclusiveAreaExited(void)
{
    return CommPolicy_MockState.exclusiveAreaExited;
}

/* --- Adapter Implementation --------------------------------------------- */

Std_ReturnType CommPolicy_Adapter_GetComMode(
    CommPolicy_LogicalChannelIdType ch,
    CommPolicy_CommModeType* mode)
{
    if (ch < COMM_POLICY_CH_NUM)
    {
        *mode = CommPolicy_MockState.requestedMode[ch];
        return E_OK;
    }
    return E_NOT_OK;
}

Std_ReturnType CommPolicy_Adapter_RequestComMode(
    CommPolicy_LogicalChannelIdType ch,
    CommPolicy_CommModeType mode)
{
    if (ch < COMM_POLICY_CH_NUM)
    {
        CommPolicy_MockState.requestedMode[ch] = mode;
        CommPolicy_MockState.requestComModeCallCount[ch]++;
        return E_OK;
    }
    return E_NOT_OK;
}

Std_ReturnType CommPolicy_Adapter_RequestMode(
    CommPolicy_ModeRequestIdType modeRequestId)
{
    CommPolicy_MockState.lastModeRequestId = modeRequestId;
    CommPolicy_MockState.modeRequestCallCount++;
    return E_OK;
}

void CommPolicy_Adapter_EnterExclusiveArea(void)
{
    CommPolicy_MockState.exclusiveAreaEntered = TRUE;
}

void CommPolicy_Adapter_ExitExclusiveArea(void)
{
    CommPolicy_MockState.exclusiveAreaExited = TRUE;
}

/* --- Snapshot Builder --------------------------------------------------- */

Std_ReturnType CommPolicy_Adapter_BuildInputSnapshot(
    CommPolicy_InputSnapshotType* snapshot)
{
    if (snapshot == NULL_PTR)
    {
        return E_NOT_OK;
    }

    CommPolicy_SourceMask_Copy(&snapshot->activeSourceMask, &CommPolicy_MockState.activeSourceMask);
    return E_OK;
}


