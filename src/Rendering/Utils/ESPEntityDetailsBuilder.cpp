#include "ESPEntityDetailsBuilder.h"
#include "ESPFormatting.h"
#include "ESPConstants.h"
#include "../../Game/GameEnums.h"
#include <sstream>
#include <iomanip>

namespace kx {

std::vector<ColoredDetail> ESPEntityDetailsBuilder::BuildNpcDetails(const RenderableNpc* npc, const NpcEspSettings& settings, bool showDebugAddresses) {
    std::vector<ColoredDetail> details;
    
    if (!settings.renderDetails) {
        return details;
    }

    details.reserve(12); // Future-proof: generous reserve for adding new fields

    if (!npc->name.empty()) {
        details.push_back({ "NPC: " + npc->name, ESPColors::DEFAULT_TEXT });
    }

    if (settings.showDetailLevel && npc->level > 0) {
        details.push_back({ "Level: " + std::to_string(npc->level), ESPColors::DEFAULT_TEXT });
    }

    if (settings.showDetailHp && npc->maxHealth > 0) {
        details.push_back({ "HP: " + std::to_string(static_cast<int>(npc->currentHealth)) + "/" + std::to_string(static_cast<int>(npc->maxHealth)), ESPColors::DEFAULT_TEXT });
    }

    if (settings.showDetailAttitude) {
        const char* attitudeName = ESPFormatting::GetAttitudeName(npc->attitude);
        details.push_back({ "Attitude: " + (attitudeName ? std::string(attitudeName) : "ID: " + std::to_string(static_cast<uint32_t>(npc->attitude))), ESPColors::DEFAULT_TEXT });
    }

    if (settings.showDetailRank) {
        const char* rankName = ESPFormatting::GetRankName(npc->rank);
        if (rankName && rankName[0] != '\0') {
            details.push_back({ "Rank: " + std::string(rankName), ESPColors::DEFAULT_TEXT });
        }
    }

    if (settings.showDetailPosition) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1)
            << "Pos: (" << npc->position.x
            << ", " << npc->position.y
            << ", " << npc->position.z << ")";
        details.push_back({ oss.str(), ESPColors::DEFAULT_TEXT });
    }


    if (showDebugAddresses) {
        char addrStr[32];
        snprintf(addrStr, sizeof(addrStr), "Addr: 0x%p", npc->address);
        details.push_back({ std::string(addrStr), ESPColors::DEFAULT_TEXT });
    }

    return details;
}

std::vector<ColoredDetail> ESPEntityDetailsBuilder::BuildGadgetDetails(const RenderableGadget* gadget, const ObjectEspSettings& settings, bool showDebugAddresses) {
    std::vector<ColoredDetail> details;
    
    if (!settings.renderDetails) {
        return details;
    }

    details.reserve(8); // Future-proof: generous reserve for adding new fields

    const char* gadgetName = ESPFormatting::GetGadgetTypeName(gadget->type);
    details.push_back({ "Type: " + (gadgetName ? std::string(gadgetName) : "ID: " + std::to_string(static_cast<uint32_t>(gadget->type))), ESPColors::DEFAULT_TEXT });

    if (gadget->maxHealth > 0) {
        details.push_back({ "HP: " + std::to_string(static_cast<int>(gadget->currentHealth)) + "/" + std::to_string(static_cast<int>(gadget->maxHealth)), ESPColors::DEFAULT_TEXT });
    }

    if (gadget->type == Game::GadgetType::ResourceNode) {
        details.push_back({ "Node: " + ESPFormatting::ResourceNodeTypeToString(gadget->resourceType), ESPColors::DEFAULT_TEXT });
    }

    if (gadget->isGatherable) {
        details.push_back({ "Status: Gatherable", ESPColors::DEFAULT_TEXT });
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1)
        << "Pos: (" << gadget->position.x
        << ", " << gadget->position.y
        << ", " << gadget->position.z << ")";
    details.push_back({ oss.str(), ESPColors::DEFAULT_TEXT });

    if (showDebugAddresses) {
        char addrStr[32];
        snprintf(addrStr, sizeof(addrStr), "Addr: 0x%p", gadget->address);
        details.push_back({ std::string(addrStr), ESPColors::DEFAULT_TEXT });
    }

    return details;
}

} // namespace kx