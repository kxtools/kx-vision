#pragma once

#include <vector>
#include "../Data/RenderableData.h"
#include "../../Core/Settings.h"

namespace kx {

/**
 * @brief Builds detailed text information for NPC and Gadget entities
 * 
 * This class is responsible for generating display information for non-player entities,
 * including NPCs (name, level, attitude, rank) and Gadgets (type, resource nodes).
 * Extracted from ESPStageRenderer for better scalability and separation of concerns.
 */
class ESPEntityDetailsBuilder {
public:
    /**
     * @brief Build NPC information details (name, level, health, attitude, rank)
     * @param npc The NPC entity to build details for
     * @param settings NPC ESP settings for filtering what to display
     * @param showDebugAddresses Whether to include memory addresses for debugging
     * @return Vector of colored text details
     */
    static std::vector<ColoredDetail> BuildNpcDetails(const RenderableNpc* npc, const NpcEspSettings& settings, bool showDebugAddresses);

    /**
     * @brief Build Gadget information details (type, resource node info, gatherable status)
     * @param gadget The gadget entity to build details for
     * @param settings Object ESP settings for filtering what to display
     * @param showDebugAddresses Whether to include memory addresses for debugging
     * @return Vector of colored text details
     */
    static std::vector<ColoredDetail> BuildGadgetDetails(const RenderableGadget* gadget, const ObjectEspSettings& settings, bool showDebugAddresses);

    /**
     * @brief Build Attack Target information details (position, agent ID, type)
     * @param attackTarget The attack target entity to build details for
     * @param settings Object ESP settings for filtering what to display
     * @param showDebugAddresses Whether to include memory addresses for debugging
     * @return Vector of colored text details
     */
    static std::vector<ColoredDetail> BuildAttackTargetDetails(const RenderableAttackTarget* attackTarget, const ObjectEspSettings& settings, bool showDebugAddresses);
};

} // namespace kx
