/* SPDX-License-Identifier: MIT */
#define _POSIX_C_SOURCE 200809L
#include "rtm/stack.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#define SAMPLES 10000
static unsigned char memory[16388];
static volatile size_t sink;
static uint64_t nanoseconds(void) {
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t) != 0) return 0;
    return (uint64_t)t.tv_sec * 1000000000u + (uint64_t)t.tv_nsec;
}
int main(void) {
    const size_t capacities[] = {1024,2048,8192,16384};
    const unsigned percentages[] = {0,25,50,90,100};
    size_t ci, pi;
    unsigned i;
    uint64_t overhead = 0;
    printf("Host stack watermark benchmark; compiler=%s; size_t_bits=%zu; samples=%u; cell=4 B\n",
           __VERSION__, sizeof(size_t) * CHAR_BIT, SAMPLES);
    for (i = 0; i < SAMPLES; ++i) {
        uint64_t begin = nanoseconds(), end = nanoseconds();
        if (begin == 0 || end < begin) return 1;
        overhead += end - begin;
    }
    printf("timer_pair_mean_ns=%.2f; raw times include clock calls, scan, guard, statistics and Host noise\n",
           (double)overhead / SAMPLES);
    puts("capacity_bytes,peak_bytes,scanned_cells,mean_ns,max_ns");
    for (ci = 0; ci < 4; ++ci) for (pi = 0; pi < 5; ++pi) {
        rtm_stack_region_t cfg = {0};
        rtm_stack_t stack;
        rtm_stack_usage_t result;
        size_t capacity = capacities[ci];
        size_t used = (capacity * percentages[pi] / 100 / 4) * 4;
        uint64_t sum = 0, maximum = 0;
        cfg.owner = RTM_STACK_APPLICATION;
        cfg.memory = memory;
        cfg.region_bytes = capacity + 4;
        cfg.guard_bytes = cfg.cell_bytes = 4;
        memset(cfg.fill_pattern, 0xaa, sizeof(cfg.fill_pattern));
        memset(cfg.guard_pattern, 0xde, sizeof(cfg.guard_pattern));
        if (rtm_stack_init(&stack, &cfg) || rtm_stack_paint(&stack)) return 2;
        memset(memory + 4 + capacity - used, 0x55, used);
        for (i = 0; i < 200; ++i) if (rtm_stack_measure(&stack, &result)) return 3;
        for (i = 0; i < SAMPLES; ++i) {
            uint64_t begin = nanoseconds(), end, delta;
            if (rtm_stack_measure(&stack, &result) || result.peak_used_bytes != used) return 4;
            end = nanoseconds();
            if (begin == 0 || end < begin) return 5;
            delta = end - begin;
            sum += delta;
            if (delta > maximum) maximum = delta;
            sink = result.peak_used_bytes;
        }
        printf("%zu,%zu,%zu,%.2f,%llu\n", capacity, used, result.scanned_cells,
               (double)sum / SAMPLES, (unsigned long long)maximum);
    }
    return 0;
}
