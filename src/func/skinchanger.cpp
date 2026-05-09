#include "skinchanger.hpp"
#include <cstring>
#include <algorithm>
#include <unordered_map>
#include <string>
#include "../ui/cfg.hpp" 

namespace skins {

SkinChanger::SkinChanger() {
    selected_skin_index = 0;
    active_skin_id = 0;
}

const SkinDefinition* SkinChanger::current_skin() const {
    const auto& defs = g_skin_database;
    if (defs.empty()) {
        return nullptr;
    }
    int index = selected_skin_index;
    if (index < 0 || index >= (int)defs.size()) {
        index = 0;
    }
    return &defs[index];
}

bool SkinChanger::apply_to_local() {
    const SkinDefinition* skin = current_skin();
    if (!skin) {
        return false;
    }
    active_skin_id = skin->id;
    uint64_t PlayerManager = get_player_manager();
    if (!PlayerManager) {
        return false;
    }
    uint64_t LocalPlayer = rpm<uint64_t>(PlayerManager + oxorany(0x70));
    if (!LocalPlayer) {
        return false;
    }
    return player::set_property<int>(LocalPlayer, oxorany("weaponSkinId"), active_skin_id);
}

SkinChanger g_skinchanger;

void rebuild_inventory() {
    cfg::skins::user_inventory.clear();
    const auto& all_defs = skins::g_skin_database;

    auto check_collection = [&](uint64_t ptr, int id_offset, bool is_dict) -> int {
        if (!ptr || ptr < 0x100000) return 0;
        int count = 0;
        int valid_skins = 0;
        if (is_dict) {
            uint64_t entries = rpm<uint64_t>(ptr + 0x18);
            if (!entries) return 0;
            int capacity = rpm<int>(entries + 0x18);
            if (capacity <= 0 || capacity > 20000) return 0;
            int checks = 0;
            for (int i = 0; i < capacity && checks < 50; ++i) {
                uint64_t entry = entries + 0x20 + (uint64_t)i * 0x18;
                uint64_t value = rpm<uint64_t>(entry + 0x10);
                if (!value) continue;
                checks++;
                int skin_id = rpm<int>(value + id_offset);
                if (skin_id > 0 && skin_id < 30000) valid_skins++;
            }
        } else {
            uint64_t items = rpm<uint64_t>(ptr + 0x10);
            if (!items) return 0;
            int size = rpm<int>(ptr + 0x18);
            if (size <= 0 || size > 20000) return 0;
            int checks = 0;
            for (int i = 0; i < size && checks < 50; ++i) {
                uint64_t item = rpm<uint64_t>(items + 0x20 + (uint64_t)i * 8);
                if (!item) continue;
                checks++;
                int skin_id = rpm<int>(item + id_offset);
                if (skin_id > 0 && skin_id < 30000) valid_skins++;
            }
        }
        return valid_skins;
    };

    auto extract_skins = [&](uint64_t ptr, int id_offset, bool is_dict) {
        if (is_dict) {
            uint64_t entries = rpm<uint64_t>(ptr + 0x18);
            int capacity = rpm<int>(entries + 0x18);
            for (int i = 0; i < capacity; ++i) {
                uint64_t entry = entries + 0x20 + (uint64_t)i * 0x18;
                uint64_t value = rpm<uint64_t>(entry + 0x10);
                if (!value) continue;
                
                int skin_id_10 = rpm<int>(value + 0x10);
                int skin_id_14 = rpm<int>(value + 0x14);
                int skin_id_18 = rpm<int>(value + 0x18);
                
                int skin_id = skin_id_10; 
                if (skin_id <= 0) {
                     if (skin_id_14 > 0) skin_id = skin_id_14;
                     else if (skin_id_18 > 0) skin_id = skin_id_18;
                }

                if (skin_id <= 0) continue;
                
                for (int idx = 0; idx < (int)all_defs.size(); ++idx) {
                    if (all_defs[idx].id == skin_id) {
                        cfg::skins::user_inventory.push_back({idx, value, skin_id});
                        break;
                    }
                }
            }
        } else {
            uint64_t items = rpm<uint64_t>(ptr + 0x10);
            int size = rpm<int>(ptr + 0x18);
            for (int i = 0; i < size; ++i) {
                uint64_t item = rpm<uint64_t>(items + 0x20 + (uint64_t)i * 8);
                if (!item) continue;
                
                int skin_id_10 = rpm<int>(item + 0x10);
                int skin_id_14 = rpm<int>(item + 0x14);
                int skin_id_18 = rpm<int>(item + 0x18);
                int skin_id_30 = rpm<int>(item + 0x30);
                
                int skin_id = skin_id_10;
                 if (skin_id <= 0) {
                     if (skin_id_14 > 0) skin_id = skin_id_14;
                     else if (skin_id_18 > 0) skin_id = skin_id_18;
                     else if (skin_id_30 > 0) skin_id = skin_id_30;
                }
                
                if (skin_id <= 0) continue;
                for (int idx = 0; idx < (int)all_defs.size(); ++idx) {
                    if (all_defs[idx].id == skin_id) {
                        cfg::skins::user_inventory.push_back({idx, item, skin_id});
                        break;
                    }
                }
            }
        }
    };

    uint64_t lib = proc::lib;
    if (!lib) {
        snprintf(cfg::skins::refresh_label, sizeof(cfg::skins::refresh_label), "Err: No Lib");
        return;
    }

    auto get_singleton_instance = [&](uint64_t class_ptr) -> uint64_t {
        if (!class_ptr || class_ptr < 0x100000) return 0;
        uint64_t parent = rpm<uint64_t>(class_ptr + 0x100);
        if (!parent || parent < 0x100000) return 0;
        uint64_t statics = rpm<uint64_t>(parent + 0x130);
        if (!statics || statics < 0x100000) return 0;
        return rpm<uint64_t>(statics + 0x0);
    };

    bool manager_found = false;
    uint64_t manager_inst = 0;

    uint64_t melodium_offset = offsets::inventory_manager;
    
    uint64_t hardcoded_class = rpm<uint64_t>(lib + melodium_offset);
    
    int inventory_list_offset = 0;
    
    

    if (hardcoded_class > 0x100000) {
        uint64_t inst = get_singleton_instance(hardcoded_class);
        
        if (inst > 0x100000) {
            manager_found = true;
            manager_inst = inst;
            inventory_list_offset = 0xF8; 
        }
    }

    if (!manager_found) {

         uint64_t start_offset = 0x814E000;
         uint64_t end_offset = 0x8160000;
         
         int max_items_found = -1;
         uint64_t best_offset = 0;

         for (uint64_t current_offset = start_offset; current_offset < end_offset; current_offset += 8) {
             uint64_t slot_addr = lib + current_offset;
             uint64_t cls = rpm<uint64_t>(slot_addr);
             
             if (!cls || cls < 0x100000) continue;
             
             uint64_t parent = rpm<uint64_t>(cls + 0x100);
             if (!parent || parent < 0x100000) continue;
             
             uint64_t statics = rpm<uint64_t>(parent + 0x130);
             if (!statics || statics < 0x100000) continue;
             
             uint64_t inst = rpm<uint64_t>(statics + 0x0);
             if (inst > 0x100000) {
                
                 for (uint64_t field = 0x20; field < 0x100; field += 8) {
                     uint64_t ptr = rpm<uint64_t>(inst + field);
                     if (!ptr || ptr < 0x100000) continue;

                     int count = check_collection(ptr, 0x30, true);
                     if (count == 0) count = check_collection(ptr, 0x10, false);

                     if (count > 0) {
                      
                         
                         if (count > max_items_found) {
                             max_items_found = count;
                             best_offset = current_offset;
                             manager_found = true;
                             manager_inst = inst;
                             inventory_list_offset = field;
                         }
                     }
                 }
             }
         }
         
         if (manager_found) {
            
         }
    }
    
    if (manager_found) {
        bool inv_found = false;
      
        uint64_t ptr_list = rpm<uint64_t>(manager_inst + (inventory_list_offset != 0 ? inventory_list_offset : 0xF8));
        
        if (check_collection(ptr_list, 0x30, true) > 0) {
                extract_skins(ptr_list, 0x30, true);
                inv_found = true;
             
        } 

        else if (check_collection(ptr_list, 0x10, false) > 0) {
                extract_skins(ptr_list, 0x10, false);
                inv_found = true;
            
        }
        else {
           
             ptr_list = rpm<uint64_t>(manager_inst + 0xF8);
             if (check_collection(ptr_list, 0x10, false) > 0) {
                  extract_skins(ptr_list, 0x10, false);
                  inv_found = true;
           
             }
             else if (check_collection(ptr_list, 0x30, true) > 0) {
                  extract_skins(ptr_list, 0x30, true);
                  inv_found = true;
    
             }
          
             else {
                 ptr_list = rpm<uint64_t>(manager_inst + 0xD8);
                 if (check_collection(ptr_list, 0x30, true) > 0) {
                     extract_skins(ptr_list, 0x30, true);
                     inv_found = true;
               
                 }
             }
        }
        
        if (inv_found) {
             snprintf(cfg::skins::refresh_label, sizeof(cfg::skins::refresh_label), "Done: %zu", cfg::skins::user_inventory.size());
        } else {
             snprintf(cfg::skins::refresh_label, sizeof(cfg::skins::refresh_label), "Found 0 Skins");
        }
    } else {
        snprintf(cfg::skins::refresh_label, sizeof(cfg::skins::refresh_label), "Err: No Manager");
    }
}

}
