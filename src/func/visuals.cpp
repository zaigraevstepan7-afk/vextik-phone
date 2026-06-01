#define IMGUI_DEFINE_MATH_OPERATORS
#include "visuals.hpp"
#include "../game/game.hpp"
#include "../game/math.hpp"
#include "../game/player.hpp"
#include "../ui/cfg.hpp"
#include "../protect/oxorany.hpp"
#include "../../includes/internal/ImGui/imgui.h"
#include "../../includes/internal/ImGui/imgui_internal.h"
#include <GLES3/gl32.h>
#include <vector>
#include <type_traits>
#include <memory>
#include <cmath>
#include <algorithm>
#include <ctime>
#include <map>


struct bullet_tracer_t {
    Vector3 src;
    Vector3 dst;
    float time;
    float max_time;

    bullet_tracer_t(const Vector3& s, const Vector3& d, float t) 
        : src(s), dst(d), time(t), max_time(t) {}
};

static std::vector<bullet_tracer_t> bullet_tracers;
static uint64_t g_last_hit_ptr = 0;
static uint64_t g_last_gun_controller = 0;

extern ImFont* espFont;
extern ImFont* fontBold;
extern ImFont* fontMedium;

static GLuint kagami_tex = 0;
static int kagami_w = 0, kagami_h = 0;

struct player_esp_cache {
    float last_head_to_neck = 0.f;
    float smoothed_radius = 0.f;
};
static std::map<uint64_t, player_esp_cache> esp_cache;

static bool load_kagami() {
    return false; 
}

void visuals::skeleton(uint64_t player, const matrix& view_matrix, float a) {
    player::bones_t bones;
    if (!player::get_bones(player, bones)) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    float thick = cfg::esp::skeleton_thickness;
    
    ImU32 col = IM_COL32(
        static_cast<int>(cfg::esp::skeleton_col.x * 255),
        static_cast<int>(cfg::esp::skeleton_col.y * 255),
        static_cast<int>(cfg::esp::skeleton_col.z * 255),
        static_cast<int>(cfg::esp::skeleton_col.w * 255 * a)
    );

    ImU32 joint_col = IM_COL32(
        static_cast<int>(cfg::esp::joint_col.x * 255),
        static_cast<int>(cfg::esp::joint_col.y * 255),
        static_cast<int>(cfg::esp::joint_col.z * 255),
        static_cast<int>(cfg::esp::joint_col.w * 255 * a)
    );

    auto draw_line = [&](const Vector3& p1, const Vector3& p2) {
        ImVec2 s1, s2;
        if (world_to_screen(p1, view_matrix, s1) && world_to_screen(p2, view_matrix, s2)) {
            dl->AddLine(s1, s2, col, thick);
        }
    };

    auto draw_joint = [&](const Vector3& p) {
        ImVec2 s;
        if (world_to_screen(p, view_matrix, s)) {
            dl->AddCircleFilled(s, cfg::esp::joint_size, joint_col, 12);
            dl->AddCircle(s, cfg::esp::joint_size, IM_COL32(0, 0, 0, (int)(180 * a)), 12, 1.f);
        }
    };

    // спина
    draw_line(bones.head, bones.neck);
    draw_line(bones.neck, bones.spine2);
    draw_line(bones.spine2, bones.spine1);
    draw_line(bones.spine1, bones.spine);
    draw_line(bones.spine, bones.pelvis);

    // левая ручень
    draw_line(bones.neck, bones.l_shoulder);
    draw_line(bones.l_shoulder, bones.l_arm);
    draw_line(bones.l_arm, bones.l_forearm);
    draw_line(bones.l_forearm, bones.l_hand);

    //правая рученька
    draw_line(bones.neck, bones.r_shoulder);
    draw_line(bones.r_shoulder, bones.r_arm);
    draw_line(bones.r_arm, bones.r_forearm);
    draw_line(bones.r_forearm, bones.r_hand);

    //левая ногаа
    draw_line(bones.pelvis, bones.l_thigh);
    draw_line(bones.l_thigh, bones.l_knee);
    draw_line(bones.l_knee, bones.l_foot);

    //правая нога
    draw_line(bones.pelvis, bones.r_thigh);
    draw_line(bones.r_thigh, bones.r_knee);
    draw_line(bones.r_knee, bones.r_foot);

    // точки блядские
    draw_joint(bones.head);
    draw_joint(bones.neck);
    draw_joint(bones.l_shoulder);
    draw_joint(bones.r_shoulder);
    draw_joint(bones.l_hand);
    draw_joint(bones.r_hand);
    draw_joint(bones.l_knee);
    draw_joint(bones.r_knee);
    draw_joint(bones.l_foot);
    draw_joint(bones.r_foot);
}

