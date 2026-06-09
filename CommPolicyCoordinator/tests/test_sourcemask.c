#include "unity.h"
#include "CommPolicy_SourceMask.h"
#include "CommPolicy_Cfg.h"

/* --- SourceMask Tool Tests ---------------------------------------------- */

void test_sourcemask_clear(void)
{
    CommPolicy_SourceMaskType mask;
    CommPolicy_SourceMask_Clear(&mask);
    CommPolicy_SourceMask_Set(&mask, 0u);
    CommPolicy_SourceMask_Set(&mask, 5u);
    CommPolicy_SourceMask_Clear(&mask);
    TEST_ASSERT_TRUE(CommPolicy_SourceMask_IsEmpty(&mask));
}

void test_sourcemask_set_and_isset(void)
{
    CommPolicy_SourceMaskType mask;
    CommPolicy_SourceMask_Clear(&mask);
    TEST_ASSERT_EQUAL(E_OK, CommPolicy_SourceMask_Set(&mask, 5u));
    TEST_ASSERT_TRUE(CommPolicy_SourceMask_IsSet(&mask, 5u));
    TEST_ASSERT_FALSE(CommPolicy_SourceMask_IsSet(&mask, 0u));
}

void test_sourcemask_reset(void)
{
    CommPolicy_SourceMaskType mask;
    CommPolicy_SourceMask_Clear(&mask);
    CommPolicy_SourceMask_Set(&mask, 5u);
    TEST_ASSERT_EQUAL(E_OK, CommPolicy_SourceMask_Reset(&mask, 5u));
    TEST_ASSERT_FALSE(CommPolicy_SourceMask_IsSet(&mask, 5u));
}

void test_sourcemask_set_out_of_bounds(void)
{
    CommPolicy_SourceMaskType mask;
    CommPolicy_SourceMask_Clear(&mask);
    TEST_ASSERT_EQUAL(E_NOT_OK, CommPolicy_SourceMask_Set(&mask, COMM_POLICY_MAX_SOURCE_NUM));
    TEST_ASSERT_EQUAL(E_NOT_OK, CommPolicy_SourceMask_Set(NULL_PTR, 0u));
}

void test_sourcemask_isallset(void)
{
    CommPolicy_SourceMaskType active, required;
    CommPolicy_SourceMask_Clear(&active);
    CommPolicy_SourceMask_Clear(&required);

    CommPolicy_SourceMask_Set(&active, 0u);
    CommPolicy_SourceMask_Set(&active, 1u);

    CommPolicy_SourceMask_Set(&required, 0u);
    TEST_ASSERT_TRUE(CommPolicy_SourceMask_IsAllSet(&active, &required));

    CommPolicy_SourceMask_Set(&required, 2u);
    TEST_ASSERT_FALSE(CommPolicy_SourceMask_IsAllSet(&active, &required));
}

void test_sourcemask_isanyset(void)
{
    CommPolicy_SourceMaskType active, required;
    CommPolicy_SourceMask_Clear(&active);
    CommPolicy_SourceMask_Clear(&required);

    CommPolicy_SourceMask_Set(&active, 1u);
    CommPolicy_SourceMask_Set(&required, 0u);
    CommPolicy_SourceMask_Set(&required, 1u);
    TEST_ASSERT_TRUE(CommPolicy_SourceMask_IsAnySet(&active, &required));
}

void test_sourcemask_isanyset_empty(void)
{
    CommPolicy_SourceMaskType active, required;
    CommPolicy_SourceMask_Clear(&active);
    CommPolicy_SourceMask_Clear(&required);
    /* Empty requiredAnyMask -> condition vacuously true */
    TEST_ASSERT_TRUE(CommPolicy_SourceMask_IsAnySet(&active, &required));
}

void test_sourcemask_isnoneset(void)
{
    CommPolicy_SourceMaskType active, forbidden;
    CommPolicy_SourceMask_Clear(&active);
    CommPolicy_SourceMask_Clear(&forbidden);

    CommPolicy_SourceMask_Set(&active, 0u);
    CommPolicy_SourceMask_Set(&forbidden, 1u);
    TEST_ASSERT_TRUE(CommPolicy_SourceMask_IsNoneSet(&active, &forbidden));

    CommPolicy_SourceMask_Set(&forbidden, 0u);
    TEST_ASSERT_FALSE(CommPolicy_SourceMask_IsNoneSet(&active, &forbidden));
}

void test_sourcemask_multiword_sourceid_33(void)
{
    CommPolicy_SourceMaskType mask;
    CommPolicy_SourceMask_Clear(&mask);

    /* sourceId 33 is in word[1] (33/32 = 1, bit 1) */
    TEST_ASSERT_EQUAL(E_OK, CommPolicy_SourceMask_Set(&mask, 33u));
    TEST_ASSERT_TRUE(CommPolicy_SourceMask_IsSet(&mask, 33u));
    TEST_ASSERT_FALSE(CommPolicy_SourceMask_IsSet(&mask, 1u));
}

void test_sourcemask_null_pointer(void)
{
    CommPolicy_SourceMaskType mask;
    CommPolicy_SourceMask_Clear(&mask);

    TEST_ASSERT_EQUAL(E_NOT_OK, CommPolicy_SourceMask_Set(NULL_PTR, 0u));
    TEST_ASSERT_EQUAL(E_NOT_OK, CommPolicy_SourceMask_Reset(NULL_PTR, 0u));
    TEST_ASSERT_FALSE(CommPolicy_SourceMask_IsSet(NULL_PTR, 0u));
    TEST_ASSERT_FALSE(CommPolicy_SourceMask_IsEmpty(NULL_PTR));
    TEST_ASSERT_FALSE(CommPolicy_SourceMask_IsAllSet(NULL_PTR, &mask));
    TEST_ASSERT_FALSE(CommPolicy_SourceMask_IsAllSet(&mask, NULL_PTR));
    TEST_ASSERT_FALSE(CommPolicy_SourceMask_IsAnySet(NULL_PTR, &mask));
    TEST_ASSERT_FALSE(CommPolicy_SourceMask_IsNoneSet(NULL_PTR, &mask));
}
