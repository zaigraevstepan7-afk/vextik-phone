#define IMGUI_DEFINE_MATH_OPERATORS
#include "widgets.hpp"
#include "Android_draw/draw.h"
#include <cmath>
#include <functional>

namespace ui {

namespace style {
    static float dt = 0.016f;

    static float lrp(float a, float b, float t) { return a + (b - a) * t; }
    static ImVec4 lrp_col(const ImVec4& a, const ImVec4& b, float t) {
        return ImVec4(lrp(a.x, b.x, t), lrp(a.y, b.y, t), lrp(a.z, b.z, t), lrp(a.w, b.w, t));
    }

    void tick() {
        clicked = false;
        static float lt = 0.f;
        float ct = ImGui::GetTime();
        if (lt > 0.f) {
            dt = ct - lt;
            dt = ImClamp(dt, 0.001f, 0.1f);
        }
        lt = ct;
    }

    float anim(const std::string& id, float tgt, float spd) {
        auto it = anims.find(id);
        if (it == anims.end()) {
            anims[id] = tgt;
            return tgt;
        }
        float t = ImClamp(spd * dt, 0.f, 1.f);
        it->second = lrp(it->second, tgt, t);
        if (fabsf(it->second - tgt) < 0.001f) it->second = tgt;
        return it->second;
    }

    ImVec4 anim_col(const std::string& id, const ImVec4& tgt, float spd) {
        auto it = anim_colors.find(id);
        if (it == anim_colors.end()) {
            anim_colors[id] = tgt;
            return tgt;
        }
        float t = ImClamp(spd * dt, 0.f, 1.f);
        it->second = lrp_col(it->second, tgt, t);
        return it->second;
    }

    ImU32 col(const ImVec4& c, float a) {
        return IM_COL32((int)(c.x * 255), (int)(c.y * 255), (int)(c.z * 255), (int)(c.w * 255 * a * content_alpha));
    }

    bool popup() { return popup_open || !active_popup.empty(); }
    void close() { active_popup = ""; popup_open = false; }

    void popups() {
        if (active_popup.empty()) return;
        float pa = anim(active_popup + "_pa", 1.f, 18.f);
        if (pa < 0.01f) { active_popup = ""; return; }
        if (pa > 0.15f && ImGui::IsMouseClicked(0) && !clicked) {
            active_popup = "";
            popup_open = false;
        }
    }
}

namespace widgets {
    using namespace style;

    bool checkbox(const char* name, bool* v, float a) {
        if (a < 0.01f) return false;

        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems) return false;

        std::string ids = std::string("cb_") + name;
        ImGuiID id = w->GetID(ids.c_str());

        float ww = content_w > 0 ? content_w : ImGui::GetContentRegionAvail().x;
        float h = 38.f * S;
        
        float t_w = 44.f * S;
        float t_h = 24.f * S;

        ImVec2 pos = w->DC.CursorPos;
        ImRect r(pos, ImVec2(pos.x + ww, pos.y + h));

        ImGui::ItemSize(r);
        if (!ImGui::ItemAdd(r, id)) return false;

        bool hov = ImGui::IsMouseHoveringRect(r.Min, r.Max) && !popup();
        bool press = hov && ImGui::IsMouseClicked(0) && !clicked;
        if (press) { *v = !*v; clicked = true; }

        float ca = anim(ids + "_c", *v ? 1.f : 0.f, 18.f);
        ImDrawList* dl = w->DrawList;

        
        ImGui::PushFont(fontMedium);
        float fs = fontMedium->FontSize * S;
        float tx = r.Min.x + 12.f * S; 
        ImVec2 ts = ImGui::CalcTextSize(name);
        float ty = r.GetCenter().y - fs * 0.5f;
        ImVec4 tc = anim_col(ids + "_tc", *v ? clr::text : clr::text_dim, 12.f);
        const char* label_end = ImGui::FindRenderedTextEnd(name);
        dl->AddText(fontMedium, fs, ImVec2(tx, ty), col(tc, a), name, label_end);
        ImGui::PopFont();

        
        ImVec2 t_pos(r.Max.x - t_w - 12.f * S, r.GetCenter().y - t_h * 0.5f); 
        ImRect t_r(t_pos, ImVec2(t_pos.x + t_w, t_pos.y + t_h));
        
