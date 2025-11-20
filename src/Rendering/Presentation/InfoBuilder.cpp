#include "InfoBuilder.h"
#include "../../Game/GameEnums.h"
#include "../../Game/Generated/StatData.h"
#include "../../../libs/ImGui/imgui.h"

#include <algorithm>
#include <array>
#include <string_view>
#include <cstdio>
#include <climits>
#include <cstdarg>

#include "Styling.h"
#include "Shared/ColorConstants.h"
#include "Formatting.h"
#include "../Renderers/TextRenderer.h"
#include "../Renderers/LayoutCursor.h"
#include "../Data/FrameData.h"
#include "../Shared/LayoutConstants.h"

namespace kx {

// ===== Private Helper Functions =====

static void DrawLine(ImDrawList* dl, LayoutCursor& cursor, const FastTextStyle& style, const char* fmt, ...) {
    char buffer[RenderingLayout::TEXT_BUFFER_SIZE];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (len > 0) {
        float h = TextRenderer::DrawCentered(dl, cursor.GetPosition(), {buffer, static_cast<size_t>(len)}, style);
        cursor.Advance(h + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
    }
}

// ===== Player Methods =====

void InfoBuilder::RenderPlayerDetails(ImDrawList* drawList, LayoutCursor& cursor, const VisualProperties& props, const RenderablePlayer* player, const PlayerEspSettings& settings, bool showDebugAddresses) {
    if (!settings.renderDetails) {
        return;
    }

    FastTextStyle style;
    style.fontSize = props.style.finalFontSize;
    style.shadow = true;
    style.background = true;
    style.fadeAlpha = props.style.finalAlpha;
    style.color = ESPColors::DEFAULT_TEXT;

    if (settings.showDetailLevel && player->level > 0) {
        if (player->scaledLevel != player->level && player->scaledLevel > 0) {
            DrawLine(drawList, cursor, style, "Level: %u (%u)", player->level, player->scaledLevel);
        } else {
            DrawLine(drawList, cursor, style, "Level: %u", player->level);
        }
    }

    if (settings.showDetailProfession && player->profession != Game::Profession::None) {
        const char* profName = Formatting::GetProfessionName(player->profession);
        if (profName) {
            DrawLine(drawList, cursor, style, "Prof: %s", profName);
        } else {
            DrawLine(drawList, cursor, style, "Prof: ID: %d", static_cast<int>(player->profession));
        }
    }

    if (settings.showDetailAttitude) {
        const char* attitudeName = Formatting::GetAttitudeName(player->attitude);
        DrawLine(drawList, cursor, style, "Attitude: %s", attitudeName ? attitudeName : "Unknown");
    }

    if (settings.showDetailRace && player->race != Game::Race::None) {
        const char* raceName = Formatting::GetRaceName(player->race);
        if (raceName) {
            DrawLine(drawList, cursor, style, "Race: %s", raceName);
        } else {
            DrawLine(drawList, cursor, style, "Race: ID: %d", static_cast<int>(player->race));
        }
    }

    if (settings.showDetailHp && player->maxHealth > 0) {
        DrawLine(drawList, cursor, style, "HP: %.0f/%.0f", player->currentHealth, player->maxHealth);
    }

    if (settings.showDetailEnergy && player->maxEndurance > 0) {
        const int energyPercent = static_cast<int>((player->currentEndurance / player->maxEndurance) * 100.0f);
        DrawLine(drawList, cursor, style, "Energy: %.0f/%.0f (%d%%)", player->currentEndurance, player->maxEndurance, energyPercent);
    }

    if (settings.showDetailPosition) {
        DrawLine(drawList, cursor, style, "Pos: (%.1f, %.1f, %.1f)", player->position.x, player->position.y, player->position.z);
    }

    if (showDebugAddresses) {
        DrawLine(drawList, cursor, style, "Addr: %#llx", reinterpret_cast<unsigned long long>(player->address));
    }
}

void InfoBuilder::RenderGearDetails(ImDrawList* drawList, LayoutCursor& cursor, const VisualProperties& props, const RenderablePlayer* player) {
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
    style.shadow = true;
    style.background = true;
    style.fadeAlpha = props.style.finalAlpha;

    for (const auto& slotEnum : displayOrder) {
        if (auto gearIt = player->gear.find(slotEnum); gearIt != player->gear.end()) {
            const char* slotName = Formatting::EquipmentSlotToString(gearIt->first);
            const GearSlotInfo& info = gearIt->second;
            ImU32 rarityColor = Styling::GetRarityColor(info.rarity);
            style.color = rarityColor;

            if (info.statId > 0) {
                if (auto statIt = data::stat::DATA.find(info.statId); statIt != data::stat::DATA.end()) {
                    DrawLine(drawList, cursor, style, "%s: %s", slotName, statIt->second.name);
                } else {
                    DrawLine(drawList, cursor, style, "%s: stat(%d)", slotName, info.statId);
                }
            } else {
                DrawLine(drawList, cursor, style, "%s: No Stats", slotName);
            }
        }
    }
}

size_t InfoBuilder::BuildCompactGearSummary(const RenderablePlayer* player, CompactStatInfo* outBuffer, size_t bufferSize) {
    if (!player || player->gear.empty() || bufferSize == 0) {
        return 0;
    }

    std::array<CompactStatInfo, 32> workingBuffer;
    size_t uniqueStatsCount = 0;
    int totalItems = 0;

    for (const auto& [slot, info] : player->gear) {
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

size_t InfoBuilder::BuildDominantStats(const RenderablePlayer* player, DominantStat* outBuffer, size_t bufferSize) {
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

void InfoBuilder::RenderNpcDetails(ImDrawList* drawList, LayoutCursor& cursor, const VisualProperties& props, const RenderableNpc* npc, const NpcEspSettings& settings, bool showDebugAddresses) {
    if (!settings.renderDetails) {
        return;
    }

    FastTextStyle style;
    style.fontSize = props.style.finalFontSize;
    style.shadow = true;
    style.background = true;
    style.fadeAlpha = props.style.finalAlpha;
    style.color = ESPColors::DEFAULT_TEXT;

    if (npc->name[0] != '\0') {
        DrawLine(drawList, cursor, style, "NPC: %s", npc->name);
    }

    if (settings.showDetailLevel && npc->level > 0) {
        DrawLine(drawList, cursor, style, "Level: %u", npc->level);
    }

    if (settings.showDetailHp && npc->maxHealth > 0) {
        DrawLine(drawList, cursor, style, "HP: %.0f/%.0f", npc->currentHealth, npc->maxHealth);
    }

    if (settings.showDetailAttitude) {
        const char* attitudeName = Formatting::GetAttitudeName(npc->attitude);
        if (attitudeName) {
            DrawLine(drawList, cursor, style, "Attitude: %s", attitudeName);
        } else {
            DrawLine(drawList, cursor, style, "Attitude: ID: %d", static_cast<int>(npc->attitude));
        }
    }

    if (settings.showDetailRank) {
        const char* rankName = Formatting::GetRankName(npc->rank);
        if (rankName && rankName[0] != '\0') {
            DrawLine(drawList, cursor, style, "Rank: %s", rankName);
        }
    }

    if (settings.showDetailPosition) {
        DrawLine(drawList, cursor, style, "Pos: (%.1f, %.1f, %.1f)", npc->position.x, npc->position.y, npc->position.z);
    }

    if (showDebugAddresses) {
        DrawLine(drawList, cursor, style, "Addr: %#llx", reinterpret_cast<unsigned long long>(npc->address));
    }
}

// ===== Gadget Methods =====

void InfoBuilder::RenderGadgetDetails(ImDrawList* drawList, LayoutCursor& cursor, const VisualProperties& props, const RenderableGadget* gadget, const ObjectEspSettings& settings, bool showDebugAddresses) {
    if (!settings.renderDetails) {
        return;
    }

    FastTextStyle style;
    style.fontSize = props.style.finalFontSize;
    style.shadow = true;
    style.background = true;
    style.fadeAlpha = props.style.finalAlpha;
    style.color = ESPColors::DEFAULT_TEXT;

    if (settings.showDetailGadgetType) {
        const char* gadgetName = Formatting::GetGadgetTypeName(gadget->type);
        if (gadgetName) {
            DrawLine(drawList, cursor, style, "Type: %s", gadgetName);
        } else {
            DrawLine(drawList, cursor, style, "Type: ID: %d", static_cast<int>(gadget->type));
        }
    }

    if (settings.showDetailHealth && gadget->maxHealth > 0) {
        DrawLine(drawList, cursor, style, "HP: %.0f/%.0f", gadget->currentHealth, gadget->maxHealth);
    }

    if (settings.showDetailResourceInfo && gadget->type == Game::GadgetType::ResourceNode) {
        const char* nodeName = Formatting::ResourceNodeTypeToString(gadget->resourceType);
        if (nodeName) {
            DrawLine(drawList, cursor, style, "Node: %s", nodeName);
        } else {
            DrawLine(drawList, cursor, style, "Node: ID %d", static_cast<int>(gadget->resourceType));
        }
    }

    if (settings.showDetailGatherableStatus && gadget->isGatherable) {
        std::string_view text("Status: Gatherable");
        float height = TextRenderer::DrawCentered(drawList, cursor.GetPosition(), text, style);
        cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
    }

    if (settings.showDetailPosition) {
        DrawLine(drawList, cursor, style, "Pos: (%.1f, %.1f, %.1f)", gadget->position.x, gadget->position.y, gadget->position.z);
    }

    if (showDebugAddresses) {
        DrawLine(drawList, cursor, style, "Addr: %#llx", reinterpret_cast<unsigned long long>(gadget->address));
    }
}

void InfoBuilder::RenderAttackTargetDetails(ImDrawList* drawList, LayoutCursor& cursor, const VisualProperties& props, const RenderableAttackTarget* attackTarget, const ObjectEspSettings& settings, bool showDebugAddresses) {
    if (!settings.renderDetails) {
        return;
    }

    FastTextStyle style;
    style.fontSize = props.style.finalFontSize;
    style.shadow = true;
    style.background = true;
    style.fadeAlpha = props.style.finalAlpha;
    style.color = ESPColors::DEFAULT_TEXT;

    std::string_view typeText("Type: Attack Target");
    float height = TextRenderer::DrawCentered(drawList, cursor.GetPosition(), typeText, style);
    cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);

    if (settings.showDetailHealth && attackTarget->maxHealth > 0) {
        DrawLine(drawList, cursor, style, "HP: %.0f/%.0f", attackTarget->currentHealth, attackTarget->maxHealth);
    }

    if (settings.showDetailPosition) {
        DrawLine(drawList, cursor, style, "Pos: (%.1f, %.1f, %.1f)", attackTarget->position.x, attackTarget->position.y, attackTarget->position.z);
    }

    DrawLine(drawList, cursor, style, "AgentID: %d", attackTarget->agentId);

    if (showDebugAddresses) {
        DrawLine(drawList, cursor, style, "Addr: %#llx", reinterpret_cast<unsigned long long>(attackTarget->address));
    }
}

// ===== Private Helper Methods =====

size_t InfoBuilder::BuildAttributeSummary(const RenderablePlayer* player, std::pair<kx::data::ApiAttribute, int>* outBuffer, size_t bufferSize) {
    if (!player || player->gear.empty() || bufferSize == 0) {
        return 0;
    }

    size_t uniqueAttributesCount = 0;

    for (const auto& [slot, info] : player->gear) {
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

