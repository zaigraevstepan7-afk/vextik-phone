#pragma once
#include "../../includes/internal/ImGui/imgui.h"
#include <vector>

namespace cfg {

namespace esp {
    inline bool box = false;
    inline int box_mode = 0; 
    inline float box_thickness = 1.0f;
    inline float box_rounding = 0.f;
    inline ImVec4 box_col = ImVec4(1.f, 1.f, 1.f, 1.f);

    inline bool line = false;
    inline int line_pos = 0;
    inline float line_thickness = 1.0f;
    inline ImVec4 line_col = ImVec4(1.f, 1.f, 1.f, 1.f);

    inline bool name = false;
    inline ImVec4 name_col = ImVec4(1.f, 1.f, 1.f, 1.f);

    inline bool health = false;
    inline ImVec4 health_col = ImVec4(1.f, 1.f, 1.f, 1.f);

    inline bool distance = false;
    inline ImVec4 distance_col = ImVec4(1.f, 1.f, 1.f, 1.f);

    inline bool skeleton = false;
    inline float skeleton_thickness = 1.2f;
    inline ImVec4 skeleton_col = ImVec4(1.f, 1.f, 1.f, 1.f);
    
    inline bool team_check = true;
    inline bool visible_only = false;
    inline float max_dist = 400.f;

    inline ImVec4 joint_col = ImVec4(1.f, 1.f, 1.f, 1.f);
    inline float joint_size = 2.5f;

    inline bool anime_esp = false;
    inline float anime_scale = 1.0f;

    inline bool head_esp = false;
    inline float head_esp_size = 1.0f;
    inline ImVec4 head_esp_col = ImVec4(1.f, 1.f, 1.f, 1.f);

    inline bool tracers = false;
    inline float tracers_duration = 1.5f;
    inline float tracers_thickness = 0.8f;
    inline ImVec4 tracers_col = ImVec4(1.f, 1.f, 1.f, 1.f);
}

namespace aim {
    inline bool enabled = false;
    inline bool visible_check = false;
    inline bool triggerbot = false;
    inline float trigger_delay = 0.05f;
    inline float trigger_range = 10.0f;
    inline bool trigger_visible_only = true;
    inline int trigger_bone_mask = 3;
    inline bool show_fov = false;
    inline float fov = 90.f;
    inline float fov_thickness = 1.0f;
    inline ImVec4 fov_col = ImVec4(1.f, 1.f, 1.f, 1.f);
    inline float smooth = 5.f;
    inline int bone = 0;
}

namespace misc {
    inline bool no_recoil = false;
    inline bool fast_reload = false;
    inline bool inf_ammo = false;
    inline bool stream_proof = false;
    inline bool watermark = true;
    inline float menu_scale = 1.0f;
}

namespace skins {
    struct UserItem {
        int def_index; 
        uint64_t ptr;  
        int applied_id = 0; 
    };
    
    inline std::vector<UserItem> user_inventory;
    inline bool inventory_initialized = false;
    inline char refresh_label[64] = "Refresh Skins";
    inline int selected_inventory = -1;
}

}
