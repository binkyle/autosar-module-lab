#include "Compiler.h"
#include "CommPolicy_Adapter.h"
#include "CommPolicy_Cfg.h"
#include "CommPolicy_Types.h"
#include "CommPolicy_SourceMask.h"
#include "ComM.h"
#include "BswM.h"
#include "SchM_CommPolicyCoordinator.h"

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

void CommPolicy_Adapter_EnterExclusiveArea(void)
{
    SchM_Enter_CommPolicyCoordinator_CommPolicyExclusiveArea();
}

void CommPolicy_Adapter_ExitExclusiveArea(void)
{
    SchM_Exit_CommPolicyCoordinator_CommPolicyExclusiveArea();
}

/* --- CommMode Mapping (core -> ComM) ------------------------------------ */

STATIC ComM_ModeType CommPolicy_VectorAdapter_MapToComMMode(CommPolicy_CommModeType mode)
{
    switch (mode)
    {
        case COMM_POLICY_NO_COMMUNICATION:      return COMM_NO_COMMUNICATION;
        case COMM_POLICY_SILENT_COMMUNICATION:  return COMM_SILENT_COMMUNICATION;
        case COMM_POLICY_FULL_COMMUNICATION:    return COMM_FULL_COMMUNICATION;
        default:                                return COMM_NO_COMMUNICATION;
    }
}

STATIC CommPolicy_CommModeType CommPolicy_VectorAdapter_MapFromComMMode(ComM_ModeType mode)
{
    switch (mode)
    {
        case COMM_NO_COMMUNICATION:     return COMM_POLICY_NO_COMMUNICATION;
        case COMM_SILENT_COMMUNICATION: return COMM_POLICY_SILENT_COMMUNICATION;
        case COMM_FULL_COMMUNICATION:   return COMM_POLICY_FULL_COMMUNICATION;
        default:                        return COMM_POLICY_NO_COMMUNICATION;
    }
}

/* --- ComM Mode ---------------------------------------------------------- */

extern const CommPolicy_VectorChannelBindingType CommPolicy_VectorChannelBindingTable[COMM_POLICY_CH_NUM];

Std_ReturnType CommPolicy_Adapter_GetComMode(
    CommPolicy_LogicalChannelIdType ch,
    CommPolicy_CommModeType* mode)
{
    ComM_ModeType commMode;
    Std_ReturnType ret;

    if (ch >= COMM_POLICY_CH_NUM)
    {
        return E_NOT_OK;
    }

    ret = ComM_GetCurrentComMode(
        CommPolicy_VectorChannelBindingTable[ch].commUser,
        &commMode);

    if (ret == E_OK)
    {
        *mode = CommPolicy_VectorAdapter_MapFromComMMode(commMode);
    }

    return ret;
}

Std_ReturnType CommPolicy_Adapter_RequestComMode(
    CommPolicy_LogicalChannelIdType ch,
    CommPolicy_CommModeType mode)
{
    ComM_ModeType commMode;

    if (ch >= COMM_POLICY_CH_NUM)
    {
        return E_NOT_OK;
    }

    commMode = CommPolicy_VectorAdapter_MapToComMMode(mode);

    return ComM_RequestComMode(
        CommPolicy_VectorChannelBindingTable[ch].commUser,
        commMode);
}

/* --- Mode Request (mapped to BswM) -------------------------------------- */

Std_ReturnType CommPolicy_Adapter_RequestMode(
    CommPolicy_ModeRequestIdType modeRequestId)
{
    /* Project-specific mapping from generic modeRequestId to BswM_RequestMode.
     * See CommPolicy_VectorCfg.c for mapping table definition.
     * TODO: replace hard-coded values with generated symbols. */
    (void)modeRequestId;
    /* BswM_RequestMode(user, mode); */
    return E_OK;
}

/* --- Snapshot Builder (Vector project-specific source mapping) ---------- */

Std_ReturnType CommPolicy_Adapter_BuildInputSnapshot(
    CommPolicy_InputSnapshotType* snapshot)
{
    Std_ReturnType ret = E_OK;
    /* TODO: replace with actual RTE reads or BSW state queries per project */
    boolean powerRun = FALSE;
    boolean networkGlobal = FALSE;
    boolean networkPartial = FALSE;
    boolean diagKeep = FALSE;
    boolean otaKeep = FALSE;
    boolean localKeep = FALSE;

    if (snapshot == NULL_PTR)
    {
        return E_NOT_OK;
    }

    CommPolicy_SourceMask_Clear(&snapshot->activeSourceMask);

    /* TODO: Rte_Read_xxx_PowerRun(&powerRun) */
    if (powerRun == TRUE)
    {
        (void)CommPolicy_SourceMask_Set(&snapshot->activeSourceMask, VECTOR_SRC_ID_IGNITION);
    }

    /* TODO: read network state from CanNm/ComM callbacks or RTE */
    if (networkGlobal == TRUE)
    {
        (void)CommPolicy_SourceMask_Set(&snapshot->activeSourceMask, VECTOR_SRC_ID_PNC_GLOBAL);
    }
    if (networkPartial == TRUE)
    {
        (void)CommPolicy_SourceMask_Set(&snapshot->activeSourceMask, VECTOR_SRC_ID_PNC_ADAS);
    }

    /* TODO: check diagnostic session active */
    if (diagKeep == TRUE)
    {
        (void)CommPolicy_SourceMask_Set(&snapshot->activeSourceMask, VECTOR_SRC_ID_DIAG_KEEP);
    }

    /* TODO: check OTA in progress */
    if (otaKeep == TRUE)
    {
        (void)CommPolicy_SourceMask_Set(&snapshot->activeSourceMask, VECTOR_SRC_ID_OTA_KEEP);
    }

    /* TODO: check local keep-awake sources */
    if (localKeep == TRUE)
    {
        (void)CommPolicy_SourceMask_Set(&snapshot->activeSourceMask, VECTOR_SRC_ID_LOCAL_KEEP);
    }

    return ret;
}
