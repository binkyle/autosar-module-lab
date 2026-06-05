#ifndef CANCOMMCOORDINATOR_TYPES_H
#define CANCOMMCOORDINATOR_TYPES_H

#include "Std_Types.h"
#include "CanCommCoordinator_Cfg.h"
#include "ComM.h"
#include "Nm.h"
#include "PduR.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- Source Abstraction ------------------------------------------------- */
/*
 * Core framework does NOT define concrete source names (IGNITION, PNC_ADAS, etc.).
 * Source IDs are project-specific and defined in project configuration layers.
 * The framework only provides the mask mechanism and tool functions.
 */

typedef uint16 CanComm_SourceIdType;

typedef struct
{
    uint32 word[CANCOMM_SOURCE_WORD_NUM];
} CanComm_SourceMaskType;

/* --- Source Config (optional, controlled by CANCOMM_ENABLE_SOURCE_CONFIG_TABLE) */

#if (CANCOMM_ENABLE_SOURCE_CONFIG_TABLE == STD_ON)

typedef uint8 CanComm_SourceCategoryType;

#if (CANCOMM_ENABLE_SOURCE_DEBUG_NAME == STD_ON)
typedef struct
{
    CanComm_SourceIdType sourceId;
    CanComm_SourceCategoryType category;
    const char* debugName;
} CanComm_SourceConfigType;
#else
typedef struct
{
    CanComm_SourceIdType sourceId;
    CanComm_SourceCategoryType category;
} CanComm_SourceConfigType;
#endif

#endif /* CANCOMM_ENABLE_SOURCE_CONFIG_TABLE */

/* --- Tx Gate ------------------------------------------------------------ */
typedef enum
{
    CANCOMM_TX_BLOCK = 0,
    CANCOMM_TX_UNBLOCK
} CanComm_AppTxGateType;

/* --- BswM Request ------------------------------------------------------- */
typedef enum
{
    CANCOMM_BSWM_NO_REQUEST = 0,
    CANCOMM_BSWM_REQUEST_FULL,
    CANCOMM_BSWM_REQUEST_NO,
    CANCOMM_BSWM_REQUEST_SLEEP_CANDIDATE
} CanComm_BswMRequestType;

/* --- Channel ID (logical, project-agnostic) ----------------------------- */
typedef uint8 CanComm_ChannelIdType;

/* --- Input Snapshot ----------------------------------------------------- */
/*
 * All inputs are abstracted as a set of active sources.
 * Concrete source semantics (ignition, PNC, diag, OTA, etc.)
 * are defined in project configuration, NOT in core types.
 */
typedef struct
{
    CanComm_SourceMaskType activeSourceMask;
} CanComm_InputSnapshotType;

/* --- Output Decision ---------------------------------------------------- */
typedef struct
{
    ComM_ModeType targetComMode[CANCOMM_CH_NUM];
    CanComm_AppTxGateType targetTxGate[CANCOMM_CH_NUM];
    CanComm_BswMRequestType bswmRequest;
    boolean sleepCandidate;
} CanComm_OutputDecisionType;

/* --- Channel Mask Constants --------------------------------------------- */
#define CANCOMM_MASK_ADAS1      0x01u
#define CANCOMM_MASK_ADAS2      0x02u
#define CANCOMM_MASK_ADAS_SUB   0x04u
#define CANCOMM_MASK_PDC        0x08u
#define CANCOMM_MASK_CAN5       0x10u
#define CANCOMM_MASK_CAN9       0x20u
#define CANCOMM_MASK_ALL        0x3Fu

/* --- Channel Configuration ---------------------------------------------- */
typedef struct
{
    CanComm_ChannelIdType logicalCh;
    ComM_UserHandleType commUser;
    NetworkHandleType nmChannel;
    PduIdType nmRxPduId;
    uint32 channelMask;   /* Bitmask identifying this channel (e.g. CANCOMM_MASK_ADAS1) */
    uint32 groupMask;     /* Bitmask for channel grouping (e.g. ADAS group, safety group) */
} CanComm_ChannelConfigType;

/* --- Output Action Set -------------------------------------------------- */
/*
 * Describes a policy output in a channel-agnostic way using bit masks.
 * The Policy engine translates masks into per-channel OutputDecision.
 */
typedef struct
{
    uint32 fullComMask;       /* Channels to set FULL_COMMUNICATION */
    uint32 noComMask;         /* Channels to set NO_COMMUNICATION */
    uint32 txUnblockMask;     /* Channels to set TX_UNBLOCK */
    uint32 txBlockMask;       /* Channels to set TX_BLOCK */
    CanComm_BswMRequestType bswmRequest;
    boolean sleepCandidate;
} CanComm_OutputActionSetType;

/* --- Policy Rule -------------------------------------------------------- */
/*
 * Generic rule matching based on source masks and priority.
 * If multiple rules match, the one with highest priority wins.
 * If priority is equal, the first match in table order wins.
 */
typedef struct
{
    uint16 priority;
    CanComm_SourceMaskType requiredAllMask;   /* Active must contain ALL of these */
    CanComm_SourceMaskType requiredAnyMask;   /* Active must contain AT LEAST ONE of these (or mask is empty) */
    CanComm_SourceMaskType forbiddenMask;     /* Active must contain NONE of these */
    const CanComm_OutputActionSetType* actionSet;
} CanComm_PolicyRuleType;

/* --- Legacy NM Parse Config --------------------------------------------- */
typedef struct
{
    uint8 pniByteOffset;
    uint8 pniBitMask;
    uint8 pncGlobalByteOffset;
    uint8 pncGlobalBitMask;
    uint8 pncAdasByteOffset;
    uint8 pncAdasBitMask;
} CanComm_LegacyNmParseConfigType;

#ifdef __cplusplus
}
#endif

#endif /* CANCOMMCOORDINATOR_TYPES_H */