void visuals::draw() {
    using namespace ui::style;
    if (cfg::misc::watermark) {
        ImDrawList* dl = ImGui::GetForegroundDrawList();
        
        char time_buf[32];
        time_t now = time(nullptr);
        struct tm* tm_info = localtime(&now);
        strftime(time_buf, sizeof(time_buf), "%H:%M:%S", tm_info);

        const char* name = "HASLINE";
        const char* link = "t.me/Hasline_Projects";
        
        ImGui::PushFont(fontBold);
        float fs = fontBold->FontSize * S;
        ImVec2 sz_name = ImGui::CalcTextSize(name);
        ImVec2 sz_link = ImGui::CalcTextSize(link);
        ImVec2 sz_time = ImGui::CalcTextSize(time_buf);
        ImGui::PopFont();

        float pad = 12.f * S;
        float gap = 16.f * S;
        float h = fs + 14.f * S;
        float w = sz_name.x + sz_link.x + sz_time.x + (gap * 2) + (pad * 2);

        ImVec2 p(20.f * S, 20.f * S);
        ImVec2 p_min = p;
        ImVec2 p_max(p.x + w, p.y + h);

        auto watermark_col = [](const ImVec4& c, float a = 1.f) -> ImU32 {
            return IM_COL32((int)(c.x * 255), (int)(c.y * 255), (int)(c.z * 255), (int)(c.w * 255 * a));
        };

        dl->AddRectFilled(p_min, p_max, watermark_col(ImVec4(0, 0, 0, 0.8f)), 10.f * S);
        dl->AddRect(p_min, p_max, watermark_col(ImVec4(1, 1, 1, 0.15f)), 10.f * S, 0, 1.2f);

        float text_y = p.y + (h - fs) * 0.5f;
        ImVec2 cur = ImVec2(p.x + pad, text_y);
        
        dl->AddText(fontBold, fs, cur, watermark_col(ui::clr::text), name);
        cur.x += sz_name.x + gap * 0.5f;
        
        dl->AddLine(ImVec2(cur.x, p.y + 6.f * S), ImVec2(cur.x, p.y + h - 6.f * S), watermark_col(ImVec4(1, 1, 1, 0.25f)));
        cur.x += gap * 0.5f;

        dl->AddText(fontBold, fs, cur, watermark_col(ImVec4(1, 1, 1, 0.6f)), link);
        cur.x += sz_link.x + gap * 0.5f;

        dl->AddLine(ImVec2(cur.x, p.y + 6.f * S), ImVec2(cur.x, p.y + h - 6.f * S), watermark_col(ImVec4(1, 1, 1, 0.25f)));
        cur.x += gap * 0.5f;

        dl->AddText(fontBold, fs, cur, watermark_col(ImVec4(1, 1, 1, 0.4f)), time_buf);
    }

    if (cfg::aim::show_fov) {
        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        ImVec2 center = ImVec2(g_sw * 0.5f, g_sh * 0.5f);
        
        ImU32 col = IM_COL32(
            static_cast<int>(cfg::aim::fov_col.x * 255),
            static_cast<int>(cfg::aim::fov_col.y * 255),
            static_cast<int>(cfg::aim::fov_col.z * 255),
            static_cast<int>(cfg::aim::fov_col.w * 255)
        );
        
        dl->AddCircle(center, cfg::aim::fov, col, 64, cfg::aim::fov_thickness);
    }

    if (cfg::esp::anime_esp) {
        load_kagami();
    }

    bool any = cfg::esp::box || cfg::esp::name || cfg::esp::health || cfg::esp::distance || cfg::esp::skeleton || cfg::esp::anime_esp || cfg::esp::head_esp || cfg::esp::tracers;
    if (!any) return;

    uint64_t PlayerManager = get_player_manager();
    if (!PlayerManager) return;

    if (player::last_player_manager != PlayerManager) {
        player::global_bone_cache.clear();
        player::last_player_manager = PlayerManager;
    }

    uint64_t LocalPlayer = rpm<uint64_t>(PlayerManager + player_manager::local_player);
    if (!LocalPlayer) return;

    matrix ViewMatrix = player::view_matrix(LocalPlayer);
    Vector3 LocalPosition = player::position(LocalPlayer);
    int LocalTeam = rpm<uint8_t>(LocalPlayer + offsets::player::team);

    uint64_t PlayerList = rpm<uint64_t>(PlayerManager + player_manager::all_players);
    if (!PlayerList) return;

    int PlayerCount = rpm<int>(PlayerList + 32); 
    if (PlayerCount <= 0 || PlayerCount > 128) return;

    uint64_t ListBuffer = rpm<uint64_t>(PlayerList + 24); 
    if (!ListBuffer) return;

    for (int i = 0; i < PlayerCount; i++) {
        uint64_t Player = rpm<uint64_t>(ListBuffer + 48 + 24 * i); 
        if (!Player || Player == LocalPlayer) continue;

        uint8_t PlayerTeam = rpm<uint8_t>(Player + offsets::player::team);
        if (cfg::esp::team_check && PlayerTeam == static_cast<uint8_t>(LocalTeam)) continue;

        if (cfg::esp::visible_only) {
            if (!player::is_visible(Player)) continue;
        }

        Vector3 PlayerPosition = player::position(Player);
        if (PlayerPosition.x == 0.f && PlayerPosition.y == 0.f && PlayerPosition.z == 0.f) continue;

        int Health = player::health(Player);
        if (Health <= 0) continue;

        float Distance = calculate_distance(PlayerPosition, LocalPosition);
        if (Distance > cfg::esp::max_dist) continue;

        Vector3 HeadPosition(PlayerPosition.x, PlayerPosition.y + 1.67f, PlayerPosition.z);

        ImVec2 ScreenHead, ScreenFoot;
        bool HeadVisible = world_to_screen(HeadPosition, ViewMatrix, ScreenHead);
        bool FootVisible = world_to_screen(PlayerPosition, ViewMatrix, ScreenFoot);
        if (!HeadVisible || !FootVisible) continue;

        float x1 = roundf(ScreenHead.x);
        float y1 = roundf(fminf(ScreenHead.y, ScreenFoot.y));
        float x2 = roundf(ScreenFoot.x);
        float y2 = roundf(fmaxf(ScreenHead.y, ScreenFoot.y));

        float bh = fabsf(y2 - y1);
        float bw = roundf(bh * 0.25f);
        float cx = roundf((x1 + x2) * 0.5f);

        ImVec2 BoxMin(cx - bw, y1);
        ImVec2 BoxMax(cx + bw, y2);

        float inv_dist = 1.f / (Distance + 0.1f);
        float font_sz = std::clamp(400.f * inv_dist, 10.f, 18.f);

        if (cfg::esp::skeleton) {
            skeleton(Player, ViewMatrix, 1.f);
        }

        if (cfg::esp::anime_esp || cfg::esp::head_esp) {
            player::bones_t b;
            if (player::get_bones(Player, b)) {
                ImVec2 head_2d, neck_2d;
                if (world_to_screen(b.head, ViewMatrix, head_2d) && world_to_screen(b.neck, ViewMatrix, neck_2d)) {
                    float head_to_neck = fabsf(head_2d.y - neck_2d.y);
                    
                    auto& cache = esp_cache[Player];
                    if (head_to_neck > 1.0f && head_to_neck < (bh * 0.5f)) {
                        cache.last_head_to_neck = head_to_neck;
                    } else if (cache.last_head_to_neck > 0.1f) {
                        head_to_neck = cache.last_head_to_neck;
                    } else {
                        if (head_to_neck < 1.0f) head_to_neck = 5.0f;
                    }

                    if (head_to_neck < 2.0f) head_to_neck = 2.0f; 

                    float target_radius_anime = head_to_neck * 3.5f * cfg::esp::anime_scale;
                    float target_radius_head = bh * 0.08f * cfg::esp::head_esp_size; 

                    if (cache.smoothed_radius < 0.1f) cache.smoothed_radius = target_radius_head;
                    cache.smoothed_radius += (target_radius_head - cache.smoothed_radius) * 0.2f;

                    if (cfg::esp::anime_esp) {
                        if (kagami_tex != 0) {
                            float head_h = target_radius_anime * 2.5f; 
                            float head_w = head_h * ((float)kagami_w / (float)kagami_h);
                            ImVec2 p_min(head_2d.x - head_w * 0.5f, head_2d.y - head_h * 0.5f);
                            ImVec2 p_max(head_2d.x + head_w * 0.5f, head_2d.y + head_h * 0.5f);
                            ImGui::GetBackgroundDrawList()->AddImage((ImTextureID)(intptr_t)kagami_tex, p_min, p_max);
                        }
                    }

                    if (cfg::esp::head_esp) {
                        ImU32 head_col_base = IM_COL32(
                            (int)(cfg::esp::head_esp_col.x * 255),
                            (int)(cfg::esp::head_esp_col.y * 255),
                            (int)(cfg::esp::head_esp_col.z * 255),
                            255 
                        );

                        ImU32 fill_col = IM_COL32(
                            (int)(cfg::esp::head_esp_col.x * 255),
                            (int)(cfg::esp::head_esp_col.y * 255),
                            (int)(cfg::esp::head_esp_col.z * 255),
                            15 
                        );
                        ImGui::GetBackgroundDrawList()->AddCircleFilled(head_2d, cache.smoothed_radius, fill_col, 32);
                        for (int i = 1; i <= 5; i++) {
                            float layer_radius = cache.smoothed_radius + (float)i * 0.8f;
                            int alpha = 100 / i; 
                            ImU32 glow_col = IM_COL32(
                                (int)(cfg::esp::head_esp_col.x * 255),
                                (int)(cfg::esp::head_esp_col.y * 255),
                                (int)(cfg::esp::head_esp_col.z * 255),
                                alpha
                            );
                            ImGui::GetBackgroundDrawList()->AddCircle(head_2d, layer_radius, glow_col, 32, 1.5f);
                        }

                        ImGui::GetBackgroundDrawList()->AddCircle(head_2d, cache.smoothed_radius, head_col_base, 32, 1.0f);
                    }
                }
            }
        }

        if (cfg::esp::box) {
            dbox(BoxMin, BoxMax, 1.f);
        }

        if (cfg::esp::line) {
            dline(ScreenFoot, ScreenHead, 1.f);
        }

        if (cfg::esp::health) {
            dhp(Health, BoxMin, BoxMax, bh, 1.f);
        }

        if (cfg::esp::name) {
            read_string PlayerName = player::name(Player);
            std::string ns = PlayerName.as_utf8();
            if (!ns.empty()) {
                dnick(ns.c_str(), BoxMin, cx, font_sz, 1.f);
            }
        }

        if (cfg::esp::distance) {
            ddist(Distance, BoxMax.x, BoxMin.y, font_sz, 1.f);
        }
    }

    if (cfg::esp::tracers) {
        update_tracers(LocalPlayer);
        draw_tracers(ViewMatrix);
    }
}

