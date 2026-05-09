#pragma once

#include "game.hpp"
#include "../other/vector3.h"
#include "../other/string.h"
#include "../protect/oxorany.hpp"
#include <cstring>
#include <string>
#include <vector>
#include <unordered_map>

namespace player {
    inline Vector3 position(uint64_t p) noexcept {
        uint64_t view = rpm<uint64_t>(p + offsets::player::player_character_view);
        if (view) {
            uint64_t transform = rpm<uint64_t>(view + oxorany(0x10));
            if (transform) {
                uint64_t internal_transform = rpm<uint64_t>(transform + offsets::transform::internal_transform);
                if (internal_transform) {
                    return rpm<Vector3>(internal_transform + oxorany(0x44));
                }
            }
        }

        uint64_t MovementController = rpm<uint64_t>(p + offsets::player::movement_controller);
        if (!MovementController) return Vector3(0, 0, 0);

        uint64_t TransformData = rpm<uint64_t>(MovementController + offsets::movement::translation_data);
        if (!TransformData) return Vector3(0, 0, 0);

        return rpm<Vector3>(TransformData + oxorany(0x44));
    }

    inline Vector3 get_transform_position(uint64_t transform) {
        if (!transform) return {0,0,0};
        uint64_t internal_transform = rpm<uint64_t>(transform + offsets::transform::internal_transform);
        if (!internal_transform) return {0,0,0};

        uint32_t index = rpm<uint32_t>(internal_transform + oxorany(0x40));
        uint64_t matrix_data = rpm<uint64_t>(internal_transform + oxorany(0x38));
        if (!matrix_data) return {0,0,0};

        uint64_t matrix_list = rpm<uint64_t>(matrix_data + oxorany(0x18));
        uint64_t matrix_indices = rpm<uint64_t>(matrix_data + oxorany(0x20));
        if (!matrix_list || !matrix_indices) return {0,0,0};

        offsets::TMatrix tm = rpm<offsets::TMatrix>(matrix_list + index * sizeof(offsets::TMatrix));
        Vector3 result = {tm.position.x, tm.position.y, tm.position.z};
        int p_idx = rpm<int>(matrix_indices + index * sizeof(int));

        int depth = 0;
        while (p_idx >= 0 && depth < 64) {
            offsets::TMatrix p_tm = rpm<offsets::TMatrix>(matrix_list + p_idx * sizeof(offsets::TMatrix));

            float rotX = p_tm.rotation.x;
            float rotY = p_tm.rotation.y;
            float rotZ = p_tm.rotation.z;
            float rotW = p_tm.rotation.w;

            float scaleX = result.x * p_tm.scale.x;
            float scaleY = result.y * p_tm.scale.y;
            float scaleZ = result.z * p_tm.scale.z;

            result.x = p_tm.position.x + scaleX +
                       (scaleX * ((rotY * rotY * -2.0f) - (rotZ * rotZ * 2.0f))) +
                       (scaleY * ((rotW * rotZ * -2.0f) - (rotY * rotX * -2.0f))) +
                       (scaleZ * ((rotZ * rotX * 2.0f) - (rotW * rotY * -2.0f)));
            result.y = p_tm.position.y + scaleY +
                       (scaleX * ((rotX * rotY * 2.0f) - (rotW * rotZ * -2.0f))) +
                       (scaleY * ((rotZ * rotZ * -2.0f) - (rotX * rotX * 2.0f))) +
                       (scaleZ * ((rotW * rotX * -2.0f) - (rotZ * rotY * -2.0f)));
            result.z = p_tm.position.z + scaleZ +
                       (scaleX * ((rotW * rotY * -2.0f) - (rotX * rotZ * -2.0f))) +
                       (scaleY * ((rotY * rotZ * 2.0f) - (rotW * rotX * -2.0f))) +
                       (scaleZ * ((rotX * rotX * -2.0f) - (rotY * rotY * 2.0f)));

            p_idx = rpm<int>(matrix_indices + p_idx * sizeof(int));
            depth++;
        }
        return result;
    }

    inline Vector3 camera_position(uint64_t p) noexcept {
        if (!p) return {0,0,0};

    
        uint64_t transform = rpm<uint64_t>(p + oxorany(0x28));
        if (transform) {
            return get_transform_position(transform);
        }
        

        uint64_t camera = rpm<uint64_t>(p + offsets::player::main_camera);
        if (camera) {
            uint64_t camera_transform = rpm<uint64_t>(camera + oxorany(0x38));
            if (camera_transform) {
                return get_transform_position(camera_transform);
            }
        }
        
        return position(p);
    }

