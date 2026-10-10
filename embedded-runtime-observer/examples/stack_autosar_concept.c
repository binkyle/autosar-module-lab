/* SPDX-License-Identifier: MIT */
#include "rtm/stack_os.h"
#include <stdio.h>

typedef struct { size_t peak; int ready; } mock_os_t;
static rtm_stack_status_t mock_os_usage(void *user, size_t *raw, rtm_stack_guard_t *guard) {
    const mock_os_t *os = user;
    if (!os->ready) return RTM_STACK_E_STATE;
    *raw = os->peak; /* Independent model of a byte-returning OS peak API. */
    *guard = RTM_STACK_GUARD_UNAVAILABLE;
    return RTM_STACK_OK;
}

int main(void) {
    mock_os_t task_os = {640, 1}, isr_os = {128, 1};
    rtm_stack_os_adapter_t task_adapter = {2048, 1, RTM_STACK_OS_PEAK_USED, mock_os_usage, &task_os};
    rtm_stack_os_adapter_t isr_adapter = {1024, 1, RTM_STACK_OS_PEAK_USED, mock_os_usage, &isr_os};
    rtm_stack_region_t cfg = {0};
    rtm_stack_t task, isr;
    rtm_stack_t *isr_to_physical_region[2] = {&isr, &isr};
    rtm_stack_usage_t s;
    cfg.owner = RTM_STACK_OS_MANAGED;
    cfg.kind = RTM_STACK_TASK;
    cfg.os_capacity_bytes = task_adapter.capacity_bytes;
    cfg.query = rtm_stack_os_query;
    cfg.user = &task_adapter;
    if (rtm_stack_init(&task, &cfg) || rtm_stack_measure(&task, &s)) return 1;
    if (s.peak_used_bytes != 640 || s.remaining_bytes != 1408) return 2;
    printf("Mock OS task stack: capacity=%zu peak=%zu remaining=%zu usage=%.2f%%\n",
           s.capacity_bytes, s.peak_used_bytes, s.remaining_bytes, s.usage_percent);
    cfg.region_id = 1;
    cfg.shared = 1;
    cfg.kind = RTM_STACK_ISR;
    cfg.os_capacity_bytes = isr_adapter.capacity_bytes;
    cfg.user = &isr_adapter;
    if (rtm_stack_init(&isr, &cfg) || rtm_stack_measure(isr_to_physical_region[1], &s)) return 3;
    if (!s.shared || isr_to_physical_region[0] != isr_to_physical_region[1]) return 4;
    printf("Mock shared ISR stack: core=%u users=2 peak=%zu usage=%.2f%%\n",
           (unsigned)s.core_id, s.peak_used_bytes, s.usage_percent);
    if (rtm_stack_paint(&isr) != RTM_STACK_E_UNSUPPORTED) return 5;
    task_os.ready = 0;
    if (rtm_stack_measure(&task, &s) != RTM_STACK_E_STATE || s.valid) return 6;
    printf("OS query unavailable: valid=%u; no stack memory read or painting\n", (unsigned)s.valid);
    return 0;
}
