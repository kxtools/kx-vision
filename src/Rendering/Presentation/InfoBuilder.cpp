#include "InfoBuilder.h"
#include "../../Game/GameEnums.h"
#include "../../Game/Generated/StatData.h"
#include "../../../libs/ImGui/imgui.h"

#include <algorithm>
#include <format>
#include <ranges>
#include <string_view>

#include "Styling.h"
#include "Shared/ColorConstants.h"
#include "Shared/Formatting.h"

namespace kx {

namespace {
    template<typename... Args>
    ColoredDetail MakeDetail(ImU32 color, std::format_string<Args...> fmt, Args&&... args) {
        char buffer[kMaxTextBufferSize];
        try {
            // Format to buffer size - 1 to leave room for null terminator
            auto result = std::format_to_n(buffer, kMaxTextBufferSize - 1, fmt, std::forward<Args>(args)...);
            *result.out = '\0'; // Null terminate
        } catch (...) {
            buffer[0] = '\0';
        }
        return ColoredDetail(buffer, color);
    }
}

// ===== Player Methods =====

std::vector<ColoredDetail> InfoBuilder::BuildPlayerDetails(const RenderablePlayer* player, const PlayerEspSettings& settings, bool showDebugAddresses) {
    std::vector<ColoredDetail> details;
    
    if (!settings.renderDetails) {
        return details;
    }
    
    details.reserve(8);

    if (settings.showDetailLevel && player->level > 0) {
        if (player->scaledLevel != player->level && player->scaledLevel > 0) {
            details.emplace_back(MakeDetail(ESPColors::DEFAULT_TEXT, "Level: {} ({})", player->level, player->scaledLevel));
        } else {
            details.emplace_back(MakeDetail(ESPColors::DEFAULT_TEXT, "Level: {}", player->level));
        }
    }

    if (settings.showDetailProfession && player->profession != Game::Profession::None) {
        const char* profName = Formatting::GetProfessionName(player->profession);
        if (profName) {
            details.emplace_back(MakeDetail(ESPColors::DEFAULT_TEXT, "Prof: {}", profName));
        } else {
            details.emplace_back(MakeDetail(ESPColors::DEFAULT_TEXT, "Prof: ID: {}", static_cast<int>(player->profession)));
        }
    }

    if (settings.showDetailAttitude) {
        const char* attitudeName = Formatting::GetAttitudeName(player->attitude);
        details.emplace_back(MakeDetail(ESPColors::DEFAULT_TEXT, "Attitude: {}", attitudeName ? attitudeName : "Unknown"));
    }

    if (settings.showDetailRace && player->race != Game::Race::None) {
        const char* raceName = Formatting::GetRaceName(player->race);
        if (raceName) {
            details.emplace_back(MakeDetail(ESPColors::DEFAULT_TEXT, "Race: {}", raceName));
        } else {
            details.emplace_back(MakeDetail(ESPColors::DEFAULT_TEXT, "Race: ID: {}", static_cast<int>(player->race)));
        }
    }

    if (settings.showDetailHp && player->maxHealth > 0) {
        details.emplace_back(MakeDetail(ESPColors::DEFAULT_TEXT, "HP: {:.0f}/{:.0f}", player->currentHealth, player->maxHealth));
    }

    if (settings.showDetailEnergy && player->maxEndurance > 0) {
        const int energyPercent = static_cast<int>((player->currentEndurance / player->maxEndurance) * 100.0f);
        details.emplace_back(MakeDetail(ESPColors::DEFAULT_TEXT, "Energy: {:.0f}/{:.0f} ({}%)", player->currentEndurance, player->maxEndurance, energyPercent));
    }

    if (settings.showDetailPosition) {
        details.emplace_back(MakeDetail(ESPColors::DEFAULT_TEXT, "Pos: ({:.1f}, {:.1f}, {:.1f})", player->position.x, player->position.y, player->position.z));
    }

    if (showDebugAddresses) {
        details.emplace_back(MakeDetail(ESPColors::DEFAULT_TEXT, "Addr: {:#x}", reinterpret_cast<uintptr_t>(player->address)));
    }

    return details;
}

std::vector<ColoredDetail> InfoBuilder::BuildGearDetails(const RenderablePlayer* player) {
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
            const char* slotName = Formatting::EquipmentSlotToString(gearIt->first);
            const GearSlotInfo& info = gearIt->second;
            ImU32 rarityColor = Styling::GetRarityColor(info.rarity);

            if (info.statId > 0) {
                if (auto statIt = data::stat::DATA.find(info.statId); statIt != data::stat::DATA.end()) {
                    gearDetails.emplace_back(MakeDetail(rarityColor, "{}: {}", slotName, statIt->second.name));
                } else {
                    gearDetails.emplace_back(MakeDetail(rarityColor, "{}: stat({})", slotName, info.statId));
                }
            } else {
                gearDetails.emplace_back(MakeDetail(rarityColor, "{}: {}", slotName, "No Stats"));
            }
        }
    }
    return gearDetails;
}

std::vector<CompactStatInfo> InfoBuilder::BuildCompactGearSummary(const RenderablePlayer* player) {
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

std::vector<DominantStat> InfoBuilder::BuildDominantStats(const RenderablePlayer* player) {
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
        const char* name = Formatting::GetAttributeShortName(attr);
        
        allStats.push_back({ name, (count / totalAttributes) * 100.0f, Styling::GetTacticalColor(attr) });
    }

    std::ranges::sort(allStats, [](const DominantStat& a, const DominantStat& b) {
        return a.percentage > b.percentage;
    });

    for (const auto& stat : allStats | std::views::take(3)) {
        result.push_back(stat);
    }

    return result;
}

