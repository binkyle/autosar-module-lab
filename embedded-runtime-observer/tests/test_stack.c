/* SPDX-License-Identifier: MIT */
#include "rtm/stack.h"
#include "rtm/stack_os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); exit(1); } } while (0)
static unsigned char memory[16432];
static rtm_stack_t stack;
static rtm_stack_usage_t result;
static rtm_stack_region_t application(size_t capacity, unsigned cell, rtm_stack_direction_t dir) {
    rtm_stack_region_t c = {0};
    c.owner = RTM_STACK_APPLICATION;
    c.kind = RTM_STACK_TASK;
    c.memory = memory;
    c.region_bytes = capacity + cell;
    c.guard_bytes = c.cell_bytes = (uint8_t)cell;
    c.direction = dir;
    memset(c.fill_pattern, 0xaa, sizeof(c.fill_pattern));
    memset(c.guard_pattern, 0xde, sizeof(c.guard_pattern));
    return c;
}
static void start(size_t capacity, unsigned cell, rtm_stack_direction_t dir) {
    rtm_stack_region_t c = application(capacity, cell, dir);
    CHECK(rtm_stack_init(&stack, &c) == RTM_STACK_OK);
    CHECK(rtm_stack_paint(&stack) == RTM_STACK_OK);
}
static void use(size_t bytes) {
    size_t begin = stack.region.direction == RTM_STACK_DOWN ? stack.last.capacity_bytes - bytes : 0;
    memset(stack.region.memory + stack.payload_offset + begin, 0x55, bytes);
}
static void measure(size_t expected) {
    CHECK(rtm_stack_measure(&stack, &result) == RTM_STACK_OK);
    CHECK(result.valid && result.peak_used_bytes == expected);
    CHECK(result.remaining_bytes + result.peak_used_bytes == result.capacity_bytes);
    CHECK(result.current_status == RTM_STACK_E_UNSUPPORTED && result.current_used_bytes == 0);
    CHECK(result.usage_percent >= 0 && result.usage_percent <= 100);
}
static void all_painted(void) { start(2048, 4, RTM_STACK_DOWN); measure(0); CHECK(result.scanned_cells == 512); }
static void partial_usage(void) { start(2048, 4, RTM_STACK_DOWN); use(640); measure(640); CHECK(result.usage_percent == 31.25); CHECK(result.scanned_cells == 353); }
static void fully_used(void) { start(1024, 4, RTM_STACK_DOWN); use(1024); measure(1024); CHECK(result.scanned_cells == 1 && result.usage_percent == 100); }
static void retreat(void) { start(2048, 4, RTM_STACK_DOWN); use(640); measure(640); measure(640); }
static void sticky_peak(void) { start(2048, 4, RTM_STACK_DOWN); use(640); measure(640); memset(memory + 4, 0xaa, 2048); measure(640); CHECK(result.scanned_cells == 512); }
static void new_epoch(void) { start(1024, 4, RTM_STACK_DOWN); use(800); measure(800); CHECK(rtm_stack_paint(&stack) == RTM_STACK_OK); measure(0); CHECK(result.epoch == 2); }
static void sizes(void) { size_t i; const size_t sizes_[] = {1024,2048,8192,16384}; for (i=0;i<4;++i) { start(sizes_[i],4,RTM_STACK_DOWN); use(sizes_[i]/2); measure(sizes_[i]/2); } }
static void upward(void) { start(1024,4,RTM_STACK_UP); use(128); measure(128); CHECK(memory[1024] == 0xde); }
static void cells(void) { unsigned cell; for (cell=1;cell<=8;cell*=2) { start(1024,cell,RTM_STACK_DOWN); use(17); measure(((17+cell-1)/cell)*cell); } }
static void patterns(void) { rtm_stack_region_t c=application(64,4,RTM_STACK_DOWN); size_t i; for(i=0;i<4;++i)c.fill_pattern[i]=(unsigned char)(0x10+i); CHECK(rtm_stack_init(&stack,&c)==RTM_STACK_OK); CHECK(rtm_stack_paint(&stack)==RTM_STACK_OK); measure(0); use(12); measure(12); }
static void unaligned(void) { rtm_stack_region_t c=application(64,4,RTM_STACK_DOWN); c.memory=memory+1; CHECK(rtm_stack_init(&stack,&c)==RTM_STACK_OK); CHECK(rtm_stack_paint(&stack)==RTM_STACK_OK); use(16); measure(16); }
static void reserves(void) { rtm_stack_region_t c=application(64,4,RTM_STACK_DOWN); memset(memory,0x11,80); c.region_bytes=80; c.reserved_low_bytes=4; c.reserved_high_bytes=8; CHECK(rtm_stack_init(&stack,&c)==RTM_STACK_OK); CHECK(rtm_stack_paint(&stack)==RTM_STACK_OK); CHECK(memory[0]==0x11 && memory[3]==0x11 && memory[72]==0x11 && memory[79]==0x11); use(64); measure(64); }
static void guard_damage(void) { start(1024,4,RTM_STACK_DOWN); use(128); measure(128); memory[1]^=1; CHECK(rtm_stack_measure(&stack,&result)==RTM_STACK_E_GUARD); CHECK(!result.valid && result.guard==RTM_STACK_GUARD_DAMAGED && result.peak_used_bytes==128); CHECK(memory[1]!=(unsigned char)0xde); }
static void upward_guard(void) { start(1024,4,RTM_STACK_UP); memory[1027]^=1; CHECK(rtm_stack_measure(&stack,&result)==RTM_STACK_E_GUARD); CHECK(!result.valid); }
static void no_guard(void) { rtm_stack_region_t c=application(64,4,RTM_STACK_DOWN); c.region_bytes=64; c.guard_bytes=0; CHECK(rtm_stack_init(&stack,&c)==RTM_STACK_OK); CHECK(rtm_stack_paint(&stack)==RTM_STACK_OK); measure(0); CHECK(result.guard==RTM_STACK_GUARD_NONE); }
static void warning(void) { rtm_stack_region_t c=application(1024,4,RTM_STACK_DOWN); c.warning_remaining_bytes=256; CHECK(rtm_stack_init(&stack,&c)==RTM_STACK_OK); CHECK(rtm_stack_paint(&stack)==RTM_STACK_OK); use(764); measure(764); CHECK(!result.warning); use(768); measure(768); CHECK(result.warning); }
static void invalid_regions(void) { rtm_stack_region_t c=application(64,4,RTM_STACK_DOWN); c.region_bytes=0; CHECK(rtm_stack_init(&stack,&c)==RTM_STACK_E_CONFIG); c=application(64,4,RTM_STACK_DOWN); c.memory=NULL; CHECK(rtm_stack_init(&stack,&c)==RTM_STACK_E_CONFIG); c=application(64,4,RTM_STACK_DOWN); c.reserved_low_bytes=SIZE_MAX; CHECK(rtm_stack_init(&stack,&c)==RTM_STACK_E_CONFIG); c=application(64,4,RTM_STACK_DOWN); c.reserved_high_bytes=SIZE_MAX; CHECK(rtm_stack_init(&stack,&c)==RTM_STACK_E_CONFIG); c=application(64,4,RTM_STACK_DOWN); c.guard_bytes=c.region_bytes; CHECK(rtm_stack_init(&stack,&c)==RTM_STACK_E_CONFIG); }
static void invalid_cells(void) { rtm_stack_region_t c=application(64,4,RTM_STACK_DOWN); c.cell_bytes=3; CHECK(rtm_stack_init(&stack,&c)==RTM_STACK_E_CONFIG); c=application(64,4,RTM_STACK_DOWN); c.region_bytes=67; CHECK(rtm_stack_init(&stack,&c)==RTM_STACK_E_CONFIG); c=application(64,4,RTM_STACK_DOWN); c.guard_bytes=3; CHECK(rtm_stack_init(&stack,&c)==RTM_STACK_E_CONFIG); }
static void invalid_options(void) { rtm_stack_region_t c=application(64,4,RTM_STACK_DOWN); c.direction=(rtm_stack_direction_t)9; CHECK(rtm_stack_init(&stack,&c)==RTM_STACK_E_CONFIG); c=application(64,4,RTM_STACK_DOWN); c.owner=(rtm_stack_owner_t)9; CHECK(rtm_stack_init(&stack,&c)==RTM_STACK_E_CONFIG); c=application(64,4,RTM_STACK_DOWN); c.kind=(rtm_stack_kind_t)9; CHECK(rtm_stack_init(&stack,&c)==RTM_STACK_E_CONFIG); c=application(64,4,RTM_STACK_DOWN); c.warning_remaining_bytes=65; CHECK(rtm_stack_init(&stack,&c)==RTM_STACK_E_CONFIG); }
static void address_wrap(void) { rtm_stack_region_t c=application(64,4,RTM_STACK_DOWN); c.memory=(unsigned char *)(uintptr_t)(UINTPTR_MAX-16); CHECK(rtm_stack_init(&stack,&c)==RTM_STACK_E_CONFIG); }
static void null_and_unpainted(void) { rtm_stack_region_t c=application(64,4,RTM_STACK_DOWN); CHECK(rtm_stack_init(NULL,&c)==RTM_STACK_E_CONFIG); CHECK(rtm_stack_init(&stack,NULL)==RTM_STACK_E_CONFIG); CHECK(rtm_stack_measure(&stack,&result)==RTM_STACK_E_STATE); CHECK(rtm_stack_init(&stack,&c)==RTM_STACK_OK); CHECK(rtm_stack_measure(&stack,&result)==RTM_STACK_E_STATE && !result.valid); CHECK(rtm_stack_measure(&stack,NULL)==RTM_STACK_E_STATE); CHECK(rtm_stack_paint(NULL)==RTM_STACK_E_STATE); CHECK(rtm_stack_snapshot(NULL,&result)==RTM_STACK_E_STATE); }
static void pattern_collision(void) { start(64,4,RTM_STACK_DOWN); memset(memory+4+48,0xaa,16); measure(0); CHECK(result.valid); /* Real writes matching paint remain indistinguishable. */ }
static void nonmonotonic_cells(void) { start(64,4,RTM_STACK_DOWN); memory[4+8]=0x55; memory[4+48]=0x55; measure(56); /* An untouched hole must not cause a binary-search shortcut. */ }
static void byte_boundaries(void) { start(64,4,RTM_STACK_DOWN); memory[4+63]=0x55; measure(4); start(64,4,RTM_STACK_UP); memory[0]=0x55; measure(4); }
static void cached_snapshot(void) { start(64,4,RTM_STACK_DOWN); use(16); measure(16); use(32); CHECK(rtm_stack_snapshot(&stack,&result)==RTM_STACK_OK && result.peak_used_bytes==16); measure(32); }
typedef struct { size_t capacity, peak; rtm_stack_status_t status; rtm_stack_guard_t guard; unsigned calls; } provider_t;
static rtm_stack_status_t provider(void *user, rtm_stack_provider_sample_t *s) { provider_t *p=user; ++p->calls; s->capacity_bytes=p->capacity; s->peak_used_bytes=p->peak; s->guard=p->guard; return p->status; }
static rtm_stack_region_t os_config(provider_t *p) { rtm_stack_region_t c={0}; c.owner=RTM_STACK_OS_MANAGED; c.os_capacity_bytes=p->capacity; c.query=provider; c.user=p; return c; }
static void os_readonly(void) { provider_t p={2048,640,RTM_STACK_OK,RTM_STACK_GUARD_UNAVAILABLE,0}; rtm_stack_region_t c=os_config(&p); unsigned char before[sizeof(memory)]; memset(memory,0x37,sizeof(memory)); memcpy(before,memory,sizeof(memory)); CHECK(rtm_stack_init(&stack,&c)==RTM_STACK_OK); CHECK(rtm_stack_paint(&stack)==RTM_STACK_E_UNSUPPORTED); measure(640); CHECK(memcmp(before,memory,sizeof(memory))==0 && p.calls==1); CHECK(result.guard==RTM_STACK_GUARD_UNAVAILABLE); CHECK(rtm_stack_snapshot(&stack,&result)==RTM_STACK_OK && p.calls==1); }
static void os_no_api(void) { provider_t p={2048,0,RTM_STACK_OK,RTM_STACK_GUARD_NONE,0}; rtm_stack_region_t c=os_config(&p); c.query=NULL; CHECK(rtm_stack_init(&stack,&c)==RTM_STACK_OK); CHECK(rtm_stack_measure(&stack,&result)==RTM_STACK_E_UNSUPPORTED && !result.valid); }
static void os_memory_rejected(void) { provider_t p={2048,0,RTM_STACK_OK,RTM_STACK_GUARD_NONE,0}; rtm_stack_region_t c=os_config(&p); c.memory=memory; CHECK(rtm_stack_init(&stack,&c)==RTM_STACK_E_CONFIG); c=os_config(&p); c.guard_bytes=4; CHECK(rtm_stack_init(&stack,&c)==RTM_STACK_E_CONFIG); }
static void os_range_and_capacity(void) { provider_t p={2048,2049,RTM_STACK_OK,RTM_STACK_GUARD_NONE,0}; rtm_stack_region_t c=os_config(&p); CHECK(rtm_stack_init(&stack,&c)==RTM_STACK_OK); CHECK(rtm_stack_measure(&stack,&result)==RTM_STACK_E_RANGE && !result.valid && result.peak_used_bytes==0); p.peak=640; p.capacity=1024; CHECK(rtm_stack_measure(&stack,&result)==RTM_STACK_E_RANGE); }
static void os_failure(void) { provider_t p={2048,640,RTM_STACK_OK,RTM_STACK_GUARD_NONE,0}; rtm_stack_region_t c=os_config(&p); CHECK(rtm_stack_init(&stack,&c)==RTM_STACK_OK); measure(640); p.status=RTM_STACK_E_PROVIDER; CHECK(rtm_stack_measure(&stack,&result)==RTM_STACK_E_PROVIDER && !result.valid && result.peak_used_bytes==640); p.status=RTM_STACK_OK; p.peak=320; measure(640); }
static void os_guard(void) { provider_t p={2048,640,RTM_STACK_OK,RTM_STACK_GUARD_DAMAGED,0}; rtm_stack_region_t c=os_config(&p); CHECK(rtm_stack_init(&stack,&c)==RTM_STACK_OK); CHECK(rtm_stack_measure(&stack,&result)==RTM_STACK_E_GUARD && !result.valid && result.peak_used_bytes==640); p.guard=(rtm_stack_guard_t)9; CHECK(rtm_stack_measure(&stack,&result)==RTM_STACK_E_PROVIDER); }
static rtm_stack_status_t raw(void *user,size_t *units,rtm_stack_guard_t *guard) { *units=*(size_t *)user; *guard=RTM_STACK_GUARD_NONE; return RTM_STACK_OK; }
static void os_unit_conversion(void) { size_t value=352; rtm_stack_os_adapter_t a={2048,4,RTM_STACK_OS_MIN_FREE,raw,&value}; rtm_stack_provider_sample_t s; CHECK(rtm_stack_os_query(&a,&s)==RTM_STACK_OK && s.peak_used_bytes==640 && s.capacity_bytes==2048); a.semantics=RTM_STACK_OS_PEAK_USED; value=160; CHECK(rtm_stack_os_query(&a,&s)==RTM_STACK_OK && s.peak_used_bytes==640); }
static void os_unit_overflow(void) { size_t value=SIZE_MAX; rtm_stack_os_adapter_t a={2048,4,RTM_STACK_OS_MIN_FREE,raw,&value}; rtm_stack_provider_sample_t s; CHECK(rtm_stack_os_query(&a,&s)==RTM_STACK_E_OVERFLOW); value=513; CHECK(rtm_stack_os_query(&a,&s)==RTM_STACK_E_RANGE); a.read=NULL; CHECK(rtm_stack_os_query(&a,&s)==RTM_STACK_E_UNSUPPORTED); CHECK(rtm_stack_os_query(NULL,&s)==RTM_STACK_E_CONFIG); }
static void large_os_capacity(void) { provider_t p={SIZE_MAX,SIZE_MAX,RTM_STACK_OK,RTM_STACK_GUARD_NONE,0}; rtm_stack_region_t c=os_config(&p); CHECK(rtm_stack_init(&stack,&c)==RTM_STACK_OK); measure(SIZE_MAX); CHECK(result.usage_percent==100 && result.remaining_bytes==0); }
static void independent_regions(void) { rtm_stack_t a,b; unsigned char buf[68]; rtm_stack_region_t c=application(64,4,RTM_STACK_DOWN); CHECK(rtm_stack_init(&a,&c)==RTM_STACK_OK && rtm_stack_paint(&a)==RTM_STACK_OK); c.memory=buf; c.region_id=4; c.core_id=2; c.kind=RTM_STACK_ISR; c.shared=1; CHECK(rtm_stack_init(&b,&c)==RTM_STACK_OK && rtm_stack_paint(&b)==RTM_STACK_OK); memory[4+32]=0x55; CHECK(rtm_stack_measure(&a,&result)==RTM_STACK_OK && result.peak_used_bytes==32); CHECK(rtm_stack_measure(&b,&result)==RTM_STACK_OK && result.peak_used_bytes==0 && result.shared && result.region_id==4 && result.core_id==2); }
static void shared_aliases(void) { provider_t p={1024,128,RTM_STACK_OK,RTM_STACK_GUARD_NONE,0}; rtm_stack_region_t c=os_config(&p); rtm_stack_t *aliases[2]={&stack,&stack}; c.shared=1; c.kind=RTM_STACK_ISR; CHECK(rtm_stack_init(&stack,&c)==RTM_STACK_OK); CHECK(rtm_stack_measure(aliases[0],&result)==RTM_STACK_OK && result.shared); p.peak=256; CHECK(rtm_stack_measure(aliases[1],&result)==RTM_STACK_OK && result.peak_used_bytes==256 && aliases[0]==aliases[1]); }
static void rebind_config(void) { start(64,4,RTM_STACK_DOWN); CHECK(rtm_stack_init(&stack,&stack.region)==RTM_STACK_OK); CHECK(!stack.painted); CHECK(rtm_stack_paint(&stack)==RTM_STACK_OK); measure(0); }
static void epoch_overflow(void) { start(64,4,RTM_STACK_DOWN); stack.last.epoch=UINT64_MAX; memory[4]=0x55; CHECK(rtm_stack_paint(&stack)==RTM_STACK_E_OVERFLOW && memory[4]==0x55); }
static uint32_t rng=12345;
static uint32_t random_u32(void) { rng=rng*1664525u+1013904223u; return rng; }
static void randomized_byte_oracle(void) {
    unsigned cell,dir; size_t operation;
    for(cell=1;cell<=8;cell*=2) for(dir=0;dir<2;++dir) {
        size_t peak=0; start(128,cell,(rtm_stack_direction_t)dir);
        for(operation=0;operation<1000;++operation) {
            size_t i, deepest=0, changed=random_u32()%128;
            memory[stack.payload_offset+changed]=(random_u32()&1)?0x55:0xaa;
            /* Independent oracle: inspect EVERY byte, select greatest depth. */
            for(i=0;i<128;++i) if(memory[stack.payload_offset+i]!=0xaa) {
                size_t depth=dir==RTM_STACK_DOWN?128-i:i+1;
                if(depth>deepest)deepest=depth;
            }
            deepest=((deepest+cell-1)/cell)*cell;
            if(deepest>peak)peak=deepest;
            measure(peak);
        }
    }
}
typedef void (*case_fn)(void);
int main(void) {
    const struct { const char *name; case_fn run; } cases[] = {
        {"all_painted",all_painted},{"partial_640_of_2048",partial_usage},{"fully_used_early_exit",fully_used},
        {"sp_retreat_keeps_watermark",retreat},{"observed_peak_survives_restoration",sticky_peak},{"painting_new_epoch",new_epoch},
        {"1_2_8_16_kib",sizes},{"upward_growth",upward},{"1_2_4_8_byte_cells",cells},{"custom_pattern",patterns},
        {"unaligned_byte_access",unaligned},{"reserved_bytes_untouched",reserves},{"guard_damage",guard_damage},
        {"upward_guard",upward_guard},{"guard_disabled",no_guard},{"warning_threshold",warning},
        {"invalid_regions_zero_reserves",invalid_regions},{"invalid_cell_sizes",invalid_cells},{"invalid_options",invalid_options},
        {"address_wrap_rejected",address_wrap},{"null_unpainted",null_and_unpainted},{"matching_pattern_limitation",pattern_collision},
        {"nonmonotonic_paint_holes",nonmonotonic_cells},{"partial_cell_boundaries",byte_boundaries},{"cached_snapshot",cached_snapshot},
        {"os_managed_readonly",os_readonly},{"os_api_unsupported",os_no_api},{"os_memory_rejected",os_memory_rejected},
        {"os_range_capacity_mismatch",os_range_and_capacity},{"os_failure_retains_invalid_diagnostics",os_failure},
        {"os_guard_states",os_guard},{"os_used_free_unit_conversion",os_unit_conversion},{"os_unit_overflow",os_unit_overflow},
        {"large_capacity_percent",large_os_capacity},{"independent_regions",independent_regions},{"shared_stack_aliases",shared_aliases},
        {"rebind_config",rebind_config},{"epoch_overflow",epoch_overflow},{"randomized_byte_oracle_8000",randomized_byte_oracle}
    };
    size_t i;
    for(i=0;i<sizeof(cases)/sizeof(cases[0]);++i) { cases[i].run(); printf("PASS %s\n",cases[i].name); }
    printf("%zu stack contract scenarios passed\n",sizeof(cases)/sizeof(cases[0]));
    return 0;
}
