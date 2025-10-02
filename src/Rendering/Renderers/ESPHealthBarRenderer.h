#pragma once

#include "glm.hpp"
#include "../../../libs/ImGui/imgui.h"
#include "../Data/ESPData.h"
#include "../../Game/GameEnums.h"

namespace kx {

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
     * @param entityType Type of entity (Player/NPC/Gadget) - determines color scheme
     * @param attitude Entity attitude (Hostile/Friendly/Neutral) - for NPCs
     */
    static void RenderStandaloneHealthBar(ImDrawList* drawList, const glm::vec2& centerPos,
                                         float healthPercent, unsigned int entityColor, 
                                         float barWidth, float barHeight,
                                         ESPEntityType entityType, Game::Attitude attitude = Game::Attitude::Neutral);
};

} // namespace kx
