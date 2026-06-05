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

/* --- Source Mask Configuration ------------------------------------------ */
#define CANCOMM_SOURCE_WORD_NUM             2u
#define CANCOMM_SOURCE_BITS_PER_WORD        32u
#define CANCOMM_MAX_SOURCE_NUM              (CANCOMM_SOURCE_WORD_NUM * CANCOMM_SOURCE_BITS_PER_WORD)

/* --- Source Config Table ------------------------------------------------ */
/* STD_OFF: 不生成 SourceConfigTable，项目仅通过 source id 宏和 PolicyRule mask 工作 */
#define CANCOMM_ENABLE_SOURCE_CONFIG_TABLE  STD_OFF

/* --- Source Debug Name -------------------------------------------------- */
/* STD_OFF: 生产代码不引入字符串字面量，避免 ROM 占用 */
#define CANCOMM_ENABLE_SOURCE_DEBUG_NAME    STD_OFF

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
