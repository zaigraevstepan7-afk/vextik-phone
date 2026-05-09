#pragma once
#include "../game/math.hpp"
#include "../../includes/internal/ImGui/imgui.h"
#include "../../includes/internal/ImGui/imgui_internal.h"
#include <vector>
#include "../game/game.hpp"

namespace visuals {
    void draw();
    void skeleton(uint64_t player, const matrix& view_matrix, float a);
    void dbox(const ImVec2& t, const ImVec2& b, float a);
    void dline(const ImVec2& foot, const ImVec2& head, float a);
    void dhp(int hp, const ImVec2& t, const ImVec2& b, float box_h, float a);
    void dnick(const char* name, const ImVec2& box_min, float cx, float sz, float a);
    void ddist(float dist, float x, float y, float sz, float a);
    void draw_text_outlined(ImDrawList* dl, ImFont* font, float size, const ImVec2& pos, ImU32 color, const char* text);
    
    void update_tracers(uint64_t local_player);
    void draw_tracers(const matrix& view_matrix);
}
