#pragma once

#include <stdint.h>

namespace player_manager {
    constexpr uint64_t type_info    = 0x3C4D5E;
    constexpr uint64_t all_players  = 0x10;
    constexpr uint64_t local_player = 0x60;
    constexpr uint64_t method_rva   = 0x3C4D5E;
}

namespace player_controller {
    constexpr uint64_t type_info                      = 0x8156B08;
    constexpr uint64_t logger                         = 0x0;
    constexpr uint64_t max_health_c_t                 = 0x8;
    constexpr uint64_t max_armor_c_t                  = 0xC;
    constexpr uint64_t max_health_t_r                 = 0x10;
    constexpr uint64_t max_armor_t_r                  = 0x14;
    constexpr uint64_t main_camera_holder             = 0x28;
    constexpr uint64_t fps_camera_holder              = 0x30;
    constexpr uint64_t fps_directive                  = 0x38;
    constexpr uint64_t level_zone_controller          = 0x40;
    constexpr uint64_t character_view1                = 0x48;
    constexpr uint64_t character_view2                = 0x50;
    constexpr uint64_t state_flag                     = 0x78;
    constexpr uint64_t team                           = 0x79;
    constexpr uint64_t floating_value1                = 0x7C;
    constexpr uint64_t aim_controller                 = 0x80;
    constexpr uint64_t weaponry_controller            = 0x88;
    constexpr uint64_t mecanim_controller             = 0x90;
    constexpr uint64_t movement_controller            = 0x98;
    constexpr uint64_t arms_animation_controller      = 0xA0;
    constexpr uint64_t player_hit_controller          = 0xA8;
    constexpr uint64_t player_material_controller     = 0xB0;
    constexpr uint64_t player_occlusion_controller    = 0xB8;
    constexpr uint64_t network_controller             = 0xC0;
    constexpr uint64_t arms_lod_group                 = 0xC8;
    constexpr uint64_t backing_field_view             = 0xD0;
    constexpr uint64_t flag1                          = 0xD8;
    constexpr uint64_t flag2                          = 0xD9;
    constexpr uint64_t floating_value2                = 0xDC;
    constexpr uint64_t player_sound_controller        = 0xE0;
    constexpr uint64_t player_main_camera             = 0xE8;
    constexpr uint64_t player_f_p_s_camera            = 0xF0;
    constexpr uint64_t player_marker_trigger          = 0xF8;
    constexpr uint64_t additional_transform           = 0x100;
    constexpr uint64_t all_controllers_array          = 0x108;
    constexpr uint64_t controllers_dict               = 0x118;
    constexpr uint64_t unity_collider                 = 0x118;
    constexpr uint64_t skinned_mesh_lod_group         = 0x120;
    constexpr uint64_t character_lod_group            = 0x128;
    constexpr uint64_t flag3                          = 0x130;
    constexpr uint64_t flag4                          = 0x131;
    constexpr uint64_t helper_field                   = 0x134;
    constexpr uint64_t nullable_int1                  = 0x138;
    constexpr uint64_t nullable_bool                  = 0x140;
    constexpr uint64_t nullable_int2                  = 0x144;
    constexpr uint64_t photon_view                    = 0x150;
    constexpr uint64_t photon_view_id1                = 0x158;
    constexpr uint64_t photon_view_id2                = 0x15C;
    constexpr uint64_t photon_player                  = 0x160;
}

namespace weaponry_controller {
    constexpr uint64_t type_info                                = 0x8156B08;
    constexpr uint64_t slots_weapon_dict                        = 0x58;
    constexpr uint64_t weapon_controllers_list                  = 0x60;
    constexpr uint64_t weapon_slots_list                        = 0x68;
    constexpr uint64_t int32_terfaces_list                      = 0x70;
    constexpr uint64_t player_controller_ref                    = 0x78;
    constexpr uint64_t mecanim_controller_ref                   = 0x80;
    constexpr uint64_t current_weapon_slot                      = 0x88;
    constexpr uint64_t state_flag                               = 0x89;
    constexpr uint64_t weapon_pickup_controller_backing_field   = 0x90;
    constexpr uint64_t kit_controller                           = 0x98;
    constexpr uint64_t current_weapon_controller                = 0xA0;
    constexpr uint64_t flag1                                    = 0xA8;
    constexpr uint64_t flag2                                    = 0xA9;
    constexpr uint64_t float_val1                               = 0xAC;
    constexpr uint64_t float_val2                               = 0xB0;
    constexpr uint64_t weapon_manager_ref                       = 0xB8;
}