        ImVec4 bg_off = ImVec4(0, 0, 0, 0.08f); 
        ImVec4 bg_c = lrp_col(bg_off, clr::accent, ca); 
        dl->AddRectFilled(t_r.Min, t_r.Max, col(bg_c, a), t_h * 0.5f);
        
        
        if (ca > 0.01f) {
            for(int i = 1; i <= 4; i++)
                dl->AddRect(t_r.Min - ImVec2(i*0.6f, i*0.6f), t_r.Max + ImVec2(i*0.6f, i*0.6f), col(clr::accent, a * 0.2f * ca / i), t_h * 0.5f + i*0.6f);
        }
        dl->AddRect(t_r.Min, t_r.Max, col(clr::glass_border, a * (0.3f + ca * 0.2f)), t_h * 0.5f);

        
        float k_sz = t_h - 4.f * S;
        float k_off = (t_w - t_h) * ca;
        ImVec2 k_pos(t_r.Min.x + 2.f * S + k_off, t_r.Min.y + 2.f * S);
        
        
        for(int i = 1; i <= 6; i++)
            dl->AddCircleFilled(ImVec2(k_pos.x + k_sz*0.5f, k_pos.y + k_sz*0.5f + i*0.4f), k_sz * 0.5f, IM_COL32(0,0,0, (int)(45 * a / i)));
        
        dl->AddCircleFilled(ImVec2(k_pos.x + k_sz*0.5f, k_pos.y + k_sz*0.5f), k_sz * 0.5f, col(ImVec4(1,1,1,1), a));
        dl->AddCircle(ImVec2(k_pos.x + k_sz*0.5f, k_pos.y + k_sz*0.5f), k_sz * 0.5f, col(clr::border, a * 0.2f), 24, 1.0f*S);

        return press;
    }

    static std::unordered_map<ImGuiID, bool> sl_active;
    static std::unordered_map<ImGuiID, bool> sl_locked;

    static std::string cp_id = "";
    static ImVec4* cp_ptr = nullptr;
    static std::unordered_map<std::string, ImVec4> hsv_cache;
    static std::string picker = "";

    bool slider(const char* name, float* v, float mn, float mx, float a, const char* fmt) {
        if (a < 0.01f) return false;

        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems) return false;

        std::string ids = std::string("sl_") + name;
        ImGuiID id = w->GetID(ids.c_str());

        float ww = content_w > 0 ? content_w : ImGui::GetContentRegionAvail().x;
        float h = 74.f * S;
        float pad = 12.f * S;

        ImVec2 pos = w->DC.CursorPos;
        ImRect r(pos, ImVec2(pos.x + ww, pos.y + h));
        
        float t_h = 8.f * S;
        ImRect track(ImVec2(r.Min.x + pad, r.Min.y + 48.f * S), ImVec2(r.Max.x - pad, r.Min.y + 48.f * S + t_h)); 

        ImGui::ItemSize(r);
        if (!ImGui::ItemAdd(r, id)) return false;

        bool hov = false, held = false;
        ImGui::ButtonBehavior(track, id, &hov, &held, ImGuiButtonFlags_None);

        if (held && !popup()) {
            if (sl_active.find(id) == sl_active.end()) {
                sl_active[id] = false;
                sl_locked[id] = false;
            }

            ImVec2 drag = ImGui::GetMouseDragDelta(0, 0.f);
            if (!sl_active[id] && !sl_locked[id]) {
                if (fabsf(drag.y) > 10.f) sl_locked[id] = true;
                else if (fabsf(drag.x) > 5.f) sl_active[id] = true;
            }

            if (sl_active[id]) {
                float mx_pos = ImGui::GetIO().MousePos.x;
                float nn = ImClamp((mx_pos - track.Min.x) / track.GetWidth(), 0.f, 1.f);
                *v = mn + nn * (mx - mn);
            }
        } else {
            sl_active.erase(id);
            sl_locked.erase(id);
        }

        float norm = (*v - mn) / (mx - mn);
        float na = anim(ids + "_n", norm, 25.f);
        
        ImDrawList* dl = w->DrawList;

        ImGui::PushFont(fontMedium);
        float fs = fontMedium->FontSize * S;
        float ty = r.Min.y + 6.f * S; 
        const char* label_end = ImGui::FindRenderedTextEnd(name);
        dl->AddText(fontMedium, fs, ImVec2(track.Min.x, ty), col(clr::text_dim, a), name, label_end);

