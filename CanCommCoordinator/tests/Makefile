# CanCommCoordinator Host Unit Test Makefile
# Usage: make / make test / make clean
#
# Build-system selection of Adapter implementation:
#   - Host test   : links project/generic/CanCommCoordinator_GenericMock.c
#   - Vector target: links project/vector/CanCommCoordinator_VectorAdapter.c
# Weak symbols are NOT used for portability.

CC      := gcc
CFLAGS  := -std=c99 -Wall -Wextra -g -O0
INCS    := -I../include -I../src -I../stub/BSW -Isupport

# Core module sources (Adapter.c is intentionally EXCLUDED;
# its implementation is selected via GenericMock or VectorAdapter)
SRCS    := ../src/CanCommCoordinator_Cfg.c \
           ../src/CanCommCoordinator_Policy.c \
           ../src/CanCommCoordinator_State.c \
           ../src/CanCommCoordinator_App.c \
           ../src/CanCommCoordinator_Debug.c \
           ../src/CanCommCoordinator.c \
           ../project/generic/CanCommCoordinator_GenericMock.c \
           support/unity.c

TESTS   := test_policy.c \
           test_state.c \
           test_cfg.c \
           test_adapter.c \
           test_legacy.c \
           test_mainfunction.c \
           test_runner.c

OBJS    := $(patsubst %.c,%.o,$(notdir $(SRCS) $(TESTS)))

VPATH   := ../src:../project/generic:support:.

TARGET  := CanCommCoordinator_tests.exe

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
