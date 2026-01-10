#include "InfoBuilder.h"
#include "../../../Game/GameEnums.h"
#include "../../../Game/Generated/StatData.h"
#include "../../../../libs/ImGui/imgui.h"

#include <algorithm>
#include <array>
#include <string_view>
#include <climits>
#include <format>

#include "Styling.h"
#include "../../../Rendering/Shared/ColorConstants.h"
#include "Formatting.h"
#include "../../../Rendering/Renderers/TextRenderer.h"
#include "../Renderers/LayoutCursor.h"
#include "../../../Game/Data/FrameData.h"
#include "../../../Rendering/Shared/LayoutConstants.h"

namespace kx {

// ===== Private Helper Functions =====

static void DrawLine(ImDrawList* dl, LayoutCursor& cursor, const FastTextStyle& style, std::string_view text) {
    if (!text.empty()) {
        float h = TextRenderer::DrawCentered(dl, cursor.GetPosition(), text, style);
        cursor.Advance(h + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
    }
}

// ===== Player Methods =====

void InfoBuilder::RenderPlayerDetails(
    ImDrawList* drawList,
    LayoutCursor& cursor,
    const VisualProperties& props,
    const PlayerEntity* player,
    const PlayerEspSettings& settings,
    const AppearanceSettings& appearance,
    bool showDebugAddresses) {
    if (!settings.renderDetails) {
        return;
    }

    FastTextStyle style;
    style.fontSize = props.style.finalFontSize;
    style.shadow = appearance.enableTextShadows;
    style.background = appearance.enableTextBackgrounds;
    style.fadeAlpha = props.style.finalAlpha;
    style.color = ESPColors::DEFAULT_TEXT;

    char buf[128];

    if (settings.showDetailLevel && player->level > 0) {
        std::format_to_n_result<char*> res;
        if (player->scaledLevel != player->level && player->scaledLevel > 0) {
            res = std::format_to_n(buf, std::size(buf), "Level: {} ({})", player->level, player->scaledLevel);
        } else {
            res = std::format_to_n(buf, std::size(buf), "Level: {}", player->level);
        }
        DrawLine(drawList, cursor, style, std::string_view(buf, res.size));
    }

    if (settings.showDetailProfession && player->profession != Game::Profession::None) {
        const char* profName = Formatting::GetProfessionName(player->profession);
        std::format_to_n_result<char*> res;
        if (profName) {
            res = std::format_to_n(buf, std::size(buf), "Prof: {}", profName);
        } else {
            res = std::format_to_n(buf, std::size(buf), "Prof: ID: {}", static_cast<int>(player->profession));
        }
        DrawLine(drawList, cursor, style, std::string_view(buf, res.size));
    }

    if (settings.showDetailAttitude) {
        const char* attitudeName = Formatting::GetAttitudeName(player->attitude);
        auto res = std::format_to_n(buf, std::size(buf), "Attitude: {}", attitudeName ? attitudeName : "Unknown");
        DrawLine(drawList, cursor, style, std::string_view(buf, res.size));
    }

    if (settings.showDetailRace && player->race != Game::Race::None) {
        const char* raceName = Formatting::GetRaceName(player->race);
        std::format_to_n_result<char*> res;
        if (raceName) {
            res = std::format_to_n(buf, std::size(buf), "Race: {}", raceName);
        } else {
            res = std::format_to_n(buf, std::size(buf), "Race: ID: {}", static_cast<int>(player->race));
        }
        DrawLine(drawList, cursor, style, std::string_view(buf, res.size));
    }

    if (settings.showDetailHp && player->maxHealth > 0) {
        auto res = std::format_to_n(buf, std::size(buf), "HP: {:.0f}/{:.0f}", player->currentHealth, player->maxHealth);
        DrawLine(drawList, cursor, style, std::string_view(buf, res.size));
    }

    if (settings.showDetailEnergy && player->maxEndurance > 0) {
        const int energyPercent = static_cast<int>((player->currentEndurance / player->maxEndurance) * 100.0f);
        auto res = std::format_to_n(buf, std::size(buf), "Energy: {:.0f}/{:.0f} ({}%)", player->currentEndurance, player->maxEndurance, energyPercent);
        DrawLine(drawList, cursor, style, std::string_view(buf, res.size));
    }

    if (settings.showDetailPosition) {
        auto res = std::format_to_n(buf, std::size(buf), "Pos: ({:.1f}, {:.1f}, {:.1f})", player->position.x, player->position.y, player->position.z);
        DrawLine(drawList, cursor, style, std::string_view(buf, res.size));
    }

    if (showDebugAddresses) {
        auto res = std::format_to_n(buf, std::size(buf), "Addr: {:#x}", reinterpret_cast<unsigned long long>(player->address));
        DrawLine(drawList, cursor, style, std::string_view(buf, res.size));
    }
}

void InfoBuilder::RenderGearDetails(
    ImDrawList* drawList,
    LayoutCursor& cursor,
    const VisualProperties& props,
    const PlayerEntity* player,
    const AppearanceSettings& appearance) {
    const std::vector<Game::EquipmentSlot> displayOrder = {
        Game::EquipmentSlot::Helm, Game::EquipmentSlot::Shoulders, Game::EquipmentSlot::Chest,
        Game::EquipmentSlot::Gloves, Game::EquipmentSlot::Pants, Game::EquipmentSlot::Boots,
        Game::EquipmentSlot::Back, Game::EquipmentSlot::Amulet, Game::EquipmentSlot::Ring1,
        Game::EquipmentSlot::Ring2, Game::EquipmentSlot::Accessory1, Game::EquipmentSlot::Accessory2,
        Game::EquipmentSlot::MainhandWeapon1, Game::EquipmentSlot::OffhandWeapon1,
        Game::EquipmentSlot::MainhandWeapon2, Game::EquipmentSlot::OffhandWeapon2,
    };

    FastTextStyle style;
    style.fontSize = props.style.finalFontSize;
    style.shadow = appearance.enableTextShadows;
    style.background = appearance.enableTextBackgrounds;
    style.fadeAlpha = props.style.finalAlpha;

    char buf[128];
    for (const auto& slotEnum : displayOrder) {
        if (const GearSlotInfo* info = player->GetGearInfo(slotEnum)) {
            const char* slotName = Formatting::EquipmentSlotToString(slotEnum);
            ImU32 rarityColor = Styling::GetRarityColor(info->rarity);
            style.color = rarityColor;

            std::format_to_n_result<char*> res;
            if (info->statId > 0) {
                if (auto statIt = data::stat::DATA.find(info->statId); statIt != data::stat::DATA.end()) {
                    res = std::format_to_n(buf, std::size(buf), "{}: {}", slotName, statIt->second.name);
                } else {
                    res = std::format_to_n(buf, std::size(buf), "{}: stat({})", slotName, info->statId);
                }
            } else {
                res = std::format_to_n(buf, std::size(buf), "{}: No Stats", slotName);
            }
            DrawLine(drawList, cursor, style, std::string_view(buf, res.size));
        }
    }
}

size_t InfoBuilder::BuildCompactGearSummary(const PlayerEntity* player, CompactStatInfo* outBuffer, size_t bufferSize) {
    if (!player || player->gearCount == 0 || bufferSize == 0) {
        return 0;
    }

    std::array<CompactStatInfo, 32> workingBuffer;
    size_t uniqueStatsCount = 0;
    int totalItems = 0;

    for (size_t entryIndex = 0; entryIndex < player->gearCount; ++entryIndex) {
        const auto& info = player->gear[entryIndex].info;
        if (info.statId > 0) {
            totalItems++;
            if (auto statIt = data::stat::DATA.find(info.statId); statIt != data::stat::DATA.end()) {
                std::string_view statName = statIt->second.name;

                // Linear search for existing stat entry
                size_t foundIndex = SIZE_MAX;
                for (size_t i = 0; i < uniqueStatsCount; ++i) {
                    if (workingBuffer[i].statName == statName) {
                        foundIndex = i;
                        break;
                    }
                }

                if (foundIndex != SIZE_MAX) {
                    // Found existing entry, update it
                    workingBuffer[foundIndex].count++;
                    if (info.rarity > workingBuffer[foundIndex].highestRarity) {
                        workingBuffer[foundIndex].highestRarity = info.rarity;
                    }
                } else if (uniqueStatsCount < workingBuffer.size()) {
                    // New entry, add it
                    workingBuffer[uniqueStatsCount].statName = statName;
                    workingBuffer[uniqueStatsCount].count = 1;
                    workingBuffer[uniqueStatsCount].percentage = 0.0f;
                    workingBuffer[uniqueStatsCount].highestRarity = info.rarity;
                    uniqueStatsCount++;
                }
            }
        }
    }

    if (uniqueStatsCount == 0 || totalItems == 0) {
        return 0;
    }

    // Calculate percentages
    for (size_t i = 0; i < uniqueStatsCount; ++i) {
        workingBuffer[i].percentage = (static_cast<float>(workingBuffer[i].count) / totalItems) * 100.0f;
    }

    // Sort by percentage (descending)
    std::sort(workingBuffer.begin(), workingBuffer.begin() + uniqueStatsCount, 
              [](const CompactStatInfo& a, const CompactStatInfo& b) {
                  return a.percentage > b.percentage;
              });

    // Copy top results to output buffer
    size_t resultCount = std::min(uniqueStatsCount, bufferSize);
    for (size_t i = 0; i < resultCount; ++i) {
        outBuffer[i] = workingBuffer[i];
    }

    return resultCount;
}

size_t InfoBuilder::BuildDominantStats(const PlayerEntity* player, DominantStat* outBuffer, size_t bufferSize) {
    if (!player || bufferSize == 0) {
        return 0;
    }

    std::array<std::pair<kx::data::ApiAttribute, int>, 32> attributeBuffer;
    size_t attributeCount = BuildAttributeSummary(player, attributeBuffer.data(), attributeBuffer.size());
    
    if (attributeCount == 0) {
        return 0;
    }

    float totalAttributes = 0.0f;
    for (size_t i = 0; i < attributeCount; ++i) {
        totalAttributes += static_cast<float>(attributeBuffer[i].second);
    }

    if (totalAttributes == 0.0f) {
        return 0;
    }

    std::array<DominantStat, 16> workingBuffer;
    size_t statsCount = 0;

    for (size_t i = 0; i < attributeCount && statsCount < workingBuffer.size(); ++i) {
        const auto& [attr, count] = attributeBuffer[i];
        const char* name = Formatting::GetAttributeShortName(attr);
        
        workingBuffer[statsCount].name = name;
        workingBuffer[statsCount].percentage = (static_cast<float>(count) / totalAttributes) * 100.0f;
        workingBuffer[statsCount].color = Styling::GetTacticalColor(attr);
        statsCount++;
    }

    // Sort by percentage (descending)
    std::sort(workingBuffer.begin(), workingBuffer.begin() + statsCount, 
              [](const DominantStat& a, const DominantStat& b) {
                  return a.percentage > b.percentage;
              });

    // Copy top results to output buffer
    size_t resultCount = std::min(statsCount, bufferSize);
    for (size_t i = 0; i < resultCount; ++i) {
        outBuffer[i] = workingBuffer[i];
    }

    return resultCount;
}

Game::ItemRarity InfoBuilder::GetHighestRarity(const PlayerEntity* player) {
    if (!player || player->gearCount == 0) {
        return Game::ItemRarity::None;
    }

    Game::ItemRarity highestRarity = Game::ItemRarity::None;
    for (size_t i = 0; i < player->gearCount; ++i) {
        const auto& info = player->gear[i].info;
        if (info.rarity > highestRarity) {
            highestRarity = info.rarity;
        }
    }
    return highestRarity;
}

// ===== NPC Methods =====

void InfoBuilder::RenderNpcDetails(
    ImDrawList* drawList,
    LayoutCursor& cursor,
    const VisualProperties& props,
    const NpcEntity* npc,
    const NpcEspSettings& settings,
    const AppearanceSettings& appearance,
    bool showDebugAddresses) {
    if (!settings.renderDetails) {
        return;
    }

    FastTextStyle style;
    style.fontSize = props.style.finalFontSize;
    style.shadow = appearance.enableTextShadows;
    style.background = appearance.enableTextBackgrounds;
    style.fadeAlpha = props.style.finalAlpha;
    style.color = ESPColors::DEFAULT_TEXT;

    char buf[128];

    if (npc->name[0] != '\0') {
        auto res = std::format_to_n(buf, std::size(buf), "NPC: {}", npc->name);
        DrawLine(drawList, cursor, style, std::string_view(buf, res.size));
    }

    if (settings.showDetailLevel && npc->level > 0) {
        auto res = std::format_to_n(buf, std::size(buf), "Level: {}", npc->level);
        DrawLine(drawList, cursor, style, std::string_view(buf, res.size));
    }

    if (settings.showDetailHp && npc->maxHealth > 0) {
        auto res = std::format_to_n(buf, std::size(buf), "HP: {:.0f}/{:.0f}", npc->currentHealth, npc->maxHealth);
        DrawLine(drawList, cursor, style, std::string_view(buf, res.size));
    }

    if (settings.showDetailAttitude) {
        const char* attitudeName = Formatting::GetAttitudeName(npc->attitude);
        std::format_to_n_result<char*> res;
        if (attitudeName) {
            res = std::format_to_n(buf, std::size(buf), "Attitude: {}", attitudeName);
        } else {
            res = std::format_to_n(buf, std::size(buf), "Attitude: ID: {}", static_cast<int>(npc->attitude));
        }
        DrawLine(drawList, cursor, style, std::string_view(buf, res.size));
    }

    if (settings.showDetailRank) {
        const char* rankName = Formatting::GetRankName(npc->rank);
        if (rankName && rankName[0] != '\0') {
            auto res = std::format_to_n(buf, std::size(buf), "Rank: {}", rankName);
            DrawLine(drawList, cursor, style, std::string_view(buf, res.size));
        }
    }

    if (settings.showDetailPosition) {
        auto res = std::format_to_n(buf, std::size(buf), "Pos: ({:.1f}, {:.1f}, {:.1f})", npc->position.x, npc->position.y, npc->position.z);
        DrawLine(drawList, cursor, style, std::string_view(buf, res.size));
    }

    if (showDebugAddresses) {
        auto res = std::format_to_n(buf, std::size(buf), "Addr: {:#x}", reinterpret_cast<unsigned long long>(npc->address));
        DrawLine(drawList, cursor, style, std::string_view(buf, res.size));
    }
}

// ===== Gadget Methods =====

void InfoBuilder::RenderGadgetDetails(
    ImDrawList* drawList,
    LayoutCursor& cursor,
    const VisualProperties& props,
    const GadgetEntity* gadget,
    const ObjectEspSettings& settings,
    const AppearanceSettings& appearance,
    bool showDebugAddresses) {
    if (!settings.renderDetails) {
        return;
    }

    FastTextStyle style;
    style.fontSize = props.style.finalFontSize;
    style.shadow = appearance.enableTextShadows;
    style.background = appearance.enableTextBackgrounds;
    style.fadeAlpha = props.style.finalAlpha;
    style.color = ESPColors::DEFAULT_TEXT;

    char buf[128];

    if (settings.showDetailGadgetType) {
        const char* gadgetName = Formatting::GetGadgetTypeName(gadget->type);
        std::format_to_n_result<char*> res;
        if (gadgetName) {
            res = std::format_to_n(buf, std::size(buf), "Type: {}", gadgetName);
        } else {
            res = std::format_to_n(buf, std::size(buf), "Type: ID: {}", static_cast<int>(gadget->type));
        }
        DrawLine(drawList, cursor, style, std::string_view(buf, res.size));
    }

    if (settings.showDetailHealth && gadget->maxHealth > 0) {
        auto res = std::format_to_n(buf, std::size(buf), "HP: {:.0f}/{:.0f}", gadget->currentHealth, gadget->maxHealth);
        DrawLine(drawList, cursor, style, std::string_view(buf, res.size));
    }

    if (settings.showDetailResourceInfo && gadget->type == Game::GadgetType::ResourceNode) {
        const char* nodeName = Formatting::ResourceNodeTypeToString(gadget->resourceType);
        std::format_to_n_result<char*> res;
        if (nodeName) {
            res = std::format_to_n(buf, std::size(buf), "Node: {}", nodeName);
        } else {
            res = std::format_to_n(buf, std::size(buf), "Node: ID {}", static_cast<int>(gadget->resourceType));
        }
        DrawLine(drawList, cursor, style, std::string_view(buf, res.size));
    }

    if (settings.showDetailGatherableStatus && gadget->isGatherable) {
        std::string_view text("Status: Gatherable");
        float height = TextRenderer::DrawCentered(drawList, cursor.GetPosition(), text, style);
        cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
    }

    if (settings.showDetailPosition) {
        auto res = std::format_to_n(buf, std::size(buf), "Pos: ({:.1f}, {:.1f}, {:.1f})", gadget->position.x, gadget->position.y, gadget->position.z);
        DrawLine(drawList, cursor, style, std::string_view(buf, res.size));
    }

    if (showDebugAddresses) {
        auto res = std::format_to_n(buf, std::size(buf), "Addr: {:#x}", reinterpret_cast<unsigned long long>(gadget->address));
        DrawLine(drawList, cursor, style, std::string_view(buf, res.size));
    }
}

void InfoBuilder::RenderAttackTargetDetails(
    ImDrawList* drawList,
    LayoutCursor& cursor,
    const VisualProperties& props,
    const AttackTargetEntity* attackTarget,
    const ObjectEspSettings& settings,
    const AppearanceSettings& appearance,
    bool showDebugAddresses) {
    if (!settings.renderDetails) {
        return;
    }

    FastTextStyle style;
    style.fontSize = props.style.finalFontSize;
    style.shadow = appearance.enableTextShadows;
    style.background = appearance.enableTextBackgrounds;
    style.fadeAlpha = props.style.finalAlpha;
    style.color = ESPColors::DEFAULT_TEXT;

    std::string_view typeText("Type: Attack Target");
    float height = TextRenderer::DrawCentered(drawList, cursor.GetPosition(), typeText, style);
    cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);

    char buf[128];

    if (settings.showDetailHealth && attackTarget->maxHealth > 0) {
        auto res = std::format_to_n(buf, std::size(buf), "HP: {:.0f}/{:.0f}", attackTarget->currentHealth, attackTarget->maxHealth);
        DrawLine(drawList, cursor, style, std::string_view(buf, res.size));
    }

    if (settings.showDetailPosition) {
        auto res = std::format_to_n(buf, std::size(buf), "Pos: ({:.1f}, {:.1f}, {:.1f})", attackTarget->position.x, attackTarget->position.y, attackTarget->position.z);
        DrawLine(drawList, cursor, style, std::string_view(buf, res.size));
    }

    auto res = std::format_to_n(buf, std::size(buf), "AgentID: {}", attackTarget->agentId);
    DrawLine(drawList, cursor, style, std::string_view(buf, res.size));

    if (showDebugAddresses) {
        auto res2 = std::format_to_n(buf, std::size(buf), "Addr: {:#x}", reinterpret_cast<unsigned long long>(attackTarget->address));
        DrawLine(drawList, cursor, style, std::string_view(buf, res2.size));
    }
}

void InfoBuilder::RenderItemDetails(
    ImDrawList* drawList,
    LayoutCursor& cursor,
    const VisualProperties& props,
    const ItemEntity* item,
    const ObjectEspSettings& settings,
    const AppearanceSettings& appearance,
    bool showDebugAddresses) {
    if (!settings.renderDetails) {
        return;
    }

    FastTextStyle style;
    style.fontSize = props.style.finalFontSize;
    style.shadow = appearance.enableTextShadows;
    style.background = appearance.enableTextBackgrounds;
    style.fadeAlpha = props.style.finalAlpha;
    style.color = ESPColors::DEFAULT_TEXT;

    std::string_view typeText("Type: Item");
    float height = TextRenderer::DrawCentered(drawList, cursor.GetPosition(), typeText, style);
    cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);

