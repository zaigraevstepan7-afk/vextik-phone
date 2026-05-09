#pragma once

#include <vector>
#include <string>

namespace ui {
    namespace config_sys {
        void init();
        void save(const std::string& name);
        void load(const std::string& name);
        void delete_config(const std::string& name);
        std::vector<std::string> get_configs();
        void apply_skins();
    }
}
