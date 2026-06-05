#include "unity.h"

void setUp(void) { }
void tearDown(void) { }

/* Test groups declared in individual test files */
extern void test_policy_kl15_on_all_full(void);
extern void test_policy_global_all_full(void);
extern void test_policy_adas_only_only_adas1(void);
extern void test_policy_no_pnc_all_no_sleep_candidate(void);
extern void test_policy_invalid_input_fallback(void);
extern void test_policy_null_pointer(void);
extern void test_policy_first_match_priority(void);
extern void test_policy_pnc_unknown_failsafe(void);
extern void test_policy_pnc_invalid_failsafe(void);
extern void test_policy_action_set_adas_only_mask(void);
extern void test_policy_action_set_all_no_mask(void);
extern void test_policy_action_set_all_full_mask(void);

extern void test_state_init_default(void);
extern void test_state_on_pnc_request_reload_hold_timer(void);
extern void test_state_tick_to_timeout(void);
extern void test_state_hold_valid_before_timeout(void);
extern void test_state_hold_invalid_after_timeout(void);
extern void test_state_exclusive_area_called(void);
extern void test_state_null_pointer_get(void);
extern void test_state_on_pnc_invalid_does_not_reload(void);
extern void test_state_on_pnc_unknown_does_not_reload(void);

extern void test_cfg_channel_count(void);
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

extern void test_adapter_build_snapshot(void);
extern void test_adapter_request_com_mode_called_when_target_differs(void);
extern void test_adapter_no_repeat_request_when_target_same(void);
extern void test_adapter_handle_comm_e_not_ok(void);
extern void test_adapter_bswm_request_called(void);
extern void test_adapter_request_com_mode_channel_bounds(void);
extern void test_adapter_build_snapshot_null(void);
extern void test_adapter_apply_decision_null(void);

extern void test_legacy_nm_parse_default_off(void);
extern void test_legacy_nm_parse_offset_config(void);
extern void test_legacy_nm_parse_mask_config(void);

extern void test_mainfunction_repeated_call_no_duplicate_request(void);
extern void test_mainfunction_init_then_run(void);

int main(void)
{
    UnityBegin("test_runner.c");

    /* Policy tests */
    RUN_TEST(test_policy_kl15_on_all_full);
    RUN_TEST(test_policy_global_all_full);
    RUN_TEST(test_policy_adas_only_only_adas1);
    RUN_TEST(test_policy_no_pnc_all_no_sleep_candidate);
    RUN_TEST(test_policy_invalid_input_fallback);
    RUN_TEST(test_policy_null_pointer);
    RUN_TEST(test_policy_first_match_priority);
    RUN_TEST(test_policy_pnc_unknown_failsafe);
    RUN_TEST(test_policy_pnc_invalid_failsafe);
    RUN_TEST(test_policy_action_set_adas_only_mask);
    RUN_TEST(test_policy_action_set_all_no_mask);
    RUN_TEST(test_policy_action_set_all_full_mask);

    /* State tests */
    RUN_TEST(test_state_init_default);
    RUN_TEST(test_state_on_pnc_request_reload_hold_timer);
    RUN_TEST(test_state_tick_to_timeout);
    RUN_TEST(test_state_hold_valid_before_timeout);
    RUN_TEST(test_state_hold_invalid_after_timeout);
    RUN_TEST(test_state_exclusive_area_called);
    RUN_TEST(test_state_null_pointer_get);
    RUN_TEST(test_state_on_pnc_invalid_does_not_reload);
    RUN_TEST(test_state_on_pnc_unknown_does_not_reload);

    /* Cfg tests */
    RUN_TEST(test_cfg_channel_count);
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
