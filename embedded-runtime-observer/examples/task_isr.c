/* SPDX-License-Identifier: MIT */
#include "rtm/rtm.h"
#include <stdio.h>
#define CHECK(call) do { if ((call) != RTM_OK) return 1; } while (0)
int main(void) {
    static rtm_core_t c;
    rtm_snapshot_t s;
    rtm_trace_event_t e;
    rtm_config_t cfg = {100, 100, 0, 32, 1};
    CHECK(rtm_init(&c, &cfg, 0, RTM_IDLE));
    CHECK(rtm_isr_enter(&c, 10, 0));
    CHECK(rtm_isr_enter(&c, 20, 1));
    CHECK(rtm_isr_exit(&c, 30, 1));
    CHECK(rtm_isr_exit(&c, 50, 0));
    CHECK(rtm_task_switch(&c, 50, 0));
    CHECK(rtm_point_begin(&c, 50, 0));
    CHECK(rtm_point_end(&c, 90, 0));
    CHECK(rtm_task_switch(&c, 90, RTM_IDLE));
    CHECK(rtm_snapshot(&c, 100, &s));
    fprintf(stderr, "ISR_A inclusive=%llu exclusive=%llu; Load=%.1f%%\n",
            (unsigned long long)s.isr[0].inclusive_ticks,
            (unsigned long long)s.isr[0].exclusive_ticks,
            rtm_cpu_load(&s.last_window.time));
    puts("core,tick,sequence,kind,id,value");
    while (rtm_trace_pop(&c, &e) == RTM_OK)
        printf("%u,%llu,%llu,%u,%u,%u\n", (unsigned)e.core_id,
               (unsigned long long)e.tick, (unsigned long long)e.sequence,
               (unsigned)e.kind, (unsigned)e.id, (unsigned)e.value);
    return 0;
}
