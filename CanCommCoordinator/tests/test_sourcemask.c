#include "unity.h"
#include "CanCommCoordinator_SourceMask.h"
#include "CanCommCoordinator_Cfg.h"

/* --- SourceMask Tool Tests ---------------------------------------------- */

void test_sourcemask_clear(void)
{
    CanComm_SourceMaskType mask;
    CanComm_SourceMask_Clear(&mask);
    CanComm_SourceMask_Set(&mask, 0u);
    CanComm_SourceMask_Set(&mask, 5u);
    CanComm_SourceMask_Clear(&mask);
    TEST_ASSERT_TRUE(CanComm_SourceMask_IsEmpty(&mask));
}

void test_sourcemask_set_and_isset(void)
{
    CanComm_SourceMaskType mask;
    CanComm_SourceMask_Clear(&mask);
    TEST_ASSERT_EQUAL(E_OK, CanComm_SourceMask_Set(&mask, 5u));
    TEST_ASSERT_TRUE(CanComm_SourceMask_IsSet(&mask, 5u));
    TEST_ASSERT_FALSE(CanComm_SourceMask_IsSet(&mask, 0u));
}

void test_sourcemask_reset(void)
{
    CanComm_SourceMaskType mask;
    CanComm_SourceMask_Clear(&mask);
    CanComm_SourceMask_Set(&mask, 5u);
    TEST_ASSERT_EQUAL(E_OK, CanComm_SourceMask_Reset(&mask, 5u));
    TEST_ASSERT_FALSE(CanComm_SourceMask_IsSet(&mask, 5u));
}

void test_sourcemask_set_out_of_bounds(void)
{
    CanComm_SourceMaskType mask;
    CanComm_SourceMask_Clear(&mask);
    TEST_ASSERT_EQUAL(E_NOT_OK, CanComm_SourceMask_Set(&mask, CANCOMM_MAX_SOURCE_NUM));
    TEST_ASSERT_EQUAL(E_NOT_OK, CanComm_SourceMask_Set(NULL_PTR, 0u));
}

void test_sourcemask_isallset(void)
{
    CanComm_SourceMaskType active, required;
    CanComm_SourceMask_Clear(&active);
    CanComm_SourceMask_Clear(&required);

    CanComm_SourceMask_Set(&active, 0u);
    CanComm_SourceMask_Set(&active, 1u);

    CanComm_SourceMask_Set(&required, 0u);
    TEST_ASSERT_TRUE(CanComm_SourceMask_IsAllSet(&active, &required));

    CanComm_SourceMask_Set(&required, 2u);
    TEST_ASSERT_FALSE(CanComm_SourceMask_IsAllSet(&active, &required));
}

void test_sourcemask_isanyset(void)
{
    CanComm_SourceMaskType active, required;
    CanComm_SourceMask_Clear(&active);
    CanComm_SourceMask_Clear(&required);

    CanComm_SourceMask_Set(&active, 1u);
    CanComm_SourceMask_Set(&required, 0u);
    CanComm_SourceMask_Set(&required, 1u);
    TEST_ASSERT_TRUE(CanComm_SourceMask_IsAnySet(&active, &required));
}

void test_sourcemask_isanyset_empty(void)
{
    CanComm_SourceMaskType active, required;
    CanComm_SourceMask_Clear(&active);
    CanComm_SourceMask_Clear(&required);
    /* Empty requiredAnyMask -> condition vacuously true */
    TEST_ASSERT_TRUE(CanComm_SourceMask_IsAnySet(&active, &required));
}

void test_sourcemask_isnoneset(void)
{
    CanComm_SourceMaskType active, forbidden;
    CanComm_SourceMask_Clear(&active);
    CanComm_SourceMask_Clear(&forbidden);

    CanComm_SourceMask_Set(&active, 0u);
    CanComm_SourceMask_Set(&forbidden, 1u);
    TEST_ASSERT_TRUE(CanComm_SourceMask_IsNoneSet(&active, &forbidden));

    CanComm_SourceMask_Set(&forbidden, 0u);
    TEST_ASSERT_FALSE(CanComm_SourceMask_IsNoneSet(&active, &forbidden));
}

void test_sourcemask_multiword_sourceid_33(void)
{
    CanComm_SourceMaskType mask;
    CanComm_SourceMask_Clear(&mask);

    /* sourceId 33 is in word[1] (33/32 = 1, bit 1) */
    TEST_ASSERT_EQUAL(E_OK, CanComm_SourceMask_Set(&mask, 33u));
    TEST_ASSERT_TRUE(CanComm_SourceMask_IsSet(&mask, 33u));
    TEST_ASSERT_FALSE(CanComm_SourceMask_IsSet(&mask, 1u));
}

void test_sourcemask_null_pointer(void)
{
    CanComm_SourceMaskType mask;
    CanComm_SourceMask_Clear(&mask);

    TEST_ASSERT_EQUAL(E_NOT_OK, CanComm_SourceMask_Set(NULL_PTR, 0u));
    TEST_ASSERT_EQUAL(E_NOT_OK, CanComm_SourceMask_Reset(NULL_PTR, 0u));
    TEST_ASSERT_FALSE(CanComm_SourceMask_IsSet(NULL_PTR, 0u));
    TEST_ASSERT_FALSE(CanComm_SourceMask_IsEmpty(NULL_PTR));
    TEST_ASSERT_FALSE(CanComm_SourceMask_IsAllSet(NULL_PTR, &mask));
    TEST_ASSERT_FALSE(CanComm_SourceMask_IsAllSet(&mask, NULL_PTR));
    TEST_ASSERT_FALSE(CanComm_SourceMask_IsAnySet(NULL_PTR, &mask));
    TEST_ASSERT_FALSE(CanComm_SourceMask_IsNoneSet(NULL_PTR, &mask));
}
