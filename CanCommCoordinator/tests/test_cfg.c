#include "unity.h"
#include "CanCommCoordinator_Cfg.h"
#include "CanCommCoordinator_Types.h"

extern const CanComm_ChannelConfigType CanComm_ChannelConfigTable[CANCOMM_CH_NUM];
extern const CanComm_PolicyRuleType CanComm_PolicyRuleTable[];
extern const uint8 CanComm_PolicyRuleCount;
extern const CanComm_OutputActionSetType* const CanComm_FallbackActionSet;

/* --- Cfg Tests ---------------------------------------------------------- */

void test_cfg_channel_count(void)
{
    TEST_ASSERT_EQUAL_UINT8(6u, CANCOMM_CH_NUM);
}

void test_cfg_adas1_channel_mask(void)
{
    TEST_ASSERT_TRUE(CANCOMM_MASK_ADAS1 == CanComm_ChannelConfigTable[CANCOMM_CH_ADAS1].channelMask);
}

void test_cfg_adas2_channel_mask(void)
{
    TEST_ASSERT_TRUE(CANCOMM_MASK_ADAS2 == CanComm_ChannelConfigTable[CANCOMM_CH_ADAS2].channelMask);
}

void test_cfg_adas_sub_channel_mask(void)
{
    TEST_ASSERT_TRUE(CANCOMM_MASK_ADAS_SUB == CanComm_ChannelConfigTable[CANCOMM_CH_ADAS_SUB].channelMask);
}

void test_cfg_pdc_channel_mask(void)
{
    TEST_ASSERT_TRUE(CANCOMM_MASK_PDC == CanComm_ChannelConfigTable[CANCOMM_CH_PDC].channelMask);
}

void test_cfg_can5_channel_mask(void)
{
    TEST_ASSERT_TRUE(CANCOMM_MASK_CAN5 == CanComm_ChannelConfigTable[CANCOMM_CH_CAN5].channelMask);
}

void test_cfg_can9_channel_mask(void)
{
    TEST_ASSERT_TRUE(CANCOMM_MASK_CAN9 == CanComm_ChannelConfigTable[CANCOMM_CH_CAN9].channelMask);
}

void test_cfg_all_masks_unique(void)
{
    uint32 combined = 0u;
    CanComm_ChannelIdType ch;

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        uint32 mask = CanComm_ChannelConfigTable[ch].channelMask;
        /* Each mask must be a single bit */
        TEST_ASSERT_TRUE((mask != 0u) && ((mask & (mask - 1u)) == 0u));
        /* No overlap between masks */
        TEST_ASSERT_TRUE(0u == (combined & mask));
        combined |= mask;
    }

    TEST_ASSERT_TRUE(CANCOMM_MASK_ALL == combined);
}

void test_cfg_policy_rule_count(void)
{
    TEST_ASSERT_TRUE(CanComm_PolicyRuleCount >= 4u);
}

void test_cfg_fallback_valid(void)
{
    TEST_ASSERT_NOT_NULL(CanComm_FallbackActionSet);
}

void test_cfg_no_business_fields_in_channel_config(void)
{
    /*
     * ChannelConfig must NOT contain scenario-specific fields like
     * keepInAdasOnly, keepInDiagOnly, releasableInXxxOnly.
     * Such policy semantics belong in OutputActionSet, not ChannelConfig.
     */
    CanComm_ChannelIdType ch;

    for (ch = 0u; ch < CANCOMM_CH_NUM; ch++)
    {
        TEST_ASSERT_TRUE(0u != CanComm_ChannelConfigTable[ch].channelMask);
    }
}
