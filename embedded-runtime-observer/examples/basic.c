/* SPDX-License-Identifier: MIT */
#include "rtm/rtm.h"
#include <stdio.h>
#define CHECK(call) do { if ((call) != RTM_OK) return 1; } while (0)
int main(void) {
    static rtm_core_t c;
    rtm_snapshot_t s;
    rtm_config_t cfg = {100, 100, 0, 32, 1};
    CHECK(rtm_init(&c, &cfg, 0, 0));
    CHECK(rtm_task_switch(&c, 20, RTM_IDLE));
    CHECK(rtm_isr_enter(&c, 50, 0));
    CHECK(rtm_isr_exit(&c, 60, 0));
    CHECK(rtm_snapshot(&c, 100, &s));
    printf("Task=%llu Idle=%llu ISR=%llu Total=%llu Load=%.1f%%\n",
           (unsigned long long)s.last_window.time.task_ticks,
           (unsigned long long)s.last_window.time.idle_ticks,
           (unsigned long long)s.last_window.time.isr_ticks,
           (unsigned long long)s.last_window.time.total_ticks,
           rtm_cpu_load(&s.last_window.time));
    return rtm_cpu_load(&s.last_window.time) == 30.0 ? 0 : 1;
}
