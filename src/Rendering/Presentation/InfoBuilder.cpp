#include "InfoBuilder.h"
#include "../../Game/GameEnums.h"
#include "../../Game/Generated/StatData.h"
#include "../../../libs/ImGui/imgui.h"

#include <algorithm>
#include <string_view>
#include <cstdio>

#include "Styling.h"
#include "Shared/ColorConstants.h"
#include "Formatting.h"
#include "../Renderers/TextRenderer.h"
#include "../Renderers/LayoutCursor.h"
#include "../Data/FrameData.h"
#include "../Shared/LayoutConstants.h"

namespace kx {

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

    char buffer[128];
    glm::vec2 pos = cursor.GetPosition();

    if (settings.showDetailLevel && player->level > 0) {
        int len;
        if (player->scaledLevel != player->level && player->scaledLevel > 0) {
            len = snprintf(buffer, sizeof(buffer), "Level: %u (%u)", player->level, player->scaledLevel);
        } else {
            len = snprintf(buffer, sizeof(buffer), "Level: %u", player->level);
        }
        if (len > 0 && static_cast<size_t>(len) < sizeof(buffer)) {
            std::string_view text(buffer, static_cast<size_t>(len));
            float height = TextRenderer::DrawCentered(drawList, pos, text, style);
            cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
            pos = cursor.GetPosition();
        }
    }

    if (settings.showDetailProfession && player->profession != Game::Profession::None) {
        const char* profName = Formatting::GetProfessionName(player->profession);
        int len;
        if (profName) {
            len = snprintf(buffer, sizeof(buffer), "Prof: %s", profName);
        } else {
            len = snprintf(buffer, sizeof(buffer), "Prof: ID: %d", static_cast<int>(player->profession));
        }
        if (len > 0 && static_cast<size_t>(len) < sizeof(buffer)) {
            std::string_view text(buffer, static_cast<size_t>(len));
            float height = TextRenderer::DrawCentered(drawList, pos, text, style);
            cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
            pos = cursor.GetPosition();
        }
    }

    if (settings.showDetailAttitude) {
        const char* attitudeName = Formatting::GetAttitudeName(player->attitude);
        int len = snprintf(buffer, sizeof(buffer), "Attitude: %s", attitudeName ? attitudeName : "Unknown");
        if (len > 0 && static_cast<size_t>(len) < sizeof(buffer)) {
            std::string_view text(buffer, static_cast<size_t>(len));
            float height = TextRenderer::DrawCentered(drawList, pos, text, style);
            cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
            pos = cursor.GetPosition();
        }
    }

    if (settings.showDetailRace && player->race != Game::Race::None) {
        const char* raceName = Formatting::GetRaceName(player->race);
        int len;
        if (raceName) {
            len = snprintf(buffer, sizeof(buffer), "Race: %s", raceName);
        } else {
            len = snprintf(buffer, sizeof(buffer), "Race: ID: %d", static_cast<int>(player->race));
        }
        if (len > 0 && static_cast<size_t>(len) < sizeof(buffer)) {
            std::string_view text(buffer, static_cast<size_t>(len));
            float height = TextRenderer::DrawCentered(drawList, pos, text, style);
            cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
            pos = cursor.GetPosition();
        }
    }

    if (settings.showDetailHp && player->maxHealth > 0) {
        int len = snprintf(buffer, sizeof(buffer), "HP: %.0f/%.0f", player->currentHealth, player->maxHealth);
        if (len > 0 && static_cast<size_t>(len) < sizeof(buffer)) {
            std::string_view text(buffer, static_cast<size_t>(len));
            float height = TextRenderer::DrawCentered(drawList, pos, text, style);
            cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
            pos = cursor.GetPosition();
        }
    }

