# CommPolicyCoordinator Host Unit Test Makefile
# Usage: make / make test / make clean
#
# Build-system selection of Adapter implementation:
#   - Host test    : links project/generic/CommPolicy_GenericMock.c
#   - Vector target: links project/vector/CommPolicy_VectorAdapter.c
# Weak symbols are NOT used for portability.

CC      := gcc
CFLAGS  := -std=c99 -Wall -Wextra -g -O0
INCS    := -I../include -I../src -I../stub/BSW -Isupport

# Core module sources (ApplyDecision is in Adapter.c;
# project layer provides primitives via GenericMock or VectorAdapter)
# Project configuration is selected via GenericCfg (host test) or VectorCfg (target)
SRCS    := ../project/generic/CommPolicy_GenericCfg.c \
           ../src/CommPolicy_Policy.c \
           ../src/CommPolicy_State.c \
           ../src/CommPolicyCoordinator_App.c \
           ../src/CommPolicy_Debug.c \
           ../src/CommPolicy_SourceMask.c \
           ../src/CommPolicy_ChannelMask.c \
           ../src/CommPolicy_Adapter.c \
           ../src/CommPolicyCoordinator.c \
           ../project/generic/CommPolicy_GenericMock.c \
           support/unity.c

TESTS   := test_sourcemask.c \
           test_channelmask.c \
           test_policy.c \
           test_state.c \
           test_cfg.c \
           test_adapter.c \
           test_mainfunction.c \
           test_runner.c

OBJS    := $(patsubst %.c,%.o,$(notdir $(SRCS) $(TESTS)))

VPATH   := ../src:../project/generic:support:.

TARGET  := CommPolicyCoordinator_tests.exe

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