    inline uint64_t photon_ptr(uint64_t p) noexcept {
        return rpm<uint64_t>(p + offsets::player::photon_player);
    }

    struct bones_t {
        Vector3 head;           
        Vector3 neck;          
        Vector3 spine;          
        Vector3 spine1;         
        Vector3 spine2;         
        Vector3 l_shoulder;     
        Vector3 l_arm;          
        Vector3 l_forearm;      
        Vector3 l_hand;         
        Vector3 r_shoulder;     
        Vector3 r_arm;          
        Vector3 r_forearm;       
        Vector3 r_hand;         
        Vector3 pelvis;         
        Vector3 l_thigh;        
        Vector3 l_knee;         
        Vector3 l_foot;         
        Vector3 l_toe;          
        Vector3 r_thigh;        
        Vector3 r_knee;         
        Vector3 r_foot;         
        Vector3 r_toe;          

        Vector3& operator[](int i) {
            return ((Vector3*)this)[i];
        }
    };

    struct bone_cache_entry_t {
        Vector3 offsets[22];
        bool valid = false;
    };
    inline std::unordered_map<uint64_t, bone_cache_entry_t> global_bone_cache;
    inline uint64_t last_player_manager = 0;

    inline void apply_default_pose(bones_t& b, const Vector3& root) {
        b.head = root + Vector3(0, 1.75f, 0);
        b.neck = root + Vector3(0, 1.6f, 0);
        b.spine2 = root + Vector3(0, 1.45f, 0);
        b.spine1 = root + Vector3(0, 1.25f, 0);
        b.spine = root + Vector3(0, 1.05f, 0);
        b.pelvis = root + Vector3(0, 0.9f, 0);
        b.l_shoulder = root + Vector3(-0.2f, 1.55f, 0);
        b.l_arm = root + Vector3(-0.4f, 1.5f, 0);
        b.l_forearm = root + Vector3(-0.4f, 1.2f, 0);
        b.l_hand = root + Vector3(-0.4f, 1.0f, 0);
        b.r_shoulder = root + Vector3(0.2f, 1.55f, 0);
        b.r_arm = root + Vector3(0.4f, 1.5f, 0);
        b.r_forearm = root + Vector3(0.4f, 1.2f, 0);
        b.r_hand = root + Vector3(0.4f, 1.0f, 0);
        b.l_thigh = root + Vector3(-0.15f, 0.9f, 0);
        b.l_knee = root + Vector3(-0.15f, 0.45f, 0);
        b.l_foot = root + Vector3(-0.15f, 0.05f, 0);
        b.l_toe = root + Vector3(-0.15f, 0.0f, 0.1f);
        b.r_thigh = root + Vector3(0.15f, 0.9f, 0);
        b.r_knee = root + Vector3(0.15f, 0.45f, 0);
        b.r_foot = root + Vector3(0.15f, 0.05f, 0);
        b.r_toe = root + Vector3(0.15f, 0.0f, 0.1f);
    }

    inline bool is_visible(uint64_t p) noexcept {
        if (!p) return false;

        uint64_t objectOccludee = rpm<uint64_t>(p + offsets::player::occlusion_controller);
        if (objectOccludee > 0x1000 && objectOccludee < 0x7FFFFFFFFFFF) {
            int visibilityState = rpm<int>(objectOccludee + oxorany(0x34));
            int occlusionState = rpm<int>(objectOccludee + oxorany(0x38));
            return (visibilityState == 2 && occlusionState != 1);
        }
        
        uint64_t view = rpm<uint64_t>(p + offsets::player::player_character_view);
        if (view > 0x1000 && view < 0x7FFFFFFFFFFF) {
            return rpm<bool>(view + oxorany(0x30));
        }
        
        return false;
    }

    inline bool is_bone_visible(uint64_t p, int bone_idx) noexcept {
        if (!p) return false;
        
        uint64_t view = rpm<uint64_t>(p + offsets::player::player_character_view);
        if (!view) return false;

        uint64_t map = rpm<uint64_t>(view + offsets::view::biped_map);
        if (!map) return false;

        uint64_t bone_ptr = rpm<uint64_t>(map + oxorany(0x20) + (bone_idx * 8));
        if (!bone_ptr) return false;
        return is_visible(p); 
    }

