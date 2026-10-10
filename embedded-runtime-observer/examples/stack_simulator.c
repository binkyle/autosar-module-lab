/* SPDX-License-Identifier: MIT */
#include "rtm/stack.h"
#include <stdio.h>
#include <string.h>

static unsigned char task_memory[2052], other_memory[1024];
static rtm_stack_t task, other;
int main(void) {
    rtm_stack_region_t cfg = {0};
    rtm_stack_usage_t s;
    cfg.owner = RTM_STACK_APPLICATION;
    cfg.kind = RTM_STACK_TASK;
    cfg.memory = task_memory;
    cfg.region_bytes = sizeof(task_memory);
    cfg.guard_bytes = cfg.cell_bytes = 4;
    memset(cfg.fill_pattern, 0xaa, sizeof(cfg.fill_pattern));
    memset(cfg.guard_pattern, 0xde, sizeof(cfg.guard_pattern));
    cfg.warning_remaining_bytes = 256;
    if (rtm_stack_init(&task, &cfg) || rtm_stack_paint(&task)) return 1;
    /* Synthetic byte array, not the host's executing stack. */
    memset(task_memory + 4 + 2048 - 320, 0x55, 320);
    if (rtm_stack_measure(&task, &s) || s.peak_used_bytes != 320) return 2;
    memset(task_memory + 4 + 2048 - 640, 0x55, 640);
    if (rtm_stack_measure(&task, &s)) return 3;
    printf("Simulated Stack Capacity : %zu B\nPeak Usage : %zu B\nRemaining : %zu B\nUsage : %.2f%%\n",
           s.capacity_bytes, s.peak_used_bytes, s.remaining_bytes, s.usage_percent);
    /* Returning from a frame does not restore its old paint. */
    if (rtm_stack_measure(&task, &s) || s.peak_used_bytes != 640) return 4;
    printf("After simulated SP retreat: peak=%zu B, current=unsupported, guard=intact\n", s.peak_used_bytes);
    cfg.memory = other_memory;
    cfg.region_bytes = sizeof(other_memory);
    cfg.guard_bytes = 0;
    cfg.region_id = 1;
    if (rtm_stack_init(&other, &cfg) || rtm_stack_paint(&other) || rtm_stack_measure(&other, &s)) return 5;
    printf("Independent region: peak=%zu B\n", s.peak_used_bytes);
    task_memory[0] ^= 1;
    if (rtm_stack_measure(&task, &s) != RTM_STACK_E_GUARD || s.valid) return 6;
    printf("Synthetic guard damage: detected, valid=%u\n", (unsigned)s.valid);
    return 0;
}
