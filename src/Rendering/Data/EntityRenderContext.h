#pragma once

#include <string>
#include <vector>
#include "../../../libs/glm/vec3.hpp"
#include "RenderableData.h"
#include "ESPData.h"

namespace kx {

/**
 * @brief Unified context structure for entity rendering
 * 
 * This structure consolidates all the data needed to render any type of entity
 * (player, NPC, or gadget) in a consistent way. It's created by ESPContextFactory
 * and consumed by ESPStageRenderer.
 * 
 * Design Benefits:
 * - Single source of truth for rendering parameters
 * - Eliminates code duplication across entity types
 * - Makes the rendering pipeline type-agnostic
 * - Simplifies adding new rendering features (add field once, applies everywhere)
 * - Improves testability (can create mock contexts easily)
 * 
 * @see ESPContextFactory for context creation
 * @see ESPStageRenderer for context consumption
 */
struct EntityRenderContext {
    // ===== Entity Data =====
    
    /** World position for real-time screen projection */
    const glm::vec3& position;
    
    /** Visual distance (from camera to entity) */
    float visualDistance;
    
    /** Gameplay distance (used for filtering and display) */
    float gameplayDistance;
    
    /** Primary color for rendering (box, dot, etc.) */
    unsigned int color;
    
    /** Pre-built detail strings with colors (level, profession, etc.) */
    const std::vector<ColoredDetail>& details;
    
    /** Health percentage [0.0 - 1.0], or -1.0f if not applicable */
    float healthPercent;

    // ===== Style and Settings =====
    
    /** Whether to render bounding box */
    bool renderBox;
    
    /** Whether to render distance text */
    bool renderDistance;
    
    /** Whether to render center dot */
    bool renderDot;
    
    /** Whether to render detail lines */
    bool renderDetails;
    
    /** Whether to render health bar */
    bool renderHealthBar;
    
    /** Whether to render player name (separate from details) */
    bool renderPlayerName;
    
    /** Entity type classification for rendering logic */
    ESPEntityType entityType;
    
    // ===== Screen Dimensions =====
    
    /** Screen width for bounds checking */
    float screenWidth;
    
    /** Screen height for bounds checking */
    float screenHeight;
    
    // ===== Player-Specific Data =====
    
    /** Player name (empty string for non-players) */
    const std::string& playerName;
    
    /** Pointer to full player object for summary rendering (nullptr for non-players) */
    const RenderablePlayer* player;
};

} // namespace kx
