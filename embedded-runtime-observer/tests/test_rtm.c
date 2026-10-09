/* SPDX-License-Identifier: MIT */
#include "rtm/rtm.h"
#include "rtm/port.h"
#include <stdio.h>
#include <stdlib.h>
#define ASSERT(x) do { if (!(x)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); exit(1); } } while (0)
#define OK(x) ASSERT((x) == RTM_OK)
static rtm_core_t c;
static rtm_snapshot_t s;
static void start(rtm_id_t task) {
    rtm_config_t cfg = {100, 100, 0, 32, 1};
    OK(rtm_init(&c, &cfg, 0, task));
}
static void invariant(const rtm_times_t *t) {
    ASSERT(t->total_ticks == t->idle_ticks + t->task_ticks + t->isr_ticks + t->kernel_ticks);
    ASSERT(rtm_cpu_load(t) >= 0.0 && rtm_cpu_load(t) <= 100.0);
}
static void snap(uint32_t now) {
    OK(rtm_snapshot(&c, now, &s));
    invariant(&s.lifetime); invariant(&s.current_window.time); invariant(&s.last_window.time);
}
static void all_idle(void) {
    start(RTM_IDLE); snap(100);
    ASSERT(s.last_window.time.idle_ticks == 100 && rtm_cpu_load(&s.lifetime) == 0.0);
}
static void all_task(void) {
    start(0); snap(100);
    ASSERT(s.task[0].exclusive_ticks == 100 && rtm_cpu_load(&s.lifetime) == 100.0);
}
static void thirty_percent(void) {
    start(0); OK(rtm_task_switch(&c, 20, RTM_IDLE));
    OK(rtm_isr_enter(&c, 50, 0)); OK(rtm_isr_exit(&c, 60, 0)); snap(100);
    ASSERT(s.lifetime.task_ticks == 20 && s.lifetime.idle_ticks == 70);
    ASSERT(s.lifetime.isr_ticks == 10 && rtm_cpu_load(&s.lifetime) == 30.0);
}
static void isr_preempts_idle(void) {
    start(RTM_IDLE); OK(rtm_isr_enter(&c, 10, 0));
    OK(rtm_isr_exit(&c, 40, 0)); snap(100);
    ASSERT(s.lifetime.idle_ticks == 70 && s.isr[0].exclusive_ticks == 30);
}
static void isr_preempts_task(void) {
    start(0); OK(rtm_isr_enter(&c, 20, 0)); OK(rtm_isr_exit(&c, 40, 0));
    OK(rtm_task_switch(&c, 100, RTM_IDLE)); snap(100);
    ASSERT(s.task[0].exclusive_ticks == 80 && s.task[0].completed == 1);
    ASSERT(s.task[0].max_exclusive_ticks == 80 && s.isr[0].exclusive_ticks == 20);
}
static void nested_isr(void) {
    start(RTM_IDLE); OK(rtm_isr_enter(&c, 10, 0)); OK(rtm_isr_enter(&c, 20, 1));
    OK(rtm_isr_exit(&c, 30, 1)); OK(rtm_isr_exit(&c, 50, 0)); snap(100);
    ASSERT(s.isr[0].inclusive_ticks == 40 && s.isr[0].exclusive_ticks == 30);
    ASSERT(s.isr[1].inclusive_ticks == 10 && s.lifetime.isr_ticks == 40);
    ASSERT(s.isr[0].completed_inclusive_ticks == 40);
}
static void recursive_isr(void) {
    start(0); OK(rtm_isr_enter(&c, 10, 0)); OK(rtm_isr_enter(&c, 20, 0));
    OK(rtm_isr_exit(&c, 30, 0)); OK(rtm_isr_exit(&c, 50, 0)); snap(100);
    ASSERT(s.isr[0].inclusive_ticks == 50 && s.isr[0].exclusive_ticks == 40);
    ASSERT(s.isr[0].completed == 2 && s.isr[0].min_inclusive_ticks == 10);
}
static void idle_isr_task(void) {
    start(RTM_IDLE); OK(rtm_isr_enter(&c, 10, 0)); OK(rtm_isr_exit(&c, 30, 0));
    OK(rtm_task_switch(&c, 30, 0)); snap(100);
    ASSERT(s.lifetime.idle_ticks == 10 && s.lifetime.task_ticks == 70 && s.lifetime.isr_ticks == 20);
}
static void task_isr_idle(void) {
    start(0); OK(rtm_isr_enter(&c, 10, 0)); OK(rtm_isr_exit(&c, 30, 0));
    OK(rtm_task_switch(&c, 30, RTM_IDLE)); snap(100);
    ASSERT(s.lifetime.task_ticks == 10 && s.lifetime.idle_ticks == 70);
}
static void idle_crosses_windows(void) {
    unsigned int i;
    start(RTM_IDLE);
    for (i = 1; i <= 4; ++i) { snap(i * 100); ASSERT(s.last_window.time.idle_ticks == 100); }
    ASSERT(s.windows_completed == 4 && s.lifetime.total_ticks == 400);
}
static void task_crosses_windows(void) {
    start(0); snap(100); snap(200); snap(250);
    ASSERT(s.last_window.begin_tick == 100 && s.current_window.time.task_ticks == 50);
    ASSERT(s.task[0].exclusive_ticks == 250 && s.task[0].completed == 0);
    OK(rtm_task_switch(&c, 250, RTM_IDLE)); snap(250);
    ASSERT(s.task[0].completed_exclusive_ticks == 250);
}
static void isr_crosses_windows(void) {
    start(RTM_IDLE); OK(rtm_isr_enter(&c, 50, 0)); snap(100); snap(200);
    ASSERT(s.last_window.time.isr_ticks == 100 && s.isr[0].completed == 0);
    OK(rtm_isr_exit(&c, 250, 0)); snap(300);
    ASSERT(s.isr[0].inclusive_ticks == 200 && s.last_window.time.isr_ticks == 50);
}
static void split_at_boundary(void) {
    start(0); OK(rtm_task_switch(&c, 80, RTM_IDLE)); snap(150);
    ASSERT(s.last_window.time.task_ticks == 80 && s.last_window.time.idle_ticks == 20);
    ASSERT(s.current_window.time.idle_ticks == 50);
}
static void timer_wrap(void) {
    rtm_config_t cfg = {100, 100, 0, 8, 1};
    OK(rtm_init(&c, &cfg, 250, 0)); OK(rtm_task_switch(&c, 4, RTM_IDLE)); snap(94);
    ASSERT(s.lifetime.total_ticks == 100 && s.lifetime.task_ticks == 10);
}
static void multiple_wraps(void) {
    rtm_config_t cfg = {100, 100, 0, 8, 1};
    unsigned int i;
    OK(rtm_init(&c, &cfg, 200, RTM_IDLE));
    for (i = 1; i <= 20; ++i) snap((200 + i * 50) & 255u);
    ASSERT(s.lifetime.total_ticks == 1000 && s.windows_completed == 10);
}
static void sixty_second_window(void) {
    rtm_config_t cfg = {UINT64_C(6000000000), 1000000000u, 0, 32, 1};
    unsigned int i;
    OK(rtm_init(&c, &cfg, 0, 0));
    for (i = 1; i <= 6; ++i) snap((uint32_t)(UINT64_C(1000000000) * i));
    ASSERT(s.last_window.time.task_ticks == UINT64_C(6000000000) && s.windows_completed == 1);
}
static void clock_gap_fault(void) {
    start(0); ASSERT(rtm_poll(&c, 101) == RTM_E_CLOCK);
    ASSERT(!c.stats.valid && c.stats.fault == RTM_E_CLOCK);
    ASSERT(rtm_snapshot(&c, 102, &s) == RTM_E_STATE && !s.valid);
}
static void backward_clock_fault(void) {
    start(0); OK(rtm_poll(&c, 10)); ASSERT(rtm_poll(&c, 9) == RTM_E_CLOCK);
}
static void nested_points(void) {
    start(0); OK(rtm_point_begin(&c, 0, 0)); OK(rtm_point_begin(&c, 10, 1));
    OK(rtm_point_end(&c, 30, 1)); OK(rtm_point_end(&c, 50, 0)); snap(50);
    ASSERT(s.point[0].inclusive_ticks == 50 && s.point[0].exclusive_ticks == 30);
    ASSERT(s.point[1].exclusive_ticks == 20 && s.point[0].max_exclusive_ticks == 30);
}
static void recursive_points(void) {
    start(0); OK(rtm_point_begin(&c, 0, 0)); OK(rtm_point_begin(&c, 10, 0));
    OK(rtm_point_end(&c, 30, 0)); OK(rtm_point_end(&c, 50, 0)); snap(50);
    ASSERT(s.point[0].inclusive_ticks == 70 && s.point[0].exclusive_ticks == 50);
    ASSERT(s.point[0].completed == 2 && rtm_average_exclusive(&s.point[0]) == 25.0);
}
static void points_survive_preemption(void) {
    start(0); OK(rtm_point_begin(&c, 0, 0)); OK(rtm_isr_enter(&c, 10, 0));
    OK(rtm_point_begin(&c, 10, 0)); OK(rtm_point_end(&c, 20, 0));
    OK(rtm_isr_exit(&c, 20, 0)); OK(rtm_point_end(&c, 30, 0)); snap(30);
    ASSERT(s.point[0].inclusive_ticks == 30 && s.point[0].completed == 2);
    ASSERT(s.point[0].max_exclusive_ticks == 20 && s.task[0].exclusive_ticks == 20);
}
static void points_survive_task_block(void) {
    start(0); OK(rtm_point_begin(&c, 0, 0)); OK(rtm_task_switch(&c, 10, 1));
    OK(rtm_point_begin(&c, 10, 1)); OK(rtm_point_end(&c, 20, 1));
    OK(rtm_task_switch(&c, 20, 0)); OK(rtm_point_end(&c, 30, 0)); snap(30);
    ASSERT(s.point[0].exclusive_ticks == 20 && s.point[1].exclusive_ticks == 10);
}
static void point_mismatch(void) {
    start(0); OK(rtm_point_begin(&c, 0, 0));
    ASSERT(rtm_point_end(&c, 10, 1) == RTM_E_STATE && !c.stats.valid);
}
static void missing_begin(void) {
    start(0); ASSERT(rtm_point_end(&c, 10, 0) == RTM_E_STATE);
}
static void unclosed_isr_point(void) {
    start(0); OK(rtm_isr_enter(&c, 0, 0)); OK(rtm_point_begin(&c, 0, 0));
    ASSERT(rtm_isr_exit(&c, 10, 0) == RTM_E_STATE);
}
static void isr_mismatch(void) {
    start(0); OK(rtm_isr_enter(&c, 0, 0)); ASSERT(rtm_isr_exit(&c, 1, 1) == RTM_E_STATE);
    start(0); ASSERT(rtm_isr_exit(&c, 0, 0) == RTM_E_STATE);
}
static void depth_overflow(void) {
    unsigned int i;
    start(0); for (i = 0; i < RTM_DEPTH; ++i) OK(rtm_isr_enter(&c, 0, 0));
    ASSERT(rtm_isr_enter(&c, 0, 0) == RTM_E_DEPTH);
    start(0); for (i = 0; i < RTM_DEPTH; ++i) OK(rtm_point_begin(&c, 0, 0));
    ASSERT(rtm_point_begin(&c, 0, 0) == RTM_E_DEPTH);
}
static void bad_identifiers(void) {
    start(0); ASSERT(rtm_task_switch(&c, 0, RTM_TASKS) == RTM_E_ID);
    start(0); ASSERT(rtm_isr_enter(&c, 0, RTM_ISRS) == RTM_E_ID);
    start(0); ASSERT(rtm_point_begin(&c, 0, RTM_POINTS) == RTM_E_ID);
}
static void illegal_isr_task_switch(void) {
    start(0); OK(rtm_isr_enter(&c, 0, 0)); ASSERT(rtm_task_switch(&c, 1, RTM_IDLE) == RTM_E_STATE);
}
static void kernel_accounting(void) {
    start(0); OK(rtm_task_switch(&c, 10, RTM_KERNEL));
    OK(rtm_isr_enter(&c, 20, 0)); OK(rtm_isr_exit(&c, 30, 0));
    OK(rtm_task_switch(&c, 40, RTM_IDLE)); snap(100);
    ASSERT(s.lifetime.kernel_ticks == 20 && rtm_cpu_load(&s.lifetime) == 40.0);
}
static void empty_and_repeated_snapshot(void) {
    start(0); snap(0); ASSERT(rtm_cpu_load(&s.lifetime) == 0.0);
    snap(50); snap(50); ASSERT(s.lifetime.total_ticks == 50 && s.task[0].completed == 0);
    ASSERT(rtm_average_exclusive(&s.task[0]) == 0.0);
}
static void reset_recovers(void) {
    start(0); OK(rtm_isr_enter(&c, 10, 0)); ASSERT(rtm_isr_exit(&c, 20, 1) == RTM_E_STATE);
    OK(rtm_reset(&c, 20, RTM_IDLE)); snap(100);
    ASSERT(s.valid && s.lifetime.total_ticks == 80 && s.lifetime.idle_ticks == 80);
}
static void counter_overflow(void) {
    start(0); c.stats.lifetime.total_ticks = UINT64_MAX;
    ASSERT(rtm_poll(&c, 1) == RTM_E_OVERFLOW);
    start(0); OK(rtm_point_begin(&c, 0, 0)); c.stats.point[0].exclusive_ticks = UINT64_MAX;
    ASSERT(rtm_poll(&c, 1) == RTM_E_OVERFLOW && !c.stats.valid);
}
static void trace_loss(void) {
    rtm_trace_event_t e;
    start(0);
#if RTM_TRACE_CAPACITY > 0
    { unsigned int i;
    for (i = 0; i < RTM_TRACE_CAPACITY + 3; ++i) OK(rtm_user_event(&c, 0, 0, i));
    ASSERT(c.stats.trace_dropped == 3);
    for (i = 0; i < RTM_TRACE_CAPACITY; ++i) {
        OK(rtm_trace_pop(&c, &e)); ASSERT(e.sequence == i && e.value == i);
    }
    ASSERT(rtm_trace_pop(&c, &e) == RTM_E_EMPTY);
    OK(rtm_user_event(&c, 1, 0, 0)); OK(rtm_trace_pop(&c, &e));
    ASSERT(e.sequence == RTM_TRACE_CAPACITY + 3); }
#else
    OK(rtm_user_event(&c, 1, 0, 0)); ASSERT(rtm_trace_pop(&c, &e) == RTM_E_EMPTY);
#endif
}
static void bad_config(void) {
    rtm_config_t cfg = {100, 100, 0, 8, 1};
    ASSERT(rtm_init(NULL, &cfg, 0, 0) == RTM_E_CONFIG);
    cfg.counter_bits = 0; ASSERT(rtm_init(&c, &cfg, 0, 0) == RTM_E_CONFIG);
    cfg.counter_bits = 1; ASSERT(rtm_init(&c, &cfg, 0, 0) == RTM_E_CONFIG);
    cfg.counter_bits = 8; cfg.max_gap_ticks = 128;
    ASSERT(rtm_init(&c, &cfg, 0, 0) == RTM_E_CONFIG);
    cfg.max_gap_ticks = 100; cfg.window_ticks = 99;
    ASSERT(rtm_init(&c, &cfg, 0, 0) == RTM_E_CONFIG);
}
typedef struct { unsigned int locked, reads; } lock_mock_t;
static rtm_lock_token_t enter(void *u) {
    lock_mock_t *m = u; ASSERT(!m->locked); m->locked = 1; return 42;
}
static void leave(void *u, rtm_lock_token_t token) {
    lock_mock_t *m = u; ASSERT(m->locked && token == 42); m->locked = 0;
}
static uint32_t read_clock(void *u) {
    lock_mock_t *m = u; ASSERT(m->locked); return ++m->reads;
}
static void port_ordering(void) {
    lock_mock_t m = {0, 0}; rtm_port_t p = {&c, &m, enter, leave, read_clock};
    start(0); OK(rtm_port_event(&p, RTM_EVENT_ISR_ENTER, 0, 0));
    OK(rtm_port_event(&p, RTM_EVENT_ISR_EXIT, 0, 0)); OK(rtm_port_snapshot(&p, &s));
    ASSERT(!m.locked && s.lifetime.total_ticks == 3 && s.lifetime.isr_ticks == 1);
    ASSERT(rtm_port_event(&p, (rtm_event_kind_t)99, 0, 0) == RTM_E_CONFIG);
    ASSERT(!m.locked); p.enter = NULL; ASSERT(rtm_port_poll(&p) == RTM_E_CONFIG);
}
/* An independent tick-by-tick oracle: counts the prior executing owner, then
   feeds random valid transitions. No use of core's accounting internals. */
