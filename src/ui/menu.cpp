#define IMGUI_DEFINE_MATH_OPERATORS
#include "menu.hpp"
#include "bar.hpp"
#include "cfg.hpp"
#include "config_sys.hpp" 
#include "theme/theme.hpp"
#include "widgets/widgets.hpp"
#include "../../includes/internal/ImGui/imgui.h"
#include "../../includes/internal/ImGui/imgui_internal.h"
#include "../../includes/internal/Android_draw/draw.h"
#include "../func/skinchanger.hpp" 
//#include "../func/dumper.hpp"
#include "../game/game.hpp"
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <string>
#include <set>
#include <unordered_map>

namespace ui::menu {
    using namespace style;
    using namespace widgets;

    static float ma = 0.f;
    static int tab = 0;
    static int stab = 0;
    
    static float mw = 940.f;
    static float mh = 620.f;
    static float sw = 180.f;

    static const char* tabs[] = {"Aimbot", "Visuals", "Skins", "Settings"};
    static constexpr int tc = sizeof(tabs) / sizeof(tabs[0]);

    static void glass_panel(ImVec2 p, ImVec2 s, float a, const char* title = nullptr) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        float rounding = 20.f * S;
        
        dl->AddRectFilled(p, p + s, col(clr::sidebar, a), rounding);
        
        for(int i = 1; i <= 6; i++)
            dl->AddRect(p - ImVec2(i*1.0f, i*1.0f), ImVec2(p.x + s.x + i*1.0f, p.y + s.y + i*1.0f), IM_COL32(0,0,0, (int)(10 * a / i)), rounding + i*1.0f);
 
        dl->AddRect(p, ImVec2(p.x + s.x, p.y + s.y), col(ImVec4(1,1,1,0.15f), a), rounding, 0, 1.0f);