    if (settings.showDetailEnergy && player->maxEndurance > 0) {
        const int energyPercent = static_cast<int>((player->currentEndurance / player->maxEndurance) * 100.0f);
        int len = snprintf(buffer, sizeof(buffer), "Energy: %.0f/%.0f (%d%%)", player->currentEndurance, player->maxEndurance, energyPercent);
        if (len > 0 && static_cast<size_t>(len) < sizeof(buffer)) {
            std::string_view text(buffer, static_cast<size_t>(len));
            float height = TextRenderer::DrawCentered(drawList, pos, text, style);
            cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
            pos = cursor.GetPosition();
        }
    }

    if (settings.showDetailPosition) {
        int len = snprintf(buffer, sizeof(buffer), "Pos: (%.1f, %.1f, %.1f)", player->position.x, player->position.y, player->position.z);
        if (len > 0 && static_cast<size_t>(len) < sizeof(buffer)) {
            std::string_view text(buffer, static_cast<size_t>(len));
            float height = TextRenderer::DrawCentered(drawList, pos, text, style);
            cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
            pos = cursor.GetPosition();
        }
    }

    if (showDebugAddresses) {
        int len = snprintf(buffer, sizeof(buffer), "Addr: %#llx", reinterpret_cast<unsigned long long>(player->address));
        if (len > 0 && static_cast<size_t>(len) < sizeof(buffer)) {
            std::string_view text(buffer, static_cast<size_t>(len));
            float height = TextRenderer::DrawCentered(drawList, pos, text, style);
            cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
        }
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

    char buffer[128];
    glm::vec2 pos = cursor.GetPosition();

    for (const auto& slotEnum : displayOrder) {
        if (auto gearIt = player->gear.find(slotEnum); gearIt != player->gear.end()) {
            const char* slotName = Formatting::EquipmentSlotToString(gearIt->first);
            const GearSlotInfo& info = gearIt->second;
            ImU32 rarityColor = Styling::GetRarityColor(info.rarity);
            style.color = rarityColor;

            int len;
            if (info.statId > 0) {
                if (auto statIt = data::stat::DATA.find(info.statId); statIt != data::stat::DATA.end()) {
                    len = snprintf(buffer, sizeof(buffer), "%s: %s", slotName, statIt->second.name);
                } else {
                    len = snprintf(buffer, sizeof(buffer), "%s: stat(%d)", slotName, info.statId);
                }
            } else {
                len = snprintf(buffer, sizeof(buffer), "%s: No Stats", slotName);
            }

            if (len > 0 && static_cast<size_t>(len) < sizeof(buffer)) {
                std::string_view text(buffer, static_cast<size_t>(len));
                float height = TextRenderer::DrawCentered(drawList, pos, text, style);
                cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
                pos = cursor.GetPosition();
            }
        }
    }
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

    std::sort(result.begin(), result.end(), [](const CompactStatInfo& a, const CompactStatInfo& b) {
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

    std::sort(allStats.begin(), allStats.end(), [](const DominantStat& a, const DominantStat& b) {
        return a.percentage > b.percentage;
    });

    size_t count = std::min(allStats.size(), size_t(3));
    for (size_t i = 0; i < count; ++i) {
        result.push_back(allStats[i]);
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

    char buffer[128];
    glm::vec2 pos = cursor.GetPosition();

    if (!npc->name.empty()) {
        int len = snprintf(buffer, sizeof(buffer), "NPC: %s", npc->name.c_str());
        if (len > 0 && static_cast<size_t>(len) < sizeof(buffer)) {
            std::string_view text(buffer, static_cast<size_t>(len));
            float height = TextRenderer::DrawCentered(drawList, pos, text, style);
            cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
            pos = cursor.GetPosition();
        }
    }

    if (settings.showDetailLevel && npc->level > 0) {
        int len = snprintf(buffer, sizeof(buffer), "Level: %u", npc->level);
        if (len > 0 && static_cast<size_t>(len) < sizeof(buffer)) {
            std::string_view text(buffer, static_cast<size_t>(len));
            float height = TextRenderer::DrawCentered(drawList, pos, text, style);
            cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
            pos = cursor.GetPosition();
        }
    }

    if (settings.showDetailHp && npc->maxHealth > 0) {
        int len = snprintf(buffer, sizeof(buffer), "HP: %.0f/%.0f", npc->currentHealth, npc->maxHealth);
        if (len > 0 && static_cast<size_t>(len) < sizeof(buffer)) {
            std::string_view text(buffer, static_cast<size_t>(len));
            float height = TextRenderer::DrawCentered(drawList, pos, text, style);
            cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
            pos = cursor.GetPosition();
        }
    }

    if (settings.showDetailAttitude) {
        const char* attitudeName = Formatting::GetAttitudeName(npc->attitude);
        int len;
        if (attitudeName) {
            len = snprintf(buffer, sizeof(buffer), "Attitude: %s", attitudeName);
        } else {
            len = snprintf(buffer, sizeof(buffer), "Attitude: ID: %d", static_cast<int>(npc->attitude));
        }
        if (len > 0 && static_cast<size_t>(len) < sizeof(buffer)) {
            std::string_view text(buffer, static_cast<size_t>(len));
            float height = TextRenderer::DrawCentered(drawList, pos, text, style);
            cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
            pos = cursor.GetPosition();
        }
    }

    if (settings.showDetailRank) {
        const char* rankName = Formatting::GetRankName(npc->rank);
        if (rankName && rankName[0] != '\0') {
            int len = snprintf(buffer, sizeof(buffer), "Rank: %s", rankName);
            if (len > 0 && static_cast<size_t>(len) < sizeof(buffer)) {
                std::string_view text(buffer, static_cast<size_t>(len));
                float height = TextRenderer::DrawCentered(drawList, pos, text, style);
                cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
                pos = cursor.GetPosition();
            }
        }
    }

    if (settings.showDetailPosition) {
        int len = snprintf(buffer, sizeof(buffer), "Pos: (%.1f, %.1f, %.1f)", npc->position.x, npc->position.y, npc->position.z);
        if (len > 0 && static_cast<size_t>(len) < sizeof(buffer)) {
            std::string_view text(buffer, static_cast<size_t>(len));
            float height = TextRenderer::DrawCentered(drawList, pos, text, style);
            cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
            pos = cursor.GetPosition();
        }
    }

    if (showDebugAddresses) {
        int len = snprintf(buffer, sizeof(buffer), "Addr: %#llx", reinterpret_cast<unsigned long long>(npc->address));
        if (len > 0 && static_cast<size_t>(len) < sizeof(buffer)) {
            std::string_view text(buffer, static_cast<size_t>(len));
            float height = TextRenderer::DrawCentered(drawList, pos, text, style);
            cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
        }
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

    char buffer[128];
    glm::vec2 pos = cursor.GetPosition();

    if (settings.showDetailGadgetType) {
        const char* gadgetName = Formatting::GetGadgetTypeName(gadget->type);
        int len;
        if (gadgetName) {
            len = snprintf(buffer, sizeof(buffer), "Type: %s", gadgetName);
        } else {
            len = snprintf(buffer, sizeof(buffer), "Type: ID: %d", static_cast<int>(gadget->type));
        }
        if (len > 0 && static_cast<size_t>(len) < sizeof(buffer)) {
            std::string_view text(buffer, static_cast<size_t>(len));
            float height = TextRenderer::DrawCentered(drawList, pos, text, style);
            cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
            pos = cursor.GetPosition();
        }
    }

    if (settings.showDetailHealth && gadget->maxHealth > 0) {
        int len = snprintf(buffer, sizeof(buffer), "HP: %.0f/%.0f", gadget->currentHealth, gadget->maxHealth);
        if (len > 0 && static_cast<size_t>(len) < sizeof(buffer)) {
            std::string_view text(buffer, static_cast<size_t>(len));
            float height = TextRenderer::DrawCentered(drawList, pos, text, style);
            cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
            pos = cursor.GetPosition();
        }
    }

    if (settings.showDetailResourceInfo && gadget->type == Game::GadgetType::ResourceNode) {
        std::string nodeTypeStr = Formatting::ResourceNodeTypeToString(gadget->resourceType);
        int len = snprintf(buffer, sizeof(buffer), "Node: %s", nodeTypeStr.c_str());
        if (len > 0 && static_cast<size_t>(len) < sizeof(buffer)) {
            std::string_view text(buffer, static_cast<size_t>(len));
            float height = TextRenderer::DrawCentered(drawList, pos, text, style);
            cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
            pos = cursor.GetPosition();
        }
    }

    if (settings.showDetailGatherableStatus && gadget->isGatherable) {
        std::string_view text("Status: Gatherable");
        float height = TextRenderer::DrawCentered(drawList, pos, text, style);
        cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
        pos = cursor.GetPosition();
    }

    if (settings.showDetailPosition) {
        int len = snprintf(buffer, sizeof(buffer), "Pos: (%.1f, %.1f, %.1f)", gadget->position.x, gadget->position.y, gadget->position.z);
        if (len > 0 && static_cast<size_t>(len) < sizeof(buffer)) {
            std::string_view text(buffer, static_cast<size_t>(len));
            float height = TextRenderer::DrawCentered(drawList, pos, text, style);
            cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
            pos = cursor.GetPosition();
        }
    }

    if (showDebugAddresses) {
        int len = snprintf(buffer, sizeof(buffer), "Addr: %#llx", reinterpret_cast<unsigned long long>(gadget->address));
        if (len > 0 && static_cast<size_t>(len) < sizeof(buffer)) {
            std::string_view text(buffer, static_cast<size_t>(len));
            float height = TextRenderer::DrawCentered(drawList, pos, text, style);
            cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
        }
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

    char buffer[128];
    glm::vec2 pos = cursor.GetPosition();

    std::string_view typeText("Type: Attack Target");
    float height = TextRenderer::DrawCentered(drawList, pos, typeText, style);
    cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
    pos = cursor.GetPosition();

    if (settings.showDetailHealth && attackTarget->maxHealth > 0) {
        int len = snprintf(buffer, sizeof(buffer), "HP: %.0f/%.0f", attackTarget->currentHealth, attackTarget->maxHealth);
        if (len > 0 && static_cast<size_t>(len) < sizeof(buffer)) {
            std::string_view text(buffer, static_cast<size_t>(len));
            height = TextRenderer::DrawCentered(drawList, pos, text, style);
            cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
            pos = cursor.GetPosition();
        }
    }

    if (settings.showDetailPosition) {
        int len = snprintf(buffer, sizeof(buffer), "Pos: (%.1f, %.1f, %.1f)", attackTarget->position.x, attackTarget->position.y, attackTarget->position.z);
        if (len > 0 && static_cast<size_t>(len) < sizeof(buffer)) {
            std::string_view text(buffer, static_cast<size_t>(len));
            height = TextRenderer::DrawCentered(drawList, pos, text, style);
            cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
            pos = cursor.GetPosition();
        }
    }

    int len = snprintf(buffer, sizeof(buffer), "AgentID: %d", attackTarget->agentId);
    if (len > 0 && static_cast<size_t>(len) < sizeof(buffer)) {
        std::string_view text(buffer, static_cast<size_t>(len));
        height = TextRenderer::DrawCentered(drawList, pos, text, style);
        cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
        pos = cursor.GetPosition();
    }

    if (showDebugAddresses) {
        len = snprintf(buffer, sizeof(buffer), "Addr: %#llx", reinterpret_cast<unsigned long long>(attackTarget->address));
        if (len > 0 && static_cast<size_t>(len) < sizeof(buffer)) {
            std::string_view text(buffer, static_cast<size_t>(len));
            TextRenderer::DrawCentered(drawList, pos, text, style);
            cursor.Advance(height + RenderingLayout::DETAILS_TEXT_LINE_SPACING);
        }
    }
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

