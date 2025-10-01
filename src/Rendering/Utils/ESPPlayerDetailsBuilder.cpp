#include "ESPPlayerDetailsBuilder.h"
#include "ESPFormatting.h"
#include "../../Game/GameEnums.h"
#include "../../Game/Generated/StatData.h"
#include "../../../libs/ImGui/imgui.h"
#include <algorithm>

#include "ESPStyling.h"

namespace kx {

const ImU32 DEFAULT_TEXT_COLOR = IM_COL32(255, 255, 255, 255); // White

std::vector<ColoredDetail> ESPPlayerDetailsBuilder::BuildPlayerDetails(const RenderablePlayer* player, const PlayerEspSettings& settings, bool showDebugAddresses) {
    std::vector<ColoredDetail> details;
    
    // Check if details rendering is enabled
    if (!settings.renderDetails) {
        return details;
    }
    
    details.reserve(16); // Future-proof: generous reserve for adding new fields

    if (!player->playerName.empty()) {
        details.push_back({ "Player: " + player->playerName, DEFAULT_TEXT_COLOR });
    }

    if (player->level > 0) {
        std::string levelText = "Level: " + std::to_string(player->level);
        if (player->scaledLevel != player->level && player->scaledLevel > 0) {
            levelText += " (" + std::to_string(player->scaledLevel) + ")";
        }
        details.push_back({ levelText, DEFAULT_TEXT_COLOR });
    }

    if (player->profession != Game::Profession::None) {
        const char* profName = Game::EnumHelpers::GetProfessionName(player->profession);
        details.push_back({ "Prof: " + (profName ? std::string(profName) : "ID: " + std::to_string(static_cast<uint32_t>(player->profession))), DEFAULT_TEXT_COLOR });
    }

    if (player->race != Game::Race::None) {
        const char* raceName = Game::EnumHelpers::GetRaceName(player->race);
        details.push_back({ "Race: " + (raceName ? std::string(raceName) : "ID: " + std::to_string(static_cast<uint8_t>(player->race))), DEFAULT_TEXT_COLOR });
    }

    if (player->maxHealth > 0) {
        details.push_back({ "HP: " + std::to_string(static_cast<int>(player->currentHealth)) + "/" + std::to_string(static_cast<int>(player->maxHealth)), DEFAULT_TEXT_COLOR });
    }

    if (player->maxEnergy > 0) {
        const int energyPercent = static_cast<int>((player->currentEnergy / player->maxEnergy) * 100.0f);
        details.push_back({ "Energy: " + std::to_string(static_cast<int>(player->currentEnergy)) + "/" + std::to_string(static_cast<int>(player->maxEnergy)) + " (" + std::to_string(energyPercent) + "%)", DEFAULT_TEXT_COLOR });
    }

    if (showDebugAddresses) {
        char addrStr[32];
        snprintf(addrStr, sizeof(addrStr), "Addr: 0x%p", player->address);
        details.push_back({ std::string(addrStr), DEFAULT_TEXT_COLOR });
    }

    return details;
}

std::vector<CompactStatInfo> ESPPlayerDetailsBuilder::BuildCompactGearSummary(const RenderablePlayer* player) {
    if (!player || player->gear.empty()) {
        return {};
    }

    // Use a map to group stats and find the highest rarity for each
    std::map<std::string, CompactStatInfo> statSummary;
    for (const auto& pair : player->gear) {
        const GearSlotInfo& info = pair.second;
        if (info.statId > 0) {
            auto statIt = kx::data::stat::DATA.find(info.statId);
            if (statIt != kx::data::stat::DATA.end()) {
                std::string statName = statIt->second.name;

                // Increment count
                statSummary[statName].count++;
                statSummary[statName].statName = statName;

                // Keep track of the highest rarity
                if (info.rarity > statSummary[statName].highestRarity) {
                    statSummary[statName].highestRarity = info.rarity;
                }
            }
        }
    }

    if (statSummary.empty()) {
        return {};
    }

    // Convert the map to a vector for ordered rendering
    std::vector<CompactStatInfo> result;
    result.reserve(statSummary.size());
    for (const auto& pair : statSummary) {
        result.push_back(pair.second);
    }
    return result;
}

std::map<kx::data::ApiAttribute, int> ESPPlayerDetailsBuilder::BuildAttributeSummary(const RenderablePlayer* player) {
    std::map<kx::data::ApiAttribute, int> attributeCounts;
    if (!player || player->gear.empty()) {
        return attributeCounts;
    }

    for (const auto& pair : player->gear) {
        const GearSlotInfo& info = pair.second;
        if (info.statId > 0) {
            auto statIt = kx::data::stat::DATA.find(info.statId);
            if (statIt != kx::data::stat::DATA.end()) {
                for (const auto& attr : statIt->second.attributes) {
                    attributeCounts[attr.attribute]++;
                }
            }
        }
    }
    return attributeCounts;
}

std::vector<DominantStat> ESPPlayerDetailsBuilder::BuildDominantStats(const RenderablePlayer* player) {
    std::vector<DominantStat> result;

    // 1. Get the raw attribute counts
    std::map<kx::data::ApiAttribute, int> attributeCounts = BuildAttributeSummary(player);
    if (attributeCounts.empty()) {
        return result;
    }

    // 2. Calculate the total number of attribute instances
    float totalAttributes = 0.0f;
    for (const auto& pair : attributeCounts) {
        totalAttributes += pair.second;
    }
    if (totalAttributes == 0) return result;

    // 3. Convert to a vector of DominantStat with percentages
    std::vector<DominantStat> allStats;
    allStats.reserve(attributeCounts.size());
    for (const auto& pair : attributeCounts) {
        const char* name = "??";
        switch (pair.first) {
        case kx::data::ApiAttribute::Power:           name = "Power"; break;
        case kx::data::ApiAttribute::Precision:       name = "Precision"; break;
        case kx::data::ApiAttribute::Toughness:       name = "Toughness"; break;
        case kx::data::ApiAttribute::Vitality:        name = "Vitality"; break;
        case kx::data::ApiAttribute::CritDamage:      name = "Ferocity"; break;
        case kx::data::ApiAttribute::Healing:         name = "Healing"; break;
        case kx::data::ApiAttribute::ConditionDamage: name = "Condi Dmg"; break;
        case kx::data::ApiAttribute::BoonDuration:    name = "Boon Dmg"; break;
        case kx::data::ApiAttribute::ConditionDuration: name = "Condi Dura"; break;
        }
        allStats.push_back({ name, (pair.second / totalAttributes) * 100.0f });
    }

    // 4. Sort the vector in descending order of percentage
    std::sort(allStats.begin(), allStats.end(), [](const DominantStat& a, const DominantStat& b) {
        return a.percentage > b.percentage;
        });

    // 5. Return the top 3
    result.reserve(3);
    for (size_t i = 0; i < allStats.size() && i < 3; ++i) {
        result.push_back(allStats[i]);
    }

    return result;
}

std::vector<ColoredDetail> ESPPlayerDetailsBuilder::BuildGearDetails(const RenderablePlayer* player) {
    std::vector<ColoredDetail> gearDetails;
    gearDetails.reserve(20); // Future-proof: generous reserve for all gear slots + extras

    const std::vector<Game::EquipmentSlot> displayOrder = {
        // Armor
        Game::EquipmentSlot::Helm,
        Game::EquipmentSlot::Shoulders,
        Game::EquipmentSlot::Chest,
        Game::EquipmentSlot::Gloves,
        Game::EquipmentSlot::Pants,
        Game::EquipmentSlot::Boots,
        // Trinkets
        Game::EquipmentSlot::Back,
        Game::EquipmentSlot::Amulet,
        Game::EquipmentSlot::Ring1,
        Game::EquipmentSlot::Ring2,
        Game::EquipmentSlot::Accessory1,
        Game::EquipmentSlot::Accessory2,
        // Weapons
        Game::EquipmentSlot::MainhandWeapon1,
        Game::EquipmentSlot::OffhandWeapon1,
        Game::EquipmentSlot::MainhandWeapon2,
        Game::EquipmentSlot::OffhandWeapon2,
    };

    for (const auto& slotEnum : displayOrder) {
        auto gearIt = player->gear.find(slotEnum);
        if (gearIt != player->gear.end()) {
            const char* slotName = ESPFormatting::EquipmentSlotToString(gearIt->first);
            const GearSlotInfo& info = gearIt->second;
            ImU32 rarityColor = ESPHelpers::GetRarityColor(info.rarity);

            std::string statName = "No Stats";
            if (info.statId > 0) {
                auto statIt = kx::data::stat::DATA.find(info.statId);
                if (statIt != kx::data::stat::DATA.end()) {
                    statName = statIt->second.name;
                }
                else {
                    statName = "stat(" + std::to_string(info.statId) + ")";
                }
            }

            gearDetails.push_back({ std::string(slotName) + ": " + statName, rarityColor });
        }
    }
    return gearDetails;
}

} // namespace kx