        if (title) {
            float hh = 48.f * S; 
            dl->AddLine(ImVec2(p.x + 15.f*S, p.y + hh), ImVec2(p.x + s.x - 15.f*S, p.y + hh), col(ImVec4(1,1,1,0.08f), a), 1.0f);

            ImGui::PushFont(fontBold);
            ImVec2 ts = ImGui::CalcTextSize(title);
            dl->AddText(fontBold, ImGui::GetFontSize(), ImVec2(p.x + 20.f * S, p.y + (hh - ts.y) * 0.5f), col(clr::text, a), title);
            ImGui::PopFont();
        }
    }

    static void aim_tab(float a) {
        float panel_w = 320.f * S;
        float gap = 24.f * S;
        float old_cw = content_w;
        content_w = panel_w - 48.f * S;

        ImVec2 p = ImGui::GetCursorScreenPos();
        float ph = 520.f * S;

        glass_panel(p, ImVec2(panel_w, ph), a, "Aimbot");
        ImGui::SetCursorScreenPos(p + ImVec2(24.f * S, 64.f * S));
        
        float ch = ph - 80.f * S;
        ImGui::BeginChild("##aim_left", ImVec2(content_w + 10.f * S, ch), false, ImGuiWindowFlags_NoBackground);
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 8.f * S);
            checkbox("Enabled", &cfg::aim::enabled, a);
            checkbox("Visible Check", &cfg::aim::visible_check, a);
            checkbox("Triggerbot", &cfg::aim::triggerbot, a);
            checkbox("Draw Fov", &cfg::aim::show_fov, a);
            
            ImGui::PopStyleVar();
        }
        ImGui::EndChild();

        ImVec2 rp = p + ImVec2(panel_w + gap, 0);
        glass_panel(rp, ImVec2(panel_w, ph), a, "Aim");
        ImGui::SetCursorScreenPos(rp + ImVec2(24.f * S, 64.f * S));
        
        ImGui::BeginChild("##aim_right", ImVec2(content_w + 12.f * S, ch), false, ImGuiWindowFlags_NoBackground);
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 18.f * S);
            auto section = [&](const char* id, bool active, auto content) {
                float sa = anim(std::string("a_sec_") + id, active ? 1.f : 0.f, 12.f);
                if (sa < 0.01f) return;

                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, sa * a);
                ImGui::BeginGroup();
                {
                    ImGui::PushFont(fontBold);
                    ImVec4 text_col = clr::accent;
                    text_col.w *= sa * a;
                    
                    ImVec2 p = ImGui::GetCursorScreenPos();
                    float fs = fontBold->FontSize * S;
                    ImVec2 ts = ImGui::CalcTextSize(id);
                    ImGui::GetWindowDrawList()->AddText(fontBold, fs, p, col(text_col, a), id);
                    ImGui::Dummy(ImVec2(0, ts.y));
                    
                    ImGui::PopFont();
                    ImGui::Spacing();
                    
                    content(sa);
                    
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                }
                ImGui::EndGroup();
                ImGui::PopStyleVar();
            };

            section("Aimbot", cfg::aim::enabled, [&](float sa) {
                slider("Smooth", &cfg::aim::smooth, 1.f, 20.f, sa, "%.1f");
                slider("Fov", &cfg::aim::fov, 1.f, 180.f, sa, "%.0f°");
                combo("Target Bone", &cfg::aim::bone, {"Head", "Neck", "Chest", "Pelvis"}, sa);
            });

            section("Triggerbot", cfg::aim::triggerbot, [&](float sa) {
                slider("Shot Delay", &cfg::aim::trigger_delay, 0.0f, 1.0f, sa, "%.2fs");
                slider("Max Distance", &cfg::aim::trigger_range, 1.0f, 1000.f, sa, "%.0fm");
                combo("Hitbox", &cfg::aim::trigger_bone_mask, {"Head Only", "Head & Neck", "Upper Body", "All"}, sa);
                checkbox("Visible Only##trigger", &cfg::aim::trigger_visible_only, sa);
            });

            section("Fov Visuals", cfg::aim::show_fov, [&](float sa) {
                slider("Thickness##fov", &cfg::aim::fov_thickness, 0.5f, 3.0f, sa, "%.1fpx");
                colorpick("Fov Color", &cfg::aim::fov_col, sa);
            });

            ImGui::PopStyleVar();
        }
        ImGui::EndChild();

        content_w = old_cw;
    }

    static void draw_keyboard(char* buf, int buf_size, float a, bool* p_open);

    static void settings_tab(float a) {
        float panel_w = 664.f * S;
        float old_cw = content_w;
        content_w = panel_w - 48.f * S;

        ImVec2 p = ImGui::GetCursorScreenPos();
        float ph = 520.f * S;

        glass_panel(p, ImVec2(panel_w, ph), a, "General Settings");

        ImGui::SetCursorScreenPos(p + ImVec2(24.f * S, 64.f * S));
        
        float ch = ph - 80.f * S;
        ImGui::BeginChild("##settings_main", ImVec2(content_w + 10.f * S, ch), false, ImGuiWindowFlags_NoBackground);
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 8.f * S);
            checkbox("Stream Proof", &cfg::misc::stream_proof, a);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Exit Cheat", ImVec2(140.f * S, 32.f * S))) {
                setInputPassThrough(true);
                exit(0);
            }
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            ImGui::PushFont(fontBold);
            ImGui::TextColored(clr::accent, "Design");
            ImGui::PopFont();
            ImGui::Spacing();

            ImGui::PushFont(fontMedium);
            ImGui::Text("Menu Scale:");
            ImGui::PopFont();
            ImGui::SameLine();
            
            auto scale_btn = [&](const char* label, float target) {
                bool active = (cfg::misc::menu_scale == target);
                std::string ids = std::string("sc_") + label;
                float sa = anim(ids, active ? 1.f : 0.f, 15.f);
                
                ImVec2 sz(70.f * S, 32.f * S);
                ImVec2 p = ImGui::GetCursorScreenPos();
                ImRect r(p, p + sz);
                
                ImGui::ItemSize(r);
                if (ImGui::ItemAdd(r, ImGui::GetID(label))) {
                    bool hov = ImGui::IsMouseHoveringRect(r.Min, r.Max) && !popup();
                    if (hov && ImGui::IsMouseClicked(0) && !clicked) {
                        cfg::misc::menu_scale = target;
                        clicked = true;
                    }

                    ImDrawList* dl = ImGui::GetWindowDrawList();
                    if (sa > 0.01f || hov) {
                        dl->AddRectFilled(r.Min, r.Max, col(clr::sidebar, a * (sa + hov*0.5f)), 8.f * S);
                        dl->AddRect(r.Min, r.Max, col(ImVec4(1, 1, 1, 0.1f * (sa + hov*0.5f)), a), 8.f * S);
                    } else {
                        dl->AddRectFilled(r.Min, r.Max, col(ImVec4(0, 0, 0, 0.15f), a), 8.f * S);
                        dl->AddRect(r.Min, r.Max, col(clr::glass_border, a * 0.4f), 8.f * S);
                    }

                    ImGui::PushFont(fontMedium);
                    float fs = ImGui::GetFontSize();
                    ImVec2 ts = ImGui::CalcTextSize(label);
                    dl->AddText(fontMedium, fs, r.GetCenter() - ImVec2(ts.x * 0.5f, fs * 0.5f), col(active ? clr::accent : clr::text_dim, a), label);
                    ImGui::PopFont();
                }
            };

            scale_btn("1.0x", 1.0f);
            ImGui::SameLine();
            scale_btn("1.4x", 1.4f);
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::PushFont(fontBold);
            ImGui::TextColored(clr::accent, "Config System");
            ImGui::PopFont();
            ImGui::Spacing();

            static std::vector<std::string> config_list;
            static int selected_config = -1;
            static bool needs_refresh = true;
            static char config_name_buf[64] = "";
            static bool keyboard_open = false;

            if (needs_refresh) {
                ui::config_sys::init(); 
                config_list = ui::config_sys::get_configs();
                if (selected_config >= (int)config_list.size()) selected_config = -1;
                needs_refresh = false;
            }

            std::vector<const char*> items;
            items.reserve(config_list.size());
            for (const auto& s : config_list) items.push_back(s.c_str());
            
            if (combo("Config List", &selected_config, items, a)) {
                if (selected_config >= 0 && selected_config < (int)config_list.size()) {
                    snprintf(config_name_buf, sizeof(config_name_buf), "%s", config_list[selected_config].c_str());
                }
            }

            ImGui::Spacing();
            
            float k_w = 32.f * S;
            float gap = 4.f * S;
            float input_w = content_w - k_w - gap;
            
            ImGui::PushItemWidth(input_w);
            
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0.2f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0, 0, 0, 0.3f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0, 0, 0, 0.4f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.f * S);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
            ImGui::PushStyleColor(ImGuiCol_Border, col(clr::glass_border, a * 0.5f));

            ImGui::PushFont(fontMedium);
            ImGui::InputTextWithHint("##cfg_name", "Config Name", config_name_buf, sizeof(config_name_buf));
            ImGui::PopFont();
            
            ImGui::PopStyleColor(4);
            ImGui::PopStyleVar(2);
            ImGui::PopItemWidth();

            ImGui::SameLine(0, gap);
            

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0.2f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0.3f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0.4f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.f * S);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
            ImGui::PushStyleColor(ImGuiCol_Border, col(clr::glass_border, a * 0.5f));

            if (ImGui::Button("K##cfg", ImVec2(k_w, ImGui::GetFrameHeight()))) {
                keyboard_open = !keyboard_open;
            }
            
            ImGui::PopStyleColor(4);
            ImGui::PopStyleVar(2);

            ImGui::Spacing();
            draw_keyboard(config_name_buf, sizeof(config_name_buf), a, &keyboard_open);
            ImGui::Spacing();

            float btn_gap = 8.f * S;
            float btn_w = (content_w - btn_gap * 2.f) / 3.f;
            
            float old_cw_btn = content_w;
            content_w = btn_w;

            if (button("Create", a)) {
                std::string name = config_name_buf;
                if (!name.empty()) {
                    ui::config_sys::save(name);
                    needs_refresh = true;
                }
            }
            ImGui::SameLine(0, btn_gap);
            if (button("Save", a)) {
                if (selected_config >= 0 && selected_config < (int)config_list.size()) {
                    ui::config_sys::save(config_list[selected_config]);
                } else {
                     std::string name = config_name_buf;
                     if (!name.empty()) {
                        ui::config_sys::save(name);
                        needs_refresh = true;
                     }
                }
            }
            ImGui::SameLine(0, btn_gap);
            if (button("Load", a)) {
                if (selected_config >= 0 && selected_config < (int)config_list.size()) {
                    ui::config_sys::load(config_list[selected_config]);
                }
            }
            content_w = old_cw_btn; 

            ImGui::Spacing();

            float btn_w2 = (old_cw_btn - btn_gap) / 2.f;
            content_w = btn_w2;

            if (button("Delete", a)) {
                if (selected_config >= 0 && selected_config < (int)config_list.size()) {
                    ui::config_sys::delete_config(config_list[selected_config]);
                    selected_config = -1;
                    memset(config_name_buf, 0, sizeof(config_name_buf));
                    needs_refresh = true;
                }
            }
            ImGui::SameLine(0, btn_gap);
            if (button("Refresh", a)) {
                needs_refresh = true;
            }
            content_w = old_cw_btn; 

            ImGui::PopStyleVar();
        }
        ImGui::EndChild();

        content_w = old_cw;
    }

    static char to_lower_char(char c) {
        if (c >= 'A' && c <= 'Z') return char(c + ('a' - 'A'));
        return c;
    }

    static bool string_contains_case_insensitive(const char* h, const char* n) {
        if (!h || !n) return false;
        if (!*n) return true;
        for (const char* p = h; *p; ++p) {
            const char* p1 = p;
            const char* p2 = n;
            while (*p1 && *p2 && to_lower_char(*p1) == to_lower_char(*p2)) {
                ++p1;
                ++p2;
            }
            if (!*p2) return true;
        }
        return false;
    }

    static void append_char_to_buffer(char* buf, int buf_size, char c) {
        if (!buf || buf_size <= 1) return;
        int len = (int)std::strlen(buf);
        if (len + 1 >= buf_size) return;
        buf[len] = c;
        buf[len + 1] = '\0';
    }

    static void backspace_buffer(char* buf) {
        if (!buf) return;
        int len = (int)std::strlen(buf);
        if (len <= 0) return;
        buf[len - 1] = '\0';
    }

    static void draw_keyboard(char* buf, int buf_size, float a, bool* p_open) {
        if (!buf || buf_size <= 1 || a < 0.01f || !p_open || !*p_open) return;

        ImGui::SetNextWindowSize(ImVec2(420.f * S, 260.f * S), ImGuiCond_FirstUseEver);
        ImGui::Begin("##skin_keyboard", p_open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
        
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 wp = ImGui::GetWindowPos();
        ImVec2 ws = ImGui::GetWindowSize();
        dl->AddRectFilled(wp, wp + ws, col(clr::bg, a), 12.f * S);
        dl->AddRect(wp, wp + ws, col(clr::border, a), 12.f * S);

        float old_cw = content_w;
        content_w = ImGui::GetContentRegionAvail().x;
        const char* row0 = "1234567890";
        const char* row1 = "QWERTYUIOP";
        const char* row2 = "ASDFGHJKL";
        const char* row3 = "ZXCVBNM";

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.f * S, 4.f * S));

        auto draw_row = [&](const char* row) {
            int len = (int)std::strlen(row);
            if (len <= 0) return;
            float gap = 2.f * S;
            float key_w = (content_w - gap * (len - 1)) / len;
            float prev_cw = content_w;
            content_w = key_w;
            for (int i = 0; i < len; ++i) {
                char label[2] = { row[i], 0 };
                if (button(label, a)) append_char_to_buffer(buf, buf_size, row[i]);
                if (i + 1 < len) ImGui::SameLine();
            }
            content_w = prev_cw;
        };

        draw_row(row0);
        draw_row(row1);
        draw_row(row2);
        draw_row(row3);

        float gap = 2.f * S;
        float prev_cw = content_w;
        float key_w = (content_w - gap * 2.f) / 3.f;
        content_w = key_w;
        if (button("Space", a)) append_char_to_buffer(buf, buf_size, ' ');
        ImGui::SameLine();
        if (button("Clear", a)) buf[0] = '\0';
        ImGui::SameLine();
        if (button("Back", a)) backspace_buffer(buf);
        content_w = prev_cw;
        
        ImGui::PopStyleVar();

        content_w = old_cw;
        ImGui::End();
    }

    static void esp_tab(float a) {
        float panel_w = 320.f * S;
        float gap = 24.f * S;
        float old_cw = content_w;
        content_w = panel_w - 48.f * S;

        ImVec2 p = ImGui::GetCursorScreenPos();
        float ph = 520.f * S;


        glass_panel(p, ImVec2(panel_w, ph), a, "Visual");
        ImGui::SetCursorScreenPos(p + ImVec2(24.f * S, 64.f * S));
        
        float ch = ph - 80.f * S;
        ImGui::BeginChild("##esp_left", ImVec2(content_w + 10.f * S, ch), false, ImGuiWindowFlags_NoBackground);
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 8.f * S);
            checkbox("Box", &cfg::esp::box, a);
            checkbox("Lines", &cfg::esp::line, a);
            checkbox("Skeleton", &cfg::esp::skeleton, a);
            checkbox("Name", &cfg::esp::name, a);
            checkbox("Health", &cfg::esp::health, a);
            checkbox("Distance", &cfg::esp::distance, a);
            checkbox("Anime", &cfg::esp::anime_esp, a);
            checkbox("Head", &cfg::esp::head_esp, a);
            checkbox("Bullet Tracers", &cfg::esp::tracers, a);
            
            ImGui::PopStyleVar();
        }
        ImGui::EndChild();


        ImVec2 rp = p + ImVec2(panel_w + gap, 0);
        glass_panel(rp, ImVec2(panel_w, ph), a, "Settings");
        ImGui::SetCursorScreenPos(rp + ImVec2(24.f * S, 64.f * S));
        
        ImGui::BeginChild("##esp_right", ImVec2(content_w + 12.f * S, ch), false, ImGuiWindowFlags_NoBackground);
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 18.f * S);
            auto section = [&](const char* id, bool active, auto content) {
                float sa = anim(std::string("v_sec_") + id, active ? 1.f : 0.f, 12.f);
                if (sa < 0.01f) return;

                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, sa * a);
                ImGui::BeginGroup();
                {
                    ImGui::PushFont(fontBold);
                    ImVec4 text_col = clr::accent;
                    text_col.w *= sa * a;
                    
                    ImVec2 p = ImGui::GetCursorScreenPos();
                    float fs = fontBold->FontSize * S;
                    ImVec2 ts = ImGui::CalcTextSize(id);
                    ImGui::GetWindowDrawList()->AddText(fontBold, fs, p, col(text_col, a), id);
                    ImGui::Dummy(ImVec2(0, ts.y));
                    
                    ImGui::PopFont();
                    ImGui::Spacing();
                    
                    content(sa);
                    
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                }
                ImGui::EndGroup();
                ImGui::PopStyleVar();
            };

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 4.f * S);
            ImGui::BeginGroup();
            {
                section("General", true, [&](float sa) {
                    slider("Max Distance", &cfg::esp::max_dist, 10.f, 1000.f, sa);
                    checkbox("Visible Only", &cfg::esp::visible_only, a);
                });

                section("Box", cfg::esp::box, [&](float sa) {
                    combo("Type##box", &cfg::esp::box_mode, {"Full", "Corners"}, sa);
                    slider("Thickness##box", &cfg::esp::box_thickness, 0.5f, 3.0f, sa, "%.1fpx");
                    slider("Rounding##box", &cfg::esp::box_rounding, 0.f, 12.f, sa, "%.0fpx");
                    colorpick("Color##box", &cfg::esp::box_col, sa);
                });

                section("Lines", cfg::esp::line, [&](float sa) {
                    combo("Position##line", &cfg::esp::line_pos, {"Bottom", "Top", "Center"}, sa);
                    slider("Thickness##line", &cfg::esp::line_thickness, 0.5f, 3.0f, sa, "%.1fpx");
                    colorpick("Color##line", &cfg::esp::line_col, sa);
                });

                section("Skeleton", cfg::esp::skeleton, [&](float sa) {
                    slider("Thickness##skel", &cfg::esp::skeleton_thickness, 0.5f, 3.0f, sa, "%.1fpx");
                    slider("Joint Size##skel", &cfg::esp::joint_size, 1.f, 5.f, sa, "%.1fpx");
                    colorpick("Bone Color##skel", &cfg::esp::skeleton_col, sa);
                    colorpick("Joint Color##skel", &cfg::esp::joint_col, sa);
                });

                section("Name", cfg::esp::name, [&](float sa) {
                    colorpick("Color##name", &cfg::esp::name_col, sa);
                });

                section("Health", cfg::esp::health, [&](float sa) {
                    colorpick("Color##hp", &cfg::esp::health_col, sa);
                });

                section("Distance", cfg::esp::distance, [&](float sa) {
                    colorpick("Color##dist", &cfg::esp::distance_col, sa);
                });

                section("Anime", cfg::esp::anime_esp, [&](float sa) {
                    slider("Scale", &cfg::esp::anime_scale, 0.5f, 2.5f, sa, "%.1fx");
                });

                section("Head", cfg::esp::head_esp, [&](float sa) {
                    slider("Size", &cfg::esp::head_esp_size, 0.5f, 3.0f, sa, "%.1fx");
                    colorpick("Color", &cfg::esp::head_esp_col, sa);
                });

                section("Bullet Tracers", cfg::esp::tracers, [&](float sa) {
                    slider("Duration", &cfg::esp::tracers_duration, 0.5f, 5.0f, sa, "%.1fs");
                    slider("Thickness", &cfg::esp::tracers_thickness, 0.1f, 3.0f, sa, "%.1fpx");
                    colorpick("Color", &cfg::esp::tracers_col, sa);
                });
            }
            ImGui::EndGroup();

            ImGui::PopStyleVar();
        }
        ImGui::EndChild();

        content_w = old_cw;
    }

    static ImVec4 get_rarity_color(int rarity, float alpha = 1.0f) {
        switch (rarity) {
            case 1: return ImVec4(0.6f, 0.6f, 0.6f, alpha); 
            case 2: return ImVec4(0.2f, 0.8f, 0.8f, alpha); 
            case 3: return ImVec4(0.2f, 0.4f, 0.8f, alpha); 
            case 4: return ImVec4(0.6f, 0.2f, 0.8f, alpha); 
            case 5: return ImVec4(0.8f, 0.2f, 0.6f, alpha); 
            case 6: return ImVec4(0.8f, 0.2f, 0.2f, alpha); 
            case 7: return ImVec4(0.9f, 0.8f, 0.2f, alpha); 
            default: return ImVec4(0.8f, 0.8f, 0.8f, alpha);
        }
    }

    static void skins_tab(float a) {
        float full_w = 664.f * S;
        float ph = 520.f * S;
        ImVec2 p = ImGui::GetCursorScreenPos();
        
        static bool show_full_list = false;
        static int selected_inventory_idx = -1;

        static bool init_db = false;
        static std::vector<std::string> collection_names_str;
        static std::vector<const char*> collection_items;

        
        static int collection_idx = 0;
        static int rarity_idx = 0;
        
        static bool filter_st = false;
        static char search_buf[64] = "";
        static bool keyboard_open = false;

        if (!init_db) {
            std::set<std::string> uniq_cols;
            for (const auto& s : skins::g_skin_database) {
                if (s.collection && s.collection[0]) uniq_cols.insert(s.collection);
            }
            collection_names_str.push_back("All");
            for (const auto& c : uniq_cols) collection_names_str.push_back(c);
            
            for(const auto& s : collection_names_str) collection_items.push_back(s.c_str());
            
            init_db = true;
        }


        if (!show_full_list) {
            glass_panel(p, ImVec2(full_w, ph), a, "Inventory");
            ImGui::SetCursorScreenPos(p + ImVec2(24.f * S, 64.f * S));

            ImGui::BeginChild("##inv_container", ImVec2(full_w - 48.f * S, ph - 80.f * S), false, ImGuiWindowFlags_NoBackground);
            
            if (button(cfg::skins::refresh_label, a)) {
                skins::rebuild_inventory();
            }
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, col(clr::text_dim, a));
            ImGui::Text("Items: %d", (int)cfg::skins::user_inventory.size());
            ImGui::PopStyleColor();

            if (!cfg::skins::inventory_initialized) {
                 cfg::skins::inventory_initialized = true;
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::BeginChild("##inv_list", ImVec2(0, 0), false, ImGuiWindowFlags_NoBackground);
            
            for (int i = 0; i < (int)cfg::skins::user_inventory.size(); ++i) {
                int def_idx = cfg::skins::user_inventory[i].def_index;
                if (def_idx < 0 || def_idx >= (int)skins::g_skin_database.size()) continue;
                
                const auto& skin = skins::g_skin_database[def_idx];
                std::string label = std::string(skin.name) + "##inv" + std::to_string(i);
                
                ImGui::PushStyleColor(ImGuiCol_Text, get_rarity_color(skin.rarity, a));
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, col(clr::accent, a * 0.2f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, col(clr::accent, a * 0.4f));

                if (ImGui::Button(label.c_str(), ImVec2(-1, 40.f * S))) {
                    selected_inventory_idx = i;
                    show_full_list = true;
                }
                ImGui::PopStyleColor(3);
                ImGui::PopStyleColor();
            }
            
            ImGui::EndChild();
            ImGui::EndChild();
        }

        else {
            glass_panel(p, ImVec2(full_w, ph), a, "Select Skin");
            
            ImVec2 title_size = ImGui::CalcTextSize("Select Skin");
            float header_h = 48.f * S;
            float title_y = p.y + (header_h - 22.f * S) * 0.5f; 
            
            ImGui::SetCursorScreenPos(p + ImVec2(24.f * S, 64.f * S));

            float filter_row_w = full_w - 48.f * S;
            
            float k_w = 32.f * S;
            float gap = 8.f * S;
               
            float search_w = filter_row_w * 0.5f;
            
            
            float start_y = ImGui::GetCursorPosY();
            float row_height = 60.f * S; 

            ImGui::SetCursorPosY(start_y + (row_height - ImGui::GetFrameHeight()) * 0.5f); 
            ImGui::PushItemWidth(search_w);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0.2f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0, 0, 0, 0.3f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0, 0, 0, 0.4f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.f * S);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
            ImGui::PushStyleColor(ImGuiCol_Border, col(clr::glass_border, a * 0.5f));
            ImGui::PushFont(fontMedium);
            
            if (ImGui::InputTextWithHint("##s", "Search...", search_buf, sizeof(search_buf))) {

            }
            
            ImGui::PopFont();
            ImGui::PopStyleColor(4);
            ImGui::PopStyleVar(2);
            ImGui::PopItemWidth();

            ImGui::SameLine(0, gap);
            
            ImGui::SetCursorPosY(start_y + (row_height - ImGui::GetFrameHeight()) * 0.5f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0.2f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0.3f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0.4f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.f * S);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
            ImGui::PushStyleColor(ImGuiCol_Border, col(clr::glass_border, a * 0.5f));
            
            if (ImGui::Button("K##kb", ImVec2(k_w, ImGui::GetFrameHeight()))) {
                keyboard_open = !keyboard_open;
            }
            
            ImGui::PopStyleColor(4);
            ImGui::PopStyleVar(2);
            
            ImGui::SetCursorPosY(start_y + row_height);
            ImGui::Spacing();
            ImGui::Separator();
            
            if (keyboard_open) {
                 draw_keyboard(search_buf, sizeof(search_buf), a, &keyboard_open);
                 ImGui::Separator();
            }

            float list_y_screen = ImGui::GetCursorScreenPos().y;
            float bottom_y_screen = p.y + ph;
            float list_h = bottom_y_screen - list_y_screen - 12.f * S; 
            
            if (list_h < 100.f * S) list_h = 100.f * S; 

            ImGui::SetCursorScreenPos(ImVec2(p.x + 12.f * S, list_y_screen));
            ImGui::BeginChild("##full_list", ImVec2(full_w - 24.f * S, list_h), false, ImGuiWindowFlags_NoBackground);
            
            static std::vector<int> filtered_indices;

            filtered_indices.clear();
            filtered_indices.reserve(skins::g_skin_database.size());

            for (int i = 0; i < (int)skins::g_skin_database.size(); ++i) {
                const auto& s = skins::g_skin_database[i];  
                
                if (filter_st && !s.is_st) continue;
                
                if (collection_idx > 0 && collection_idx < (int)collection_names_str.size()) {
                    if (s.collection != collection_names_str[collection_idx]) continue;
                }

                if (search_buf[0] != '\0') {
                    if (!string_contains_case_insensitive(s.name, search_buf)) continue;
                }

                filtered_indices.push_back(i);
            }

            ImGuiListClipper clipper;
            clipper.Begin((int)filtered_indices.size(), 40.f * S);
            while (clipper.Step()) {
                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                    int idx = filtered_indices[i];
                    const auto& s = skins::g_skin_database[idx];
                    
                    ImGui::PushStyleColor(ImGuiCol_Text, get_rarity_color(s.rarity, a));
                    
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, col(clr::accent, a * 0.2f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, col(clr::accent, a * 0.4f));
                    
                    if (ImGui::Button(s.name, ImVec2(-1, 35.f * S))) {
                        if (!popup()) {
                            if (selected_inventory_idx >= 0 && selected_inventory_idx < (int)cfg::skins::user_inventory.size()) {
                                auto& inv_item = cfg::skins::user_inventory[selected_inventory_idx];
                                if (inv_item.ptr) {
                                    wpm<int>(inv_item.ptr + 0x10, s.id);
                                    inv_item.def_index = idx;
                                    inv_item.applied_id = s.id;
                                    show_full_list = false;
                                }
                            }
                        }
                    }
                    ImGui::PopStyleColor(3); 
                    ImGui::PopStyleColor(); 
                }
            }

            ImGui::EndChild();
        }
    }

    void render() {
        float base_s = 1.0f;
        static float last_cfg_scale = -1.0f;
        if (last_cfg_scale != cfg::misc::menu_scale) {
            S = base_s * cfg::misc::menu_scale;
            last_cfg_scale = cfg::misc::menu_scale;
            ImGui::GetIO().FontGlobalScale = S;
        }

        ImGuiIO& io = ImGui::GetIO();
        float dt = io.DeltaTime;
        ma = ImLerp(ma, bar::g_open ? 1.f : 0.f, ImClamp(12.f * dt, 0.f, 1.f));

        if (ma < 0.01f) return;

        tick();

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ma);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 18.f * S); 
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 12.f * S);

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(0, 0, 0, 0)); 
        ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(1, 1, 1, 0.25f));
        ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, ImVec4(1, 1, 1, 0.45f));
        ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive, ImVec4(1, 1, 1, 0.65f));

        ImVec2 wsz(mw * S, mh * S);
        ImGui::SetNextWindowSize(wsz, ImGuiCond_Always);


        static ImVec2 wpos = ImVec2(-1, -1);
        if (wpos.x == -1 && g_sw > 100) {
            wpos = ImVec2((g_sw - wsz.x) * 0.5f, (g_sh - wsz.y) * 0.5f);
        }
        

        static bool is_dragging = false;
        static ImVec2 drag_offset = ImVec2(0, 0);
        ImVec2 title_pos = wpos + ImVec2(36.f * S, 55.f * S);
        ImVec2 title_size = ImVec2(100.f * S, 30.f * S); 
        
        if (ma > 0.1f && !popup()) {
            if (ImGui::IsMouseClicked(0)) {
                ImVec2 m = ImGui::GetIO().MousePos;
                if (m.x >= title_pos.x - 10.f*S && m.x <= title_pos.x + title_size.x + 10.f*S && 
                    m.y >= title_pos.y - 10.f*S && m.y <= title_pos.y + title_size.y + 10.f*S) {
                    is_dragging = true;
                    drag_offset = m - wpos;
                }
            }
            
            if (is_dragging) {
                if (ImGui::IsMouseDown(0)) {
                    wpos = ImGui::GetIO().MousePos - drag_offset;
                } else {
                    is_dragging = false;
                }
            }
        }


        static ImVec2 last_wpos = wpos;
        static ImVec2 velocity = ImVec2(0, 0);
        velocity = ImLerp(velocity, (wpos - last_wpos) * 0.1f, ImGui::GetIO().DeltaTime * 10.f);
        last_wpos = wpos;

        ImGui::SetNextWindowPos(wpos, ImGuiCond_Always);

        if (ImGui::Begin("##m", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground)) {
            ImVec2 wp = ImGui::GetWindowPos();
            ImDrawList* dl = ImGui::GetWindowDrawList();
            float rounding = 32.f * S;

            for (int i = 1; i <= 5; i++) {
                dl->AddRectFilled(wp - ImVec2(i * 1.0f, i * 1.0f), wp + wsz + ImVec2(i * 1.0f, i * 1.0f), IM_COL32(255, 255, 255, (int)(8 * ma / i)), rounding + i * 1.0f);
            }

 
            dl->AddRectFilled(wp, wp + wsz, col(ImVec4(0,0,0,0.3f), ma), rounding);

    
            for(int i = 0; i < 9; i++) {
                float dist = (i + 1) * 1.8f * S;
                float alpha = 0.08f / (i + 1);
                dl->AddRectFilled(wp - ImVec2(dist, dist), wp + wsz + ImVec2(dist, dist), col(clr::bg, ma * alpha), rounding + dist);
            }

   
            dl->AddRectFilled(wp, wp + wsz, col(clr::bg, ma * 0.95f), rounding);

            for (int i = 0; i < 2000; i++) {
                float gx = (float)(rand() % 1000) / 1000.f * wsz.x;
                float gy = (float)(rand() % 1000) / 1000.f * wsz.y;
                float sz = ((rand() % 10) / 10.f + 0.5f) * S;
                float ga = ((rand() % 100) / 100.f * 0.035f + 0.015f);
                dl->AddRectFilled(wp + ImVec2(gx, gy), wp + ImVec2(gx + sz, gy + sz), col(ImVec4(1, 1, 1, ga), ma), 0);
            }

    
            dl->AddRect(wp, wp + wsz, col(ImVec4(1,1,1,0.22f), ma), rounding);
            
         
            dl->PushClipRect(wp, wp + wsz, true);

            float cur_time = (float)ImGui::GetTime();

 
            for (int i = 0; i < 18; i++) {
                float wave_t = cur_time * (0.4f + i*0.02f);
                float shift_x = sinf(wave_t + i * 0.8f) * 35.f * S + (velocity.x * (i + 1) * 2.0f);
                float shift_y = cosf(wave_t * 0.8f + i * 1.1f) * 25.f * S + (velocity.y * (i + 1) * 2.0f);
                
                float zoom = 1.0f + (sinf(wave_t * 0.5f + i) * 0.05f);
                ImVec2 zoom_s = wsz * (zoom * 0.95f); 
                ImVec2 zoom_p = wp - (zoom_s - wsz) * 0.5f + ImVec2(shift_x, shift_y);
                
                dl->AddRectFilled(zoom_p, zoom_p + zoom_s, col(ImVec4(1,1,1,0.012f), ma * 0.4f), rounding * zoom);
                dl->AddRectFilled(zoom_p - ImVec2(shift_x*0.3f, shift_y*0.3f), zoom_p + zoom_s - ImVec2(shift_x*0.3f, shift_y*0.3f), col(ImVec4(0,0,0,0.01f), ma * 0.4f), rounding * zoom);
            }

            
            dl->PopClipRect();

       
            for(int i = 1; i <= 25; i++)
                dl->AddRect(wp - ImVec2(i*1.2f, i*1.2f), wp + wsz + ImVec2(i*1.2f, i*1.2f), col(ImVec4(0,0,0, 1.f), ma * 0.15f / i), rounding + i*1.2f);

 
            dl->AddRect(wp, wp + wsz, col(clr::glass_border, ma * 0.5f), rounding, 0, 1.2f);

     
            float sb_w = sw * S;
            dl->AddRectFilled(wp, ImVec2(wp.x + sb_w, wp.y + wsz.y), col(clr::sidebar, ma), rounding, ImDrawFlags_RoundCornersLeft);
            dl->AddLine(ImVec2(wp.x + sb_w, wp.y + 60.f * S), ImVec2(wp.x + sb_w, wp.y + wsz.y - 60.f * S), col(clr::border, ma), 1.5f);

      
            ImGui::PushFont(fontBold);
            float fs_title = ImGui::GetFontSize();
            
            const char* final_title = oxorany("HASLINE");

            ImVec2 ts_title = ImGui::CalcTextSize(final_title);
            ImVec2 title_p = wp + ImVec2((sb_w - ts_title.x) * 0.5f, 55.f * S);
            dl->AddText(fontBold, fs_title, title_p, col(clr::text, ma), final_title);
            ImGui::PopFont();

    
            float ty = wp.y + 130.f * S;
            for (int i = 0; i < tc; i++) {
                bool sel = (tab == i);
                float sa = anim("t_" + std::to_string(i), sel ? 1.f : 0.f, 15.f);
                float ha = anim("th_" + std::to_string(i), ImGui::IsMouseHoveringRect(wp + ImVec2(20.f*S, ty), wp + ImVec2(sb_w - 20.f*S, ty + 44.f*S)) ? 1.f : 0.f, 15.f);
                
                ImVec2 t_pos(wp.x + 24.f * S, ty);
                ImVec2 t_sz(sb_w - 48.f * S, 44.f * S);
                ImRect t_r(t_pos, t_pos + t_sz);

                if (sa > 0.01f || ha > 0.01f) {
                    dl->AddRectFilled(t_r.Min, t_r.Max, col(ImVec4(1,1,1,0.08f * (sa + ha*0.5f)), ma), 14.f * S);
                    dl->AddRect(t_r.Min, t_r.Max, col(ImVec4(1,1,1,0.06f * (sa + ha*0.5f)), ma), 14.f * S);
                }
                
                if (sa > 0.01f) {
                    float ay_h = 24.f * S * sa;
                    dl->AddRectFilled(ImVec2(t_r.Min.x - 6.f * S, t_r.GetCenter().y - ay_h*0.5f), ImVec2(t_r.Min.x - 2.f * S, t_r.GetCenter().y + ay_h*0.5f), col(clr::accent, ma), 2.f*S);
                }

                ImGui::PushFont(fontMedium);
                float fs = ImGui::GetFontSize();
                dl->AddText(fontMedium, fs, ImVec2(t_r.Min.x + 16.f * S, t_r.GetCenter().y - fs * 0.5f), col(sel ? clr::text : clr::text_dim, ma), tabs[i]);
                ImGui::PopFont();

                if (ImGui::IsMouseHoveringRect(t_r.Min, t_r.Max) && ImGui::IsMouseClicked(0) && !clicked) {
                    tab = i;
                    clicked = true;
                }
                ty += 58.f * S;
            }


            float total_content_w = 664.f * S;
            float content_x_off = sb_w + (wsz.x - sb_w - total_content_w) * 0.5f;
            ImGui::SetCursorScreenPos(ImVec2(wp.x + content_x_off, wp.y + 60.f * S));
            ImGui::BeginGroup();
            {
                content_alpha = ma;
                switch (tab) {
                    case 0: aim_tab(ma); break;
                    case 1: esp_tab(ma); break;
                    case 2: skins_tab(ma); break;
                    case 3: settings_tab(ma); break;
                }
            }
            ImGui::EndGroup();
        }
        ImGui::End();
        ImGui::PopStyleColor(5);
        ImGui::PopStyleVar(5);
        popups();
    }
}
