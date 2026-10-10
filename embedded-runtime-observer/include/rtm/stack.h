/* SPDX-License-Identifier: MIT */
#ifndef RTM_STACK_H
#define RTM_STACK_H
#include <stddef.h>
#include <stdint.h>

#define RTM_STACK_PATTERN_BYTES 8
typedef enum {
    RTM_STACK_OK = 0, RTM_STACK_E_CONFIG, RTM_STACK_E_STATE,
    RTM_STACK_E_UNSUPPORTED, RTM_STACK_E_PROVIDER, RTM_STACK_E_RANGE,
    RTM_STACK_E_GUARD, RTM_STACK_E_OVERFLOW
} rtm_stack_status_t;
typedef enum { RTM_STACK_APPLICATION, RTM_STACK_OS_MANAGED } rtm_stack_owner_t;
typedef enum { RTM_STACK_DOWN, RTM_STACK_UP } rtm_stack_direction_t;
typedef enum {
    RTM_STACK_TASK, RTM_STACK_ISR, RTM_STACK_KERNEL, RTM_STACK_CUSTOM
} rtm_stack_kind_t;
typedef enum {
    RTM_STACK_GUARD_NONE, RTM_STACK_GUARD_INTACT,
    RTM_STACK_GUARD_DAMAGED, RTM_STACK_GUARD_UNAVAILABLE
} rtm_stack_guard_t;
typedef struct {
    /* Provider must report historical PEAK USED BYTES, never current SP usage. */
    size_t capacity_bytes, peak_used_bytes; /* Effective capacity must match region. */
    rtm_stack_guard_t guard;
} rtm_stack_provider_sample_t;
typedef rtm_stack_status_t (*rtm_stack_query_fn)(
    void *user, rtm_stack_provider_sample_t *sample);

typedef struct {
    uint16_t region_id, core_id;
    rtm_stack_kind_t kind;
    uint8_t shared;
    rtm_stack_owner_t owner;
    rtm_stack_direction_t direction;
    /* APPLICATION: one writable, contiguous C object [memory, memory+bytes).
       Usable interval excludes both reserves and the deepest-end guard.
       Byte accesses permit unaligned addresses without uint32_t aliasing.
       Observer, config, adapter and output objects must be outside this memory. */
    unsigned char *memory;
    size_t region_bytes, reserved_low_bytes, reserved_high_bytes, guard_bytes;
    uint8_t cell_bytes; /* 1, 2, 4 or 8; capacity/guard must be cell multiples. */
    unsigned char fill_pattern[RTM_STACK_PATTERN_BYTES];
    unsigned char guard_pattern[RTM_STACK_PATTERN_BYTES];
    /* OS_MANAGED: all memory/region/guard fields must be zero. */
    size_t os_capacity_bytes;
    rtm_stack_query_fn query;
    void *user;
    /* 0 disables warning; otherwise warning when remaining <= this value. */
    size_t warning_remaining_bytes;
} rtm_stack_region_t;

typedef struct {
    size_t capacity_bytes, peak_used_bytes, remaining_bytes;
    double usage_percent;
    size_t current_used_bytes; /* 0 with current_status=E_UNSUPPORTED. */
    rtm_stack_status_t measurement_status, current_status;
    rtm_stack_guard_t guard;
    size_t scanned_cells; /* Last payload scan, including first changed cell. */
    uint64_t epoch;
    uint16_t region_id, core_id;
    rtm_stack_kind_t kind;
    rtm_stack_owner_t owner;
    uint8_t valid, warning, shared;
} rtm_stack_usage_t;

/* Caller-owned object; no registry, heap, RTM core or Trace dependency.
   Serialize EVERY access plus buffer writes. Scan only quiescent memory.
   volatile byte reads are observations, NOT a concurrency/memory-order fence.
   Do not mutate config/internal fields after init; reinitialize to rebind. */
typedef struct {
    rtm_stack_region_t region;
    rtm_stack_usage_t last;
    size_t payload_offset, guard_offset;
    uint8_t initialized, painted;
} rtm_stack_t;

/* No memory reads/writes. Failure leaves an unusable, zeroed object. */
rtm_stack_status_t rtm_stack_init(rtm_stack_t *stack,
                                  const rtm_stack_region_t *region);
/* Destructive APPLICATION-only operation. Caller guarantees offline/unused
   memory, not the executing stack. Excludes reserves; new paint clears peak
   and starts a new epoch. Never paints an OS-managed region. */
rtm_stack_status_t rtm_stack_paint(rtm_stack_t *stack);
/* Linear early-exit watermark scan OR authorized OS callback. Historical
   maximum of OBSERVED watermarks: matching-pattern writes can be missed.
   Guard damage returns E_GUARD/valid=0 with bounded diagnostic values. */
rtm_stack_status_t rtm_stack_measure(rtm_stack_t *stack, rtm_stack_usage_t *out);
/* Copies the last measurement; no scan/provider call, no freshness claim. */
rtm_stack_status_t rtm_stack_snapshot(const rtm_stack_t *stack,
                                      rtm_stack_usage_t *out);
#endif
