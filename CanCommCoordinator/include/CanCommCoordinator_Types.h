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

/* --- Ignition Mode ------------------------------------------------------ */
typedef enum
{
    CANCOMM_IGN_OFF = 0,
    CANCOMM_IGN_ON,
    CANCOMM_IGN_UNKNOWN
} CanComm_IgnitionModeType;

/* --- PNC Request -------------------------------------------------------- */
typedef enum
{
    CANCOMM_PNC_NONE = 0,
    CANCOMM_PNC_GLOBAL,
    CANCOMM_PNC_ADAS_ONLY,
    CANCOMM_PNC_INVALID,
    CANCOMM_PNC_UNKNOWN
} CanComm_PncRequestType;

/* --- Hold State --------------------------------------------------------- */
typedef enum
{
    CANCOMM_HOLD_INVALID = 0,
    CANCOMM_HOLD_VALID
} CanComm_HoldStateType;

/* --- Local Keep Awake --------------------------------------------------- */
typedef enum
{
    CANCOMM_LOCAL_KEEP_NONE = 0,
    CANCOMM_LOCAL_KEEP_ACTIVE
} CanComm_LocalKeepAwakeType;

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

/* --- Fallback Strategy -------------------------------------------------- */
typedef enum
{
    CANCOMM_FALLBACK_ALL_FULL = 0,
    CANCOMM_FALLBACK_ALL_NO,
    CANCOMM_FALLBACK_KEEP_LAST
} CanComm_FallbackStrategyType;

/* --- Channel ID (logical, project-agnostic) ----------------------------- */
typedef uint8 CanComm_ChannelIdType;

/* --- Input Snapshot ----------------------------------------------------- */
typedef struct
{
    CanComm_IgnitionModeType ignition;
    CanComm_PncRequestType pncRequest;
    CanComm_HoldStateType holdState;
    CanComm_LocalKeepAwakeType localKeepAwake;
    boolean diagKeepAwake;
    boolean otaKeepAwake;
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
typedef struct
{
    CanComm_IgnitionModeType ignition;
    CanComm_PncRequestType pncRequest;
    CanComm_HoldStateType holdState;
    CanComm_LocalKeepAwakeType localKeepAwake;
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
