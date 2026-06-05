#include "unity.h"
#include <string.h>

Unity_t Unity;

static const char* UnityStrFail       = "FAIL";
static const char* UnityStrIgnore     = "IGNORE";
static const char* UnityStrOk         = "PASS";

void UnityPrint(const char* string)
{
    if (string != NULL)
    {
        while (*string)
        {
            putchar(*string);
            string++;
        }
    }
}

void UnityPrintNumberByStyle(const long number, const char style)
{
    if (style == 'x')
    {
        printf("0x%08lX", number);
    }
    else if (style == 'u')
    {
        printf("%lu", number);
    }
    else if (style == 'p')
    {
        printf("%p", (void*)number);
    }
    else
    {
        printf("%ld", number);
    }
}

void UnityBegin(const char* filename)
{
    Unity.TestFile = filename;
    Unity.CurrentTestName = NULL;
    Unity.CurrentTestLineNumber = 0;
    Unity.NumberOfTests = 0;
    Unity.TestFailures = 0;
    Unity.TestIgnores = 0;
}

int UnityEnd(void)
{
    printf("\n-----------------------\n");
    printf("%u Tests %u Failures %u Ignored\n",
           Unity.NumberOfTests, Unity.TestFailures, Unity.TestIgnores);
    printf("-----------------------\n");
    return (int)(Unity.TestFailures);
}

void UnityDefaultTestRun(UnityTestFunction Func, const char* FuncName, const int FuncLineNum)
{
    if (TEST_PROTECT())
    {
        setUp();
        Unity.CurrentTestName = FuncName;
        Unity.CurrentTestLineNumber = (unsigned)FuncLineNum;
        Func();
    }
    if (TEST_PROTECT())
    {
        tearDown();
    }
    Unity.CurrentTestName = NULL;
    Unity.NumberOfTests++;
}

void UnityAssertEqualNumber(const long expected,
                            const long actual,
                            const char* msg,
                            const unsigned short lineNum,
                            const char style)
{
    if (expected != actual)
    {
        Unity.TestFailures++;
        printf("\n%s:%u: %s: Expected ", Unity.TestFile, lineNum, Unity.CurrentTestName);
        UnityPrintNumberByStyle(expected, style);
        printf(" Was ");
        UnityPrintNumberByStyle(actual, style);
        if (msg != NULL)
        {
            printf(" : %s", msg);
        }
        printf("\n");
        TEST_ABORT();
    }
}

void UnityAssertEqualIntArray(const void* expected,
                              const void* actual,
                              const unsigned int num_elements,
                              const char* msg,
                              const unsigned short lineNum,
                              const char style)
{
    /* Simplified: not used directly in this module's tests */
    (void)expected;
    (void)actual;
    (void)num_elements;
    (void)msg;
    (void)lineNum;
    (void)style;
}

void UnityFail(const char* message, const unsigned short line)
{
    Unity.TestFailures++;
    printf("\n%s:%u: %s: %s\n", Unity.TestFile, line, Unity.CurrentTestName,
           message ? message : UnityStrFail);
    TEST_ABORT();
}

void UnityIgnore(const char* message, const unsigned short line)
{
    Unity.TestIgnores++;
    printf("\n%s:%u: %s: %s\n", Unity.TestFile, line, Unity.CurrentTestName,
           message ? message : UnityStrIgnore);
    TEST_ABORT();
}
