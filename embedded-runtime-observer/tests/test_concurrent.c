/* SPDX-License-Identifier: MIT */
#include "rtm/rtm.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
typedef struct {
    pthread_mutex_t mutex;
    rtm_core_t core;
    uint32_t tick;
} shared_t;
static shared_t shared[2];
#define CHECK(x) do { if (!(x)) abort(); } while (0)
static void *writer(void *ptr) {
    shared_t *s = ptr; unsigned int i;
    for (i = 0; i < 50000; ++i) {
        CHECK(pthread_mutex_lock(&s->mutex) == 0);
        CHECK(rtm_task_switch(&s->core, ++s->tick, i & 1u ? 0 : RTM_IDLE) == RTM_OK);
        CHECK(pthread_mutex_unlock(&s->mutex) == 0);
    }
    return NULL;
}
static void *reader(void *ptr) {
    shared_t *s = ptr; rtm_snapshot_t result; unsigned int i;
    for (i = 0; i < 50000; ++i) {
        CHECK(pthread_mutex_lock(&s->mutex) == 0);
        CHECK(rtm_snapshot(&s->core, s->tick, &result) == RTM_OK);
        CHECK(pthread_mutex_unlock(&s->mutex) == 0);
        CHECK(result.valid);
        CHECK(result.lifetime.total_ticks == result.lifetime.task_ticks + result.lifetime.idle_ticks);
        CHECK(result.lifetime.total_ticks <= 50000);
    }
    return NULL;
}
int main(void) {
    pthread_t threads[4]; unsigned int i;
    for (i = 0; i < 2; ++i) {
        rtm_config_t cfg = {100, 100, (uint16_t)i, 32, 1};
        CHECK(pthread_mutex_init(&shared[i].mutex, NULL) == 0);
        CHECK(rtm_init(&shared[i].core, &cfg, 0, RTM_IDLE) == RTM_OK);
        CHECK(pthread_create(&threads[2*i], NULL, writer, &shared[i]) == 0);
        CHECK(pthread_create(&threads[2*i+1], NULL, reader, &shared[i]) == 0);
    }
    for (i = 0; i < 4; ++i) CHECK(pthread_join(threads[i], NULL) == 0);
    for (i = 0; i < 2; ++i) {
        CHECK(shared[i].core.stats.lifetime.total_ticks == 50000);
        CHECK(shared[i].core.stats.lifetime.task_ticks == 24999);
        CHECK(pthread_mutex_destroy(&shared[i].mutex) == 0);
    }
    puts("PASS 2 owner cores, 2 concurrent snapshot readers, 200000 serialized operations");
    return 0;
}
