#pragma once

#include <string>
#include "../Game/GameEnums.h"
#include "../../../libs/ImGui/imgui.h"

namespace kx {

// Player-specific rendering data structures
// Used for displaying gear, stats, and other player-specific information

struct GearSlotInfo {
    uint32_t itemId = 0;
    uint32_t statId = 0;
    Game::ItemRarity rarity = Game::ItemRarity::None;
    // We can add fields for upgrades, rarity, etc. here later.
};

struct CompactStatInfo {
    std::string statName;
    int count = 0;
    float percentage = 0.0f;
    Game::ItemRarity highestRarity = Game::ItemRarity::None;
};

struct DominantStat {
    std::string name;
    float percentage;
    ImU32 color;
};

} // namespace kx
