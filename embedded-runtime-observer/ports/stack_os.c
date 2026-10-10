/* SPDX-License-Identifier: MIT */
#include "rtm/stack_os.h"

rtm_stack_status_t rtm_stack_os_query(void *adapter,
                                      rtm_stack_provider_sample_t *sample) {
    const rtm_stack_os_adapter_t *os = adapter;
    size_t units = 0, bytes;
    rtm_stack_status_t status;
    rtm_stack_guard_t guard = RTM_STACK_GUARD_UNAVAILABLE;
    if (os == NULL || sample == NULL || os->capacity_bytes == 0 || os->bytes_per_unit == 0 ||
        (os->semantics != RTM_STACK_OS_PEAK_USED && os->semantics != RTM_STACK_OS_MIN_FREE))
        return RTM_STACK_E_CONFIG;
    if (os->read == NULL) return RTM_STACK_E_UNSUPPORTED;
    status = os->read(os->user, &units, &guard);
    if (status != RTM_STACK_OK) return status;
    if (units > SIZE_MAX / os->bytes_per_unit) return RTM_STACK_E_OVERFLOW;
    bytes = units * os->bytes_per_unit;
    if (bytes > os->capacity_bytes) return RTM_STACK_E_RANGE;
    sample->capacity_bytes = os->capacity_bytes;
    sample->peak_used_bytes = os->semantics == RTM_STACK_OS_PEAK_USED ? bytes : os->capacity_bytes - bytes;
    sample->guard = guard;
    return RTM_STACK_OK;
}