void visuals::update_tracers(uint64_t local_player) {
    float dt = ImGui::GetIO().DeltaTime;
    if (dt <= 0.f || dt > 0.1f) dt = 0.016f;

    if (!bullet_tracers.empty()) {
        bullet_tracers.erase(
            std::remove_if(bullet_tracers.begin(), bullet_tracers.end(),
                [dt](bullet_tracer_t& tracer) {
                    tracer.time -= dt;
                    return tracer.time <= 0.0f;
                }),
            bullet_tracers.end()
        );
    }

    if (!local_player) {
        g_last_hit_ptr = 0;
        bullet_tracers.clear();
        return;
    }

    uint64_t weaponry = rpm<uint64_t>(local_player + offsets::player::weaponry_controller);
    if (!weaponry) return;

    uint64_t gun_controller = rpm<uint64_t>(weaponry + weaponry_controller::current_weapon_controller);
    if (!gun_controller) return;

    if (gun_controller != g_last_gun_controller) {
        g_last_hit_ptr = 0;
        g_last_gun_controller = gun_controller;
    }

    uint64_t weapon_params = rpm<uint64_t>(gun_controller + oxorany(0xB0));
    if (!weapon_params) return;

    uint64_t list_ptr = rpm<uint64_t>(gun_controller + oxorany(0x198));
    if (!list_ptr) return;

    int list_size = rpm<int>(list_ptr + oxorany(0x18)); 
    uint64_t items_ptr = rpm<uint64_t>(list_ptr + oxorany(0x10)); 
    if (!items_ptr || list_size <= 0 || list_size > 100) return;

    for (int i = 0; i < list_size; ++i) {
        uint64_t element = rpm<uint64_t>(items_ptr + oxorany(0x20) + i * sizeof(uint64_t));
        if (!element) continue;

        uint64_t shot_list = rpm<uint64_t>(element + oxorany(0x18));
        if (!shot_list) continue;

        int capacity = rpm<int>(shot_list + oxorany(0x18));
        if (capacity <= 0 || capacity > 100) continue;

        uint64_t newest_in_this_list = 0;
        for (int j = 0; j < capacity; ++j) {
            uint64_t shot = rpm<uint64_t>(shot_list + oxorany(0x20) + j * sizeof(uint64_t));
            if (!shot || shot == g_last_hit_ptr) break;

            if (j == 0) newest_in_this_list = shot;

            Vector3 src = rpm<Vector3>(shot + oxorany(0x10));
            Vector3 dst = rpm<Vector3>(shot + oxorany(0x1C));

            bullet_tracers.emplace_back(src, dst, cfg::esp::tracers_duration);
        }

        if (newest_in_this_list) {
            g_last_hit_ptr = newest_in_this_list;
        }
    }
}

