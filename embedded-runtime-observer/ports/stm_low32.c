/* SPDX-License-Identifier: MIT */
#include "stm_low32.h"
uint32_t rtm_stm_low32_read(void *timer) {
    const rtm_stm_low32_t *stm = timer;
    /* Valid, aligned, readable peripheral address supplied by the BSP; no
       hard-coded address, no clock-frequency assumption, no CAP race. */
    return *stm->tim0;
}
