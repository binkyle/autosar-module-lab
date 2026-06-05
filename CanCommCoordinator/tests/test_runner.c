#include "unity.h"

void setUp(void) { }
void tearDown(void) { }

/* SourceMask tests */
extern void test_sourcemask_clear(void);
extern void test_sourcemask_set_and_isset(void);
extern void test_sourcemask_reset(void);
extern void test_sourcemask_set_out_of_bounds(void);
extern void test_sourcemask_isallset(void);
extern void test_sourcemask_isanyset(void);
extern void test_sourcemask_isanyset_empty(void);
extern void test_sourcemask_isnoneset(void);
extern void test_sourcemask_multiword_sourceid_33(void);
extern void test_sourcemask_null_pointer(void);

/* Policy tests */
extern void test_policy_ignition_on_all_full(void);
extern void test_policy_pnc_global_all_full(void);
extern void test_policy_pnc_adas_adas_only(void);
extern void test_policy_diag_keep_all_full(void);
extern void test_policy_no_source_all_no(void);
extern void test_policy_null_pointer(void);
extern void test_policy_priority_ignition_beats_pnc(void);
extern void test_policy_multi_source_any_match(void);
extern void test_policy_priority_same_first_match_wins(void);
extern void test_policy_fallback_when_no_rule_matches(void);

/* State tests */
extern void test_state_init_default(void);
extern void test_state_on_source_active_sets_bit(void);
extern void test_state_on_source_inactive_clears_bit(void);
extern void test_state_tick_to_timeout(void);
extern void test_state_exclusive_area_called(void);
extern void test_state_null_pointer_get(void);
extern void test_state_on_pnc_invalid_does_not_reload(void);

/* Cfg tests */
extern void test_cfg_channel_count(void);
extern void test_cfg_source_word_num(void);
extern void test_cfg_source_debug_name_disabled(void);
extern void test_cfg_source_config_table_disabled(void);
extern void test_cfg_adas1_channel_mask(void);
extern void test_cfg_adas2_channel_mask(void);
extern void test_cfg_adas_sub_channel_mask(void);
extern void test_cfg_pdc_channel_mask(void);
extern void test_cfg_can5_channel_mask(void);
extern void test_cfg_can9_channel_mask(void);
extern void test_cfg_all_masks_unique(void);
extern void test_cfg_policy_rule_count(void);
extern void test_cfg_fallback_valid(void);
extern void test_cfg_no_business_fields_in_channel_config(void);

/* Adapter tests */
extern void test_adapter_build_snapshot(void);
extern void test_adapter_request_com_mode_called_when_target_differs(void);
extern void test_adapter_no_repeat_request_when_target_same(void);
extern void test_adapter_handle_comm_e_not_ok(void);
extern void test_adapter_bswm_request_called(void);
extern void test_adapter_request_com_mode_channel_bounds(void);
extern void test_adapter_build_snapshot_null(void);
extern void test_adapter_apply_decision_null(void);

/* Legacy tests */
extern void test_legacy_nm_parse_default_off(void);
extern void test_legacy_nm_parse_offset_config(void);
extern void test_legacy_nm_parse_mask_config(void);

/* MainFunction integration tests */
extern void test_mainfunction_repeated_call_no_duplicate_request(void);
extern void test_mainfunction_init_then_run(void);

int main(void)
{
    UnityBegin("test_runner.c");

    /* SourceMask tests */
    RUN_TEST(test_sourcemask_clear);
    RUN_TEST(test_sourcemask_set_and_isset);
    RUN_TEST(test_sourcemask_reset);
    RUN_TEST(test_sourcemask_set_out_of_bounds);
    RUN_TEST(test_sourcemask_isallset);
    RUN_TEST(test_sourcemask_isanyset);
    RUN_TEST(test_sourcemask_isanyset_empty);
    RUN_TEST(test_sourcemask_isnoneset);
    RUN_TEST(test_sourcemask_multiword_sourceid_33);
    RUN_TEST(test_sourcemask_null_pointer);

    /* Policy tests */
    RUN_TEST(test_policy_ignition_on_all_full);
    RUN_TEST(test_policy_pnc_global_all_full);
    RUN_TEST(test_policy_pnc_adas_adas_only);
    RUN_TEST(test_policy_diag_keep_all_full);
    RUN_TEST(test_policy_no_source_all_no);
    RUN_TEST(test_policy_null_pointer);
    RUN_TEST(test_policy_priority_ignition_beats_pnc);
    RUN_TEST(test_policy_multi_source_any_match);
    RUN_TEST(test_policy_priority_same_first_match_wins);
    RUN_TEST(test_policy_fallback_when_no_rule_matches);

    /* State tests */
    RUN_TEST(test_state_init_default);
    RUN_TEST(test_state_on_source_active_sets_bit);
    RUN_TEST(test_state_on_source_inactive_clears_bit);
    RUN_TEST(test_state_tick_to_timeout);
    RUN_TEST(test_state_exclusive_area_called);
    RUN_TEST(test_state_null_pointer_get);
    RUN_TEST(test_state_on_pnc_invalid_does_not_reload);

    /* Cfg tests */
    RUN_TEST(test_cfg_channel_count);
    RUN_TEST(test_cfg_source_word_num);
    RUN_TEST(test_cfg_source_debug_name_disabled);
    RUN_TEST(test_cfg_source_config_table_disabled);
    RUN_TEST(test_cfg_adas1_channel_mask);
    RUN_TEST(test_cfg_adas2_channel_mask);
    RUN_TEST(test_cfg_adas_sub_channel_mask);
    RUN_TEST(test_cfg_pdc_channel_mask);
    RUN_TEST(test_cfg_can5_channel_mask);
    RUN_TEST(test_cfg_can9_channel_mask);
    RUN_TEST(test_cfg_all_masks_unique);
    RUN_TEST(test_cfg_policy_rule_count);
    RUN_TEST(test_cfg_fallback_valid);
    RUN_TEST(test_cfg_no_business_fields_in_channel_config);

    /* Adapter tests */
    RUN_TEST(test_adapter_build_snapshot);
    RUN_TEST(test_adapter_request_com_mode_called_when_target_differs);
    RUN_TEST(test_adapter_no_repeat_request_when_target_same);
    RUN_TEST(test_adapter_handle_comm_e_not_ok);
    RUN_TEST(test_adapter_bswm_request_called);
    RUN_TEST(test_adapter_request_com_mode_channel_bounds);
    RUN_TEST(test_adapter_build_snapshot_null);
    RUN_TEST(test_adapter_apply_decision_null);

    /* Legacy tests */
    RUN_TEST(test_legacy_nm_parse_default_off);
    RUN_TEST(test_legacy_nm_parse_offset_config);
    RUN_TEST(test_legacy_nm_parse_mask_config);

    /* MainFunction integration tests */
    RUN_TEST(test_mainfunction_repeated_call_no_duplicate_request);
    RUN_TEST(test_mainfunction_init_then_run);

    return UnityEnd();
}
