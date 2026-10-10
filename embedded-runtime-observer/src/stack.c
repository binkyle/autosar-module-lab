/* SPDX-License-Identifier: MIT */
#include "rtm/stack.h"
#include <string.h>

static int valid_cell(uint8_t bytes) {
    return bytes == 1 || bytes == 2 || bytes == 4 || bytes == 8;
}

static int valid_guard(rtm_stack_guard_t guard) {
    return guard == RTM_STACK_GUARD_NONE || guard == RTM_STACK_GUARD_INTACT ||
           guard == RTM_STACK_GUARD_DAMAGED || guard == RTM_STACK_GUARD_UNAVAILABLE;
}

static void update_numbers(rtm_stack_t *stack) {
    rtm_stack_usage_t *s = &stack->last;
    s->remaining_bytes = s->capacity_bytes - s->peak_used_bytes;
    s->usage_percent = ((double)s->peak_used_bytes / (double)s->capacity_bytes) * 100.0;
    s->warning = (uint8_t)(stack->region.warning_remaining_bytes != 0 &&
                          s->remaining_bytes <= stack->region.warning_remaining_bytes);
}

rtm_stack_status_t rtm_stack_init(rtm_stack_t *stack,
                                  const rtm_stack_region_t *region) {
    rtm_stack_region_t cfg;
    size_t available, capacity, payload = 0, guard = 0;
    if (stack == NULL) return RTM_STACK_E_CONFIG;
    /* Save config before clearing stack, even when reusing &stack->region. */
    if (region == NULL) {
        memset(stack, 0, sizeof(*stack));
        return RTM_STACK_E_CONFIG;
    }
    cfg = *region;
    memset(stack, 0, sizeof(*stack));
    if (cfg.shared > 1 || cfg.kind < RTM_STACK_TASK || cfg.kind > RTM_STACK_CUSTOM ||
        (cfg.direction != RTM_STACK_DOWN && cfg.direction != RTM_STACK_UP))
        return RTM_STACK_E_CONFIG;
    if (cfg.owner == RTM_STACK_APPLICATION) {
        if (cfg.memory == NULL || cfg.region_bytes == 0 || !valid_cell(cfg.cell_bytes) ||
            cfg.query != NULL || cfg.os_capacity_bytes != 0 ||
            cfg.reserved_low_bytes > cfg.region_bytes)
            return RTM_STACK_E_CONFIG;
        available = cfg.region_bytes - cfg.reserved_low_bytes;
        if (cfg.reserved_high_bytes > available) return RTM_STACK_E_CONFIG;
        available -= cfg.reserved_high_bytes;
        if (cfg.guard_bytes >= available || cfg.guard_bytes % cfg.cell_bytes != 0)
            return RTM_STACK_E_CONFIG;
        capacity = available - cfg.guard_bytes;
        if (capacity % cfg.cell_bytes != 0) return RTM_STACK_E_CONFIG;
        /* Reject numeric address wrap before forming any derived pointer.
           Caller still proves that region_bytes fits the actual C object. */
        if (cfg.region_bytes - 1 > UINTPTR_MAX ||
            (uintptr_t)cfg.memory > UINTPTR_MAX - (cfg.region_bytes - 1))
            return RTM_STACK_E_CONFIG;
        payload = cfg.reserved_low_bytes;
        guard = cfg.reserved_low_bytes;
        if (cfg.direction == RTM_STACK_DOWN) payload += cfg.guard_bytes;
        else guard += capacity;
    } else if (cfg.owner == RTM_STACK_OS_MANAGED) {
        if (cfg.memory != NULL || cfg.region_bytes != 0 || cfg.reserved_low_bytes != 0 ||
            cfg.reserved_high_bytes != 0 || cfg.guard_bytes != 0 || cfg.cell_bytes != 0 ||
            cfg.os_capacity_bytes == 0)
            return RTM_STACK_E_CONFIG;
        capacity = cfg.os_capacity_bytes;
    } else {
        return RTM_STACK_E_CONFIG;
    }
    if (cfg.warning_remaining_bytes > capacity) return RTM_STACK_E_CONFIG;
    stack->region = cfg;
    stack->payload_offset = payload;
    stack->guard_offset = guard;
    stack->initialized = 1;
    stack->last.capacity_bytes = capacity;
    stack->last.current_status = RTM_STACK_E_UNSUPPORTED;
    stack->last.measurement_status = RTM_STACK_E_STATE;
    stack->last.guard = RTM_STACK_GUARD_UNAVAILABLE;
    stack->last.epoch = 1;
    stack->last.region_id = cfg.region_id;
    stack->last.core_id = cfg.core_id;
    stack->last.kind = cfg.kind;
    stack->last.owner = cfg.owner;
    stack->last.shared = cfg.shared;
    update_numbers(stack);
    return RTM_STACK_OK;
}