void visuals::draw_tracers(const matrix& view_matrix) {
    if (bullet_tracers.empty()) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();

    for (const auto& tracer : bullet_tracers) {
        float alpha_pct = tracer.time / tracer.max_time;
        if (alpha_pct <= 0.f) continue;

        Vector3 p1 = tracer.src;
        Vector3 p2 = tracer.dst;

        auto get_sw = [&](const Vector3& p) {
            return view_matrix.m14 * p.x + view_matrix.m24 * p.y + view_matrix.m34 * p.z + view_matrix.m44;
        };

        float sw1 = get_sw(p1);
        float sw2 = get_sw(p2);

        if (sw1 < 0.01f && sw2 < 0.01f) continue;

        if (sw1 < 0.01f) {
            float t = (0.01f - sw1) / (sw2 - sw1);
            p1.x = p1.x + t * (p2.x - p1.x);
            p1.y = p1.y + t * (p2.y - p1.y);
            p1.z = p1.z + t * (p2.z - p1.z);
        } else if (sw2 < 0.01f) {
            float t = (0.01f - sw2) / (sw1 - sw2);
            p2.x = p2.x + t * (p1.x - p2.x);
            p2.y = p2.y + t * (p1.y - p2.y);
            p2.z = p2.z + t * (p1.z - p2.z);
        }

        ImVec2 s1, s2;
        if (world_to_screen(p1, view_matrix, s1, false) && world_to_screen(p2, view_matrix, s2, false)) {
            ImU32 col = IM_COL32(
                static_cast<int>(cfg::esp::tracers_col.x * 255),
                static_cast<int>(cfg::esp::tracers_col.y * 255),
                static_cast<int>(cfg::esp::tracers_col.z * 255),
                static_cast<int>(cfg::esp::tracers_col.w * 255 * alpha_pct)
            );

            ImU32 glow_col = IM_COL32(
                static_cast<int>(cfg::esp::tracers_col.x * 255),
                static_cast<int>(cfg::esp::tracers_col.y * 255),
                static_cast<int>(cfg::esp::tracers_col.z * 255),
                static_cast<int>(cfg::esp::tracers_col.w * 100 * alpha_pct)
            );

            float thick = cfg::esp::tracers_thickness;
            for (int i = 1; i <= 3; i++) {
                dl->AddLine(s1, s2, glow_col, thick + (float)i * 1.2f);
            }

            dl->AddLine(s1, s2, col, thick);

            if (sw2 >= 0.01f) {
                dl->AddCircleFilled(s2, thick * 1.8f + 1.5f, glow_col, 12);
                dl->AddCircleFilled(s2, thick * 0.8f + 0.5f, col, 12);
            }
        }
    }
}

