#pragma once

#include <vector>
#include "../Data/RenderableData.h"
#include "../Data/EntityRenderContext.h"
#include "../../Core/Settings.h"

namespace kx {

class CombatStateManager; // Forward declaration

struct FactoryContext
{
    const Settings& settings;
    const CombatStateManager& stateManager;
    float screenWidth;
    float screenHeight;
    uint64_t now;
};

/**
 * @brief Factory class for creating EntityRenderContext objects
 * 
 * This class encapsulates the logic for translating specific renderable entity types
 * (RenderablePlayer, RenderableNpc, RenderableGadget) into generic EntityRenderContext
 * structures used by the rendering pipeline.
 * 
 * Benefits:
 * - Centralizes context creation logic
 * - Reduces code duplication in rendering loops
 * - Makes maintenance easier (change once, applies everywhere)
 * - Improves readability of main rendering code
 */
class ESPContextFactory {
public:
    /**
     * @brief Create rendering context for a player entity
     * @param player Player data to render
     * @param settings Application settings
     * @param stateManager The combat state manager for state-aware context.
     * @param details Pre-built detail strings for display
     * @param screenWidth Screen width in pixels
     * @param screenHeight Screen height in pixels
     * @return Fully constructed EntityRenderContext
     */
    static EntityRenderContext CreateContextForPlayer(const RenderablePlayer* player, const std::vector<ColoredDetail>& details, const FactoryContext& context);

    /**
     * @brief Create rendering context for an NPC entity
     * @param npc NPC data to render
     * @param settings Application settings
     * @param stateManager The combat state manager for state-aware context.
     * @param details Pre-built detail strings for display
     * @param screenWidth Screen width in pixels
     * @param screenHeight Screen height in pixels
     * @return Fully constructed EntityRenderContext
     */
    static EntityRenderContext CreateContextForNpc(const RenderableNpc* npc, const std::vector<ColoredDetail>& details, const FactoryContext& context);

    /**
     * @brief Create rendering context for a gadget/object entity
     * @param gadget Gadget data to render
     * @param settings Application settings
     * @param stateManager The combat state manager for state-aware context.
     * @param details Pre-built detail strings for display
     * @param screenWidth Screen width in pixels
     * @param screenHeight Screen height in pixels
     * @return Fully constructed EntityRenderContext
     */
    static EntityRenderContext CreateContextForGadget(const RenderableGadget* gadget, const std::vector<ColoredDetail>& details, const FactoryContext& context);
};

} // namespace kx