static void random_conservation(void) {
    uint32_t rng = 0x12345678u, tick;
    unsigned int depth = 0, task = 0;
    uint64_t idle = 0, tasks = 0, isrs = 0;
    start(RTM_IDLE);
    for (tick = 1; tick <= 100000; ++tick) {
        if (depth) ++isrs; else if (task) ++tasks; else ++idle;
        rng = rng * 1664525u + 1013904223u;
        if ((rng & 7u) == 0 && depth < RTM_DEPTH) {
            OK(rtm_isr_enter(&c, tick, (rtm_id_t)depth)); ++depth;
        } else if ((rng & 7u) < 3 && depth) {
            --depth; OK(rtm_isr_exit(&c, tick, (rtm_id_t)depth));
        } else if (!depth) {
            task = (rng >> 8) & 1u; OK(rtm_task_switch(&c, tick, task ? 0 : RTM_IDLE));
        } else OK(rtm_poll(&c, tick));
        if (tick % 100 == 0) {
            snap(tick); ASSERT(s.lifetime.idle_ticks == idle);
            ASSERT(s.lifetime.task_ticks == tasks && s.lifetime.isr_ticks == isrs);
        }
    }
}
int main(void) {
    unsigned int count = 0;
#define RUN(test) do { test(); ++count; printf("PASS %s\n", #test); } while (0)
    RUN(all_idle); RUN(all_task); RUN(thirty_percent); RUN(isr_preempts_idle);
    RUN(isr_preempts_task); RUN(nested_isr); RUN(recursive_isr); RUN(idle_isr_task);
    RUN(task_isr_idle); RUN(idle_crosses_windows); RUN(task_crosses_windows);
    RUN(isr_crosses_windows); RUN(split_at_boundary); RUN(timer_wrap); RUN(multiple_wraps);
    RUN(sixty_second_window); RUN(clock_gap_fault); RUN(backward_clock_fault);
    RUN(nested_points); RUN(recursive_points); RUN(points_survive_preemption);
    RUN(points_survive_task_block); RUN(point_mismatch); RUN(missing_begin);
    RUN(unclosed_isr_point); RUN(isr_mismatch); RUN(depth_overflow); RUN(bad_identifiers);
    RUN(illegal_isr_task_switch); RUN(kernel_accounting); RUN(empty_and_repeated_snapshot);
    RUN(reset_recovers); RUN(counter_overflow); RUN(trace_loss); RUN(bad_config);
    RUN(port_ordering); RUN(random_conservation);
    printf("%u contract scenarios passed\n", count);
    return 0;
}