namespace aim_controller {
    constexpr uint64_t type_info                      = 0x8156B08;
    constexpr uint64_t override_spine_rotation_flag   = 0x53;
    constexpr uint64_t helper_object                  = 0x54;
    constexpr uint64_t sensitivity_x                  = 0x58;
    constexpr uint64_t sensitivity_y                  = 0x5C;
    constexpr uint64_t minimum_x                      = 0x60;
    constexpr uint64_t maximum_x                      = 0x64;
    constexpr uint64_t fps_position_g_o               = 0x68;
    constexpr uint64_t spine_director                 = 0x70;
    constexpr uint64_t fps_camera                     = 0x78;
    constexpr uint64_t cam_transform                  = 0x80;
    constexpr uint64_t aiming_parameters              = 0x88;
    constexpr uint64_t aiming_data                    = 0x90;
    constexpr uint64_t int32_terpolators_bunch        = 0x98;
    constexpr uint64_t tuning_params                  = 0xA0;
    constexpr uint64_t head_damping_speed             = 0xA8;
    constexpr uint64_t player_controller_ref          = 0xB0;
    constexpr uint64_t movement_controller_ref        = 0xB8;
    constexpr uint64_t transform_ref1                 = 0xC0;
    constexpr uint64_t mecanim_controller_ref         = 0xC8;
    constexpr uint64_t transform_ref2                 = 0xD0;
    constexpr uint64_t state_flag                     = 0xD8;
    constexpr uint64_t aim_state                      = 0xE0;
    constexpr uint64_t state_machine1                 = 0xE8;
    constexpr uint64_t position_vector                = 0xF0;
    constexpr uint64_t state_machine2                 = 0x100;
    constexpr uint64_t state_machine3                 = 0x108;
    constexpr uint64_t transform_t_r1                 = 0x110;
    constexpr uint64_t transform_t_r2                 = 0x118;
    constexpr uint64_t pose1                          = 0x120;
    constexpr uint64_t pose2                          = 0x13C;
    constexpr uint64_t pose3                          = 0x158;
    constexpr uint64_t pose4                          = 0x174;
    constexpr uint64_t pose5                          = 0x190;
    constexpr uint64_t weapon_animation_parameters    = 0x1B0;
    constexpr uint64_t scope_state                    = 0x1B8;
    constexpr uint64_t action_callback1               = 0x1C0;
    constexpr uint64_t action_callback2               = 0x1C8;
    constexpr uint64_t action_callback3               = 0x1D0;
    constexpr uint64_t action_callback4               = 0x1D8;
    constexpr uint64_t is_a_d_s                       = 0x1E0;
    constexpr uint64_t float_backing_field1           = 0x1E4;
    constexpr uint64_t float_backing_field2           = 0x1E8;
    constexpr uint64_t float_val1                     = 0x1F8;
    constexpr uint64_t float_val2                     = 0x1FC;
    constexpr uint64_t int32_terfaces_array           = 0x200;
    constexpr uint64_t weaponry_controller_ref        = 0x208;
    constexpr uint64_t tool_pivot_tuning_enabled      = 0x220;
    constexpr uint64_t spine_rotation_enabled         = 0x221;
}

namespace movement_controller {
    constexpr uint64_t type_info                                = 0x8156B08;
    constexpr uint64_t walk_speed                               = 0x20;
    constexpr uint64_t run_speed                                = 0x24;
    constexpr uint64_t speed_multiplier                         = 0x50;
    constexpr uint64_t player_controller_ref                    = 0x58;
    constexpr uint64_t player_occlusion_controller_ref          = 0x60;
    constexpr uint64_t never_idle                               = 0x68;
    constexpr uint64_t character_transform_backing_field        = 0x70;
    constexpr uint64_t movement_state_backing_field             = 0x78;
    constexpr uint64_t float_backing_field1                     = 0x80;
    constexpr uint64_t float_backing_field2                     = 0x84;
    constexpr uint64_t unity_collider                           = 0x88;
    constexpr uint64_t unity_trigger                            = 0x90;
    constexpr uint64_t int32_terfaces_array                     = 0x98;
    constexpr uint64_t translation_params                       = 0xA0;
    constexpr uint64_t translation_parameters                   = 0xA8;
    constexpr uint64_t translation_data                         = 0xB0;
    constexpr uint64_t transform_data                           = 0xB0;
    constexpr uint64_t movement_state                           = 0xB8;
    constexpr uint64_t character_transform                      = 0xC0;
    constexpr uint64_t mecanim_controller_backing_field         = 0xC8;
    constexpr uint64_t float_val1                               = 0xD0;
    constexpr uint64_t movement_handlers_list1                  = 0xD8;
    constexpr uint64_t movement_handlers_list2                  = 0xE0;
    constexpr uint64_t movement_handlers_array                  = 0xE8;
}

