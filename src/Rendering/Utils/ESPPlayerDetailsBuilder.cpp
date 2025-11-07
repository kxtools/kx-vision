#include "ESPPlayerDetailsBuilder.h"
#include "ESPFormatting.h"
#include "ESPConstants.h"
#include "../../Game/GameEnums.h"
#include "../../Game/Generated/StatData.h"
#include "../../../libs/ImGui/imgui.h"
#include <algorithm>
#include <format>
#include <sstream>
#include <iomanip>

#include "ESPStyling.h"

namespace kx {

std::vector<ColoredDetail> ESPPlayerDetailsBuilder::BuildPlayerDetails(const RenderablePlayer* player, const PlayerEspSettings& settings, bool showDebugAddresses) {
    std::vector<ColoredDetail> details;
    
    // Check if details rendering is enabled
    if (!settings.renderDetails) {
        return details;
    }
    
    details.reserve(16); // Future-proof: generous reserve for adding new fields

    if (settings.showDetailLevel && player->level > 0) {
        std::string levelText = "Level: " + std::to_string(player->level);
        if (player->scaledLevel != player->level && player->scaledLevel > 0) {
            levelText += " (" + std::to_string(player->scaledLevel) + ")";
        }
        details.push_back({ levelText, ESPColors::DEFAULT_TEXT });
    }

    if (settings.showDetailProfession && player->profession != Game::Profession::None) {
        const char* profName = ESPFormatting::GetProfessionName(player->profession);
        details.push_back({ "Prof: " + (profName ? std::string(profName) : "ID: " + std::to_string(static_cast<uint32_t>(player->profession))), ESPColors::DEFAULT_TEXT });
    }

    // Display player attitude
    if (settings.showDetailAttitude) {
        const char* attitudeName = ESPFormatting::GetAttitudeName(player->attitude);
        details.push_back({ "Attitude: " + (attitudeName ? std::string(attitudeName) : "Unknown"), ESPColors::DEFAULT_TEXT });
	}

    if (settings.showDetailRace && player->race != Game::Race::None) {
        const char* raceName = ESPFormatting::GetRaceName(player->race);
        details.push_back({ "Race: " + (raceName ? std::string(raceName) : "ID: " + std::to_string(static_cast<uint8_t>(player->race))), ESPColors::DEFAULT_TEXT });
    }

    if (settings.showDetailHp && player->maxHealth > 0) {
        details.push_back({ "HP: " + std::to_string(static_cast<int>(player->currentHealth)) + "/" + std::to_string(static_cast<int>(player->maxHealth)), ESPColors::DEFAULT_TEXT });
    }

    if (settings.showDetailEnergy && player->maxEnergy > 0) {
        const int energyPercent = static_cast<int>((player->currentEnergy / player->maxEnergy) * 100.0f);
        details.push_back({ "Energy: " + std::to_string(static_cast<int>(player->currentEnergy)) + "/" + std::to_string(static_cast<int>(player->maxEnergy)) + " (" + std::to_string(energyPercent) + "%)", ESPColors::DEFAULT_TEXT });
    }

    if (settings.showDetailPosition) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1)
            << "Pos: (" << player->position.x
            << ", " << player->position.y
            << ", " << player->position.z << ")";
        details.push_back({ oss.str(), ESPColors::DEFAULT_TEXT });
    }

    if (showDebugAddresses) {
        std::string addrStr = std::format("Addr: 0x{:X}", reinterpret_cast<uintptr_t>(player->address));
        details.push_back({ addrStr, ESPColors::DEFAULT_TEXT });

        /*details.push_back({ "AgentType: " + ESPFormatting::GetAgentTypeName(player->agentType), ESPColors::DEFAULT_TEXT });
        details.push_back({ "AgentID: " + std::to_string(player->agentId), ESPColors::DEFAULT_TEXT });*/
    }

    return details;
}

