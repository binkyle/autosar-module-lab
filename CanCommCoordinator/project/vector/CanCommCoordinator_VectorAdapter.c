#include "CanCommCoordinator_Adapter.h"
#include "CanCommCoordinator_Cfg.h"
#include "CanCommCoordinator_Types.h"
#include "CanCommCoordinator_SourceMask.h"
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

/* --- Project-specific source ID mapping (Vector project) ---------------- */
/* TODO: synchronize with DaVinci Configurator source definitions */
#define VECTOR_SRC_ID_IGNITION       0u
#define VECTOR_SRC_ID_PNC_GLOBAL     1u
#define VECTOR_SRC_ID_PNC_ADAS       2u
#define VECTOR_SRC_ID_DIAG_KEEP      3u
#define VECTOR_SRC_ID_OTA_KEEP       4u
#define VECTOR_SRC_ID_LOCAL_KEEP     5u
#define VECTOR_SRC_ID_HOLD_VALID     6u

/* --- Exclusive Area (Vector SchM) --------------------------------------- */

void CanComm_Adapter_EnterExclusiveArea(void)
{
    SchM_Enter_CanCommCoordinator_CanCommExclusiveArea();
}

void CanComm_Adapter_ExitExclusiveArea(void)
{
    SchM_Exit_CanCommCoordinator_CanCommExclusiveArea();
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

/* --- Snapshot Builder (Vector project-specific source mapping) ---------- */

Std_ReturnType CanComm_Adapter_BuildInputSnapshot(
    CanComm_InputSnapshotType* snapshot)
{
    Std_ReturnType ret = E_OK;
    /* TODO: replace with actual RTE reads or BSW state queries per project */
    boolean ignitionOn = FALSE;
    boolean pncGlobal = FALSE;
    boolean pncAdas = FALSE;
    boolean diagKeep = FALSE;
    boolean otaKeep = FALSE;
    boolean localKeep = FALSE;

    if (snapshot == NULL_PTR)
    {
        return E_NOT_OK;
    }

    CanComm_SourceMask_Clear(&snapshot->activeSourceMask);

    /* TODO: Rte_Read_xxx_Ignition(&ignitionOn) */
    if (ignitionOn == TRUE)
    {
        (void)CanComm_SourceMask_Set(&snapshot->activeSourceMask, VECTOR_SRC_ID_IGNITION);
    }

    /* TODO: read PNC state from CanNm/ComM callbacks or RTE */
    if (pncGlobal == TRUE)
    {
        (void)CanComm_SourceMask_Set(&snapshot->activeSourceMask, VECTOR_SRC_ID_PNC_GLOBAL);
    }
    if (pncAdas == TRUE)
    {
        (void)CanComm_SourceMask_Set(&snapshot->activeSourceMask, VECTOR_SRC_ID_PNC_ADAS);
    }

    /* TODO: check diagnostic session active */
    if (diagKeep == TRUE)
    {
        (void)CanComm_SourceMask_Set(&snapshot->activeSourceMask, VECTOR_SRC_ID_DIAG_KEEP);
    }

    /* TODO: check OTA in progress */
    if (otaKeep == TRUE)
    {
        (void)CanComm_SourceMask_Set(&snapshot->activeSourceMask, VECTOR_SRC_ID_OTA_KEEP);
    }

    /* TODO: check local keep-awake sources */
    if (localKeep == TRUE)
    {
        (void)CanComm_SourceMask_Set(&snapshot->activeSourceMask, VECTOR_SRC_ID_LOCAL_KEEP);
    }

    return ret;
}
