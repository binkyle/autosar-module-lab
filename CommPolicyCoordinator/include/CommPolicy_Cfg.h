#ifndef COMMPOLICY_CFG_H
#define COMMPOLICY_CFG_H

#include "Std_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- General Configuration ---------------------------------------------- */
#define COMM_POLICY_MAIN_PERIOD_MS                  5u

/* --- Source Mask Configuration ------------------------------------------ */
#define COMM_POLICY_SOURCE_WORD_NUM             2u
#define COMM_POLICY_SOURCE_BITS_PER_WORD        32u
#define COMM_POLICY_MAX_SOURCE_NUM              (COMM_POLICY_SOURCE_WORD_NUM * COMM_POLICY_SOURCE_BITS_PER_WORD)

/* --- Channel Mask Configuration ----------------------------------------- */
#define COMM_POLICY_CHANNEL_WORD_NUM            2u
#define COMM_POLICY_CHANNEL_BITS_PER_WORD       32u
#define COMM_POLICY_MAX_CHANNEL_NUM             (COMM_POLICY_CHANNEL_WORD_NUM * COMM_POLICY_CHANNEL_BITS_PER_WORD)

/* --- Source Config Table ------------------------------------------------ */
/* STD_OFF: do not generate SourceConfigTable; project works via source id macros and PolicyRule masks only */
#define COMM_POLICY_ENABLE_SOURCE_CONFIG_TABLE  STD_OFF

/* --- Source Debug Name -------------------------------------------------- */
/* STD_OFF: production code does not introduce string literals to avoid ROM usage */
#define COMM_POLICY_ENABLE_SOURCE_DEBUG_NAME    STD_OFF

/* --- Channel Count ------------------------------------------------------ */
#define COMM_POLICY_CH_NUM                          6u

/* --- Fallback Strategy -------------------------------------------------- */
#define COMM_POLICY_FALLBACK_ALL_FULL               0u
#define COMM_POLICY_FALLBACK_ALL_NO                 1u
#define COMM_POLICY_FALLBACK_STRATEGY               COMM_POLICY_FALLBACK_ALL_FULL

/* --- Version ------------------------------------------------------------ */
#define COMM_POLICY_SW_MAJOR_VERSION                2u
#define COMM_POLICY_SW_MINOR_VERSION                0u
#define COMM_POLICY_SW_PATCH_VERSION                0u

#ifdef __cplusplus
}
#endif

#endif /* COMMPOLICY_CFG_H */
