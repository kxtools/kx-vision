#pragma once

#include <string>
#include <vector>
#include "RenderableData.h"
#include "ESPEntityTypes.h"
#include "../../Game/GameEnums.h"

namespace kx {

/**
 * @brief Holds all transient, frame-specific animation state for health bars.
 *
 * This data is calculated once by the ESPContextFactory and then consumed by
 * the ESPHealthBarRenderer, which becomes a "dumb" component that only draws
 * what it's given. This decouples rendering from state calculation.
 */
struct HealthBarAnimationState {
    // Overall fade alpha for the entire bar (combines distance, death, etc.)
    float healthBarFadeAlpha = 1.0f;

    // --- Damage Accumulator ---
    // The percentage of the bar the damage accumulator should cover (0 if inactive)
    float damageAccumulatorPercent = 0.0f;
    // Alpha for the damage accumulator's fade-out animation
    float damageAccumulatorAlpha = 1.0f;

    float damageNumberToDisplay = 0.0f;
    float damageNumberAlpha = 0.0f;
    float damageNumberYOffset = 0.0f; // For the upward scroll effect

    // --- Healing Overlay ---
    // The starting health percentage for the current heal overlay (0 if inactive)
    float healOverlayStartPercent = 0.0f;
    // The ending health percentage for the current heal overlay
    float healOverlayEndPercent = 0.0f;
    // Alpha multiplier for the heal overlay's fade-out animation
    float healOverlayAlpha = 0.0f;

    // --- Flashes ---
    // Alpha for the instant damage flash
    float damageFlashAlpha = 0.0f;
    // The health percentage where the damage flash should start
    float damageFlashStartPercent = 0.0f;
    // Alpha for the instant heal flash
    float healFlashAlpha = 0.0f;

    // --- Barrier ---
    // The animated barrier value for smooth transitions
    float animatedBarrier = 0.0f;

    // --- Death Animation ---
    // The alpha of the death "burst" effect
    float deathBurstAlpha = 0.0f;
    // The width of the death "burst" effect, from 0.0 to 1.0
    float deathBurstWidth = 0.0f;
};


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
    
    /** Gameplay distance (used for filtering and display) */
    float gameplayDistance;
    
    /** Primary color for rendering (box, dot, etc.) */
    unsigned int color;
    
    /** Pre-built detail strings with colors (level, profession, etc.) */
    std::vector<ColoredDetail> details;
    
    /** Calculated live burst DPS for the current damage window */
    float burstDPS;

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
    /** Whether to render health percentage text */
    bool renderHealthPercentage;
    /** Whether to render energy bar */
    bool renderEnergyBar;
    
    /** Whether to render player name (separate from details) */
    bool renderPlayerName;
    
    /** Entity type classification for rendering logic */
    ESPEntityType entityType;
    
    /** Attitude/relationship for NPCs and players (used for health bar coloring) */
    Game::Attitude attitude;

    // ===== Entity-Specific Data =====
    
    /** Pointer to the original entity for state lookup */
    const RenderableEntity* entity;

    /** Player name (empty string for non-players) */
    const std::string& playerName;

    /** Transient animation state for the health bar */
    HealthBarAnimationState healthBarAnim;
};

} // namespace kx
