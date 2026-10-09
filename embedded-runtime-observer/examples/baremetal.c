/* SPDX-License-Identifier: MIT. Bare-metal event model with a mock register. */
#include "rtm/rtm.h"
#include "../ports/stm_low32.h"
#include <stdio.h>
#define CHECK(call) do { if ((call) != RTM_OK) return 1; } while (0)
int main(void) {
    static rtm_core_t c;
    volatile uint32_t mock_tim0 = 0;
    rtm_stm_low32_t timer = {&mock_tim0};
    rtm_config_t cfg = {100, 100, 0, 32, 1};
    rtm_snapshot_t s;
    /* This single-threaded simulation needs no mask. On target, use port.h. */
    CHECK(rtm_init(&c, &cfg, rtm_stm_low32_read(&timer), RTM_IDLE));
    mock_tim0 = 10;
    CHECK(rtm_task_switch(&c, rtm_stm_low32_read(&timer), 0));
    mock_tim0 = 40;
    CHECK(rtm_task_switch(&c, rtm_stm_low32_read(&timer), RTM_IDLE));
    mock_tim0 = 100;
    CHECK(rtm_snapshot(&c, rtm_stm_low32_read(&timer), &s));
    printf("Bare-metal mock register: load=%.1f%%\n", rtm_cpu_load(&s.lifetime));
    return 0;
}
