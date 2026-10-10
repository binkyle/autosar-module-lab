CC ?= cc
CFLAGS ?= -O2 -std=c99 -Wall -Wextra -Wpedantic -Werror
CPPFLAGS += -Iinclude
BUILD_DIR ?= build
RTM_BUILD_STACK ?= 1
SOURCES = src/rtm.c src/port.c
HEADERS = include/rtm/rtm.h include/rtm/port.h
STACK_SOURCES = src/stack.c ports/stack_os.c
STACK_HEADERS = include/rtm/stack.h include/rtm/stack_os.h
RUNTIME_PROGRAMS = rtm_tests rtm_concurrent_tests basic task_isr multicore autosar_concept baremetal benchmark
STACK_PROGRAMS = stack_tests stack_concurrent_tests stack_simulator stack_autosar_concept stack_benchmark
PROGRAMS = $(RUNTIME_PROGRAMS)
ifeq ($(RTM_BUILD_STACK),1)
PROGRAMS += $(STACK_PROGRAMS)
endif
.PHONY: all test runtime-test stack-test stack-benchmark clean
all: $(addprefix $(BUILD_DIR)/,$(PROGRAMS))
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
$(BUILD_DIR)/rtm_tests: tests/test_rtm.c $(SOURCES) $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(SOURCES) -o $@
$(BUILD_DIR)/rtm_concurrent_tests: tests/test_concurrent.c $(SOURCES) $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(SOURCES) -pthread -o $@
$(BUILD_DIR)/benchmark: tools/benchmark.c $(SOURCES) $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(SOURCES) -o $@
$(BUILD_DIR)/baremetal: examples/baremetal.c ports/stm_low32.c $(SOURCES) $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< ports/stm_low32.c $(SOURCES) -o $@
$(BUILD_DIR)/stack_tests: tests/test_stack.c $(STACK_SOURCES) $(STACK_HEADERS) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(STACK_SOURCES) -o $@
$(BUILD_DIR)/stack_concurrent_tests: tests/test_stack_concurrent.c $(STACK_SOURCES) $(STACK_HEADERS) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(STACK_SOURCES) -pthread -o $@
$(BUILD_DIR)/stack_benchmark: tools/stack_benchmark.c $(STACK_SOURCES) $(STACK_HEADERS) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(STACK_SOURCES) -o $@
$(BUILD_DIR)/stack_%: examples/stack_%.c $(STACK_SOURCES) $(STACK_HEADERS) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(STACK_SOURCES) -o $@
$(BUILD_DIR)/%: examples/%.c $(SOURCES) $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(SOURCES) -o $@
runtime-test: $(addprefix $(BUILD_DIR)/,$(RUNTIME_PROGRAMS))
	$(BUILD_DIR)/rtm_tests
	$(BUILD_DIR)/rtm_concurrent_tests
	$(BUILD_DIR)/basic
	$(BUILD_DIR)/task_isr
	$(BUILD_DIR)/multicore
	$(BUILD_DIR)/autosar_concept
	$(BUILD_DIR)/baremetal
	python3 tests/test_tools.py
stack-test: $(addprefix $(BUILD_DIR)/,$(STACK_PROGRAMS))
	$(BUILD_DIR)/stack_tests
	$(BUILD_DIR)/stack_concurrent_tests
	$(BUILD_DIR)/stack_simulator
	$(BUILD_DIR)/stack_autosar_concept
stack-benchmark: $(BUILD_DIR)/stack_benchmark
	$(BUILD_DIR)/stack_benchmark
test: runtime-test
ifeq ($(RTM_BUILD_STACK),1)
test: stack-test
endif
clean:
	rm -rf $(BUILD_DIR)
