#include "ESPEntityDetailsBuilder.h"
#include "ESPFormatting.h"
#include "../../Game/GameEnums.h"
#include "../../../libs/ImGui/imgui.h"

namespace kx {

const ImU32 DEFAULT_TEXT_COLOR = IM_COL32(255, 255, 255, 255); // White

std::vector<ColoredDetail> ESPEntityDetailsBuilder::BuildNpcDetails(const RenderableNpc* npc, const NpcEspSettings& settings, bool showDebugAddresses) {
    std::vector<ColoredDetail> details;
    
    if (!settings.renderDetails) {
        return details;
    }

    details.reserve(12); // Future-proof: generous reserve for adding new fields

    if (!npc->name.empty()) {
        details.push_back({ "NPC: " + npc->name, DEFAULT_TEXT_COLOR });
    }

    if (npc->level > 0) {
        details.push_back({ "Level: " + std::to_string(npc->level), DEFAULT_TEXT_COLOR });
    }

    if (npc->maxHealth > 0) {
        details.push_back({ "HP: " + std::to_string(static_cast<int>(npc->currentHealth)) + "/" + std::to_string(static_cast<int>(npc->maxHealth)), DEFAULT_TEXT_COLOR });
    }

    const char* attitudeName = Game::EnumHelpers::GetAttitudeName(npc->attitude);
    details.push_back({ "Attitude: " + (attitudeName ? std::string(attitudeName) : "ID: " + std::to_string(static_cast<uint32_t>(npc->attitude))), DEFAULT_TEXT_COLOR });
    
    const char* rankName = Game::EnumHelpers::GetRankName(npc->rank);
    if (rankName && rankName[0] != '\0') {
        details.push_back({ "Rank: " + std::string(rankName), DEFAULT_TEXT_COLOR });
    }

    if (showDebugAddresses) {
        char addrStr[32];
        snprintf(addrStr, sizeof(addrStr), "Addr: 0x%p", npc->address);
        details.push_back({ std::string(addrStr), DEFAULT_TEXT_COLOR });
    }

    return details;
}

std::vector<ColoredDetail> ESPEntityDetailsBuilder::BuildGadgetDetails(const RenderableGadget* gadget, const ObjectEspSettings& settings, bool showDebugAddresses) {
    std::vector<ColoredDetail> details;
    
    if (!settings.renderDetails) {
        return details;
    }

    details.reserve(8); // Future-proof: generous reserve for adding new fields

    const char* gadgetName = Game::EnumHelpers::GetGadgetTypeName(gadget->type);
    details.push_back({ "Type: " + (gadgetName ? std::string(gadgetName) : "ID: " + std::to_string(static_cast<uint32_t>(gadget->type))), DEFAULT_TEXT_COLOR });

    if (gadget->type == Game::GadgetType::ResourceNode) {
        details.push_back({ "Node: " + ESPFormatting::ResourceNodeTypeToString(gadget->resourceType), DEFAULT_TEXT_COLOR });
    }

    if (gadget->isGatherable) {
        details.push_back({ "Status: Gatherable", DEFAULT_TEXT_COLOR });
    }

    if (showDebugAddresses) {
        char addrStr[32];
        snprintf(addrStr, sizeof(addrStr), "Addr: 0x%p", gadget->address);
        details.push_back({ std::string(addrStr), DEFAULT_TEXT_COLOR });
    }

    return details;
}

} // namespace kx
