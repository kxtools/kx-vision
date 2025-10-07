#pragma once

#include "glm.hpp"
#include "../../../libs/ImGui/imgui.h"
#include "../Data/ESPData.h"
#include "../../Game/GameEnums.h"

namespace kx {

	// Forward declarations
	class CombatStateManager;
	struct EntityRenderContext;
	struct EntityCombatState;

/**
 * @brief Utility functions for rendering health bars
 * 
 * This class contains specialized methods for rendering different types of health bars
 * used in the ESP system. Separated for better organization and maintainability.
 */
class ESPHealthBarRenderer {
public:
    /**
     * @brief Render a standalone health bar below an entity with informative color coding
     * @param drawList ImGui draw list for rendering
     * @param centerPos Center position of the entity
     * @param healthPercent Health percentage (0.0-1.0)
     * @param entityColor Entity color (contains alpha for distance fading)
     * @param barWidth Width of the health bar
     * @param barHeight Height of the health bar
     * @param stateManager Reference to the combat state manager for damage flash effects
     */
    static void RenderStandaloneHealthBar(ImDrawList* drawList, const glm::vec2& centerPos,
                                         const EntityRenderContext& context, unsigned int entityColor, 
                                         float barWidth, float barHeight,
                                         const CombatStateManager& stateManager);

    /**
     * @brief Render a standalone energy bar below the health bar
     * @param drawList ImGui draw list for rendering
     * @param centerPos Center position of the entity
     * @param energyPercent Energy percentage (0.0-1.0)
     * @param fadeAlpha Distance-based fade alpha
     * @param barWidth Width of the bar
     * @param barHeight Height of the bar
     * @param healthBarHeight Height of the health bar (for correct vertical positioning)
     */
    static void RenderStandaloneEnergyBar(ImDrawList* drawList, const glm::vec2& centerPos,
                                         float energyPercent, float fadeAlpha,
                                         float barWidth, float barHeight, float healthBarHeight);

private:
    // Specialist function for rendering a living entity's health bar and effects
    static void RenderAliveState(ImDrawList* drawList, const EntityRenderContext& context, const EntityCombatState* state,
                                 const ImVec2& barMin, const ImVec2& barMax, float barWidth, unsigned int entityColor, float fadeAlpha);

    // Specialist function for rendering the death animation
    static void RenderDeadState(ImDrawList* drawList, const EntityCombatState* state,
                                const ImVec2& barMin, const ImVec2& barMax, float barWidth, float fadeAlpha);
};

} // namespace kx
