#include "config_sys.hpp"
#include "cfg.hpp"
#include "../protect/oxorany.hpp"
#include "../game/game.hpp"
#include "../func/skinchanger.hpp"
#include "../func/skins_db.hpp"
#include "../../includes/internal/ImGui/imgui.h"
#include <fstream>
#include <filesystem>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <vector>

namespace ui {
    namespace config_sys {
        
        static std::string get_config_dir() {
            return oxorany("/data/property/.HASLINE/");
        }

        void init() {
            std::string dir = get_config_dir();
            mkdir(dir.c_str(), 0777);
        }

        static void encrypt_decrypt(std::vector<uint8_t>& data, uint32_t seed) {
            for (size_t i = 0; i < data.size(); ++i) {
                data[i] ^= (uint8_t)((seed >> ((i % 4) * 8)) & 0xFF);
                data[i] ^= 0x5A;
                seed = (seed * 1664525 + 1013904223);
            }
        }

        template<typename T>
        static void write_raw(std::vector<uint8_t>& buf, const T& val) {
            const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&val);
            for (size_t i = 0; i < sizeof(T); ++i) buf.push_back(ptr[i]);
        }

        template<typename T>
        static void read_raw(const std::vector<uint8_t>& buf, size_t& offset, T& val) {
            if (offset + sizeof(T) > buf.size()) return;
            val = *reinterpret_cast<const T*>(&buf[offset]);
            offset += sizeof(T);
        }

        void save(const std::string& name) {
            if (name.empty()) return;
            std::vector<uint8_t> data;
            uint32_t magic = 0xDEADBEEF;
            write_raw(data, magic);

            write_raw(data, cfg::esp::box);
            write_raw(data, cfg::esp::box_mode);
            write_raw(data, cfg::esp::box_thickness);
            write_raw(data, cfg::esp::box_rounding);
            write_raw(data, cfg::esp::box_col);
            write_raw(data, cfg::esp::line);
            write_raw(data, cfg::esp::line_pos);
            write_raw(data, cfg::esp::line_thickness);
            write_raw(data, cfg::esp::line_col);
            write_raw(data, cfg::esp::name);
            write_raw(data, cfg::esp::name_col);
            write_raw(data, cfg::esp::health);
            write_raw(data, cfg::esp::health_col);
            write_raw(data, cfg::esp::distance);
            write_raw(data, cfg::esp::distance_col);
            write_raw(data, cfg::esp::skeleton);
            write_raw(data, cfg::esp::skeleton_thickness);
            write_raw(data, cfg::esp::skeleton_col);
            write_raw(data, cfg::esp::team_check);
            write_raw(data, cfg::esp::visible_only);
            write_raw(data, cfg::esp::max_dist);
            write_raw(data, cfg::esp::joint_col);
            write_raw(data, cfg::esp::joint_size);
            write_raw(data, cfg::esp::anime_esp);
            write_raw(data, cfg::esp::anime_scale);
            write_raw(data, cfg::esp::head_esp);
            write_raw(data, cfg::esp::head_esp_size);
            write_raw(data, cfg::esp::head_esp_col);
            write_raw(data, cfg::esp::tracers);
            write_raw(data, cfg::esp::tracers_duration);
            write_raw(data, cfg::esp::tracers_thickness);
            write_raw(data, cfg::esp::tracers_col);


            write_raw(data, cfg::aim::enabled);
            write_raw(data, cfg::aim::visible_check);
            write_raw(data, cfg::aim::triggerbot);
            write_raw(data, cfg::aim::trigger_delay);
            write_raw(data, cfg::aim::trigger_range);
            write_raw(data, cfg::aim::trigger_visible_only);
            write_raw(data, cfg::aim::trigger_bone_mask);
            write_raw(data, cfg::aim::show_fov);
            write_raw(data, cfg::aim::fov);
            write_raw(data, cfg::aim::fov_thickness);
            write_raw(data, cfg::aim::fov_col);
            write_raw(data, cfg::aim::smooth);
            write_raw(data, cfg::aim::bone);

            write_raw(data, cfg::misc::stream_proof);
            write_raw(data, cfg::misc::menu_scale);

            uint32_t skin_count = (uint32_t)cfg::skins::user_inventory.size();
            write_raw(data, skin_count);
            for (const auto& item : cfg::skins::user_inventory) {
                write_raw(data, item.applied_id);
            }

            uint32_t seed = 0;
            for (char c : name) seed = (seed * 31) + c;
            encrypt_decrypt(data, seed);

            std::string path = get_config_dir() + name + oxorany(".bin");
            std::ofstream file(path, std::ios::binary);
            if (file.is_open()) {
                file.write(reinterpret_cast<const char*>(data.data()), data.size());
            }
        }