rtm_stack_status_t rtm_stack_paint(rtm_stack_t *stack) {
    size_t i;
    unsigned char *memory;
    if (stack == NULL || !stack->initialized) return RTM_STACK_E_STATE;
    if (stack->region.owner != RTM_STACK_APPLICATION) return RTM_STACK_E_UNSUPPORTED;
    if (stack->painted && stack->last.epoch == UINT64_MAX) return RTM_STACK_E_OVERFLOW;
    memory = stack->region.memory;
    for (i = 0; i < stack->last.capacity_bytes; ++i)
        memory[stack->payload_offset + i] = stack->region.fill_pattern[i % stack->region.cell_bytes];
    for (i = 0; i < stack->region.guard_bytes; ++i)
        memory[stack->guard_offset + i] = stack->region.guard_pattern[i % stack->region.cell_bytes];
    if (stack->painted) ++stack->last.epoch;
    stack->painted = 1;
    stack->last.peak_used_bytes = 0;
    stack->last.scanned_cells = 0;
    stack->last.valid = 1;
    stack->last.measurement_status = RTM_STACK_OK;
    stack->last.guard = stack->region.guard_bytes == 0 ? RTM_STACK_GUARD_NONE : RTM_STACK_GUARD_INTACT;
    update_numbers(stack);
    return RTM_STACK_OK;
}

static int cell_matches(const rtm_stack_t *stack, size_t offset) {
    const volatile unsigned char *memory = stack->region.memory;
    size_t j;
    for (j = 0; j < stack->region.cell_bytes; ++j)
        if (memory[offset + j] != stack->region.fill_pattern[j]) return 0;
    return 1;
}

static rtm_stack_guard_t check_guard(const rtm_stack_t *stack) {
    const volatile unsigned char *memory = stack->region.memory;
    size_t i;
    if (stack->region.guard_bytes == 0) return RTM_STACK_GUARD_NONE;
    for (i = 0; i < stack->region.guard_bytes; ++i)
        if (memory[stack->guard_offset + i] != stack->region.guard_pattern[i % stack->region.cell_bytes])
            return RTM_STACK_GUARD_DAMAGED;
    return RTM_STACK_GUARD_INTACT;
}

rtm_stack_status_t rtm_stack_measure(rtm_stack_t *stack, rtm_stack_usage_t *out) {
    rtm_stack_status_t status = RTM_STACK_OK;
    rtm_stack_guard_t guard = RTM_STACK_GUARD_UNAVAILABLE;
    size_t used = 0;
    if (stack == NULL || out == NULL || !stack->initialized) return RTM_STACK_E_STATE;
    stack->last.scanned_cells = 0;
    if (stack->region.owner == RTM_STACK_APPLICATION) {
        if (!stack->painted) status = RTM_STACK_E_STATE;
        else {
            size_t cells = stack->last.capacity_bytes / stack->region.cell_bytes;
            size_t unused = 0;
            while (unused < cells) {
                size_t index = stack->region.direction == RTM_STACK_DOWN ? unused : cells - 1 - unused;
                size_t offset = stack->payload_offset + index * stack->region.cell_bytes;
                ++stack->last.scanned_cells;
                if (!cell_matches(stack, offset)) break;
                ++unused;
            }
            used = stack->last.capacity_bytes - unused * stack->region.cell_bytes;
            guard = check_guard(stack);
        }
    } else if (stack->region.query == NULL) status = RTM_STACK_E_UNSUPPORTED;
    else {
        rtm_stack_provider_sample_t sample = {0, 0, RTM_STACK_GUARD_UNAVAILABLE};
        status = stack->region.query(stack->region.user, &sample);
        if (status == RTM_STACK_OK) {
            if (sample.capacity_bytes != stack->last.capacity_bytes ||
                sample.peak_used_bytes > stack->last.capacity_bytes) status = RTM_STACK_E_RANGE;
            else if (!valid_guard(sample.guard)) status = RTM_STACK_E_PROVIDER;
            else {
                used = sample.peak_used_bytes;
                guard = sample.guard;
            }
        }
    }
    if (status == RTM_STACK_OK) {
        if (used > stack->last.peak_used_bytes) stack->last.peak_used_bytes = used;
        if (guard == RTM_STACK_GUARD_DAMAGED) status = RTM_STACK_E_GUARD;
    }
    stack->last.guard = guard;
    stack->last.valid = (uint8_t)(status == RTM_STACK_OK);
    stack->last.measurement_status = status;
    update_numbers(stack);
    *out = stack->last;
    return status;
}

rtm_stack_status_t rtm_stack_snapshot(const rtm_stack_t *stack,
                                      rtm_stack_usage_t *out) {
    if (stack == NULL || out == NULL || !stack->initialized) return RTM_STACK_E_STATE;
    *out = stack->last;
    return out->measurement_status;
}
