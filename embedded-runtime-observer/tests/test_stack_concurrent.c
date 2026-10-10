/* SPDX-License-Identifier: MIT */
#include "rtm/stack.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) abort(); } while (0)
#define OPERATIONS 50000
typedef struct {
    pthread_mutex_t mutex;
    unsigned char memory[516];
    rtm_stack_t stack;
} region_t;
static region_t regions[2];
static void *writer(void *arg) {
    region_t *r = arg;
    unsigned i;
    for (i = 0; i < OPERATIONS; ++i) {
        rtm_stack_usage_t s;
        size_t used = (i % 128 + 1) * 4;
        CHECK(pthread_mutex_lock(&r->mutex) == 0);
        memset(r->memory + 4 + 512 - used, 0x55, used);
        CHECK(rtm_stack_measure(&r->stack, &s) == RTM_STACK_OK);
        CHECK(s.valid && s.peak_used_bytes >= used && s.capacity_bytes == 512);
        CHECK(pthread_mutex_unlock(&r->mutex) == 0);
    }
    return NULL;
}
static void *reader(void *arg) {
    size_t previous[2] = {0, 0};
    unsigned i;
    (void)arg;
    for (i = 0; i < OPERATIONS; ++i) {
        unsigned index = i % 2;
        region_t *r = &regions[index];
        rtm_stack_usage_t s;
        CHECK(pthread_mutex_lock(&r->mutex) == 0);
        CHECK(rtm_stack_snapshot(&r->stack, &s) == RTM_STACK_OK);
        CHECK(s.valid && s.region_id == index && s.peak_used_bytes >= previous[index]);
        CHECK(s.peak_used_bytes + s.remaining_bytes == 512 && s.guard == RTM_STACK_GUARD_INTACT);
        previous[index] = s.peak_used_bytes;
        CHECK(pthread_mutex_unlock(&r->mutex) == 0);
    }
    return NULL;
}
int main(void) {
    pthread_t threads[4];
    unsigned i;
    for (i = 0; i < 2; ++i) {
        rtm_stack_region_t c = {0};
        c.owner = RTM_STACK_APPLICATION;
        c.memory = regions[i].memory;
        c.region_bytes = sizeof(regions[i].memory);
        c.guard_bytes = c.cell_bytes = 4;
        c.region_id = (uint16_t)i;
        memset(c.fill_pattern, 0xaa, sizeof(c.fill_pattern));
        memset(c.guard_pattern, 0xde, sizeof(c.guard_pattern));
        CHECK(pthread_mutex_init(&regions[i].mutex, NULL) == 0);
        CHECK(rtm_stack_init(&regions[i].stack, &c) == RTM_STACK_OK);
        CHECK(rtm_stack_paint(&regions[i].stack) == RTM_STACK_OK);
        CHECK(pthread_create(&threads[i], NULL, writer, &regions[i]) == 0);
    }
    CHECK(pthread_create(&threads[2], NULL, reader, NULL) == 0);
    CHECK(pthread_create(&threads[3], NULL, reader, NULL) == 0);
    for (i = 0; i < 4; ++i) CHECK(pthread_join(threads[i], NULL) == 0);
    for (i = 0; i < 2; ++i) CHECK(pthread_mutex_destroy(&regions[i].mutex) == 0);
    printf("PASS stack: 2 regions, 2 writers, 2 readers, %u serialized operations\n", 4u * OPERATIONS);
    return 0;
}
