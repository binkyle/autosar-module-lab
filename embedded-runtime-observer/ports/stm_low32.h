/* SPDX-License-Identifier: MIT */
#ifndef RTM_STM_LOW32_H
#define RTM_STM_LOW32_H
#include <stdint.h>
/* Experimental TC3xx integration strategy. BSP owns the mapped TIM0 pointer. */
typedef struct {
    const volatile uint32_t *tim0;
} rtm_stm_low32_t;
uint32_t rtm_stm_low32_read(void *timer);
#endif
