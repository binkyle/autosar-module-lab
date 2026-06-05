#include "CanCommCoordinator_Adapter.h"
#include "CanCommCoordinator_Cfg.h"
#include "CanCommCoordinator_Types.h"
#include "CanCommCoordinator_SourceMask.h"

/*
 * Generic Mock Adapter for Host Unit Testing
 * ------------------------------------------
 * No dependencies on Vector/MICROSAR.
 * All outputs are controllable via test-settable variables.
 */

/* --- Test-Settable Mock State ------------------------------------------- */
typedef struct
{
    CanComm_SourceMaskType activeSourceMask;

    ComM_ModeType requestedMode[CANCOMM_CH_NUM];
    uint8 requestComModeCallCount[CANCOMM_CH_NUM];
    CanComm_BswMRequestType lastBswMRequest;
    uint8 bswmRequestCallCount;

    boolean exclusiveAreaEntered;
    boolean exclusiveAreaExited;
} CanComm_MockStateType;

static CanComm_MockStateType CanComm_MockState;

/* --- Mock Control API --------------------------------------------------- */

void CanComm_Mock_Reset(void)
{
    CanComm_ChannelIdType ch;

    CanComm_SourceMask_Clear(&CanComm_MockState.activeSourceMask);

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        CanComm_MockState.requestedMode[ch] = COMM_NO_COMMUNICATION;
        CanComm_MockState.requestComModeCallCount[ch] = 0u;
    }

    CanComm_MockState.lastBswMRequest = CANCOMM_BSWM_NO_REQUEST;
    CanComm_MockState.bswmRequestCallCount = 0u;
    CanComm_MockState.exclusiveAreaEntered = FALSE;
    CanComm_MockState.exclusiveAreaExited = FALSE;

    CanComm_Adapter_Reset();
}

void CanComm_Mock_SetSourceActive(CanComm_SourceIdType sourceId)
{
    (void)CanComm_SourceMask_Set(&CanComm_MockState.activeSourceMask, sourceId);
}

void CanComm_Mock_SetSourceInactive(CanComm_SourceIdType sourceId)
{
    (void)CanComm_SourceMask_Reset(&CanComm_MockState.activeSourceMask, sourceId);
}

void CanComm_Mock_ClearAllSources(void)
{
    CanComm_SourceMask_Clear(&CanComm_MockState.activeSourceMask);
}

const CanComm_SourceMaskType* CanComm_Mock_GetActiveSourceMask(void)
{
    return &CanComm_MockState.activeSourceMask;
}

ComM_ModeType CanComm_Mock_GetRequestedMode(CanComm_ChannelIdType ch)
{
    if (ch < CANCOMM_CH_NUM)
    {
        return CanComm_MockState.requestedMode[ch];
    }
    return COMM_NO_COMMUNICATION;
}

uint8 CanComm_Mock_GetRequestComModeCallCount(CanComm_ChannelIdType ch)
{
    if (ch < CANCOMM_CH_NUM)
    {
        return CanComm_MockState.requestComModeCallCount[ch];
    }
    return 0u;
}

CanComm_BswMRequestType CanComm_Mock_GetLastBswMRequest(void)
{
    return CanComm_MockState.lastBswMRequest;
}

uint8 CanComm_Mock_GetBswMRequestCallCount(void)
{
    return CanComm_MockState.bswmRequestCallCount;
}

boolean CanComm_Mock_GetExclusiveAreaEntered(void)
{
    return CanComm_MockState.exclusiveAreaEntered;
}

boolean CanComm_Mock_GetExclusiveAreaExited(void)
{
    return CanComm_MockState.exclusiveAreaExited;
}

/* --- Adapter Implementation --------------------------------------------- */

Std_ReturnType CanComm_Adapter_GetComMode(
    CanComm_ChannelIdType ch,
    ComM_ModeType* mode)
{
    if (ch < CANCOMM_CH_NUM)
    {
        *mode = CanComm_MockState.requestedMode[ch];
        return E_OK;
    }
    return E_NOT_OK;
}

Std_ReturnType CanComm_Adapter_RequestComMode(
    CanComm_ChannelIdType ch,
    ComM_ModeType mode)
{
    if (ch < CANCOMM_CH_NUM)
    {
        CanComm_MockState.requestedMode[ch] = mode;
        CanComm_MockState.requestComModeCallCount[ch]++;
        return E_OK;
    }
    return E_NOT_OK;
}

Std_ReturnType CanComm_Adapter_RequestBswMMode(
    CanComm_BswMRequestType mode)
{
    CanComm_MockState.lastBswMRequest = mode;
    CanComm_MockState.bswmRequestCallCount++;
    return E_OK;
}

void CanComm_Adapter_EnterExclusiveArea(void)
{
    CanComm_MockState.exclusiveAreaEntered = TRUE;
}

void CanComm_Adapter_ExitExclusiveArea(void)
{
    CanComm_MockState.exclusiveAreaExited = TRUE;
}

/* --- Snapshot Builder --------------------------------------------------- */

Std_ReturnType CanComm_Adapter_BuildInputSnapshot(
    CanComm_InputSnapshotType* snapshot)
{
    if (snapshot == NULL_PTR)
    {
        return E_NOT_OK;
    }

    CanComm_SourceMask_Copy(&snapshot->activeSourceMask, &CanComm_MockState.activeSourceMask);
    return E_OK;
}


