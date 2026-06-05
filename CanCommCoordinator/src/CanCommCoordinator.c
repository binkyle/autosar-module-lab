#include "CanCommCoordinator.h"
#include "CanCommCoordinator_Cfg.h"
#include "CanCommCoordinator_Types.h"
#include "CanCommCoordinator_App.h"
#include "CanCommCoordinator_State.h"
#include "CanCommCoordinator_Cfg.h"

#if (CANCOMM_ENABLE_LEGACY_NM_PARSE == STD_ON)
#include "CanCommCoordinator_Adapter.h"
#endif

/* --- External Config ---------------------------------------------------- */
extern const CanComm_ChannelConfigType CanComm_ChannelConfigTable[CANCOMM_CH_NUM];
extern const CanComm_LegacyNmParseConfigType CanComm_LegacyNmParseConfig;

/* --- Public API --------------------------------------------------------- */

void CanCommCoordinator_Init(void)
{
    CanCommCoordinator_App_Init();
}

void CanCommCoordinator_MainFunction(void)
{
    CanCommCoordinator_App_MainFunction();
}

#if (CANCOMM_ENABLE_LEGACY_NM_PARSE == STD_ON)
void CanCommCoordinator_OnNmRxIndication(
    PduIdType rxPduId,
    const PduInfoType* pduInfo)
{
    boolean pni = FALSE;
    boolean pncGlobal = FALSE;
    boolean pncAdas = FALSE;
    CanComm_PncRequestType pncRequest = CANCOMM_PNC_NONE;
    uint8 pniByte;
    uint8 pncGlobalByte;
    uint8 pncAdasByte;

    (void)rxPduId; /* rxPduId can be used to validate expected NM PDU */

    if ((pduInfo == NULL_PTR) || (pduInfo->SduDataPtr == NULL_PTR))
    {
        return;
    }

    /* Parse legacy NM bytes according to Cfg offsets/masks */
    if (pduInfo->SduLength > CanComm_LegacyNmParseConfig.pniByteOffset)
    {
        pniByte = pduInfo->SduDataPtr[CanComm_LegacyNmParseConfig.pniByteOffset];
        pni = ((pniByte & CanComm_LegacyNmParseConfig.pniBitMask) != 0u) ? TRUE : FALSE;
    }

    if (pduInfo->SduLength > CanComm_LegacyNmParseConfig.pncGlobalByteOffset)
    {
        pncGlobalByte = pduInfo->SduDataPtr[CanComm_LegacyNmParseConfig.pncGlobalByteOffset];
        pncGlobal = ((pncGlobalByte & CanComm_LegacyNmParseConfig.pncGlobalBitMask) != 0u) ? TRUE : FALSE;
    }

    if (pduInfo->SduLength > CanComm_LegacyNmParseConfig.pncAdasByteOffset)
    {
        pncAdasByte = pduInfo->SduDataPtr[CanComm_LegacyNmParseConfig.pncAdasByteOffset];
        pncAdas = ((pncAdasByte & CanComm_LegacyNmParseConfig.pncAdasBitMask) != 0u) ? TRUE : FALSE;
    }

    /* Project-private combination policy (legacy compatibility) */
    if (pni == TRUE)
    {
        if ((pncGlobal == FALSE) && (pncAdas == TRUE))
        {
            pncRequest = CANCOMM_PNC_ADAS_ONLY;
        }
        else
        {
            pncRequest = CANCOMM_PNC_GLOBAL;
        }
    }
    else
    {
        pncRequest = CANCOMM_PNC_NONE;
    }

    /* Feed into state machine */
    CanComm_State_OnPncRequest(pncRequest);

    /* Optionally notify adapter for immediate processing */
    /* CanComm_Adapter_OnNmRxIndication(rxPduId, pduInfo); */
}
#endif /* CANCOMM_ENABLE_LEGACY_NM_PARSE */