        char vb[32];
        snprintf(vb, sizeof(vb), fmt, *v);
        ImVec2 vs = ImGui::CalcTextSize(vb);
        dl->AddText(fontMedium, fs, ImVec2(track.Max.x - vs.x, ty), col(clr::text, a), vb);
        ImGui::PopFont();

        dl->AddRectFilled(track.Min, track.Max, col(ImVec4(0,0,0,0.06f), a), t_h * 0.5f);
        dl->AddRect(track.Min, track.Max, col(clr::glass_border, a * 0.5f), t_h * 0.5f);

        float fw = track.GetWidth() * na;
        if (fw > 0.f) {
            dl->AddRectFilled(track.Min, ImVec2(track.Min.x + fw, track.Max.y), col(clr::accent, a), t_h * 0.5f);
            dl->AddRectFilledMultiColor(track.Min, ImVec2(track.Min.x + fw, track.Min.y + t_h*0.5f), 
                col(ImVec4(1,1,1,0.4f), a), col(ImVec4(1,1,1,0.4f), a), 
                col(ImVec4(1,1,1,0), a), col(ImVec4(1,1,1,0), a));
        }

        float k_sz = t_h + 10.f * S;
        ImVec2 k_pos(track.Min.x + track.GetWidth() * na, track.GetCenter().y);
        for(int i = 1; i <= 8; i++)
            dl->AddCircleFilled(ImVec2(k_pos.x, k_pos.y + i*0.5f), k_sz * 0.5f, IM_COL32(0,0,0, (int)(45 * a / i)));
        
        dl->AddCircleFilled(k_pos, k_sz * 0.5f, col(ImVec4(1,1,1,1), a));
        dl->AddCircle(k_pos, k_sz * 0.5f, col(clr::border, a * 0.2f), 24, 1.0f*S);
        dl->AddCircleFilled(k_pos, 3.f * S, col(clr::accent, a), 24);