namespace mecanim_controller {
    constexpr uint64_t type_info                        = 0x8156B08;
    constexpr uint64_t anim_param_hashes35              = 0x0;
    constexpr uint64_t animator_state                   = 0x58;
    constexpr uint64_t player_controller_ref            = 0x60;
    constexpr uint64_t player_occlusion_controller_ref  = 0x68;
    constexpr uint64_t unity_animator                   = 0x70;
    constexpr uint64_t ik_controller                    = 0x78;
    constexpr uint64_t player_mecanim_config            = 0x80;
    constexpr uint64_t mecanim_sync                     = 0x88;
    constexpr uint64_t helper_object1                   = 0x90;
    constexpr uint64_t helper_object2                   = 0x98;
    constexpr uint64_t action_callback                  = 0xA0;
    constexpr uint64_t action_list                      = 0xA8;
    constexpr uint64_t int32_terfaces_array             = 0xB0;
    constexpr uint64_t action_callback2                 = 0xB8;
    constexpr uint64_t float_val                        = 0xC0;
}

namespace arms_animation_controller {
    constexpr uint64_t type_info                                = 0x8156B08;
    constexpr uint64_t arms_offset_point32_ts                   = 0x58;
    constexpr uint64_t arms_offset_info                         = 0x60;
    constexpr uint64_t fps_directive                            = 0x68;
    constexpr uint64_t fp_camera_placeholder                    = 0x70;
    constexpr uint64_t target_point32_t_number                  = 0x78;
    constexpr uint64_t reset_to_cam_position                    = 0x7C;
    constexpr uint64_t tuning_mode_enabled                      = 0x7D;
    constexpr uint64_t footstep_trace_params                    = 0x80;
    constexpr uint64_t arms_animation_params                    = 0x88;
    constexpr uint64_t mecanim_controller_ref                   = 0x90;
    constexpr uint64_t movement_controller_ref                  = 0x98;
    constexpr uint64_t player_occlusion_controller_ref          = 0xA0;
    constexpr uint64_t helper_object1                           = 0xA8;
    constexpr uint64_t float_val1                               = 0xAC;
    constexpr uint64_t sound_interfaces_array                   = 0xB0;
    constexpr uint64_t helper_object2                           = 0xB8;
    constexpr uint64_t helper_object3                           = 0xBC;
    constexpr uint64_t enabled_am_tr                            = 0xC0;
    constexpr uint64_t transform_t_r                            = 0xC8;
    constexpr uint64_t float_val2                               = 0xD0;
    constexpr uint64_t float_val3                               = 0xD4;
    constexpr uint64_t cur_footstep_trace_progress              = 0xD8;
    constexpr uint64_t float_val4                               = 0xDC;
    constexpr uint64_t cur_footstep_cycle_progress              = 0xE0;
    constexpr uint64_t half_offset_footstep_cycle_progress      = 0xE4;
    constexpr uint64_t static_vector                            = 0xE8;
    constexpr uint64_t float_val567                             = 0xF4;
}

namespace player_hit_controller {
    constexpr uint64_t type_info                = 0x5F6D7E;
    constexpr uint64_t hit_affect_curve         = 0xA0;
    constexpr uint64_t float_val1_damage        = 0xA8;
    constexpr uint64_t float_val2_damage        = 0xAC;
    constexpr uint64_t float_val3_damage        = 0xB0;
    constexpr uint64_t state_flag               = 0xB4;
    constexpr uint64_t hit_action_callback1     = 0xB8;
    constexpr uint64_t hit_action_callback2     = 0xC0;
    constexpr uint64_t action_callback          = 0xC8;
    constexpr uint64_t photon_player_source     = 0xD0;
    constexpr uint64_t int32_terfaces_array     = 0xD8;
    constexpr uint64_t binary_stream_hit_data   = 0xE0;
    constexpr uint64_t hit_data_static          = 0xE8;
    constexpr uint64_t objects_array            = 0xF0;
    constexpr uint64_t complex_hit_callback     = 0xF8;
    constexpr uint64_t local_time               = 0x100;
    constexpr uint64_t state_flag2              = 0x104;
    constexpr uint64_t method_rva               = 0x5F6D7E;
}

namespace network_controller {
    constexpr uint64_t type_info                    = 0x8156B08;
    constexpr uint64_t network_buffer1              = 0x58;
    constexpr uint64_t network_buffer2              = 0x60;
    constexpr uint64_t binary_stream1               = 0x68;
    constexpr uint64_t binary_stream2               = 0x70;
    constexpr uint64_t binary_stream3               = 0x78;
    constexpr uint64_t stopwatch                    = 0x80;
    constexpr uint64_t character_player_static      = 0x88;
    constexpr uint64_t player_controller_ref        = 0x90;
    constexpr uint64_t photon_view                  = 0x98;
    constexpr uint64_t int32_t_val1                 = 0xA0;
    constexpr uint64_t int32_t_val2                 = 0xA4;
    constexpr uint64_t float_val                    = 0xA8;
    constexpr uint64_t snapshot_logging_enabled     = 0xAC;
    constexpr uint64_t network_snapshot_time        = 0xB0;
    constexpr uint64_t int32_t_backing_field        = 0xB8;
    constexpr uint64_t state_flag_backing_field     = 0xBC;
}

