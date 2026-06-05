#include "CanCommCoordinator_Adapter.h"
#include "CanCommCoordinator_Cfg.h"
#include "CanCommCoordinator_Types.h"
#include "ComM.h"
#include "Nm.h"
#include "BswM.h"
#include "SchM_CanCommCoordinator.h"

/*
 * Vector / MICROSAR Project Binding Layer
 * ---------------------------------------
 * This file contains the ONLY places where Vector/DaVinci generated symbols
 * and standard BSW APIs are directly called.
 *
 * Rules:
 *   - Do NOT call ComM_RequestComMode from Policy or core logic.
 *   - Do NOT call Nm_Enable/DisableCommunication from core logic.
 *   - Do NOT call Com_IpduGroupStart/Stop from core logic (migrate to BswM).
 */

/* --- Exclusive Area (Vector SchM) --------------------------------------- */

void CanComm_Adapter_EnterExclusiveArea(void)
{
    SchM_Enter_CanCommCoordinator_CanCommExclusiveArea();
}

void CanComm_Adapter_ExitExclusiveArea(void)
{
    SchM_Exit_CanCommCoordinator_CanCommExclusiveArea();
}

/* --- Ignition (example: read from RTE or IoHwAb) ------------------------ */

Std_ReturnType CanComm_Adapter_GetIgnition(
    CanComm_IgnitionModeType* ignition)
{
    /* TODO: replace with Rte_Read_xxx or IoHwAb_DigIo_xxx per project */
    *ignition = CANCOMM_IGN_OFF;
    return E_OK;
}

/* --- PNC Request (should eventually come from Nm/ComM PNC state) -------- */

Std_ReturnType CanComm_Adapter_GetPncRequest(
    CanComm_PncRequestType* pncRequest)
{
    /*
     * In a pure-BSW design, this would not exist.
     * The module receives PNC state via CanNm/ComM callbacks.
     * This stub remains for transitional projects.
     */
    *pncRequest = CANCOMM_PNC_NONE;
    return E_OK;
}

/* --- ComM Mode ---------------------------------------------------------- */

Std_ReturnType CanComm_Adapter_GetComMode(
    CanComm_ChannelIdType ch,
    ComM_ModeType* mode)
{
    extern const CanComm_ChannelConfigType CanComm_ChannelConfigTable[CANCOMM_CH_NUM];

    if (ch >= CANCOMM_CH_NUM)
    {
        return E_NOT_OK;
    }

    return ComM_GetCurrentComMode(
        CanComm_ChannelConfigTable[ch].commUser,
        mode);
}

Std_ReturnType CanComm_Adapter_RequestComMode(
    CanComm_ChannelIdType ch,
    ComM_ModeType mode)
{
    extern const CanComm_ChannelConfigType CanComm_ChannelConfigTable[CANCOMM_CH_NUM];
    Std_ReturnType ret;

    if (ch >= CANCOMM_CH_NUM)
    {
        return E_NOT_OK;
    }

    ret = ComM_RequestComMode(
        CanComm_ChannelConfigTable[ch].commUser,
        mode);

    return ret;
}

/* --- BswM Request ------------------------------------------------------- */

Std_ReturnType CanComm_Adapter_RequestBswMMode(
    CanComm_BswMRequestType mode)
{
#if (CANCOMM_USE_BSWM_ACTION == STD_ON)
    /* Map internal request to BswM mode request. Project-specific. */
    switch (mode)
    {
        case CANCOMM_BSWM_REQUEST_FULL:
            BswM_RequestMode(0u, 0u); /* TODO: map to real mode request IDs */
            break;
        case CANCOMM_BSWM_REQUEST_NO:
            BswM_RequestMode(0u, 1u);
            break;
        case CANCOMM_BSWM_REQUEST_SLEEP_CANDIDATE:
            BswM_RequestMode(0u, 2u);
            break;
        default:
            break;
    }
#endif
    return E_OK;
}

/* --- Local Keep Awake (example: Diag/OTA/Run-app flags) ----------------- */

Std_ReturnType CanComm_Adapter_GetLocalKeepAwake(
    CanComm_LocalKeepAwakeType* keepAwake)
{
    /* TODO: aggregate local keep-awake sources (Diag session, OTA active, etc.) */
    *keepAwake = CANCOMM_LOCAL_KEEP_NONE;
    return E_OK;
}

Std_ReturnType CanComm_Adapter_GetDiagKeepAwake(
    boolean* diagKeepAwake)
{
    /* TODO: check if diagnostic session is active */
    *diagKeepAwake = FALSE;
    return E_OK;
}

Std_ReturnType CanComm_Adapter_GetOtaKeepAwake(
    boolean* otaKeepAwake)
{
    /* TODO: check if OTA is in progress */
    *otaKeepAwake = FALSE;
    return E_OK;
}
