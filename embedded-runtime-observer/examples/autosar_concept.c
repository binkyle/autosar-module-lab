/* SPDX-License-Identifier: MIT. Mock hooks, no vendor source. */
#include "rtm/port.h"
#include <stdio.h>
typedef struct { uint32_t now; unsigned int masked; } mock_stm_t;
static rtm_lock_token_t mask(void *u) {
    mock_stm_t *s = u;
    unsigned int old = s->masked;
    s->masked = 1;
    return old;
}
static void restore(void *u, rtm_lock_token_t token) {
    ((mock_stm_t *)u)->masked = (unsigned int)token;
}
static uint32_t stm(void *u) { return ((mock_stm_t *)u)->now; }
/* These are adapter examples, NOT the standard/vendor hook function signatures. */
static rtm_status_t mock_pre_taskhook(rtm_port_t *p, rtm_id_t id) {
    return rtm_port_event(p, RTM_EVENT_SWITCH, id, 0);
}
static rtm_status_t mock_post_taskhook(rtm_port_t *p) {
    return rtm_port_event(p, RTM_EVENT_SWITCH, RTM_KERNEL, 0);
}
#define CHECK(call) do { if ((call) != RTM_OK) return 1; } while (0)
int main(void) {
    static rtm_core_t c;
    mock_stm_t clock = {0, 0};
    rtm_port_t p = {&c, &clock, mask, restore, stm};
    rtm_config_t cfg = {100, 100, 0, 32, 0};
    rtm_snapshot_t s;
    CHECK(rtm_init(&c, &cfg, 0, RTM_IDLE));
    clock.now = 40;
    CHECK(rtm_port_event(&p, RTM_EVENT_ISR_ENTER, 0, 0));
    clock.now = 50;
    CHECK(rtm_port_event(&p, RTM_EVENT_ISR_EXIT, 0, 0));
    /* Real ordering and Idle hook availability must be checked in the OS port. */
    CHECK(mock_post_taskhook(&p));
    clock.now = 52;
    CHECK(mock_pre_taskhook(&p, 0));
    clock.now = 82;
    CHECK(mock_post_taskhook(&p));
    clock.now = 84;
    CHECK(mock_pre_taskhook(&p, RTM_IDLE));
    clock.now = 100;
    CHECK(rtm_port_snapshot(&p, &s));
    printf("Mock hooks: task=%llu kernel=%llu ISR=%llu idle=%llu load=%.1f%%\n",
           (unsigned long long)s.lifetime.task_ticks,
           (unsigned long long)s.lifetime.kernel_ticks,
           (unsigned long long)s.lifetime.isr_ticks,
           (unsigned long long)s.lifetime.idle_ticks, rtm_cpu_load(&s.lifetime));
    return 0;
}
