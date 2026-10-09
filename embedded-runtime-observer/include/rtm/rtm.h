/* SPDX-License-Identifier: MIT */
#ifndef RTM_RTM_H
#define RTM_RTM_H
#include <stddef.h>
#include <stdint.h>

#ifndef RTM_TASKS
#define RTM_TASKS 16
#endif
#ifndef RTM_ISRS
#define RTM_ISRS 16
#endif
#ifndef RTM_POINTS
#define RTM_POINTS 16
#endif
#ifndef RTM_DEPTH
#define RTM_DEPTH 8
#endif
#ifndef RTM_TRACE_CAPACITY
#define RTM_TRACE_CAPACITY 64
#endif
#if RTM_TASKS < 1 || RTM_TASKS > 65533 || RTM_ISRS < 1 || RTM_ISRS > 65535 || RTM_POINTS < 1 || RTM_POINTS > 65535 || RTM_DEPTH < 1 || RTM_DEPTH > 255 || RTM_TRACE_CAPACITY < 0
#error "Invalid RTM static capacities"
#endif

/* All translation units using the public structures need identical capacities. */
#define RTM_IDLE UINT16_MAX
#define RTM_KERNEL (UINT16_MAX - 1u)
typedef uint16_t rtm_id_t;
typedef uint32_t rtm_timestamp_t;
typedef enum {
    RTM_OK = 0, RTM_E_CONFIG, RTM_E_ID, RTM_E_STATE, RTM_E_DEPTH,
    RTM_E_CLOCK, RTM_E_OVERFLOW, RTM_E_EMPTY
} rtm_status_t;
typedef enum {
    RTM_EVENT_SWITCH, RTM_EVENT_ISR_ENTER, RTM_EVENT_ISR_EXIT,
    RTM_EVENT_POINT_BEGIN, RTM_EVENT_POINT_END, RTM_EVENT_USER
} rtm_event_kind_t;

typedef struct {
    uint64_t exclusive_ticks, inclusive_ticks;
    uint64_t completed, completed_exclusive_ticks;
    uint64_t min_exclusive_ticks, max_exclusive_ticks;
    uint64_t completed_inclusive_ticks, min_inclusive_ticks, max_inclusive_ticks;
} rtm_metric_t;

typedef struct {
    uint64_t total_ticks, idle_ticks, task_ticks, isr_ticks, kernel_ticks;
} rtm_times_t;

typedef struct {
    /* max_gap <= window_ticks AND < 2^(counter_bits-1). Poll within this gap. */
    uint64_t window_ticks;
    uint32_t max_gap_ticks;
    uint16_t core_id;
    uint8_t counter_bits;
    /* A port's coverage declaration; not an automatic accuracy certification. */
    uint8_t complete_coverage;
} rtm_config_t;

typedef struct {
    uint64_t begin_tick, number;
    rtm_times_t time;
} rtm_window_t;

typedef struct {
    rtm_config_t config;
    uint8_t valid;
    rtm_status_t fault;
    rtm_times_t lifetime;
    rtm_window_t current_window, last_window;
    uint64_t windows_completed, trace_dropped;
    rtm_metric_t task[RTM_TASKS], isr[RTM_ISRS], point[RTM_POINTS];
} rtm_snapshot_t;

typedef struct {
    uint64_t tick, sequence;
    uint16_t core_id, id;
    uint8_t kind;
    uint32_t value;
} rtm_trace_event_t;

typedef struct {
    rtm_id_t id;
    uint64_t inclusive, exclusive;
} rtm_point_frame_t;
typedef struct {
    uint8_t depth;
    rtm_point_frame_t stack[RTM_DEPTH];
} rtm_point_scope_t;
typedef struct {
    rtm_id_t id;
    uint64_t inclusive, exclusive;
    rtm_point_scope_t points;
} rtm_isr_frame_t;

/* Caller-owned, no allocation. One owner core; serialize every API access. */
typedef struct {
    rtm_snapshot_t stats;
    rtm_timestamp_t previous, mask;
    rtm_id_t task;
    uint64_t segment_ticks;
    uint8_t isr_depth;
    rtm_isr_frame_t isr_stack[RTM_DEPTH];
    /* Tasks keep separate stacks while preempted or blocked; two extra scopes
       belong to Idle and Kernel. ISR scopes live in invocation frames. */
    rtm_point_scope_t task_points[RTM_TASKS + 2];
#if RTM_TRACE_CAPACITY > 0
    rtm_trace_event_t trace[RTM_TRACE_CAPACITY];
    size_t trace_head, trace_count;
    uint64_t trace_sequence;
#endif
} rtm_core_t;

rtm_status_t rtm_init(rtm_core_t *core, const rtm_config_t *config,
                      rtm_timestamp_t now, rtm_id_t initial_task);
/* Reset discards all statistics AND open invocations. Establish a known state. */
rtm_status_t rtm_reset(rtm_core_t *core, rtm_timestamp_t now, rtm_id_t task);
rtm_status_t rtm_poll(rtm_core_t *core, rtm_timestamp_t now);
/* Valid only outside ISR. PostTaskHook -> KERNEL; PreTaskHook -> next Task/Idle. */
rtm_status_t rtm_task_switch(rtm_core_t *core, rtm_timestamp_t now, rtm_id_t next);
rtm_status_t rtm_isr_enter(rtm_core_t *core, rtm_timestamp_t now, rtm_id_t id);
/* LIFO; exit restores the suspended owner. A following task switch is separate. */
rtm_status_t rtm_isr_exit(rtm_core_t *core, rtm_timestamp_t now, rtm_id_t id);
/* Per-execution-owner LIFO CPU-time points, supports recursive same-ID begin. */
rtm_status_t rtm_point_begin(rtm_core_t *core, rtm_timestamp_t now, rtm_id_t id);
rtm_status_t rtm_point_end(rtm_core_t *core, rtm_timestamp_t now, rtm_id_t id);
rtm_status_t rtm_snapshot(rtm_core_t *core, rtm_timestamp_t now,
                         rtm_snapshot_t *out);
rtm_status_t rtm_user_event(rtm_core_t *core, rtm_timestamp_t now,
                           rtm_id_t id, uint32_t value);
/* Pop under the same serialization as the producer. NOT a lock-free queue. */
rtm_status_t rtm_trace_pop(rtm_core_t *core, rtm_trace_event_t *out);
/* Returns 0 for an empty interval; no multiplication overflow at large totals. */
double rtm_cpu_load(const rtm_times_t *time);
double rtm_average_exclusive(const rtm_metric_t *metric);

#endif
