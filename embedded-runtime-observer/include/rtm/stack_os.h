/* SPDX-License-Identifier: MIT */
#ifndef RTM_STACK_OS_H
#define RTM_STACK_OS_H
#include "rtm/stack.h"
typedef enum { RTM_STACK_OS_PEAK_USED, RTM_STACK_OS_MIN_FREE } rtm_stack_os_semantics_t;
typedef rtm_stack_status_t (*rtm_stack_os_read_fn)(
    void *user, size_t *raw_units, rtm_stack_guard_t *guard);
typedef struct {
    size_t capacity_bytes, bytes_per_unit;
    rtm_stack_os_semantics_t semantics;
    rtm_stack_os_read_fn read;
    void *user;
} rtm_stack_os_adapter_t;
/* Normalize a documented OS API to historical peak USED BYTES.
   Vector-style byte peaks: PEAK_USED, unit=1.
   FreeRTOS-style minimum free words: MIN_FREE, unit=sizeof(StackType_t).
   This adapter performs no OS-stack memory access and imports no vendor API.
   The callback enforces readiness, IDs, core/context and access permission.
   Schedule potentially blocking remote queries in a permitted report task. */
rtm_stack_status_t rtm_stack_os_query(void *adapter,
                                      rtm_stack_provider_sample_t *sample);
#endif