Game::ItemRarity InfoBuilder::GetHighestRarity(const RenderablePlayer* player) {
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

std::vector<ColoredDetail> InfoBuilder::BuildNpcDetails(const RenderableNpc* npc, const NpcEspSettings& settings, bool showDebugAddresses) {
    std::vector<ColoredDetail> details;
    
    if (!settings.renderDetails) {
        return details;
    }

    details.reserve(6);

    if (!npc->name.empty()) {
        details.emplace_back(MakeDetail(ESPColors::DEFAULT_TEXT, "NPC: {}", npc->name.c_str()));
    }

    if (settings.showDetailLevel && npc->level > 0) {
        details.emplace_back(MakeDetail(ESPColors::DEFAULT_TEXT, "Level: {}", npc->level));
    }

    if (settings.showDetailHp && npc->maxHealth > 0) {
        details.emplace_back(MakeDetail(ESPColors::DEFAULT_TEXT, "HP: {:.0f}/{:.0f}", npc->currentHealth, npc->maxHealth));
    }

    if (settings.showDetailAttitude) {
        const char* attitudeName = Formatting::GetAttitudeName(npc->attitude);
        if (attitudeName) {
            details.emplace_back(MakeDetail(ESPColors::DEFAULT_TEXT, "Attitude: {}", attitudeName));
        } else {
            details.emplace_back(MakeDetail(ESPColors::DEFAULT_TEXT, "Attitude: ID: {}", static_cast<int>(npc->attitude)));
        }
    }

    if (settings.showDetailRank) {
        const char* rankName = Formatting::GetRankName(npc->rank);
        if (rankName && rankName[0] != '\0') {
            details.emplace_back(MakeDetail(ESPColors::DEFAULT_TEXT, "Rank: {}", rankName));
        }
    }

    if (settings.showDetailPosition) {
        details.emplace_back(MakeDetail(ESPColors::DEFAULT_TEXT, "Pos: ({:.1f}, {:.1f}, {:.1f})", npc->position.x, npc->position.y, npc->position.z));
    }

    if (showDebugAddresses) {
        details.emplace_back(MakeDetail(ESPColors::DEFAULT_TEXT, "Addr: {:#x}", reinterpret_cast<uintptr_t>(npc->address)));
    }

    return details;
}

// ===== Gadget Methods =====

std::vector<ColoredDetail> InfoBuilder::BuildGadgetDetails(const RenderableGadget* gadget, const ObjectEspSettings& settings, bool showDebugAddresses) {
    std::vector<ColoredDetail> details;
    
    if (!settings.renderDetails) {
        return details;
    }

    details.reserve(5);

    if (settings.showDetailGadgetType) {
        const char* gadgetName = Formatting::GetGadgetTypeName(gadget->type);
        if (gadgetName) {
            details.emplace_back(MakeDetail(ESPColors::DEFAULT_TEXT, "Type: {}", gadgetName));
        } else {
            details.emplace_back(MakeDetail(ESPColors::DEFAULT_TEXT, "Type: ID: {}", static_cast<int>(gadget->type)));
        }
    }

    if (settings.showDetailHealth && gadget->maxHealth > 0) {
        details.emplace_back(MakeDetail(ESPColors::DEFAULT_TEXT, "HP: {:.0f}/{:.0f}", gadget->currentHealth, gadget->maxHealth));
    }

    if (settings.showDetailResourceInfo && gadget->type == Game::GadgetType::ResourceNode) {
        details.emplace_back(MakeDetail(ESPColors::DEFAULT_TEXT, "Node: {}", Formatting::ResourceNodeTypeToString(gadget->resourceType)));
    }

    if (settings.showDetailGatherableStatus && gadget->isGatherable) {
        details.emplace_back("Status: Gatherable", ESPColors::DEFAULT_TEXT);
    }

    if (settings.showDetailPosition) {
        details.emplace_back(MakeDetail(ESPColors::DEFAULT_TEXT, "Pos: ({:.1f}, {:.1f}, {:.1f})", gadget->position.x, gadget->position.y, gadget->position.z));
    }

    if (showDebugAddresses) {
        details.emplace_back(MakeDetail(ESPColors::DEFAULT_TEXT, "Addr: {:#x}", reinterpret_cast<uintptr_t>(gadget->address)));
    }

    return details;
}

std::vector<ColoredDetail> InfoBuilder::BuildAttackTargetDetails(const RenderableAttackTarget* attackTarget, const ObjectEspSettings& settings, bool showDebugAddresses) {
    std::vector<ColoredDetail> details;
    
    if (!settings.renderDetails) {
        return details;
    }

    details.reserve(5);
    details.emplace_back("Type: Attack Target", ESPColors::DEFAULT_TEXT);

    if (settings.showDetailHealth && attackTarget->maxHealth > 0) {
        details.emplace_back(MakeDetail(ESPColors::DEFAULT_TEXT, "HP: {:.0f}/{:.0f}", attackTarget->currentHealth, attackTarget->maxHealth));
    }

    if (settings.showDetailPosition) {
        details.emplace_back(MakeDetail(ESPColors::DEFAULT_TEXT, "Pos: ({:.1f}, {:.1f}, {:.1f})", attackTarget->position.x, attackTarget->position.y, attackTarget->position.z));
    }

    details.emplace_back(MakeDetail(ESPColors::DEFAULT_TEXT, "AgentID: {}", attackTarget->agentId));

    if (showDebugAddresses) {
        details.emplace_back(MakeDetail(ESPColors::DEFAULT_TEXT, "Addr: {:#x}", reinterpret_cast<uintptr_t>(attackTarget->address)));
    }

    return details;
}

// ===== Private Helper Methods =====

std::map<data::ApiAttribute, int> InfoBuilder::BuildAttributeSummary(const RenderablePlayer* player) {
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

