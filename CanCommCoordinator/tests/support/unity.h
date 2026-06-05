#ifndef UNITY_H
#define UNITY_H

#include "unity_internals.h"

#define TEST_PROTECT() (setjmp(Unity.AbortFrame) == 0)
#define TEST_ABORT() longjmp(Unity.AbortFrame, 1)

#define TEST_ASSERT_EQUAL(expected, actual) \
    UNITY_TEST_ASSERT_EQUAL_INT((expected), (actual), __LINE__, NULL)

#define TEST_ASSERT_EQUAL_UINT8(expected, actual) \
    UNITY_TEST_ASSERT_EQUAL_HEX8((expected), (actual), __LINE__, NULL)

#define TEST_ASSERT_EQUAL_UINT16(expected, actual) \
    UNITY_TEST_ASSERT_EQUAL_HEX16((expected), (actual), __LINE__, NULL)

#define TEST_ASSERT_EQUAL_INT(expected, actual) \
    UNITY_TEST_ASSERT_EQUAL_INT((expected), (actual), __LINE__, NULL)

#define TEST_ASSERT_EQUAL_HEX8(expected, actual) \
    UNITY_TEST_ASSERT_EQUAL_HEX8((expected), (actual), __LINE__, NULL)

#define TEST_ASSERT_EQUAL_PTR(expected, actual) \
    UNITY_TEST_ASSERT_EQUAL_PTR((expected), (actual), __LINE__, NULL)

#define TEST_ASSERT_NOT_NULL(pointer) \
    UNITY_TEST_ASSERT_NOT_NULL((pointer), __LINE__, NULL)

#define TEST_ASSERT_NULL(pointer) \
    UNITY_TEST_ASSERT_NULL((pointer), __LINE__, NULL)

#define TEST_ASSERT_TRUE(condition) \
    UNITY_TEST_ASSERT((condition), __LINE__, " Expected TRUE")

#define TEST_ASSERT_FALSE(condition) \
    UNITY_TEST_ASSERT(!(condition), __LINE__, " Expected FALSE")

#define TEST_ASSERT_MESSAGE(condition, message) \
    UNITY_TEST_ASSERT((condition), __LINE__, (message))

#define TEST_FAIL() TEST_FAIL_MESSAGE(NULL)

#define TEST_IGNORE() TEST_IGNORE_MESSAGE(NULL)

#define RUN_TEST(func) UnityDefaultTestRun(func, #func, __LINE__)

void setUp(void);
void tearDown(void);

#endif /* UNITY_H */
