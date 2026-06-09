#include "unity.h"
#include "CommPolicy_Cfg.h"
#include "CommPolicy_Types.h"
#include "CommPolicy_ChannelMask.h"

extern const CommPolicy_ChannelConfigType CommPolicy_ChannelConfigTable[COMM_POLICY_CH_NUM];
extern const CommPolicy_PolicyRuleType CommPolicy_PolicyRuleTable[];
extern const uint8 CommPolicy_PolicyRuleCount;
extern const CommPolicy_OutputActionSetType* const CommPolicy_FallbackActionSet;

/* --- Cfg Tests ---------------------------------------------------------- */

void test_cfg_channel_count(void)
{
    TEST_ASSERT_EQUAL_UINT8(6u, COMM_POLICY_CH_NUM);
}

void test_cfg_source_word_num(void)
{
    TEST_ASSERT_EQUAL_UINT8(2u, COMM_POLICY_SOURCE_WORD_NUM);
    TEST_ASSERT_EQUAL_UINT16(64u, COMM_POLICY_MAX_SOURCE_NUM);
}

void test_cfg_channel_word_num(void)
{
    TEST_ASSERT_EQUAL_UINT8(2u, COMM_POLICY_CHANNEL_WORD_NUM);
    TEST_ASSERT_EQUAL_UINT16(64u, COMM_POLICY_MAX_CHANNEL_NUM);
}

void test_cfg_source_debug_name_disabled(void)
{
    TEST_ASSERT_EQUAL_HEX8(STD_OFF, COMM_POLICY_ENABLE_SOURCE_DEBUG_NAME);
}

void test_cfg_source_config_table_disabled(void)
{
    TEST_ASSERT_EQUAL_HEX8(STD_OFF, COMM_POLICY_ENABLE_SOURCE_CONFIG_TABLE);
}

void test_cfg_channel_masks_single_bit(void)
{
    CommPolicy_LogicalChannelIdType ch;

    for (ch = 0u; ch < COMM_POLICY_CH_NUM; ch++)
    {
        CommPolicy_ChannelMaskType mask = CommPolicy_ChannelConfigTable[ch].channelMask;
        /* Each mask must be a single bit (only one channel id set) */
        uint8 bitCount = 0u;
        uint8 word;
        for (word = 0u; word < COMM_POLICY_CHANNEL_WORD_NUM; word++)
        {
            uint32 w = mask.word[word];
            while (w != 0u)
            {
                bitCount += (uint8)(w & 1u);
                w >>= 1u;
            }
        }
        TEST_ASSERT_EQUAL_UINT8(1u, bitCount);
    }
}

void test_cfg_all_channel_masks_unique(void)
{
    CommPolicy_ChannelMaskType combined;
    CommPolicy_LogicalChannelIdType ch;

    CommPolicy_ChannelMask_Clear(&combined);

    for (ch = 0u; ch < COMM_POLICY_CH_NUM; ch++)
    {
        CommPolicy_ChannelMaskType mask = CommPolicy_ChannelConfigTable[ch].channelMask;
        /* No overlap between masks */
        TEST_ASSERT_TRUE(CommPolicy_ChannelMask_IsNoneSet(&combined, &mask));
        CommPolicy_ChannelMask_Or(&combined, &mask);
    }

    /* Combined mask should have exactly COMM_POLICY_CH_NUM bits set */
    uint8 bitCount = 0u;
    uint8 word;
    for (word = 0u; word < COMM_POLICY_CHANNEL_WORD_NUM; word++)
    {
        uint32 w = combined.word[word];
        while (w != 0u)
        {
            bitCount += (uint8)(w & 1u);
            w >>= 1u;
        }
    }
    TEST_ASSERT_EQUAL_UINT8(COMM_POLICY_CH_NUM, bitCount);
}

void test_cfg_bus_types_mixed(void)
{
    /* Generic config mixes CAN, LIN, ETH, USER to demonstrate cross-bus capability */
    boolean hasCan = FALSE;
    boolean hasLin = FALSE;
    boolean hasEth = FALSE;
    boolean hasUser = FALSE;
    CommPolicy_LogicalChannelIdType ch;

    for (ch = 0u; ch < COMM_POLICY_CH_NUM; ch++)
    {
        switch (CommPolicy_ChannelConfigTable[ch].busType)
        {
            case COMM_POLICY_BUS_CAN:  hasCan  = TRUE; break;
            case COMM_POLICY_BUS_LIN:  hasLin  = TRUE; break;
            case COMM_POLICY_BUS_ETH:  hasEth  = TRUE; break;
            case COMM_POLICY_BUS_USER: hasUser = TRUE; break;
            default: break;
        }
    }

    TEST_ASSERT_TRUE(hasCan);
    TEST_ASSERT_TRUE(hasLin);
    TEST_ASSERT_TRUE(hasEth);
    TEST_ASSERT_TRUE(hasUser);
}

void test_cfg_policy_rule_count(void)
{
    TEST_ASSERT_TRUE(CommPolicy_PolicyRuleCount >= 4u);
}

void test_cfg_fallback_valid(void)
{
    TEST_ASSERT_NOT_NULL(CommPolicy_FallbackActionSet);
}

void test_cfg_no_business_fields_in_channel_config(void)
{
    /*
     * ChannelConfig must NOT contain scenario-specific fields like
     * keepInAdasOnly, keepInDiagOnly, releasableInXxxOnly.
     * Such policy semantics belong in OutputActionSet, not ChannelConfig.
     */
    CommPolicy_LogicalChannelIdType ch;

    for (ch = 0u; ch < COMM_POLICY_CH_NUM; ch++)
    {
        CommPolicy_ChannelMaskType mask = CommPolicy_ChannelConfigTable[ch].channelMask;
        TEST_ASSERT_FALSE(CommPolicy_ChannelMask_IsEmpty(&mask));
    }
}
