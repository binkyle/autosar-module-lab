/* SPDX-License-Identifier: MIT */
#include "rtm/rtm.h"
#include <stdio.h>
int main(void) {
    static rtm_core_t cores[2];
    rtm_snapshot_t snapshots[2];
    unsigned int i;
    for (i = 0; i < 2; ++i) {
        rtm_config_t cfg = {100, 100, (uint16_t)i, 32, 1};
        /* Independent epochs: 0 and 1000. No global alignment is claimed. */
        if (rtm_init(&cores[i], &cfg, i * 1000u, i ? 0 : RTM_IDLE) != RTM_OK)
            return 1;
        if (rtm_snapshot(&cores[i], i * 1000u + 100u, &snapshots[i]) != RTM_OK)
            return 1;
        printf("Core %u: %.1f%%\n", i, rtm_cpu_load(&snapshots[i].last_window.time));
    }
    return 0;
}
