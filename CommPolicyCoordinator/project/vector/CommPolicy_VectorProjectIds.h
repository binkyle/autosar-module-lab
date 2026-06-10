#ifndef COMMPOLICY_VECTORPROJECTIDS_H
#define COMMPOLICY_VECTORPROJECTIDS_H

/*
 * Vector Project-Specific Source IDs and Mode Request IDs
 * --------------------------------------------------------
 * This header is part of the Vector/MICROSAR binding layer.
 * Do NOT include from core include/src or project/generic.
 *
 * These IDs must be synchronized with DaVinci Configurator definitions.
 */

/* --- Source IDs (must match DaVinci Configurator source definitions) ------ */
#define VECTOR_SRC_ID_IGNITION       0u
#define VECTOR_SRC_ID_PNC_GLOBAL     1u
#define VECTOR_SRC_ID_PNC_ADAS       2u
#define VECTOR_SRC_ID_DIAG_KEEP      3u
#define VECTOR_SRC_ID_OTA_KEEP       4u
#define VECTOR_SRC_ID_LOCAL_KEEP     5u
#define VECTOR_SRC_ID_HOLD_VALID     6u

/* --- Mode Request IDs --------------------------------------------------- */
#define VECTOR_MODE_REQ_ALL_FULL     0u
#define VECTOR_MODE_REQ_ADAS_ONLY    1u
#define VECTOR_MODE_REQ_ALL_NO       2u
#define VECTOR_MODE_REQ_SLEEP        3u

#endif /* COMMPOLICY_VECTORPROJECTIDS_H */