    inline bool get_bones(uint64_t p, bones_t& b) noexcept {
        uint64_t current_pm = get_player_manager();
        if (current_pm != last_player_manager) {
            global_bone_cache.clear();
            last_player_manager = current_pm;
        }

        Vector3 root = position(p);
        if (root.x == 0 && root.y == 0 && root.z == 0) return false;

        auto apply_cache_or_default = [&]() {
            auto it = global_bone_cache.find(p);
            if (it != global_bone_cache.end() && it->second.valid) {
                for (int i = 0; i < 22; i++) b[i] = root + it->second.offsets[i];
                return true;
            }
            apply_default_pose(b, root);
            return true;
        };

        if (!is_visible(p)) {
            return apply_cache_or_default();
        }

        uint64_t view = rpm<uint64_t>(p + offsets::player::player_character_view);
        if (!view) return apply_cache_or_default();

        uint64_t map = rpm<uint64_t>(view + offsets::view::biped_map);
        if (!map) return apply_cache_or_default();

        uint64_t ptrs[22];
        if (!mem_read(map + oxorany(0x20), ptrs, sizeof(ptrs))) return apply_cache_or_default();

        uint32_t transform_indices[22];
        uint64_t matrix_list = 0;
        uint64_t matrix_indices = 0;
        uint32_t max_index = 0;

        for (int i = 0; i < 22; i++) {
            transform_indices[i] = 0xFFFFFFFF;
            if (ptrs[i]) {
                uint64_t internal_transform = rpm<uint64_t>(ptrs[i] + offsets::transform::internal_transform);
                if (internal_transform) {
                    uint32_t idx = rpm<uint32_t>(internal_transform + oxorany(0x40));
                    transform_indices[i] = idx;
                    if (idx > max_index && idx < 10000) max_index = idx;

                    if (!matrix_list) {
                        uint64_t matrix_data = rpm<uint64_t>(internal_transform + oxorany(0x38));
                        if (matrix_data) {
                            matrix_list = rpm<uint64_t>(matrix_data + oxorany(0x18));
                            matrix_indices = rpm<uint64_t>(matrix_data + oxorany(0x20));
                        }
                    }
                }
            }
        }

        if (!matrix_list || !matrix_indices || max_index == 0) return apply_cache_or_default();

        uint32_t count = max_index + 1;
        if (count > 10000) return apply_cache_or_default();

        std::vector<offsets::TMatrix> all_matrices(count);
        std::vector<int> all_parents(count);

        if (!mem_read(matrix_list, all_matrices.data(), count * sizeof(offsets::TMatrix))) return apply_cache_or_default();
        if (!mem_read(matrix_indices, all_parents.data(), count * sizeof(int))) return apply_cache_or_default();

        auto get_bone_pos_from_cache = [&](uint32_t index) -> Vector3 {
            if (index >= count) return {0,0,0};
            
            offsets::TMatrix tm = all_matrices[index];
            Vector3 result = {tm.position.x, tm.position.y, tm.position.z};
            int p_idx = all_parents[index];

            int depth = 0;
            while (p_idx >= 0 && p_idx < (int)count && depth < 64) {
                offsets::TMatrix p_tm = all_matrices[p_idx];

                float rotX = p_tm.rotation.x;
                float rotY = p_tm.rotation.y;
                float rotZ = p_tm.rotation.z;
                float rotW = p_tm.rotation.w;

                float scaleX = result.x * p_tm.scale.x;
                float scaleY = result.y * p_tm.scale.y;
                float scaleZ = result.z * p_tm.scale.z;

                result.x = p_tm.position.x + scaleX +
                           (scaleX * ((rotY * rotY * -2.0f) - (rotZ * rotZ * 2.0f))) +
                           (scaleY * ((rotW * rotZ * -2.0f) - (rotY * rotX * -2.0f))) +
                           (scaleZ * ((rotZ * rotX * 2.0f) - (rotW * rotY * -2.0f)));
                result.y = p_tm.position.y + scaleY +
                           (scaleX * ((rotX * rotY * 2.0f) - (rotW * rotZ * -2.0f))) +
                           (scaleY * ((rotZ * rotZ * -2.0f) - (rotX * rotX * 2.0f))) +
                           (scaleZ * ((rotW * rotX * -2.0f) - (rotZ * rotY * -2.0f)));
                result.z = p_tm.position.z + scaleZ +
                           (scaleX * ((rotW * rotY * -2.0f) - (rotX * rotZ * -2.0f))) +
                           (scaleY * ((rotY * rotZ * 2.0f) - (rotW * rotX * -2.0f))) +
                           (scaleZ * ((rotX * rotX * -2.0f) - (rotY * rotY * 2.0f)));

                p_idx = all_parents[p_idx];
                depth++;
            }
            return result;
        };

        bool any_valid = false;
        for (int i = 0; i < 22; i++) {
            if (transform_indices[i] != 0xFFFFFFFF) {
                b[i] = get_bone_pos_from_cache(transform_indices[i]);
                if (b[i].x != 0 || b[i].y != 0) any_valid = true;
            } else {
                b[i] = {0, 0, 0};
            }
        }

        if (any_valid) {
            float dist_to_root = (b[1] - root).magnitude();
            if (dist_to_root > 0.1f && dist_to_root < 3.0f) {
                auto& cache = global_bone_cache[p];
                for (int i = 0; i < 22; i++) cache.offsets[i] = b[i] - root;
                cache.valid = true;
                return true;
            }
        }

        return apply_cache_or_default();
    }

