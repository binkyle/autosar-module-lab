#include "unity.h"
#include "CommPolicy_ChannelMask.h"
#include "CommPolicy_Cfg.h"

/* --- ChannelMask Tool Tests --------------------------------------------- */

void test_channelmask_clear(void)
{
    CommPolicy_ChannelMaskType mask;
    CommPolicy_ChannelMask_Clear(&mask);
    CommPolicy_ChannelMask_Set(&mask, 0u);
    CommPolicy_ChannelMask_Set(&mask, 5u);
    CommPolicy_ChannelMask_Clear(&mask);
    TEST_ASSERT_TRUE(CommPolicy_ChannelMask_IsEmpty(&mask));
}

void test_channelmask_set_and_isset(void)
{
    CommPolicy_ChannelMaskType mask;
    CommPolicy_ChannelMask_Clear(&mask);
    TEST_ASSERT_EQUAL(E_OK, CommPolicy_ChannelMask_Set(&mask, 5u));
    TEST_ASSERT_TRUE(CommPolicy_ChannelMask_IsSet(&mask, 5u));
    TEST_ASSERT_FALSE(CommPolicy_ChannelMask_IsSet(&mask, 0u));
}

void test_channelmask_reset(void)
{
    CommPolicy_ChannelMaskType mask;
    CommPolicy_ChannelMask_Clear(&mask);
    CommPolicy_ChannelMask_Set(&mask, 5u);
    TEST_ASSERT_EQUAL(E_OK, CommPolicy_ChannelMask_Reset(&mask, 5u));
    TEST_ASSERT_FALSE(CommPolicy_ChannelMask_IsSet(&mask, 5u));
}

void test_channelmask_set_out_of_bounds(void)
{
    CommPolicy_ChannelMaskType mask;
    CommPolicy_ChannelMask_Clear(&mask);
    TEST_ASSERT_EQUAL(E_NOT_OK, CommPolicy_ChannelMask_Set(&mask, COMM_POLICY_MAX_CHANNEL_NUM));
    TEST_ASSERT_EQUAL(E_NOT_OK, CommPolicy_ChannelMask_Set(NULL_PTR, 0u));
}

void test_channelmask_isallset(void)
{
    CommPolicy_ChannelMaskType active, required;
    CommPolicy_ChannelMask_Clear(&active);
    CommPolicy_ChannelMask_Clear(&required);

    CommPolicy_ChannelMask_Set(&active, 0u);
    CommPolicy_ChannelMask_Set(&active, 1u);

    CommPolicy_ChannelMask_Set(&required, 0u);
    TEST_ASSERT_TRUE(CommPolicy_ChannelMask_IsAllSet(&active, &required));

    CommPolicy_ChannelMask_Set(&required, 2u);
    TEST_ASSERT_FALSE(CommPolicy_ChannelMask_IsAllSet(&active, &required));
}

void test_channelmask_isanyset(void)
{
    CommPolicy_ChannelMaskType active, required;
    CommPolicy_ChannelMask_Clear(&active);
    CommPolicy_ChannelMask_Clear(&required);

    CommPolicy_ChannelMask_Set(&active, 1u);
    CommPolicy_ChannelMask_Set(&required, 0u);
    CommPolicy_ChannelMask_Set(&required, 1u);
    TEST_ASSERT_TRUE(CommPolicy_ChannelMask_IsAnySet(&active, &required));
}

void test_channelmask_isanyset_empty(void)
{
    CommPolicy_ChannelMaskType active, required;
    CommPolicy_ChannelMask_Clear(&active);
    CommPolicy_ChannelMask_Clear(&required);
    /* Empty requiredAnyMask -> condition vacuously true */
    TEST_ASSERT_TRUE(CommPolicy_ChannelMask_IsAnySet(&active, &required));
}

void test_channelmask_isnoneset(void)
{
    CommPolicy_ChannelMaskType active, forbidden;
    CommPolicy_ChannelMask_Clear(&active);
    CommPolicy_ChannelMask_Clear(&forbidden);

    CommPolicy_ChannelMask_Set(&active, 0u);
    CommPolicy_ChannelMask_Set(&forbidden, 1u);
    TEST_ASSERT_TRUE(CommPolicy_ChannelMask_IsNoneSet(&active, &forbidden));

    CommPolicy_ChannelMask_Set(&forbidden, 0u);
    TEST_ASSERT_FALSE(CommPolicy_ChannelMask_IsNoneSet(&active, &forbidden));
}

void test_channelmask_multiword_channelid_33(void)
{
    CommPolicy_ChannelMaskType mask;
    CommPolicy_ChannelMask_Clear(&mask);

    /* channelId 33 is in word[1] (33/32 = 1, bit 1) */
    TEST_ASSERT_EQUAL(E_OK, CommPolicy_ChannelMask_Set(&mask, 33u));
    TEST_ASSERT_TRUE(CommPolicy_ChannelMask_IsSet(&mask, 33u));
    TEST_ASSERT_FALSE(CommPolicy_ChannelMask_IsSet(&mask, 1u));
}

void test_channelmask_null_pointer(void)
{
    CommPolicy_ChannelMaskType mask;
    CommPolicy_ChannelMask_Clear(&mask);

    TEST_ASSERT_EQUAL(E_NOT_OK, CommPolicy_ChannelMask_Set(NULL_PTR, 0u));
    TEST_ASSERT_EQUAL(E_NOT_OK, CommPolicy_ChannelMask_Reset(NULL_PTR, 0u));
    TEST_ASSERT_FALSE(CommPolicy_ChannelMask_IsSet(NULL_PTR, 0u));
    TEST_ASSERT_FALSE(CommPolicy_ChannelMask_IsEmpty(NULL_PTR));
    TEST_ASSERT_FALSE(CommPolicy_ChannelMask_IsAllSet(NULL_PTR, &mask));
    TEST_ASSERT_FALSE(CommPolicy_ChannelMask_IsAllSet(&mask, NULL_PTR));
    TEST_ASSERT_FALSE(CommPolicy_ChannelMask_IsAnySet(NULL_PTR, &mask));
    TEST_ASSERT_FALSE(CommPolicy_ChannelMask_IsNoneSet(NULL_PTR, &mask));
}
