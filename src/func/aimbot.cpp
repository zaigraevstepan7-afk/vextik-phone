#include "aimbot.hpp"
#include "../ui/cfg.hpp"
#include "../game/game.hpp"
#include "../game/math.hpp"
#include "../game/player.hpp"
#include "../other/memory.hpp"
#include "../protect/oxorany.hpp"
#include <cmath>
#include <algorithm>
#include "imgui.h"

namespace aimbot {
    static int last_weapon = 0;
    static float last_pitch = 0.0f;
    static float last_yaw = 0.0f;
    static bool is_shooting = false;
    static float trigger_timer = 0.0f;
    static float shot_duration = 0.0f;
    static bool pulse_state = false;
    
    void handle() {
        if (!cfg::aim::enabled) return;

        uint64_t PlayerManager = get_player_manager();
        if (!PlayerManager) return;

        uint64_t LocalPlayer = rpm<uint64_t>(PlayerManager + oxorany(0x70));
        if (!LocalPlayer) return;

        uint64_t AimController = rpm<uint64_t>(LocalPlayer + offsets::player::aim_controller);
        if (!AimController) return;

        uint64_t AimingData = rpm<uint64_t>(AimController + offsets::aim::aiming_data);
        if (!AimingData) return;

        uint64_t weaponry = rpm<uint64_t>(LocalPlayer + offsets::player::weaponry_controller);
        uint64_t weapon = weaponry ? rpm<uint64_t>(weaponry + oxorany(0xA8)) : 0;
        
        int current_weapon_id = 0;
        if (weapon) {
            uint64_t parameters = rpm<uint64_t>(weapon + oxorany(0xB0));
            if (parameters) {
                current_weapon_id = rpm<int>(parameters + oxorany(0x18));
            }
        }

        float current_pitch = rpm<float>(AimingData + 0x18);
        float current_yaw = rpm<float>(AimingData + 0x1C);
        if (std::isnan(current_pitch) || std::isnan(current_yaw)) return;

        while (current_yaw > 180.0f) current_yaw -= 360.0f;
        while (current_yaw < -180.0f) current_yaw += 360.0f;

        bool weapon_changed = (current_weapon_id != last_weapon);
        float p_delta_last = std::abs(current_pitch - last_pitch);
        float y_delta_last = std::abs(current_yaw - last_yaw);
        if (y_delta_last > 180.0f) y_delta_last = 360.0f - y_delta_last;

        if (weapon_changed || p_delta_last > 45.0f || y_delta_last > 45.0f) {
            last_weapon = current_weapon_id;
            last_pitch = current_pitch;
            last_yaw = current_yaw;
        }

        matrix ViewMatrix = player::view_matrix(LocalPlayer);
        Vector3 CameraPos = player::camera_position(LocalPlayer);
        int LocalTeam = rpm<uint8_t>(LocalPlayer + offsets::player::team);

        uint64_t PlayerList = rpm<uint64_t>(PlayerManager + oxorany(0x28));
        if (!PlayerList) return;

        int PlayerCount = rpm<int>(PlayerList + oxorany(0x20));
        if (PlayerCount <= 0 || PlayerCount > 128) return;

        uint64_t ListBuffer = rpm<uint64_t>(PlayerList + oxorany(0x18));
        if (!ListBuffer) return;

        float best_fov = cfg::aim::fov;
        uint64_t best_target = 0;
        Vector3 best_bone_pos;

        bool can_trigger = false;
        if (cfg::aim::triggerbot && weapon) {
            for (int i = 0; i < PlayerCount; i++) {
                uint64_t Player = rpm<uint64_t>(ListBuffer + oxorany(0x30) + oxorany(0x18) * i);
                if (!Player || Player == LocalPlayer) continue;

                uint8_t PlayerTeam = rpm<uint8_t>(Player + offsets::player::team);
                if (PlayerTeam == static_cast<uint8_t>(LocalTeam)) continue;

                if (player::health(Player) <= 0) continue;
                if (cfg::aim::trigger_visible_only && !player::is_visible(Player)) continue;

                player::bones_t tbones;
                if (player::get_bones(Player, tbones)) {
                    struct bone_info { int id; Vector3 pos; };
                    bone_info targets[4];
                    int target_count = 0;
                    
                    if (cfg::aim::trigger_bone_mask == 0) { 
                        targets[target_count++] = {0, tbones.head};
                    } else if (cfg::aim::trigger_bone_mask == 1) {
                        targets[target_count++] = {0, tbones.head};
                        targets[target_count++] = {1, tbones.neck};
                    } else if (cfg::aim::trigger_bone_mask == 2) { 
                        targets[target_count++] = {0, tbones.head};
                        targets[target_count++] = {1, tbones.neck};
                        targets[target_count++] = {4, tbones.spine2};
                    } else { 
                        targets[target_count++] = {0, tbones.head};
                        targets[target_count++] = {1, tbones.neck};
                        targets[target_count++] = {4, tbones.spine2};
                        targets[target_count++] = {13, tbones.pelvis};
                    }
                    
                    for (int j = 0; j < target_count; j++) {
                        const auto& target = targets[j];
                        float dist_to_player = (target.pos - CameraPos).magnitude();
                        if (dist_to_player > cfg::aim::trigger_range) continue;

                        ImVec2 screen_pos;
                        if (world_to_screen(target.pos, ViewMatrix, screen_pos)) {
                            float dx = screen_pos.x - (g_sw * 0.5f);
                            float dy = screen_pos.y - (g_sh * 0.5f);
                            float crosshair_dist = sqrtf(dx * dx + dy * dy);
                            
                            if (crosshair_dist < 20.0f) { 
                                if (!cfg::aim::trigger_visible_only || player::is_bone_visible(Player, target.id)) {
                                    can_trigger = true;
                                    break;
                                }
                            }
                        }
                    }
                }
                if (can_trigger) break;
            }

            if (can_trigger) {
                if (!pulse_state) {
                    trigger_timer += ImGui::GetIO().DeltaTime;
                    if (trigger_timer >= cfg::aim::trigger_delay) {
                        wpm<uint8_t>(weapon + oxorany(0x148), 3);
                        pulse_state = true;
                        shot_duration = 0.0f;
                    }
                } else {
                    shot_duration += ImGui::GetIO().DeltaTime;
                    if (shot_duration >= 0.1f) {
                        wpm<uint8_t>(weapon + oxorany(0x148), 2); 
                        pulse_state = false;
                        trigger_timer = 0.0f;
                    }
                }
            } else {
                if (pulse_state || trigger_timer > 0.0f) {
                    wpm<uint8_t>(weapon + oxorany(0x148), 2);
                    pulse_state = false;
                    trigger_timer = 0.0f;
                }
            }
        }

        for (int i = 0; i < PlayerCount; i++) {
            uint64_t Player = rpm<uint64_t>(ListBuffer + oxorany(0x30) + oxorany(0x18) * i);
            if (!Player || Player == LocalPlayer) continue;

            uint8_t PlayerTeam = rpm<uint8_t>(Player + offsets::player::team);
            if (PlayerTeam == static_cast<uint8_t>(LocalTeam)) continue;

            int Health = player::health(Player);
            if (Health <= 0) continue;

            if (cfg::aim::visible_check) {
                if (!player::is_visible(Player)) continue;
            }

            player::bones_t bones;
            if (!player::get_bones(Player, bones)) continue;

            Vector3 target_pos;
            switch(cfg::aim::bone) {
                case 0: target_pos = bones.head; break;
                case 1: target_pos = bones.neck; break;
                case 2: target_pos = bones.spine2; break;
                case 3: target_pos = bones.pelvis; break;
                default: target_pos = bones.head; break;
            }

            ImVec2 screen_pos;
            if (world_to_screen(target_pos, ViewMatrix, screen_pos)) {
                float dx = screen_pos.x - (g_sw * 0.5f);
                float dy = screen_pos.y - (g_sh * 0.5f);
                float dist = sqrtf(dx * dx + dy * dy);
                if (dist < best_fov) {
                    best_fov = dist;
                    best_target = Player;
                    best_bone_pos = target_pos;
                }
            }
        }

        if (best_target) {
            Vector3 direction = best_bone_pos - CameraPos;
            float distance = direction.magnitude();
            if (distance < 0.1f) return;
            float pitch = -asinf(direction.y / distance) * Rad2Deg;
            float yaw = atan2f(direction.x, direction.z) * Rad2Deg;
            while (yaw > 180.0f) yaw -= 360.0f;
            while (yaw < -180.0f) yaw += 360.0f;
            float smooth_factor = 1.0f;
            float cfg_smooth = cfg::aim::smooth;
            
            if (cfg_smooth <= 0.1f) {
                smooth_factor = 1.0f;
            } else if (cfg_smooth <= 1.0f) {
                smooth_factor = 0.7f + (0.3f * cfg_smooth);
            } else if (cfg_smooth <= 5.0f) {
                smooth_factor = 0.2f + (0.5f * (5.0f - cfg_smooth)) / 4.0f;
            } else {
                float normalized_smooth = (cfg_smooth - 5.0f) / 15.0f;
                if (normalized_smooth > 1.0f) normalized_smooth = 1.0f;
                smooth_factor = 0.2f * (1.0f - normalized_smooth) + 0.01f * normalized_smooth;
            }

            float p_delta = pitch - current_pitch;
            float y_delta = yaw - current_yaw;
            if (y_delta > 180.0f) y_delta -= 360.0f;
            if (y_delta < -180.0f) y_delta += 360.0f;

            float delta_len = sqrtf(p_delta * p_delta + y_delta * y_delta);
            if (cfg_smooth > 0.1f) {
                if (delta_len > 10.0f) smooth_factor *= 0.7f;
                else if (delta_len < 2.0f) smooth_factor *= 0.9f;
            }

            float smoothed_p = current_pitch + p_delta * smooth_factor;
            float smoothed_y = current_yaw + y_delta * smooth_factor;

            smoothed_p = std::clamp(smoothed_p, -89.0f, 89.0f);

            while (smoothed_y > 180.0f) smoothed_y -= 360.0f;
            while (smoothed_y < -180.0f) smoothed_y += 360.0f;

            if (AimingData && rpm<uint64_t>(AimController + offsets::aim::aiming_data) == AimingData) {
                wpm<float>(AimingData + 0x18, smoothed_p);
                wpm<float>(AimingData + 0x1C, smoothed_y);
                wpm<float>(AimingData + 0x24, smoothed_p);
                wpm<float>(AimingData + 0x28, smoothed_y);
                
                last_pitch = smoothed_p;
                last_yaw = smoothed_y;
            }
        }
    }
}
