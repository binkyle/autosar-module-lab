/* SPDX-License-Identifier: MIT */
#include "rtm/port.h"
static int usable(const rtm_port_t *p) {
    return p && p->core && p->enter && p->leave && p->read_clock;
}
rtm_status_t rtm_port_event(rtm_port_t *p, rtm_event_kind_t kind,
                           rtm_id_t id, uint32_t val) {
    rtm_lock_token_t token;
    rtm_timestamp_t now;
    rtm_status_t rc;
    if (!usable(p)) return RTM_E_CONFIG;
    token = p->enter(p->user);
    now = p->read_clock(p->user);
    switch (kind) {
    case RTM_EVENT_SWITCH: rc = rtm_task_switch(p->core, now, id); break;
    case RTM_EVENT_ISR_ENTER: rc = rtm_isr_enter(p->core, now, id); break;
    case RTM_EVENT_ISR_EXIT: rc = rtm_isr_exit(p->core, now, id); break;
    case RTM_EVENT_POINT_BEGIN: rc = rtm_point_begin(p->core, now, id); break;
    case RTM_EVENT_POINT_END: rc = rtm_point_end(p->core, now, id); break;
    case RTM_EVENT_USER: rc = rtm_user_event(p->core, now, id, val); break;
    default: rc = RTM_E_CONFIG; break;
    }
    p->leave(p->user, token);
    return rc;
}
rtm_status_t rtm_port_poll(rtm_port_t *p) {
    rtm_lock_token_t token;
    rtm_status_t rc;
    if (!usable(p)) return RTM_E_CONFIG;
    token = p->enter(p->user);
    rc = rtm_poll(p->core, p->read_clock(p->user));
    p->leave(p->user, token);
    return rc;
}
rtm_status_t rtm_port_snapshot(rtm_port_t *p, rtm_snapshot_t *out) {
    rtm_lock_token_t token;
    rtm_status_t rc;
    if (!usable(p) || !out) return RTM_E_CONFIG;
    token = p->enter(p->user);
    rc = rtm_snapshot(p->core, p->read_clock(p->user), out);
    p->leave(p->user, token);
    return rc;
}
