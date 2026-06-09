#ifndef COMMPOLICY_TYPES_H
#define COMMPOLICY_TYPES_H

#include "Std_Types.h"
#include "CommPolicy_Cfg.h"
#include "ComM.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- Source Abstraction ------------------------------------------------- */
/*
 * Core framework does NOT define concrete source names (IGNITION, PNC_ADAS, etc.).
 * Source IDs are project-specific and defined in project configuration layers.
 * The framework only provides the mask mechanism and tool functions.
 */

typedef uint16 CommPolicy_SourceIdType;

typedef struct
{
    uint32 word[COMM_POLICY_SOURCE_WORD_NUM];
} CommPolicy_SourceMaskType;

/* --- Source Config (optional, controlled by COMM_POLICY_ENABLE_SOURCE_CONFIG_TABLE) */

#if (COMM_POLICY_ENABLE_SOURCE_CONFIG_TABLE == STD_ON)

typedef uint8 CommPolicy_SourceCategoryType;

#if (COMM_POLICY_ENABLE_SOURCE_DEBUG_NAME == STD_ON)
typedef struct
{
    CommPolicy_SourceIdType sourceId;
    CommPolicy_SourceCategoryType category;
    const char* debugName;
} CommPolicy_SourceConfigType;
#else
typedef struct
{
    CommPolicy_SourceIdType sourceId;
    CommPolicy_SourceCategoryType category;
} CommPolicy_SourceConfigType;
#endif

#endif /* COMM_POLICY_ENABLE_SOURCE_CONFIG_TABLE */

/* --- Tx Gate ------------------------------------------------------------ */
typedef enum
{
    COMM_POLICY_TX_BLOCK = 0,
    COMM_POLICY_TX_UNBLOCK
} CommPolicy_AppTxGateType;

/* --- Mode Request ID (generic, project-defined) ------------------------- */
typedef uint16 CommPolicy_ModeRequestIdType;

#define COMM_POLICY_MODE_REQUEST_NONE       0xFFFFu

/* --- Logical Channel ID ------------------------------------------------- */
typedef uint16 CommPolicy_LogicalChannelIdType;

/* --- Channel Mask ------------------------------------------------------- */
typedef struct
{
    uint32 word[COMM_POLICY_CHANNEL_WORD_NUM];
} CommPolicy_ChannelMaskType;

/* --- Bus Type ----------------------------------------------------------- */
typedef enum
{
    COMM_POLICY_BUS_UNKNOWN = 0,
    COMM_POLICY_BUS_CAN,
    COMM_POLICY_BUS_LIN,
    COMM_POLICY_BUS_ETH,
    COMM_POLICY_BUS_FR,
    COMM_POLICY_BUS_USER
} CommPolicy_BusType;

/* --- Input Snapshot ----------------------------------------------------- */
/*
 * All inputs are abstracted as a set of active sources.
 * Concrete source semantics (ignition, PNC, diag, OTA, etc.)
 * are defined in project configuration, NOT in core types.
 */
typedef struct
{
    CommPolicy_SourceMaskType activeSourceMask;
} CommPolicy_InputSnapshotType;

/* --- Output Decision ---------------------------------------------------- */
typedef struct
{
    ComM_ModeType targetComMode[COMM_POLICY_CH_NUM];
    CommPolicy_AppTxGateType targetTxGate[COMM_POLICY_CH_NUM];
    CommPolicy_ModeRequestIdType modeRequestId;
    boolean sleepCandidate;
} CommPolicy_OutputDecisionType;

/* --- Channel Configuration ---------------------------------------------- */
typedef struct
{
    CommPolicy_LogicalChannelIdType logicalChannel;
    CommPolicy_BusType busType;
    ComM_UserHandleType commUser;
    NetworkHandleType networkHandle;
    CommPolicy_ChannelMaskType channelMask;   /* Mask with single bit identifying this channel */
    CommPolicy_ChannelMaskType groupMask;     /* Mask for channel grouping */
} CommPolicy_ChannelConfigType;

/* --- Output Action Set -------------------------------------------------- */
/*
 * Describes a policy output in a channel-agnostic way using channel masks.
 * The Policy engine translates masks into per-channel OutputDecision.
 */
typedef struct
{
    CommPolicy_ChannelMaskType fullComMask;       /* Channels to set FULL_COMMUNICATION */
    CommPolicy_ChannelMaskType noComMask;         /* Channels to set NO_COMMUNICATION */
    CommPolicy_ChannelMaskType txUnblockMask;     /* Channels to set TX_UNBLOCK */
    CommPolicy_ChannelMaskType txBlockMask;       /* Channels to set TX_BLOCK */
    CommPolicy_ModeRequestIdType modeRequestId;
    boolean sleepCandidate;
} CommPolicy_OutputActionSetType;

/* --- Source Timer Configuration ----------------------------------------- */
/*
 * Maps trigger sources to hold sources with a shared hold timer.
 * When triggerSourceId becomes active, the State layer sets holdSourceId
 * and reloads the timer. When the timer expires, all holdSourceIds are cleared.
 */
typedef struct
{
    CommPolicy_SourceIdType triggerSourceId;  /* Source that triggers the hold timer */
    CommPolicy_SourceIdType holdSourceId;     /* Source set while timer is running */
    uint16 holdTicks;                         /* Timer duration in MainFunction ticks */
} CommPolicy_SourceTimerConfigType;

/* --- Policy Rule -------------------------------------------------------- */
/*
 * Generic rule matching based on source masks and priority.
 * If multiple rules match, the one with highest priority wins.
 * If priority is equal, the first match in table order wins.
 */
typedef struct
{
    uint16 priority;
    CommPolicy_SourceMaskType requiredAllMask;   /* Active must contain ALL of these */
    CommPolicy_SourceMaskType requiredAnyMask;   /* Active must contain AT LEAST ONE of these (or mask is empty) */
    CommPolicy_SourceMaskType forbiddenMask;     /* Active must contain NONE of these */
    const CommPolicy_OutputActionSetType* actionSet;
} CommPolicy_PolicyRuleType;

#ifdef __cplusplus
}
#endif

#endif /* COMMPOLICY_TYPES_H */
