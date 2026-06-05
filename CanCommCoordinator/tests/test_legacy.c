#include "unity.h"
#include "CanCommCoordinator_Cfg.h"
#include "CanCommCoordinator_Types.h"

extern const CanComm_LegacyNmParseConfigType CanComm_LegacyNmParseConfig;

/* --- Legacy Configuration Tests ----------------------------------------- */

void test_legacy_nm_parse_default_off(void)
{
    /* Legacy NM parse must be STD_OFF by default in standard mode */
    TEST_ASSERT_EQUAL_HEX8(STD_OFF, CANCOMM_ENABLE_LEGACY_NM_PARSE);
}

void test_legacy_nm_parse_offset_config(void)
{
    /* Verify configured offsets match project migration specification */
    TEST_ASSERT_EQUAL_UINT8(5u, CanComm_LegacyNmParseConfig.pniByteOffset);
    TEST_ASSERT_EQUAL_UINT8(5u, CanComm_LegacyNmParseConfig.pncGlobalByteOffset);
    TEST_ASSERT_EQUAL_UINT8(6u, CanComm_LegacyNmParseConfig.pncAdasByteOffset);
}

void test_legacy_nm_parse_mask_config(void)
{
    /* Verify configured bit masks */
    TEST_ASSERT_EQUAL_HEX8(0x40u, CanComm_LegacyNmParseConfig.pniBitMask);
    TEST_ASSERT_EQUAL_HEX8(0x08u, CanComm_LegacyNmParseConfig.pncGlobalBitMask);
    TEST_ASSERT_EQUAL_HEX8(0x01u, CanComm_LegacyNmParseConfig.pncAdasBitMask);
}
