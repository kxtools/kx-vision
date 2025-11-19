#include "ESPInfoBuilder.h"
#include "ESPFormatting.h"
#include "ESPConstants.h"
#include "../../Game/GameEnums.h"
#include "../../Game/Generated/StatData.h"
#include "../../Game/HavokEnums.h"
#include "../../../libs/ImGui/imgui.h"
#include "ESPStyling.h"

#include <algorithm>
#include <format>
#include <ranges>
#include <string_view>

namespace kx {

// ===== Player Methods =====

std::vector<ColoredDetail> ESPInfoBuilder::BuildPlayerDetails(const RenderablePlayer* player, const PlayerEspSettings& settings, bool showDebugAddresses) {
    std::vector<ColoredDetail> details;
    
    if (!settings.renderDetails) {
        return details;
    }
    
    details.reserve(8);

    if (settings.showDetailLevel && player->level > 0) {
        if (player->scaledLevel != player->level && player->scaledLevel > 0) {
            details.emplace_back(std::format("Level: {} ({})", player->level, player->scaledLevel), ESPColors::DEFAULT_TEXT);
        } else {
            details.emplace_back(std::format("Level: {}", player->level), ESPColors::DEFAULT_TEXT);
        }
    }

    if (settings.showDetailProfession && player->profession != Game::Profession::None) {
        const char* profName = ESPFormatting::GetProfessionName(player->profession);
        if (profName) {
            details.emplace_back(std::format("Prof: {}", profName), ESPColors::DEFAULT_TEXT);
        } else {
            details.emplace_back(std::format("Prof: ID: {}", std::to_underlying(player->profession)), ESPColors::DEFAULT_TEXT);
        }
    }

    if (settings.showDetailAttitude) {
        const char* attitudeName = ESPFormatting::GetAttitudeName(player->attitude);
        details.emplace_back(std::format("Attitude: {}", attitudeName ? attitudeName : "Unknown"), ESPColors::DEFAULT_TEXT);
    }

    if (settings.showDetailRace && player->race != Game::Race::None) {
        const char* raceName = ESPFormatting::GetRaceName(player->race);
        if (raceName) {
            details.emplace_back(std::format("Race: {}", raceName), ESPColors::DEFAULT_TEXT);
        } else {
            details.emplace_back(std::format("Race: ID: {}", std::to_underlying(player->race)), ESPColors::DEFAULT_TEXT);
        }
    }

    if (settings.showDetailHp && player->maxHealth > 0) {
        details.emplace_back(std::format("HP: {:.0f}/{:.0f}", player->currentHealth, player->maxHealth), ESPColors::DEFAULT_TEXT);
    }

    if (settings.showDetailEnergy && player->maxEndurance > 0) {
        const int energyPercent = static_cast<int>((player->currentEndurance / player->maxEndurance) * 100.0f);
        details.emplace_back(std::format("Energy: {:.0f}/{:.0f} ({}%)", player->currentEndurance, player->maxEndurance, energyPercent), ESPColors::DEFAULT_TEXT);
    }

    if (settings.showDetailPosition) {
        details.emplace_back(std::format("Pos: ({:.1f}, {:.1f}, {:.1f})", player->position.x, player->position.y, player->position.z), ESPColors::DEFAULT_TEXT);
    }

    if (showDebugAddresses) {
        details.emplace_back(std::format("Addr: {:#x}", reinterpret_cast<uintptr_t>(player->address)), ESPColors::DEFAULT_TEXT);
    }

    return details;
}

std::vector<ColoredDetail> ESPInfoBuilder::BuildGearDetails(const RenderablePlayer* player) {
    std::vector<ColoredDetail> gearDetails;
    gearDetails.reserve(16);

    const std::vector<Game::EquipmentSlot> displayOrder = {
        Game::EquipmentSlot::Helm, Game::EquipmentSlot::Shoulders, Game::EquipmentSlot::Chest,
        Game::EquipmentSlot::Gloves, Game::EquipmentSlot::Pants, Game::EquipmentSlot::Boots,
        Game::EquipmentSlot::Back, Game::EquipmentSlot::Amulet, Game::EquipmentSlot::Ring1,
        Game::EquipmentSlot::Ring2, Game::EquipmentSlot::Accessory1, Game::EquipmentSlot::Accessory2,
        Game::EquipmentSlot::MainhandWeapon1, Game::EquipmentSlot::OffhandWeapon1,
        Game::EquipmentSlot::MainhandWeapon2, Game::EquipmentSlot::OffhandWeapon2,
    };

    for (const auto& slotEnum : displayOrder) {
        if (auto gearIt = player->gear.find(slotEnum); gearIt != player->gear.end()) {
            const char* slotName = ESPFormatting::EquipmentSlotToString(gearIt->first);
            const GearSlotInfo& info = gearIt->second;
            ImU32 rarityColor = ESPStyling::GetRarityColor(info.rarity);

            std::string statName = "No Stats";
            if (info.statId > 0) {
                if (auto statIt = data::stat::DATA.find(info.statId); statIt != data::stat::DATA.end()) {
                    statName = statIt->second.name;
                } else {
                    statName = std::format("stat({})", info.statId);
                }
            }

            gearDetails.emplace_back(std::format("{}: {}", slotName, statName), rarityColor);
        }
    }
    return gearDetails;
}

std::vector<CompactStatInfo> ESPInfoBuilder::BuildCompactGearSummary(const RenderablePlayer* player) {
    if (!player || player->gear.empty()) {
        return {};
    }

    std::map<std::string_view, CompactStatInfo> statSummary;
    int totalItems = 0;

    for (const auto& [slot, info] : player->gear) {
        if (info.statId > 0) {
            totalItems++;
            if (auto statIt = data::stat::DATA.find(info.statId); statIt != data::stat::DATA.end()) {
                std::string_view statName = statIt->second.name;

                auto& entry = statSummary[statName];
                entry.statName = statName;
                entry.count++;

                if (info.rarity > entry.highestRarity) {
                    entry.highestRarity = info.rarity;
                }
            }
        }
    }

    if (statSummary.empty()) {
        return {};
    }

    std::vector<CompactStatInfo> result;
    result.reserve(statSummary.size());
    
    if (totalItems > 0) {
        for (const auto& [name, info] : statSummary) {
            auto& item = result.emplace_back(info);
            item.percentage = (static_cast<float>(info.count) / totalItems) * 100.0f;
        }
    }

    std::ranges::sort(result, [](const CompactStatInfo& a, const CompactStatInfo& b) {
        return a.percentage > b.percentage;
    });

    if (result.size() > 3) {
        result.resize(3);
    }
    
    return result;
}

std::vector<DominantStat> ESPInfoBuilder::BuildDominantStats(const RenderablePlayer* player) {
    std::vector<DominantStat> result;
    auto attributeCounts = BuildAttributeSummary(player);
    
    if (attributeCounts.empty()) {
        return result;
    }

    float totalAttributes = 0.0f;
    for (const auto& [attr, count] : attributeCounts) {
        totalAttributes += count;
    }

    if (totalAttributes == 0) return result;

    std::vector<DominantStat> allStats;
    allStats.reserve(attributeCounts.size());

    for (const auto& [attr, count] : attributeCounts) {
        const char* name = "??";
        switch (attr) {
            using enum data::ApiAttribute;
            case Power:             name = "Power"; break;
            case Precision:         name = "Precision"; break;
            case Toughness:         name = "Toughness"; break;
            case Vitality:          name = "Vitality"; break;
            case CritDamage:        name = "Ferocity"; break;
            case Healing:           name = "Healing"; break;
            case ConditionDamage:   name = "Condi Dmg"; break;
            case BoonDuration:      name = "Boon Dura"; break;
            case ConditionDuration: name = "Condi Dura"; break;
            default:                break;
        }
        
        allStats.push_back({ name, (count / totalAttributes) * 100.0f, ESPStyling::GetTacticalColor(attr) });
    }

    std::ranges::sort(allStats, [](const DominantStat& a, const DominantStat& b) {
        return a.percentage > b.percentage;
    });

    for (const auto& stat : allStats | std::views::take(3)) {
        result.push_back(stat);
    }

    return result;
}

Game::ItemRarity ESPInfoBuilder::GetHighestRarity(const RenderablePlayer* player) {
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

// ===== NPC Methods =====

std::vector<ColoredDetail> ESPInfoBuilder::BuildNpcDetails(const RenderableNpc* npc, const NpcEspSettings& settings, bool showDebugAddresses) {
    std::vector<ColoredDetail> details;
    
    if (!settings.renderDetails) {
        return details;
    }

    details.reserve(6);

    if (!npc->name.empty()) {
        details.emplace_back(std::format("NPC: {}", npc->name), ESPColors::DEFAULT_TEXT);
    }

    if (settings.showDetailLevel && npc->level > 0) {
        details.emplace_back(std::format("Level: {}", npc->level), ESPColors::DEFAULT_TEXT);
    }

    if (settings.showDetailHp && npc->maxHealth > 0) {
        details.emplace_back(std::format("HP: {:.0f}/{:.0f}", npc->currentHealth, npc->maxHealth), ESPColors::DEFAULT_TEXT);
    }

    if (settings.showDetailAttitude) {
        const char* attitudeName = ESPFormatting::GetAttitudeName(npc->attitude);
        if (attitudeName) {
            details.emplace_back(std::format("Attitude: {}", attitudeName), ESPColors::DEFAULT_TEXT);
        } else {
            details.emplace_back(std::format("Attitude: ID: {}", std::to_underlying(npc->attitude)), ESPColors::DEFAULT_TEXT);
        }
    }

    if (settings.showDetailRank) {
        const char* rankName = ESPFormatting::GetRankName(npc->rank);
        if (rankName && rankName[0] != '\0') {
            details.emplace_back(std::format("Rank: {}", rankName), ESPColors::DEFAULT_TEXT);
        }
    }

    if (settings.showDetailPosition) {
        details.emplace_back(std::format("Pos: ({:.1f}, {:.1f}, {:.1f})", npc->position.x, npc->position.y, npc->position.z), ESPColors::DEFAULT_TEXT);
    }

    if (showDebugAddresses) {
        details.emplace_back(std::format("Addr: {:#x}", reinterpret_cast<uintptr_t>(npc->address)), ESPColors::DEFAULT_TEXT);
    }

    return details;
}

// ===== Gadget Methods =====

std::vector<ColoredDetail> ESPInfoBuilder::BuildGadgetDetails(const RenderableGadget* gadget, const ObjectEspSettings& settings, bool showDebugAddresses) {
    std::vector<ColoredDetail> details;
    
    if (!settings.renderDetails) {
        return details;
    }

    details.reserve(5);

    if (settings.showDetailGadgetType) {
        const char* gadgetName = ESPFormatting::GetGadgetTypeName(gadget->type);
        if (gadgetName) {
            details.emplace_back(std::format("Type: {}", gadgetName), ESPColors::DEFAULT_TEXT);
        } else {
            details.emplace_back(std::format("Type: ID: {}", std::to_underlying(gadget->type)), ESPColors::DEFAULT_TEXT);
        }
    }

    if (settings.showDetailHealth && gadget->maxHealth > 0) {
        details.emplace_back(std::format("HP: {:.0f}/{:.0f}", gadget->currentHealth, gadget->maxHealth), ESPColors::DEFAULT_TEXT);
    }

    if (settings.showDetailResourceInfo && gadget->type == Game::GadgetType::ResourceNode) {
        details.emplace_back(std::format("Node: {}", ESPFormatting::ResourceNodeTypeToString(gadget->resourceType)), ESPColors::DEFAULT_TEXT);
    }

    if (settings.showDetailGatherableStatus && gadget->isGatherable) {
        details.emplace_back("Status: Gatherable", ESPColors::DEFAULT_TEXT);
    }

    if (settings.showDetailPosition) {
        details.emplace_back(std::format("Pos: ({:.1f}, {:.1f}, {:.1f})", gadget->position.x, gadget->position.y, gadget->position.z), ESPColors::DEFAULT_TEXT);
    }

    if (showDebugAddresses) {
        details.emplace_back(std::format("Addr: {:#x}", reinterpret_cast<uintptr_t>(gadget->address)), ESPColors::DEFAULT_TEXT);
    }

    return details;
}

std::vector<ColoredDetail> ESPInfoBuilder::BuildAttackTargetDetails(const RenderableAttackTarget* attackTarget, const ObjectEspSettings& settings, bool showDebugAddresses) {
    std::vector<ColoredDetail> details;
    
    if (!settings.renderDetails) {
        return details;
    }

    details.reserve(5);
    details.emplace_back("Type: Attack Target", ESPColors::DEFAULT_TEXT);

    if (settings.showDetailHealth && attackTarget->maxHealth > 0) {
        details.emplace_back(std::format("HP: {:.0f}/{:.0f}", attackTarget->currentHealth, attackTarget->maxHealth), ESPColors::DEFAULT_TEXT);
    }

    if (settings.showDetailPosition) {
        details.emplace_back(std::format("Pos: ({:.1f}, {:.1f}, {:.1f})", attackTarget->position.x, attackTarget->position.y, attackTarget->position.z), ESPColors::DEFAULT_TEXT);
    }

    details.emplace_back(std::format("AgentID: {}", attackTarget->agentId), ESPColors::DEFAULT_TEXT);

    if (showDebugAddresses) {
        details.emplace_back(std::format("Addr: {:#x}", reinterpret_cast<uintptr_t>(attackTarget->address)), ESPColors::DEFAULT_TEXT);
    }

    return details;
}

// ===== Private Helper Methods =====

std::map<data::ApiAttribute, int> ESPInfoBuilder::BuildAttributeSummary(const RenderablePlayer* player) {
    std::map<data::ApiAttribute, int> attributeCounts;
    if (!player || player->gear.empty()) {
        return attributeCounts;
    }

    for (const auto& [slot, info] : player->gear) {
        if (info.statId > 0) {
            if (auto statIt = data::stat::DATA.find(info.statId); statIt != data::stat::DATA.end()) {
                for (const auto& attr : statIt->second.attributes) {
                    attributeCounts[attr.attribute]++;
                }
            }
        }
    }
    return attributeCounts;
}

} // namespace kx

