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

/* ChannelMask tests */
extern void test_channelmask_clear(void);
extern void test_channelmask_set_and_isset(void);
extern void test_channelmask_reset(void);
extern void test_channelmask_set_out_of_bounds(void);
extern void test_channelmask_isallset(void);
extern void test_channelmask_isanyset(void);
extern void test_channelmask_isanyset_empty(void);
extern void test_channelmask_isnoneset(void);
extern void test_channelmask_multiword_channelid_33(void);
extern void test_channelmask_null_pointer(void);

/* Policy tests */
extern void test_policy_power_run_all_full(void);
extern void test_policy_network_global_all_full(void);
extern void test_policy_network_partial_partial_only(void);
extern void test_policy_diag_keep_all_full(void);
extern void test_policy_no_source_all_no(void);
extern void test_policy_null_pointer(void);
extern void test_policy_priority_power_beats_network(void);
extern void test_policy_multi_source_any_match(void);
extern void test_policy_same_priority_first_match_wins(void);
extern void test_policy_fallback_when_no_rule_matches(void);

/* State tests */
extern void test_state_init_default(void);
extern void test_state_on_source_active_sets_bit(void);
extern void test_state_on_source_inactive_clears_bit(void);
extern void test_state_tick_to_timeout(void);
extern void test_state_exclusive_area_called(void);
extern void test_state_null_pointer_get(void);
extern void test_state_on_unrelated_source_does_not_reload(void);

/* Cfg tests */
extern void test_cfg_channel_count(void);
extern void test_cfg_source_word_num(void);
extern void test_cfg_channel_word_num(void);
extern void test_cfg_source_debug_name_disabled(void);
extern void test_cfg_source_config_table_disabled(void);
extern void test_cfg_channel_masks_single_bit(void);
extern void test_cfg_all_channel_masks_unique(void);
extern void test_cfg_bus_types_mixed(void);
extern void test_cfg_policy_rule_count(void);
extern void test_cfg_fallback_valid(void);
extern void test_cfg_no_business_fields_in_channel_config(void);

/* Adapter tests */
extern void test_adapter_build_snapshot(void);
extern void test_adapter_request_com_mode_called_when_target_differs(void);
extern void test_adapter_no_repeat_request_when_target_same(void);
extern void test_adapter_handle_comm_e_not_ok(void);
extern void test_adapter_get_com_mode_null_ptr(void);
extern void test_adapter_mode_request_called(void);
extern void test_adapter_mode_request_adas_only(void);
extern void test_adapter_mode_request_none(void);
extern void test_adapter_request_com_mode_channel_bounds(void);
extern void test_adapter_build_snapshot_null(void);
extern void test_adapter_apply_decision_null(void);

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

    /* ChannelMask tests */
    RUN_TEST(test_channelmask_clear);
    RUN_TEST(test_channelmask_set_and_isset);
    RUN_TEST(test_channelmask_reset);
    RUN_TEST(test_channelmask_set_out_of_bounds);
    RUN_TEST(test_channelmask_isallset);
    RUN_TEST(test_channelmask_isanyset);
    RUN_TEST(test_channelmask_isanyset_empty);
    RUN_TEST(test_channelmask_isnoneset);
    RUN_TEST(test_channelmask_multiword_channelid_33);
    RUN_TEST(test_channelmask_null_pointer);

    /* Policy tests */
    RUN_TEST(test_policy_power_run_all_full);
    RUN_TEST(test_policy_network_global_all_full);
    RUN_TEST(test_policy_network_partial_partial_only);
    RUN_TEST(test_policy_diag_keep_all_full);
    RUN_TEST(test_policy_no_source_all_no);
    RUN_TEST(test_policy_null_pointer);
    RUN_TEST(test_policy_priority_power_beats_network);
    RUN_TEST(test_policy_multi_source_any_match);
    RUN_TEST(test_policy_same_priority_first_match_wins);
    RUN_TEST(test_policy_fallback_when_no_rule_matches);

    /* State tests */
    RUN_TEST(test_state_init_default);
    RUN_TEST(test_state_on_source_active_sets_bit);
    RUN_TEST(test_state_on_source_inactive_clears_bit);
    RUN_TEST(test_state_tick_to_timeout);
    RUN_TEST(test_state_exclusive_area_called);
    RUN_TEST(test_state_null_pointer_get);
    RUN_TEST(test_state_on_unrelated_source_does_not_reload);

    /* Cfg tests */
    RUN_TEST(test_cfg_channel_count);
    RUN_TEST(test_cfg_source_word_num);
    RUN_TEST(test_cfg_channel_word_num);
    RUN_TEST(test_cfg_source_debug_name_disabled);
    RUN_TEST(test_cfg_source_config_table_disabled);
    RUN_TEST(test_cfg_channel_masks_single_bit);
    RUN_TEST(test_cfg_all_channel_masks_unique);
    RUN_TEST(test_cfg_bus_types_mixed);
    RUN_TEST(test_cfg_policy_rule_count);
    RUN_TEST(test_cfg_fallback_valid);
    RUN_TEST(test_cfg_no_business_fields_in_channel_config);

    /* Adapter tests */
    RUN_TEST(test_adapter_build_snapshot);
    RUN_TEST(test_adapter_request_com_mode_called_when_target_differs);
    RUN_TEST(test_adapter_no_repeat_request_when_target_same);
    RUN_TEST(test_adapter_handle_comm_e_not_ok);
    RUN_TEST(test_adapter_get_com_mode_null_ptr);
    RUN_TEST(test_adapter_mode_request_called);
    RUN_TEST(test_adapter_mode_request_adas_only);
    RUN_TEST(test_adapter_mode_request_none);
    RUN_TEST(test_adapter_request_com_mode_channel_bounds);
    RUN_TEST(test_adapter_build_snapshot_null);
    RUN_TEST(test_adapter_apply_decision_null);

    /* MainFunction integration tests */
    RUN_TEST(test_mainfunction_repeated_call_no_duplicate_request);
    RUN_TEST(test_mainfunction_init_then_run);

    return UnityEnd();
}