void visuals::dbox(const ImVec2& t, const ImVec2& b, float a) {
    if (a < 0.01f) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();

    float x1 = t.x, y1 = t.y, x2 = b.x, y2 = b.y;
    float r = cfg::esp::box_rounding;
    float thick = cfg::esp::box_thickness;

    ImU32 col = IM_COL32(
        static_cast<int>(cfg::esp::box_col.x * 255),
        static_cast<int>(cfg::esp::box_col.y * 255),
        static_cast<int>(cfg::esp::box_col.z * 255),
        static_cast<int>(cfg::esp::box_col.w * 255 * a)
    );

    ImU32 black = IM_COL32(0, 0, 0, (int)(150 * a));

    if (cfg::esp::box_mode == 0) { 
        dl->AddRect(ImVec2(x1 + 1.f, y1 + 1.f), ImVec2(x2 + 1.f, y2 + 1.f), black, r, 0, thick + 1.f);
        dl->AddRect(ImVec2(x1 - 1.f, y1 - 1.f), ImVec2(x2 - 1.f, y2 - 1.f), black, r, 0, thick + 1.f);
        dl->AddRect(ImVec2(x1, y1), ImVec2(x2, y2), col, r, 0, thick);
    } else { 
        float w = x2 - x1;
        float h = y2 - y1;
        float l = w * 0.25f;

        auto corner = [&](ImVec2 p1, ImVec2 p2, ImVec2 p3) {
            dl->AddLine(p1 + ImVec2(1,1), p2 + ImVec2(1,1), black, thick + 1.f);
            dl->AddLine(p1 + ImVec2(1,1), p3 + ImVec2(1,1), black, thick + 1.f);
            dl->AddLine(p1, p2, col, thick);
            dl->AddLine(p1, p3, col, thick);
        };


        corner(ImVec2(x1, y1), ImVec2(x1 + l, y1), ImVec2(x1, y1 + l));

        corner(ImVec2(x2, y1), ImVec2(x2 - l, y1), ImVec2(x2, y1 + l));

        corner(ImVec2(x1, y2), ImVec2(x1 + l, y2), ImVec2(x1, y2 - l));

        corner(ImVec2(x2, y2), ImVec2(x2 - l, y2), ImVec2(x2, y2 - l));
    }
}