std::vector<CompactStatInfo> ESPPlayerDetailsBuilder::BuildCompactGearSummary(const RenderablePlayer* player) {
    if (!player || player->gear.empty()) {
        return {};
    }

    // Use a map to group stats and find the highest rarity for each
    std::map<std::string, CompactStatInfo> statSummary;
    int totalItems = 0;
    for (const auto& pair : player->gear) {
        const GearSlotInfo& info = pair.second;
        if (info.statId > 0) {
            totalItems++;
            auto statIt = data::stat::DATA.find(info.statId);
            if (statIt != data::stat::DATA.end()) {
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

    // Convert the map to a vector and calculate percentages
    std::vector<CompactStatInfo> result;
    result.reserve(statSummary.size());
    if (totalItems > 0) {
        for (auto& pair : statSummary) {
            pair.second.percentage = (static_cast<float>(pair.second.count) / totalItems) * 100.0f;
            result.push_back(pair.second);
        }
    }

    // Sort by percentage descending
    std::sort(result.begin(), result.end(), [](const CompactStatInfo& a, const CompactStatInfo& b) {
        return a.percentage > b.percentage;
    });

    // Keep only top 3
    if (result.size() > 3) {
        result.resize(3);
    }
    
    return result;
}

std::map<data::ApiAttribute, int> ESPPlayerDetailsBuilder::BuildAttributeSummary(const RenderablePlayer* player) {
    std::map<data::ApiAttribute, int> attributeCounts;
    if (!player || player->gear.empty()) {
        return attributeCounts;
    }

    for (const auto& pair : player->gear) {
        const GearSlotInfo& info = pair.second;
        if (info.statId > 0) {
            auto statIt = data::stat::DATA.find(info.statId);
            if (statIt != data::stat::DATA.end()) {
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
    std::map<data::ApiAttribute, int> attributeCounts = BuildAttributeSummary(player);
    if (attributeCounts.empty()) {
        return result;
    }

    // 2. Calculate the total number of attribute instances
    float totalAttributes = 0.0f;
    for (const auto& pair : attributeCounts) {
        totalAttributes += pair.second;
    }
    if (totalAttributes == 0) return result;

    // 3. Convert to a vector of DominantStat with percentages and tactical colors
    std::vector<DominantStat> allStats;
    allStats.reserve(attributeCounts.size());
    for (const auto& pair : attributeCounts) {
        const char* name = "??";
        switch (pair.first) {
        case data::ApiAttribute::Power:           name = "Power"; break;
        case data::ApiAttribute::Precision:       name = "Precision"; break;
        case data::ApiAttribute::Toughness:       name = "Toughness"; break;
        case data::ApiAttribute::Vitality:        name = "Vitality"; break;
        case data::ApiAttribute::CritDamage:      name = "Ferocity"; break;
        case data::ApiAttribute::Healing:         name = "Healing"; break;
        case data::ApiAttribute::ConditionDamage: name = "Condi Dmg"; break;
        case data::ApiAttribute::BoonDuration:    name = "Boon Dura"; break;
        case data::ApiAttribute::ConditionDuration: name = "Condi Dura"; break;
        }
        
        // Assign the name, percentage, AND the new tactical color
        allStats.push_back({ name, (pair.second / totalAttributes) * 100.0f, ESPStyling::GetTacticalColor(pair.first) });
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
            ImU32 rarityColor = ESPStyling::GetRarityColor(info.rarity);

            std::string statName = "No Stats";
            if (info.statId > 0) {
                auto statIt = data::stat::DATA.find(info.statId);
                if (statIt != data::stat::DATA.end()) {
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

Game::ItemRarity ESPPlayerDetailsBuilder::GetHighestRarity(const RenderablePlayer* player) {
    if (!player || player->gear.empty()) {
        return Game::ItemRarity::None;
    }

    Game::ItemRarity highestRarity = Game::ItemRarity::None;
    for (const auto& pair : player->gear) {
        if (pair.second.rarity > highestRarity) {
            highestRarity = pair.second.rarity;
        }
    }
    return highestRarity;
}

} // namespace kx