        void load(const std::string& name) {
            std::string path = get_config_dir() + name + oxorany(".bin");
            std::ifstream file(path, std::ios::binary);
            if (!file.is_open()) return;

            std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            file.close();
            if (data.empty()) return;

            uint32_t seed = 0;
            for (char c : name) seed = (seed * 31) + c;
            encrypt_decrypt(data, seed);

            size_t offset = 0;
            uint32_t magic;
            read_raw(data, offset, magic);
            if (magic != 0xDEADBEEF) return;

            read_raw(data, offset, cfg::esp::box);
            read_raw(data, offset, cfg::esp::box_mode);
            read_raw(data, offset, cfg::esp::box_thickness);
            read_raw(data, offset, cfg::esp::box_rounding);
            read_raw(data, offset, cfg::esp::box_col);
            read_raw(data, offset, cfg::esp::line);
            read_raw(data, offset, cfg::esp::line_pos);
            read_raw(data, offset, cfg::esp::line_thickness);
            read_raw(data, offset, cfg::esp::line_col);
            read_raw(data, offset, cfg::esp::name);
            read_raw(data, offset, cfg::esp::name_col);
            read_raw(data, offset, cfg::esp::health);
            read_raw(data, offset, cfg::esp::health_col);
            read_raw(data, offset, cfg::esp::distance);
            read_raw(data, offset, cfg::esp::distance_col);
            read_raw(data, offset, cfg::esp::skeleton);
            read_raw(data, offset, cfg::esp::skeleton_thickness);
            read_raw(data, offset, cfg::esp::skeleton_col);
            read_raw(data, offset, cfg::esp::team_check);
            read_raw(data, offset, cfg::esp::visible_only);
            read_raw(data, offset, cfg::esp::max_dist);
            read_raw(data, offset, cfg::esp::joint_col);
            read_raw(data, offset, cfg::esp::joint_size);
            read_raw(data, offset, cfg::esp::anime_esp);
            read_raw(data, offset, cfg::esp::anime_scale);
            read_raw(data, offset, cfg::esp::head_esp);
            read_raw(data, offset, cfg::esp::head_esp_size);
            read_raw(data, offset, cfg::esp::head_esp_col);
            read_raw(data, offset, cfg::esp::tracers);
            read_raw(data, offset, cfg::esp::tracers_duration);
            read_raw(data, offset, cfg::esp::tracers_thickness);
            read_raw(data, offset, cfg::esp::tracers_col);


            read_raw(data, offset, cfg::aim::enabled);
            read_raw(data, offset, cfg::aim::visible_check);
            read_raw(data, offset, cfg::aim::triggerbot);
            read_raw(data, offset, cfg::aim::trigger_delay);
            read_raw(data, offset, cfg::aim::trigger_range);
            read_raw(data, offset, cfg::aim::trigger_visible_only);
            read_raw(data, offset, cfg::aim::trigger_bone_mask);
            read_raw(data, offset, cfg::aim::show_fov);
            read_raw(data, offset, cfg::aim::fov);
            read_raw(data, offset, cfg::aim::fov_thickness);
            read_raw(data, offset, cfg::aim::fov_col);
            read_raw(data, offset, cfg::aim::smooth);
            read_raw(data, offset, cfg::aim::bone);

            read_raw(data, offset, cfg::misc::stream_proof);
            read_raw(data, offset, cfg::misc::menu_scale);

            uint32_t skin_count = 0;
            read_raw(data, offset, skin_count);
            
            if (skin_count > 0) {
                skins::rebuild_inventory();
                
                if (cfg::skins::user_inventory.size() > 0) {
                    int apply_limit = std::min((int)skin_count, (int)cfg::skins::user_inventory.size());
                    
                    for (int i = 0; i < apply_limit; ++i) {
                        int saved_id = 0;
                        read_raw(data, offset, saved_id);
                        
                        if (saved_id > 0) {
                            uint64_t ptr = cfg::skins::user_inventory[i].ptr;
                            if (ptr) {
                                wpm<int>(ptr + 0x10, saved_id); 
                                cfg::skins::user_inventory[i].applied_id = saved_id;     
                                const auto& all_defs = skins::g_skin_database;
                                for(int d=0; d<(int)all_defs.size(); ++d) {
                                    if(all_defs[d].id == saved_id) {
                                        cfg::skins::user_inventory[i].def_index = d;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        void delete_config(const std::string& name) {
            std::string path = get_config_dir() + name + oxorany(".bin");
            unlink(path.c_str());
        }

        std::vector<std::string> get_configs() {
            std::vector<std::string> configs;
            DIR* dir = opendir(get_config_dir().c_str());
            if (dir) {
                struct dirent* entry;
                while ((entry = readdir(dir)) != NULL) {
                    std::string fname = entry->d_name;
                    if (fname.length() > 4 && fname.substr(fname.length() - 4) == oxorany(".bin")) {
                        configs.push_back(fname.substr(0, fname.length() - 4));
                    }
                }
                closedir(dir);
            }
            return configs;
        }
        
        void apply_skins() {
        }
    }
}
