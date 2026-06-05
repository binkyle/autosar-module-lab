#include "CanCommCoordinator_Adapter.h"
#include "CanCommCoordinator_Cfg.h"
#include "CanCommCoordinator_Types.h"

/*
 * Generic Mock Adapter for Host Unit Testing
 * ------------------------------------------
 * No dependencies on Vector/MICROSAR.
 * All outputs are controllable via test-settable variables.
 */

/* --- Test-Settable Mock State ------------------------------------------- */
typedef struct
{
    CanComm_IgnitionModeType ignition;
    CanComm_PncRequestType pncRequest;
    CanComm_LocalKeepAwakeType localKeepAwake;
    boolean diagKeepAwake;
    boolean otaKeepAwake;

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

    CanComm_MockState.ignition = CANCOMM_IGN_UNKNOWN;
    CanComm_MockState.pncRequest = CANCOMM_PNC_UNKNOWN;
    CanComm_MockState.localKeepAwake = CANCOMM_LOCAL_KEEP_NONE;
    CanComm_MockState.diagKeepAwake = FALSE;
    CanComm_MockState.otaKeepAwake = FALSE;

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        CanComm_MockState.requestedMode[ch] = COMM_NO_COMMUNICATION;
        CanComm_MockState.requestComModeCallCount[ch] = 0u;
    }

    CanComm_MockState.lastBswMRequest = CANCOMM_BSWM_NO_REQUEST;
    CanComm_MockState.bswmRequestCallCount = 0u;
    CanComm_MockState.exclusiveAreaEntered = FALSE;
    CanComm_MockState.exclusiveAreaExited = FALSE;
}

void CanComm_Mock_SetIgnition(CanComm_IgnitionModeType ignition)
{
    CanComm_MockState.ignition = ignition;
}

void CanComm_Mock_SetPncRequest(CanComm_PncRequestType pnc)
{
    CanComm_MockState.pncRequest = pnc;
}

void CanComm_Mock_SetLocalKeepAwake(CanComm_LocalKeepAwakeType keep)
{
    CanComm_MockState.localKeepAwake = keep;
}

void CanComm_Mock_SetDiagKeepAwake(boolean active)
{
    CanComm_MockState.diagKeepAwake = active;
}

void CanComm_Mock_SetOtaKeepAwake(boolean active)
{
    CanComm_MockState.otaKeepAwake = active;
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

/* --- Adapter Implementation (overrides weak stubs) ---------------------- */

Std_ReturnType CanComm_Adapter_GetIgnition(
    CanComm_IgnitionModeType* ignition)
{
    *ignition = CanComm_MockState.ignition;
    return E_OK;
}

Std_ReturnType CanComm_Adapter_GetPncRequest(
    CanComm_PncRequestType* pncRequest)
{
    *pncRequest = CanComm_MockState.pncRequest;
    return E_OK;
}

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

Std_ReturnType CanComm_Adapter_GetLocalKeepAwake(
    CanComm_LocalKeepAwakeType* keepAwake)
{
    *keepAwake = CanComm_MockState.localKeepAwake;
    return E_OK;
}

Std_ReturnType CanComm_Adapter_GetDiagKeepAwake(
    boolean* diagKeepAwake)
{
    *diagKeepAwake = CanComm_MockState.diagKeepAwake;
    return E_OK;
}

Std_ReturnType CanComm_Adapter_GetOtaKeepAwake(
    boolean* otaKeepAwake)
{
    *otaKeepAwake = CanComm_MockState.otaKeepAwake;
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

/* --- Snapshot Builder (override) ---------------------------------------- */

Std_ReturnType CanComm_Adapter_BuildInputSnapshot(
    CanComm_InputSnapshotType* snapshot)
{
    if (snapshot == NULL_PTR)
    {
        return E_NOT_OK;
    }

    snapshot->ignition = CanComm_MockState.ignition;
    snapshot->pncRequest = CanComm_MockState.pncRequest;
    snapshot->holdState = CANCOMM_HOLD_INVALID;
    snapshot->localKeepAwake = CanComm_MockState.localKeepAwake;
    snapshot->diagKeepAwake = CanComm_MockState.diagKeepAwake;
    snapshot->otaKeepAwake = CanComm_MockState.otaKeepAwake;

    return E_OK;
}

/* --- Decision Application (override) ------------------------------------ */

Std_ReturnType CanComm_Adapter_ApplyDecision(
    const CanComm_OutputDecisionType* decision)
{
    CanComm_ChannelIdType ch;

    if (decision == NULL_PTR)
    {
        return E_NOT_OK;
    }

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        CanComm_MockState.requestedMode[ch] = decision->targetComMode[ch];
        CanComm_MockState.requestComModeCallCount[ch]++;
    }

    CanComm_MockState.lastBswMRequest = decision->bswmRequest;
    CanComm_MockState.bswmRequestCallCount++;

    return E_OK;
}
