/* SPDX-License-Identifier: MIT */
#ifndef RTM_PORT_H
#define RTM_PORT_H
#include "rtm/rtm.h"
typedef uintptr_t rtm_lock_token_t;
typedef struct {
    rtm_core_t *core;
    void *user;
    /* Must mask every local writer before read_clock; save/restore prior mask.
       Host ports can instead use a mutex; target ISR ports must not block. */
    rtm_lock_token_t (*enter)(void *user);
    void (*leave)(void *user, rtm_lock_token_t token);
    rtm_timestamp_t (*read_clock)(void *user);
} rtm_port_t;
rtm_status_t rtm_port_event(rtm_port_t *port, rtm_event_kind_t kind,
                           rtm_id_t id, uint32_t value);
rtm_status_t rtm_port_poll(rtm_port_t *port);
rtm_status_t rtm_port_snapshot(rtm_port_t *port, rtm_snapshot_t *out);
#endif