void visuals::dline(const ImVec2& foot, const ImVec2& head, float a) {
    if (a < 0.01f) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();

    ImVec2 start;
    if (cfg::esp::line_pos == 0) 
        start = ImVec2(g_sw * 0.5f, g_sh);
    else if (cfg::esp::line_pos == 1) 
        start = ImVec2(g_sw * 0.5f, 0);
    else 
        start = ImVec2(g_sw * 0.5f, g_sh * 0.5f);

    ImU32 col = IM_COL32(
        static_cast<int>(cfg::esp::line_col.x * 255),
        static_cast<int>(cfg::esp::line_col.y * 255),
        static_cast<int>(cfg::esp::line_col.z * 255),
        static_cast<int>(cfg::esp::line_col.w * 255 * a)
    );

    dl->AddLine(start, foot, col, cfg::esp::line_thickness);
}

void visuals::dhp(int hp, const ImVec2& t, const ImVec2& b, float box_h, float a) {
    if (a < 0.01f) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();

    hp = std::clamp(hp, 0, 100);
    float pct = hp / 100.f;

    float bx = roundf(t.x - 6.f);
    float bw = 3.f;
    float bh = roundf(b.y - t.y);
    float fh = bh * pct;

    float top_y = t.y;
    float bot_y = b.y;
    float fill_top = roundf(bot_y - fh);

    dl->AddRectFilled(ImVec2(bx - 1.f, top_y - 1.f), ImVec2(bx + bw + 1.f, bot_y + 1.f), IM_COL32(0, 0, 0, (int)(180 * a)));
    
    ImU32 col_top = IM_COL32(
        static_cast<int>(cfg::esp::health_col.x * 255),
        static_cast<int>(cfg::esp::health_col.y * 255),
        static_cast<int>(cfg::esp::health_col.z * 255),
        static_cast<int>(cfg::esp::health_col.w * 255 * a)
    );
    ImU32 col_bot = IM_COL32(
        static_cast<int>(cfg::esp::health_col.x * 150),
        static_cast<int>(cfg::esp::health_col.y * 150),
        static_cast<int>(cfg::esp::health_col.z * 150),
        static_cast<int>(cfg::esp::health_col.w * 255 * a)
    );

    if (fh > 1.f) {
        dl->AddRectFilledMultiColor(ImVec2(bx, fill_top), ImVec2(bx + bw, bot_y), col_top, col_top, col_bot, col_bot);
    }

    if (hp < 100 && espFont) {
        char txt[8];
        snprintf(txt, sizeof(txt), "%d", hp);
        float fs = 10.f;
        ImVec2 ts = espFont->CalcTextSizeA(fs, FLT_MAX, 0.f, txt);
        float tx = roundf(bx + (bw - ts.x) * 0.5f);
        float ty = roundf(fill_top - ts.y * 0.5f);
        
        if (ty < top_y) ty = top_y;
        
        draw_text_outlined(dl, espFont, fs, ImVec2(tx, ty), IM_COL32(255, 255, 255, (int)(255 * a)), txt);
    }
}