    char buf[128];

    if (item->itemId > 0) {
        auto res = std::format_to_n(buf, std::size(buf), "ItemID: {}", item->itemId);
        DrawLine(drawList, cursor, style, std::string_view(buf, res.size));
    }

    if (item->rarity != Game::ItemRarity::None) {
        const char* rarityName = "Unknown";
        switch (item->rarity) {
            case Game::ItemRarity::Junk: rarityName = "Junk"; break;
            case Game::ItemRarity::Common: rarityName = "Common"; break;
            case Game::ItemRarity::Fine: rarityName = "Fine"; break;
            case Game::ItemRarity::Masterwork: rarityName = "Masterwork"; break;
            case Game::ItemRarity::Rare: rarityName = "Rare"; break;
            case Game::ItemRarity::Exotic: rarityName = "Exotic"; break;
            case Game::ItemRarity::Ascended: rarityName = "Ascended"; break;
            case Game::ItemRarity::Legendary: rarityName = "Legendary"; break;
            default: break;
        }
        
        ImU32 rarityColor = Styling::GetRarityColor(item->rarity);
        style.color = rarityColor;
        auto res = std::format_to_n(buf, std::size(buf), "Rarity: {}", rarityName);
        DrawLine(drawList, cursor, style, std::string_view(buf, res.size));
        style.color = ESPColors::DEFAULT_TEXT; // Reset color
    }

