CC ?= cc
CFLAGS ?= -O2 -std=c99 -Wall -Wextra -Wpedantic -Werror
CPPFLAGS += -Iinclude
SOURCES = src/rtm.c src/port.c
PROGRAMS = rtm_tests rtm_concurrent_tests basic task_isr multicore autosar_concept baremetal benchmark
.PHONY: all test clean
all: $(addprefix build/,$(PROGRAMS))
build:
	mkdir -p build
build/rtm_tests: tests/test_rtm.c $(SOURCES) include/rtm/rtm.h include/rtm/port.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(SOURCES) -o $@
build/rtm_concurrent_tests: tests/test_concurrent.c $(SOURCES) include/rtm/rtm.h include/rtm/port.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(SOURCES) -pthread -o $@
build/benchmark: tools/benchmark.c $(SOURCES) include/rtm/rtm.h include/rtm/port.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(SOURCES) -o $@
build/baremetal: examples/baremetal.c ports/stm_low32.c $(SOURCES) include/rtm/rtm.h include/rtm/port.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $< ports/stm_low32.c $(SOURCES) -o $@
build/%: examples/%.c $(SOURCES) include/rtm/rtm.h include/rtm/port.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(SOURCES) -o $@
test: all
	build/rtm_tests
	build/rtm_concurrent_tests
	build/basic
	build/task_isr
	build/multicore
	build/autosar_concept
	build/baremetal
	python3 tests/test_tools.py
clean:
	rm -rf build
