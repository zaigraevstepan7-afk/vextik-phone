#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include "../game/game.hpp"
#include "../game/player.hpp"
#include "../other/memory.hpp"
#include "../protect/oxorany.hpp"

#include "skins_db.hpp"

namespace skins {

using SkinDefinition = SkinInfo;



struct SkinChanger {
    int selected_skin_index;
    int active_skin_id;
    int StatTrack;

    SkinChanger();

    const SkinDefinition* current_skin() const;
    bool apply_to_local();
};

extern SkinChanger g_skinchanger;


void rebuild_inventory();

}