    if (settings.showDetailPosition) {
        auto res = std::format_to_n(buf, std::size(buf), "Pos: ({:.1f}, {:.1f}, {:.1f})", item->position.x, item->position.y, item->position.z);
        DrawLine(drawList, cursor, style, std::string_view(buf, res.size));
    }

    if (showDebugAddresses) {
        auto res = std::format_to_n(buf, std::size(buf), "Addr: {:#x}", reinterpret_cast<unsigned long long>(item->address));
        DrawLine(drawList, cursor, style, std::string_view(buf, res.size));
    }
}

// ===== Private Helper Methods =====

size_t InfoBuilder::BuildAttributeSummary(const PlayerEntity* player, std::pair<kx::data::ApiAttribute, int>* outBuffer, size_t bufferSize) {
    if (!player || player->gearCount == 0 || bufferSize == 0) {
        return 0;
    }

    size_t uniqueAttributesCount = 0;

    for (size_t entryIndex = 0; entryIndex < player->gearCount; ++entryIndex) {
        const auto& info = player->gear[entryIndex].info;
        if (info.statId > 0) {
            if (auto statIt = data::stat::DATA.find(info.statId); statIt != data::stat::DATA.end()) {
                for (const auto& attr : statIt->second.attributes) {
                    kx::data::ApiAttribute attrType = attr.attribute;

                    // Linear search for existing attribute entry
                    size_t foundIndex = SIZE_MAX;
                    for (size_t i = 0; i < uniqueAttributesCount; ++i) {
                        if (outBuffer[i].first == attrType) {
                            foundIndex = i;
                            break;
                        }
                    }

                    if (foundIndex != SIZE_MAX) {
                        // Found existing entry, increment count
                        outBuffer[foundIndex].second++;
                    } else if (uniqueAttributesCount < bufferSize) {
                        // New entry, add it
                        outBuffer[uniqueAttributesCount].first = attrType;
                        outBuffer[uniqueAttributesCount].second = 1;
                        uniqueAttributesCount++;
                    }
                }
            }
        }
    }

    return uniqueAttributesCount;
}

} // namespace kx

