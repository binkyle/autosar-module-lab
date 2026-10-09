/* SPDX-License-Identifier: MIT. Host measurements only, not target WCET. */
#define _POSIX_C_SOURCE 200809L
#include "rtm/rtm.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
static uint64_t ns(void) {
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t) != 0) abort();
    return (uint64_t)t.tv_sec * UINT64_C(1000000000) + (uint64_t)t.tv_nsec;
}
static void run(unsigned int nested) {
    static rtm_core_t c;
    rtm_config_t cfg = {1000000, 1000000, 0, 32, 1};
    uint64_t sum = 0, maximum = 0, baseline = 0;
    unsigned int i;
    if (rtm_init(&c, &cfg, 0, 0) != RTM_OK) abort();
    if (nested) {
        for (i = 0; i < RTM_DEPTH; ++i)
            if (rtm_isr_enter(&c, 0, 0) != RTM_OK) abort();
        for (i = 0; i < RTM_DEPTH; ++i)
            if (rtm_point_begin(&c, 0, 0) != RTM_OK) abort();
    }
    for (i = 0; i < 200000; ++i) {
        uint64_t begin = ns(), delta;
        if (rtm_poll(&c, i + 1u) != RTM_OK) abort();
        delta = ns() - begin; sum += delta;
        if (delta > maximum) maximum = delta;
    }
    for (i = 0; i < 200000; ++i) { uint64_t begin = ns(); baseline += ns() - begin; }
    printf("poll_%s: samples=200000 raw_average_ns=%.2f timer_pair_baseline_ns=%.2f max_observed_ns=%llu\n",
           nested ? "nested" : "task", (double)sum / 200000.0,
           (double)baseline / 200000.0, (unsigned long long)maximum);
}
int main(void) {
    printf("sizeof(core)=%lu sizeof(snapshot)=%lu trace_capacity=%u depth=%u\n",
           (unsigned long)sizeof(rtm_core_t), (unsigned long)sizeof(rtm_snapshot_t),
           (unsigned)RTM_TRACE_CAPACITY, (unsigned)RTM_DEPTH);
    run(0); run(1);
    return 0;
}