namespace weapon_manager {
    constexpr uint64_t type_info                            = 0x8156B08;
    constexpr uint64_t logger                               = 0x0;
    constexpr uint64_t shoot_event                          = 0x28;
    constexpr uint64_t set_weapon_event                     = 0x30;
    constexpr uint64_t switch_weapon_event                  = 0x38;
    constexpr uint64_t weapon_config1                       = 0x40;
    constexpr uint64_t weapon_config2                       = 0x48;
    constexpr uint64_t weapon_config3                       = 0x50;
    constexpr uint64_t weapon_config4                       = 0x58;
    constexpr uint64_t helper_object                        = 0x60;
    constexpr uint64_t stat_track_controller                = 0x68;
    constexpr uint64_t static_transform                     = 0x70;
    constexpr uint64_t current_weapon_config_backing_field  = 0x78;
}

namespace hit_manager {
    constexpr uint64_t type_info                    = 0x5F6D7E;
    constexpr uint64_t logger                       = 0x0;
    constexpr uint64_t state_flag                   = 0x21;
    constexpr uint64_t hit_handlers_list1           = 0x28;
    constexpr uint64_t hit_handlers_list2           = 0x30;
    constexpr uint64_t hitbox_objects_hash_set      = 0x38;
    constexpr uint64_t objects_hash_set             = 0x40;
    constexpr uint64_t hit_validation_interface     = 0x48;
    constexpr uint64_t method_rva                   = 0x5F6D7E;
}

namespace player_material_controller {
    constexpr uint64_t type_info                = 0x8A9B0C;
    constexpr uint64_t base_material_immutable  = 0x58;
    constexpr uint64_t chams_visibility_material = 0x60;
    constexpr uint64_t player_controller_ref    = 0x68;
    constexpr uint64_t network_controller_ref   = 0x70;
    constexpr uint64_t additional_material      = 0x78;
    constexpr uint64_t method_rva               = 0x8A9B0C;
}

namespace player_main_camera {
    constexpr uint64_t type_info                        = 0x8156B08;
    constexpr uint64_t self_reference                   = 0x0;
    constexpr uint64_t state_flag                       = 0x8;
    constexpr uint64_t matrix_ptr                       = 0x10;
    constexpr uint64_t unity_camera_component           = 0x20;
    constexpr uint64_t camera_scope_zoomer              = 0x28;
    constexpr uint64_t camera_animation_controller      = 0x30;
    constexpr uint64_t camera_transform                 = 0x38;
    constexpr uint64_t main_camera_object               = 0x40;
    constexpr uint64_t player_controller_backing_field  = 0x48;
    constexpr uint64_t view_matrix                      = 0x100;
}

namespace player_sound_controller {
    constexpr uint64_t type_info                        = 0x8156B08;
    constexpr uint64_t fps_surface_track_interval       = 0x54;
    constexpr uint64_t tps_surface_track_interval       = 0x58;
    constexpr uint64_t surface_raycast_distance         = 0x5C;
    constexpr uint64_t sound_parameters                 = 0x60;
    constexpr uint64_t rotation_speed_for_sound         = 0x68;
    constexpr uint64_t player_reverberation_controller  = 0x70;
    constexpr uint64_t movement_sound_channels1234      = 0x78;
    constexpr uint64_t weapon_sound_channels1234        = 0x98;
    constexpr uint64_t hit_sounds                       = 0xB8;
    constexpr uint64_t environmental_sound_channels123456 = 0xC0;
    constexpr uint64_t environmental_sound_trigger      = 0xF0;
    constexpr uint64_t weaponry_controller_ref          = 0xF8;
    constexpr uint64_t playback_controller              = 0x100;
    constexpr uint64_t sound_dict_by_weapon_id1         = 0x108;
    constexpr uint64_t sound_dict_by_weapon_id2         = 0x110;
    constexpr uint64_t int32_t_counter                  = 0x118;
    constexpr uint64_t float_timer                      = 0x11C;
    constexpr uint64_t sound_event_instance             = 0x120;
    constexpr uint64_t state_flag                       = 0x128;
    constexpr uint64_t helper_object1                   = 0x130;
    constexpr uint64_t helper_object2                   = 0x138;
}