        return held && sl_active[id];
    }

    static ImVec2 cmb_pos;
    static float cmb_w;
    static std::vector<const char*> cmb_items;
    static int* cmb_ptr = nullptr;

    bool combo(const char* name, int* v, const std::vector<const char*>& items, float a) {
        if (a < 0.01f) return false;

        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems) return false;

        std::string ids = std::string("cmb_") + name;
        ImGuiID id = w->GetID(ids.c_str());

        float ww = content_w > 0 ? content_w : ImGui::GetContentRegionAvail().x;
        float h = 76.f * S;
        float pad = 12.f * S;

        ImVec2 pos = w->DC.CursorPos;
        ImRect r(pos, ImVec2(pos.x + ww, pos.y + h));

        float bh = 32.f * S; 
        ImRect btn(ImVec2(r.Min.x + pad, r.Min.y + 38.f * S), ImVec2(r.Max.x - pad, r.Min.y + 38.f * S + bh)); 

        ImGui::ItemSize(r);
        if (!ImGui::ItemAdd(r, id)) return false;

        bool hov = ImGui::IsMouseHoveringRect(btn.Min, btn.Max) && !popup();
        bool open = (active_popup == ids);
        bool press = hov && ImGui::IsMouseClicked(0) && !clicked;

        if (press) {
            if (open) { active_popup = ""; popup_open = false; }
            else {
                active_popup = ids;
                popup_open = true;
                cmb_pos = ImVec2(btn.Min.x, btn.Max.y + 4.f * S);
                cmb_w = btn.GetWidth();
                cmb_items = items;
                cmb_ptr = v;
            }
            clicked = true;
        }

        float oa = anim(ids + "_o", open ? 1.f : 0.f, 15.f);
        float ha = anim(ids + "_h", hov ? 1.f : 0.f, 12.f);
        ImDrawList* dl = w->DrawList;

        ImGui::PushFont(fontMedium);
        float fs = fontMedium->FontSize * S;
        float ty = r.Min.y + 6.f * S; 
        const char* label_end = ImGui::FindRenderedTextEnd(name);
        dl->AddText(fontMedium, fs, ImVec2(r.Min.x + pad, ty), col(clr::text_dim, a), name, label_end);
        ImGui::PopFont();

        ImVec4 bg_col = lrp_col(clr::widget, ImVec4(0,0,0,0.08f), ha);
        dl->AddRectFilled(btn.Min, btn.Max, col(bg_col, a), 8.f * S);
        dl->AddRect(btn.Min, btn.Max, col(clr::glass_border, a * (0.4f + ha * 0.2f)), 8.f * S);

        ImGui::PushFont(fontMedium);
        const char* prev = (*v >= 0 && *v < (int)items.size()) ? items[*v] : "";
        ImVec2 ts = ImGui::CalcTextSize(prev);
        float py = btn.GetCenter().y - fs * 0.5f;
        dl->AddText(fontMedium, fs, ImVec2(btn.Min.x + 12.f * S, py), col(clr::text, a), prev);
        ImGui::PopFont();

        float ax = btn.Max.x - 16.f * S;
        float ay = btn.GetCenter().y;
        float as = 4.f * S;
        dl->AddLine(ImVec2(ax - as, ay - as*0.5f), ImVec2(ax, ay + as*0.5f), col(clr::text_dim, a), 1.2f * S);
        dl->AddLine(ImVec2(ax + as, ay - as*0.5f), ImVec2(ax, ay + as*0.5f), col(clr::text_dim, a), 1.2f * S);

        if (oa > 0.01f && cmb_ptr == v) {
            ImDrawList* fg = ImGui::GetForegroundDrawList();
            float ih = 32.f * S;
            float ph = items.size() * ih + 12.f * S;
            ImVec2 pmin = cmb_pos;
            ImVec2 pmax(pmin.x + cmb_w, pmin.y + ph * oa);

            fg->AddRectFilled(pmin, pmax, col(ImVec4(20/255.f, 22/255.f, 28/255.f, 0.95f), oa), 12.f * S);
            
            for(int i = 1; i <= 12; i++)
                fg->AddRect(pmin - ImVec2(i*0.8f, i*0.8f), pmax + ImVec2(i*0.8f, i*0.8f), IM_COL32(0,0,0, (int)(22 * oa / i)), 12.f * S + i*0.5f);
            
            fg->AddRect(pmin, pmax, col(clr::glass_border, oa * 0.8f), 12.f * S);

            fg->PushClipRect(pmin, pmax, true);
            float iy = pmin.y + 6.f * S;
            for (int i = 0; i < (int)items.size(); i++) {
                ImRect ir(ImVec2(pmin.x + 4.f*S, iy), ImVec2(pmax.x - 4.f*S, iy + ih));
                bool ihov = ImGui::IsMouseHoveringRect(ir.Min, ir.Max, false);
                float iha = anim(ids + "_ih_" + std::to_string(i), ihov ? 1.f : 0.f, 15.f);
                
                if (iha > 0.01f) {
                    fg->AddRectFilled(ir.Min, ir.Max, col(ImVec4(0,0,0,0.05f * iha), oa), 8.f * S);
                }

                if (ihov && ImGui::IsMouseClicked(0) && !clicked) {
                    *v = i;
                    active_popup = "";
                    popup_open = false;
                    clicked = true;
                }

                ImGui::PushFont(fontMedium);
                float fs = fontMedium->FontSize * S;
                fg->AddText(fontMedium, fs, ImVec2(ir.Min.x + 10.f * S, ir.GetCenter().y - fs * 0.5f), col(i == *v ? clr::accent : (ihov ? clr::text : clr::text_dim), oa), items[i]);
                ImGui::PopFont();
                iy += ih;
            }
            fg->PopClipRect();
        }

        return false;
    }

    void colorpick(const char* name, ImVec4* v, float a) {
        if (a < 0.01f) return;

        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems) return;

        std::string ids = std::string("cp_") + name;
        ImGuiID id = w->GetID(ids.c_str());

        float ww = content_w > 0 ? content_w : ImGui::GetContentRegionAvail().x;
        float h = 38.f * S; 
        float pad = 8.f * S;

        ImVec2 pos = w->DC.CursorPos;
        ImRect r(pos, ImVec2(pos.x + ww, pos.y + h));

        ImGui::ItemSize(r);
        if (!ImGui::ItemAdd(r, id)) return;

        bool hov = ImGui::IsMouseHoveringRect(r.Min, r.Max);
        bool open = (active_popup == ids);
        bool press = hov && ImGui::IsMouseClicked(0) && !clicked;

        if (press) {
            if (open) { active_popup = ""; popup_open = false; }
            else {
                active_popup = ids;
                popup_open = true;
                cp_id = ids;
                cp_ptr = v;
                float hh, ss, vv;
                ImGui::ColorConvertRGBtoHSV(v->x, v->y, v->z, hh, ss, vv);
                hsv_cache[ids] = ImVec4(hh, ss, vv, 1.f);
            }
            clicked = true;
        }

        float oa = anim(ids + "_o", open ? 1.f : 0.f, 15.f);
        ImDrawList* dl = w->DrawList;

        ImGui::PushFont(fontMedium);
        float fs = fontMedium->FontSize * S;
        const char* label_end = ImGui::FindRenderedTextEnd(name);
        ImVec2 ts = ImGui::CalcTextSize(name);
        dl->AddText(fontMedium, fs, ImVec2(r.Min.x + 12.f * S, r.GetCenter().y - fs * 0.5f), col(clr::text, a), name, label_end);
        ImGui::PopFont();

        float bw = 28.f * S;
        float bh = 18.f * S;
        ImRect btn(ImVec2(r.Max.x - bw - 12.f * S, r.GetCenter().y - bh*0.5f), ImVec2(r.Max.x - 12.f * S, r.GetCenter().y + bh*0.5f));

        dl->AddRectFilled(btn.Min, btn.Max, IM_COL32((int)(v->x*255), (int)(v->y*255), (int)(v->z*255), (int)(255*a)), 4.f * S);
        dl->AddRect(btn.Min, btn.Max, col(clr::glass_border, a), 4.f * S);

        if (oa > 0.01f && cp_ptr == v) {
            ImDrawList* fg = ImGui::GetForegroundDrawList();
            ImVec4& hsv = hsv_cache[ids];
            float sz = 210.f * S; 
            ImVec2 pmin(btn.Max.x - sz, btn.Max.y + 8.f * S);
            ImVec2 pmax(pmin.x + sz, pmin.y + sz + 60.f * S); 

            fg->AddRectFilled(pmin, pmax, col(ImVec4(20/255.f, 22/255.f, 28/255.f, 0.98f), oa), 12.f * S);

            for(int i = 1; i <= 12; i++)
                fg->AddRect(pmin - ImVec2(i*0.8f, i*0.8f), pmax + ImVec2(i*0.8f, i*0.8f), IM_COL32(0,0,0, (int)(25 * oa / i)), 12.f * S + i*0.5f);
                
            fg->AddRect(pmin, pmax, col(clr::glass_border, oa), 12.f * S);

            ImRect sv_rect(ImVec2(pmin.x + 10.f*S, pmin.y + 10.f*S), ImVec2(pmax.x - 10.f*S, pmax.y - 50.f*S));
            ImRect hue_rect(ImVec2(pmin.x + 10.f*S, pmax.y - 36.f*S), ImVec2(pmax.x - 10.f*S, pmax.y - 12.f*S));

            ImVec4 c_hue;
            ImGui::ColorConvertHSVtoRGB(hsv.x, 1.f, 1.f, c_hue.x, c_hue.y, c_hue.z);
            fg->AddRectFilledMultiColor(sv_rect.Min, sv_rect.Max, 
                IM_COL32(255,255,255, (int)(255*oa)), col(c_hue, oa), 
                col(c_hue, oa), IM_COL32(255,255,255, (int)(255*oa)));
            fg->AddRectFilledMultiColor(sv_rect.Min, sv_rect.Max, 
                IM_COL32(0,0,0,0), IM_COL32(0,0,0,0), 
                IM_COL32(0,0,0, (int)(255*oa)), IM_COL32(0,0,0, (int)(255*oa)));
            
            fg->AddRect(sv_rect.Min, sv_rect.Max, col(clr::glass_border, oa*0.5f), 4.f*S);

            for (int i = 0; i < 6; i++) {
                ImVec4 c1, c2;
                ImGui::ColorConvertHSVtoRGB(i / 6.f, 1.f, 1.f, c1.x, c1.y, c1.z);
                ImGui::ColorConvertHSVtoRGB((i + 1) / 6.f, 1.f, 1.f, c2.x, c2.y, c2.z);
                fg->AddRectFilledMultiColor(
                    ImVec2(hue_rect.Min.x + (hue_rect.GetWidth()/6.f)*i, hue_rect.Min.y),
                    ImVec2(hue_rect.Min.x + (hue_rect.GetWidth()/6.f)*(i+1), hue_rect.Max.y),
                    col(c1, oa), col(c2, oa), col(c2, oa), col(c1, oa)
                );
            }
            fg->AddRect(hue_rect.Min, hue_rect.Max, col(clr::glass_border, oa * 0.7f), 4.f*S);

            if (ImGui::IsMouseDown(0)) {
                ImVec2 mp = ImGui::GetIO().MousePos;
                if (picker.empty() && !clicked) {
                    if (sv_rect.Contains(mp)) picker = ids + "_sv";
                    else if (hue_rect.Contains(mp)) picker = ids + "_hue";
                }

                if (!picker.empty()) {
                    clicked = true; 
                    if (picker == ids + "_sv") {
                        hsv.y = ImClamp((mp.x - sv_rect.Min.x) / sv_rect.GetWidth(), 0.f, 1.f);
                        hsv.z = 1.f - ImClamp((mp.y - sv_rect.Min.y) / sv_rect.GetHeight(), 0.f, 1.f);
                    } else if (picker == ids + "_hue") {
                        hsv.x = ImClamp((mp.x - hue_rect.Min.x) / hue_rect.GetWidth(), 0.f, 1.f);
                    }
                    ImGui::ColorConvertHSVtoRGB(hsv.x, hsv.y, hsv.z, v->x, v->y, v->z);
                }
            } else {
                picker = "";
            }

            float cx = sv_rect.Min.x + hsv.y * sv_rect.GetWidth();
            float cy = sv_rect.Min.y + (1.f - hsv.z) * sv_rect.GetHeight();
            fg->AddCircle(ImVec2(cx, cy), 4.f*S, IM_COL32(255,255,255, (int)(255*oa)), 12, 2.f*S);
            
            float hx = hue_rect.Min.x + hsv.x * hue_rect.GetWidth();
            fg->AddRectFilled(ImVec2(hx - 2.f*S, hue_rect.Min.y - 2.f*S), ImVec2(hx + 2.f*S, hue_rect.Max.y + 2.f*S), IM_COL32(255,255,255, (int)(255*oa)), 2.f*S);
        }
    }

    void separator(float a) {
        if (a < 0.01f) return;

        ImGuiWindow* w = ImGui::GetCurrentWindow();
        ImVec2 pos = w->DC.CursorPos;
        float ww = content_w > 0 ? content_w : ImGui::GetContentRegionAvail().x;

        ImDrawList* dl = w->DrawList;
        dl->AddRectFilled(ImVec2(pos.x + 8.f*S, pos.y + 18.f*S), ImVec2(pos.x + ww - 8.f*S, pos.y + 19.f*S), col(clr::glass_border, a * 0.45f));
        ImGui::Dummy(ImVec2(0, 32.f * S));
    }

    bool button(const char* name, float a, std::function<void()> on_click) {
        if (a < 0.01f) return false;

        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems) return false;

        std::string ids = std::string("btn_") + name;
        ImGuiID id = w->GetID(ids.c_str());

        float ww = content_w > 0 ? content_w : ImGui::GetContentRegionAvail().x;
        float h = 40.f * S;

        ImVec2 pos = w->DC.CursorPos;
        ImRect r(pos, ImVec2(pos.x + ww, pos.y + h));

        ImGui::ItemSize(r);
        if (!ImGui::ItemAdd(r, id)) return false;

        bool hov = ImGui::IsMouseHoveringRect(r.Min, r.Max) && !popup();
        bool press = hov && ImGui::IsMouseClicked(0) && !clicked;
        if (press) clicked = true;

        float ha = anim(ids + "_h", hov ? 1.f : 0.f, 14.f);
        ImDrawList* dl = w->DrawList;

        ImVec4 bg_col = lrp_col(clr::widget, ImVec4(0,0,0,0.06f), ha);
        dl->AddRectFilled(r.Min, r.Max, col(bg_col, a), 12.f * S);
        dl->AddRect(r.Min, r.Max, col(clr::glass_border, a * (0.4f + ha * 0.2f)), 12.f * S);

        ImGui::PushFont(fontMedium);
        ImVec2 ts = ImGui::CalcTextSize(name);
        dl->AddText(fontMedium, ImGui::GetFontSize(), r.GetCenter() - ts * 0.5f, col(clr::text, a), name);
        ImGui::PopFont();

        if (press && on_click) on_click();
        return press;
    }
}

}