void visuals::dnick(const char* name, const ImVec2& box_min, float cx, float sz, float a) {
    if (!espFont || a < 0.01f || !name) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();

    ImVec2 ts = espFont->CalcTextSizeA(sz, FLT_MAX, 0.f, name);
    ImVec2 tp(roundf(cx - ts.x * 0.5f), roundf(box_min.y - ts.y - 4.f));

    ImU32 col = IM_COL32(
        static_cast<int>(cfg::esp::name_col.x * 255),
        static_cast<int>(cfg::esp::name_col.y * 255),
        static_cast<int>(cfg::esp::name_col.z * 255),
        static_cast<int>(cfg::esp::name_col.w * 255 * a)
    );
    draw_text_outlined(dl, espFont, sz, tp, col, name);
}

void visuals::ddist(float dist, float x, float y, float sz, float a) {
    if (!espFont || a < 0.01f) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();

    char txt[16];
    snprintf(txt, sizeof(txt), "%dm", static_cast<int>(dist));

    ImVec2 tp(roundf(x + 5.f), roundf(y));

    ImU32 col = IM_COL32(
        static_cast<int>(cfg::esp::distance_col.x * 255),
        static_cast<int>(cfg::esp::distance_col.y * 255),
        static_cast<int>(cfg::esp::distance_col.z * 255),
        static_cast<int>(cfg::esp::distance_col.w * 255 * a)
    );
    draw_text_outlined(dl, espFont, sz, tp, col, txt);
}

void visuals::draw_text_outlined(ImDrawList* dl, ImFont* font, float size, const ImVec2& pos, ImU32 color, const char* text) {
    if (!font || !dl) return;
    int a = (color >> IM_COL32_A_SHIFT) & 0xFF;
    int s1 = static_cast<int>(a * 0.4f);
    int s2 = static_cast<int>(a * 0.7f);
    dl->AddText(font, size, ImVec2(pos.x + 2.f, pos.y + 2.f), IM_COL32(0, 0, 0, s1), text);
    dl->AddText(font, size, ImVec2(pos.x + 1.f, pos.y + 1.f), IM_COL32(0, 0, 0, s2), text);
    dl->AddText(font, size, pos, color, text);
}
