#ifndef UNITY_INTERNALS_H
#define UNITY_INTERNALS_H

#include <stdio.h>
#include <setjmp.h>

#define UNITY_VERSION_MAJOR 2
#define UNITY_VERSION_MINOR 5
#define UNITY_VERSION_BUILD 2

typedef void (*UnityTestFunction)(void);

typedef struct
{
    const char* TestFile;
    const char* CurrentTestName;
    unsigned CurrentTestLineNumber;
    unsigned NumberOfTests;
    unsigned TestFailures;
    unsigned TestIgnores;
    jmp_buf AbortFrame;
} Unity_t;

extern Unity_t Unity;

void UnityBegin(const char* filename);
int UnityEnd(void);
void UnityDefaultTestRun(UnityTestFunction Func, const char* FuncName, const int FuncLineNum);

void UnityPrint(const char* string);
void UnityPrintNumberByStyle(const long number, const char style);

void UnityAssertEqualNumber(const long expected,
                            const long actual,
                            const char* msg,
                            const unsigned short lineNum,
                            const char style);

void UnityAssertEqualIntArray(const void* expected,
                              const void* actual,
                              const unsigned int num_elements,
                              const char* msg,
                              const unsigned short lineNum,
                              const char style);

void UnityFail(const char* message, const unsigned short line);
void UnityIgnore(const char* message, const unsigned short line);

#define TEST_FAIL_MESSAGE(message)  UnityFail((message), (__LINE__))
#define TEST_IGNORE_MESSAGE(message) UnityIgnore((message), (__LINE__))

#define UNITY_TEST_ASSERT_EQUAL_INT(expected, actual, line, message) \
    UnityAssertEqualNumber((long)(expected), (long)(actual), (message), (line), 'd')

#define UNITY_TEST_ASSERT_EQUAL_UINT(expected, actual, line, message) \
    UnityAssertEqualNumber((long)(expected), (long)(actual), (message), (line), 'u')

#define UNITY_TEST_ASSERT_EQUAL_HEX8(expected, actual, line, message) \
    UnityAssertEqualNumber((long)(expected), (long)(actual), (message), (line), 'x')

#define UNITY_TEST_ASSERT_EQUAL_HEX16(expected, actual, line, message) \
    UnityAssertEqualNumber((long)(expected), (long)(actual), (message), (line), 'x')

#define UNITY_TEST_ASSERT_EQUAL_PTR(expected, actual, line, message) \
    UnityAssertEqualNumber((long)(expected), (long)(actual), (message), (line), 'p')

#define UNITY_TEST_ASSERT_NOT_NULL(pointer, line, message) \
    UnityAssertEqualNumber((long)(1), (long)(pointer != NULL), (message), (line), 'd')

#define UNITY_TEST_ASSERT_NULL(pointer, line, message) \
    UnityAssertEqualNumber((long)(0), (long)(pointer != NULL), (message), (line), 'd')

#define UNITY_TEST_ASSERT(condition, line, message) \
    UnityAssertEqualNumber((long)(1), (long)(condition), (message), (line), 'd')

#endif /* UNITY_INTERNALS_H */