    template<typename T>
    inline T property(uint64_t p, const char* tag) noexcept {
        T result{};
        uint64_t PhotonPlayer = photon_ptr(p);
        if (!PhotonPlayer) return result;

        uint64_t PropertiesRegistry = rpm<uint64_t>(PhotonPlayer + oxorany(0x38));
        if (!PropertiesRegistry) return result;

        int Count = rpm<int>(PropertiesRegistry + oxorany(0x20));
        if (Count <= 0) return result;

        uint64_t PropertiesList = rpm<uint64_t>(PropertiesRegistry + oxorany(0x18));
        if (!PropertiesList) return result;

        for (int i = 0; i < Count; i++) {
            uint64_t Key = rpm<uint64_t>(PropertiesList + oxorany(0x28) + oxorany(0x18) * i);
            if (!Key) continue;

            read_string KeyString = rpm<read_string>(Key);
            string KeyStr = KeyString.as_utf8();

            if (strstr(KeyStr.c_str(), tag)) {
                uint64_t Value = rpm<uint64_t>(PropertiesList + oxorany(0x30) + oxorany(0x18) * i);
                if (Value) result = rpm<T>(Value + oxorany(0x10));
                break;
            }
        }

        return result;
    }

    template<typename T>
    inline bool set_property(uint64_t p, const char* tag, const T& value) noexcept {
        uint64_t PhotonPlayer = photon_ptr(p);
        if (!PhotonPlayer) return false;

        uint64_t PropertiesRegistry = rpm<uint64_t>(PhotonPlayer + oxorany(0x38));
        if (!PropertiesRegistry) return false;

        int Count = rpm<int>(PropertiesRegistry + oxorany(0x20));
        if (Count <= 0) return false;

        uint64_t PropertiesList = rpm<uint64_t>(PropertiesRegistry + oxorany(0x18));
        if (!PropertiesList) return false;

        for (int i = 0; i < Count; i++) {
            uint64_t Key = rpm<uint64_t>(PropertiesList + oxorany(0x28) + oxorany(0x18) * i);
            if (!Key) continue;

            read_string KeyString = rpm<read_string>(Key);
            string KeyStr = KeyString.as_utf8();

            if (strstr(KeyStr.c_str(), tag)) {
                uint64_t Value = rpm<uint64_t>(PropertiesList + oxorany(0x30) + oxorany(0x18) * i);
                if (!Value) return false;
                return wpm(Value + oxorany(0x10), value);
            }
        }

        return false;
    }

    inline int health(uint64_t p) noexcept {
        return property<int>(p, oxorany("health"));
    }

    inline read_string name(uint64_t p) noexcept {
        uint64_t PhotonPlayer = photon_ptr(p);
        if (!PhotonPlayer) return {};
        return rpm<read_string>(rpm<uint64_t>(PhotonPlayer + oxorany(0x20)));
    }

    inline matrix view_matrix(uint64_t p) noexcept {
        if (!p) return {};

        uint64_t camera = rpm<uint64_t>(p + offsets::player::main_camera); 
        if (!camera) return {};

        uint64_t camera_obj = rpm<uint64_t>(camera + oxorany(0x20));
        if (!camera_obj) return {};

        uint64_t internal_camera = rpm<uint64_t>(camera_obj + oxorany(0x10));
        if (!internal_camera) return {};

        return rpm<matrix>(internal_camera + oxorany(0x100));
    }
}
