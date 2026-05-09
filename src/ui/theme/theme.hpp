#pragma once
#include "../../../includes/internal/ImGui/imgui.h"
#include "../../../includes/internal/ImGui/imgui_internal.h"
#include <string>
#include <unordered_map>

inline float g_sw, g_sh;

namespace ui {

namespace clr {
    inline ImVec4 bg           = ImVec4(12/255.f, 15/255.f, 22/255.f, 0.92f); 
    inline ImVec4 sidebar      = ImVec4(255/255.f, 255/255.f, 255/255.f, 0.04f);
    inline ImVec4 panel        = ImVec4(255/255.f, 255/255.f, 255/255.f, 0.07f);
    inline ImVec4 widget       = ImVec4(255/255.f, 255/255.f, 255/255.f, 0.05f);
    
    inline ImVec4 accent       = ImVec4(255/255.f, 255/255.f, 255/255.f, 1.0f); 
    inline ImVec4 accent_green = ImVec4(48/255.f, 209/255.f, 88/255.f, 1.f);
    inline ImVec4 accent_pink  = ImVec4(255/255.f, 55/255.f, 95/255.f, 1.f); 
    inline ImVec4 accent_purple = ImVec4(191/255.f, 90/255.f, 242/255.f, 1.f);
    
    inline ImVec4 text         = ImVec4(255/255.f, 255/255.f, 255/255.f, 1.0f);
    inline ImVec4 text_dim     = ImVec4(255/255.f, 255/255.f, 255/255.f, 0.6f);
    
    inline ImVec4 border       = ImVec4(255/255.f, 255/255.f, 255/255.f, 0.12f);
    inline ImVec4 glass_border = ImVec4(255/255.f, 255/255.f, 255/255.f, 0.20f);
    inline ImVec4 shadow       = ImVec4(0/255.f, 0/255.f, 0/255.f, 0.25f);
}

namespace style {
    inline std::unordered_map<std::string, float> anims;
    inline std::unordered_map<std::string, ImVec4> anim_colors;
    inline float S = 1.0f;
    inline float content_w = 0.f;
    inline float content_alpha = 1.f;
    inline bool popup_open = false;
    inline std::string active_popup = "";
    inline bool clicked = false;

    void tick();
    float anim(const std::string& id, float tgt, float spd = 12.f);
    ImVec4 anim_col(const std::string& id, const ImVec4& tgt, float spd = 12.f);
    ImU32 col(const ImVec4& c, float a = 1.f);
    inline ImVec4 col_to_vec4(ImU32 c) {
        return ImVec4(((c >> 0) & 0xFF) / 255.f, ((c >> 8) & 0xFF) / 255.f, ((c >> 16) & 0xFF) / 255.f, ((c >> 24) & 0xFF) / 255.f);
    }
    bool popup();
    void close();
    void popups();
}

}


