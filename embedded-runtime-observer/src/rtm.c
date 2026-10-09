/* SPDX-License-Identifier: MIT */
#include "rtm/rtm.h"
#include <string.h>

static int task_id_valid(rtm_id_t id) {
    return id < RTM_TASKS || id == RTM_IDLE || id == RTM_KERNEL;
}
static rtm_status_t fail(rtm_core_t *c, rtm_status_t error) {
    if (c->stats.valid) {
        c->stats.fault = error;
        c->stats.valid = 0;
    }
    return error;
}
static void add(rtm_core_t *c, uint64_t *value, uint64_t delta) {
    if (UINT64_MAX - *value < delta) {
        *value = UINT64_MAX;
        (void)fail(c, RTM_E_OVERFLOW);
    } else {
        *value += delta;
    }
}
static rtm_point_scope_t *scope(rtm_core_t *c) {
    size_t index = c->task;
    if (c->isr_depth) {
        return &c->isr_stack[c->isr_depth - 1u].points;
    }
    if (c->task == RTM_IDLE) index = RTM_TASKS;
    if (c->task == RTM_KERNEL) index = RTM_TASKS + 1u;
    return &c->task_points[index];
}
static void sample(rtm_core_t *c, rtm_metric_t *m, uint64_t incl, uint64_t excl) {
    if (m->completed == 0 || excl < m->min_exclusive_ticks)
        m->min_exclusive_ticks = excl;
    if (m->completed == 0 || incl < m->min_inclusive_ticks)
        m->min_inclusive_ticks = incl;
    if (excl > m->max_exclusive_ticks) m->max_exclusive_ticks = excl;
    if (incl > m->max_inclusive_ticks) m->max_inclusive_ticks = incl;
    add(c, &m->completed, 1);
    add(c, &m->completed_exclusive_ticks, excl);
    add(c, &m->completed_inclusive_ticks, incl);
}
static void classify(rtm_core_t *c, rtm_times_t *t, uint64_t delta) {
    add(c, &t->total_ticks, delta);
    if (c->isr_depth) add(c, &t->isr_ticks, delta);
    else if (c->task == RTM_IDLE) add(c, &t->idle_ticks, delta);
    else if (c->task == RTM_KERNEL) add(c, &t->kernel_ticks, delta);
    else add(c, &t->task_ticks, delta);
}
static void account(rtm_core_t *c, uint32_t delta) {
    unsigned int i;
    rtm_point_scope_t *s = scope(c);
    classify(c, &c->stats.lifetime, delta);
    classify(c, &c->stats.current_window.time, delta);
    if (c->isr_depth) {
        for (i = 0; i < c->isr_depth; ++i) {
            rtm_isr_frame_t *f = &c->isr_stack[i];
            add(c, &f->inclusive, delta);
            add(c, &c->stats.isr[f->id].inclusive_ticks, delta);
        }
        {
            rtm_isr_frame_t *f = &c->isr_stack[c->isr_depth - 1u];
            add(c, &f->exclusive, delta);
            add(c, &c->stats.isr[f->id].exclusive_ticks, delta);
        }
    } else if (c->task < RTM_TASKS) {
        add(c, &c->segment_ticks, delta);
        add(c, &c->stats.task[c->task].exclusive_ticks, delta);
        add(c, &c->stats.task[c->task].inclusive_ticks, delta);
    }
    for (i = 0; i < s->depth; ++i) {
        rtm_point_frame_t *f = &s->stack[i];
        add(c, &f->inclusive, delta);
        add(c, &c->stats.point[f->id].inclusive_ticks, delta);
    }
    if (s->depth) {
        rtm_point_frame_t *f = &s->stack[s->depth - 1u];
        add(c, &f->exclusive, delta);
        add(c, &c->stats.point[f->id].exclusive_ticks, delta);
    }
}
static void trace(rtm_core_t *c, rtm_event_kind_t kind, rtm_id_t id, uint32_t val) {
#if RTM_TRACE_CAPACITY > 0
    rtm_trace_event_t *e;
    uint64_t seq = c->trace_sequence;
    if (c->trace_sequence != UINT64_MAX) ++c->trace_sequence;
    if (c->trace_count == RTM_TRACE_CAPACITY) {
        /* Drop new; existing records are preserved. Loss is explicit. */
        if (c->stats.trace_dropped != UINT64_MAX) ++c->stats.trace_dropped;
        return;
    }
    e = &c->trace[(c->trace_head + c->trace_count) % RTM_TRACE_CAPACITY];
    e->tick = c->stats.lifetime.total_ticks;
    e->sequence = seq;
    e->core_id = c->stats.config.core_id;
    e->id = id;
    e->kind = (uint8_t)kind;
    e->value = val;
    ++c->trace_count;
#else
    (void)c; (void)kind; (void)id; (void)val;
#endif
}
rtm_status_t rtm_init(rtm_core_t *c, const rtm_config_t *cfg,
                      rtm_timestamp_t now, rtm_id_t task) {
    uint64_t half;
    rtm_config_t copy;
    if (!c || !cfg) return RTM_E_CONFIG;
    if (cfg->counter_bits < 2 || cfg->counter_bits > 32) return RTM_E_CONFIG;
    half = UINT64_C(1) << (cfg->counter_bits - 1u);
    if (!cfg->window_ticks || !cfg->max_gap_ticks ||
        cfg->max_gap_ticks > cfg->window_ticks || cfg->max_gap_ticks >= half ||
        !task_id_valid(task)) return RTM_E_CONFIG;
    copy = *cfg;
    memset(c, 0, sizeof(*c));
    c->stats.config = copy;
    c->stats.valid = 1;
    c->mask = cfg->counter_bits == 32 ? UINT32_MAX :
              (uint32_t)((UINT64_C(1) << cfg->counter_bits) - 1u);
    c->previous = now & c->mask;
    c->task = task;
    return RTM_OK;
}
rtm_status_t rtm_reset(rtm_core_t *c, rtm_timestamp_t now, rtm_id_t task) {
    rtm_config_t cfg;
    if (!c) return RTM_E_CONFIG;
    cfg = c->stats.config;
    return rtm_init(c, &cfg, now, task);
}
rtm_status_t rtm_poll(rtm_core_t *c, rtm_timestamp_t now) {
    uint32_t delta;
    if (!c) return RTM_E_CONFIG;
    if (!c->stats.valid) return RTM_E_STATE;
    now &= c->mask;
    delta = (now - c->previous) & c->mask;
    if (delta > c->stats.config.max_gap_ticks) return fail(c, RTM_E_CLOCK);
    if (UINT64_MAX - c->stats.lifetime.total_ticks < delta)
        return fail(c, RTM_E_OVERFLOW);
    /* max_gap <= window means at most two account() calls and one close. */
    while (delta) {
        uint64_t room = c->stats.config.window_ticks -
                        c->stats.current_window.time.total_ticks;
        uint32_t step = delta < room ? delta : (uint32_t)room;
        account(c, step);
        delta -= step;
        if (c->stats.current_window.time.total_ticks == c->stats.config.window_ticks) {
            c->stats.last_window = c->stats.current_window;
            add(c, &c->stats.windows_completed, 1);
            memset(&c->stats.current_window, 0, sizeof(c->stats.current_window));
            c->stats.current_window.begin_tick = c->stats.lifetime.total_ticks;
            c->stats.current_window.number = c->stats.windows_completed;
        }
    }
    c->previous = now;
    return c->stats.valid ? RTM_OK : c->stats.fault;
}
rtm_status_t rtm_task_switch(rtm_core_t *c, rtm_timestamp_t now, rtm_id_t next) {
    rtm_status_t rc;
    if (!c) return RTM_E_CONFIG;
    if (!task_id_valid(next)) return fail(c, RTM_E_ID);
    if (c->isr_depth) return fail(c, RTM_E_STATE);
    rc = rtm_poll(c, now);
    if (rc != RTM_OK) return rc;
    if (c->task != next) {
        if (c->task < RTM_TASKS)
            sample(c, &c->stats.task[c->task], c->segment_ticks, c->segment_ticks);
        c->segment_ticks = 0;
        c->task = next;
        trace(c, RTM_EVENT_SWITCH, next, 0);
    }
    return c->stats.valid ? RTM_OK : c->stats.fault;
}
rtm_status_t rtm_isr_enter(rtm_core_t *c, rtm_timestamp_t now, rtm_id_t id) {
    rtm_status_t rc;
    rtm_isr_frame_t *f;
    if (!c) return RTM_E_CONFIG;
    if (id >= RTM_ISRS) return fail(c, RTM_E_ID);
    if (c->isr_depth == RTM_DEPTH) return fail(c, RTM_E_DEPTH);
    rc = rtm_poll(c, now);
    if (rc != RTM_OK) return rc;
    f = &c->isr_stack[c->isr_depth++];
    memset(f, 0, sizeof(*f));
    f->id = id;
    trace(c, RTM_EVENT_ISR_ENTER, id, 0);
    return RTM_OK;
}
rtm_status_t rtm_isr_exit(rtm_core_t *c, rtm_timestamp_t now, rtm_id_t id) {
    rtm_status_t rc;
    rtm_isr_frame_t *f;
    if (!c) return RTM_E_CONFIG;
    if (!c->isr_depth) return fail(c, RTM_E_STATE);
    f = &c->isr_stack[c->isr_depth - 1u];
    if (id != f->id || f->points.depth) return fail(c, RTM_E_STATE);
    rc = rtm_poll(c, now);
    if (rc != RTM_OK) return rc;
    sample(c, &c->stats.isr[id], f->inclusive, f->exclusive);
    --c->isr_depth;
    trace(c, RTM_EVENT_ISR_EXIT, id, 0);
    return c->stats.valid ? RTM_OK : c->stats.fault;
}
rtm_status_t rtm_point_begin(rtm_core_t *c, rtm_timestamp_t now, rtm_id_t id) {
    rtm_status_t rc;
    rtm_point_scope_t *s;
    rtm_point_frame_t *f;
    if (!c) return RTM_E_CONFIG;
    if (!c->stats.valid) return RTM_E_STATE;
    if (id >= RTM_POINTS) return fail(c, RTM_E_ID);
    s = scope(c);
    if (s->depth == RTM_DEPTH) return fail(c, RTM_E_DEPTH);
    rc = rtm_poll(c, now);
    if (rc != RTM_OK) return rc;
    f = &s->stack[s->depth++];
    memset(f, 0, sizeof(*f));
    f->id = id;
    trace(c, RTM_EVENT_POINT_BEGIN, id, 0);
    return RTM_OK;
}
rtm_status_t rtm_point_end(rtm_core_t *c, rtm_timestamp_t now, rtm_id_t id) {
    rtm_status_t rc;
    rtm_point_scope_t *s;
    rtm_point_frame_t *f;
    if (!c) return RTM_E_CONFIG;
    if (!c->stats.valid) return RTM_E_STATE;
    s = scope(c);
    if (!s->depth || s->stack[s->depth - 1u].id != id) return fail(c, RTM_E_STATE);
    rc = rtm_poll(c, now);
    if (rc != RTM_OK) return rc;
    f = &s->stack[s->depth - 1u];
    sample(c, &c->stats.point[id], f->inclusive, f->exclusive);
    --s->depth;
    trace(c, RTM_EVENT_POINT_END, id, 0);
    return c->stats.valid ? RTM_OK : c->stats.fault;
}
rtm_status_t rtm_snapshot(rtm_core_t *c, rtm_timestamp_t now, rtm_snapshot_t *out) {
    rtm_status_t rc;
    if (!c || !out) return RTM_E_CONFIG;
    rc = rtm_poll(c, now);
    *out = c->stats;
    return rc;
}
rtm_status_t rtm_user_event(rtm_core_t *c, rtm_timestamp_t now,
                           rtm_id_t id, uint32_t value) {
    rtm_status_t rc = rtm_poll(c, now);
    if (rc == RTM_OK) trace(c, RTM_EVENT_USER, id, value);
    return rc;
}
rtm_status_t rtm_trace_pop(rtm_core_t *c, rtm_trace_event_t *out) {
    if (!c || !out) return RTM_E_CONFIG;
#if RTM_TRACE_CAPACITY > 0
    if (!c->trace_count) return RTM_E_EMPTY;
    *out = c->trace[c->trace_head];
    c->trace_head = (c->trace_head + 1u) % RTM_TRACE_CAPACITY;
    --c->trace_count;
    return RTM_OK;
#else
    return RTM_E_EMPTY;
#endif
}
double rtm_cpu_load(const rtm_times_t *t) {
    if (!t || !t->total_ticks || t->idle_ticks > t->total_ticks) return 0.0;
    return 100.0 * ((double)(t->total_ticks - t->idle_ticks) / (double)t->total_ticks);
}
double rtm_average_exclusive(const rtm_metric_t *m) {
    if (!m || !m->completed) return 0.0;
    return (double)m->completed_exclusive_ticks / (double)m->completed;
}
