#ifndef CANCOMMCOORDINATOR_CFG_H
#define CANCOMMCOORDINATOR_CFG_H

#include "Std_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- General Configuration ---------------------------------------------- */
#define CANCOMM_MAIN_PERIOD_MS                  5u
#define CANCOMM_PNC_HOLD_TIME_MS                750u
#define CANCOMM_PNC_HOLD_TICKS                  (CANCOMM_PNC_HOLD_TIME_MS / CANCOMM_MAIN_PERIOD_MS)

/* --- Channel Count ------------------------------------------------------ */
#define CANCOMM_CH_NUM                          6u

#define CANCOMM_CH_ADAS1                        0u
#define CANCOMM_CH_ADAS2                        1u
#define CANCOMM_CH_ADAS_SUB                     2u
#define CANCOMM_CH_PDC                          3u
#define CANCOMM_CH_CAN5                         4u
#define CANCOMM_CH_CAN9                         5u
/* CAN10 etc. can be added here without changing core logic */

/* --- Legacy Compatibility ----------------------------------------------- */
/* Default OFF: legacy NM parse is a migration-only feature.
 * In standard mode, PNC handling should be done by CanNm/Nm/ComM/BswM. */
#define CANCOMM_ENABLE_LEGACY_NM_PARSE          STD_OFF

/* --- Fallback Strategy -------------------------------------------------- */
#define CANCOMM_FALLBACK_STRATEGY               CANCOMM_FALLBACK_ALL_FULL

/* --- BswM Integration --------------------------------------------------- */
#define CANCOMM_USE_BSWM_ACTION                 STD_ON

/* --- Version ------------------------------------------------------------ */
#define CANCOMM_SW_MAJOR_VERSION                1u
#define CANCOMM_SW_MINOR_VERSION                0u
#define CANCOMM_SW_PATCH_VERSION                0u

#ifdef __cplusplus
}
#endif

#endif /* CANCOMMCOORDINATOR_CFG_H */
