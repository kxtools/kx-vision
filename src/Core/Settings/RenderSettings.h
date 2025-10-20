#pragma once

#include "../../libs/nlohmann/json.hpp"

namespace kx {

    /**
     * @brief Distance-based rendering configuration
     * 
     * Controls how entities are culled and faded based on distance.
     * Two modes with different philosophies:
     * - Limit Mode: Natural Integration - mimics game's native behavior
     * - No Limit Mode: Maximum Information Clarity - adaptive to scene
     */
    struct DistanceSettings {
        // --- Distance Limiting ---
        bool useDistanceLimit = true;           // Enable/disable distance-based culling
        float renderDistanceLimit = 90.0f;      // Hard cutoff distance (mimics game's native culling range)
        
        // --- Advanced Fading (No Limit Mode Only) ---
        bool enablePlayerNpcFade = true;        // Apply subtle fade to players/NPCs at long range (80m-120m)
        float playerNpcMinAlpha = 0.5f;         // Minimum opacity for players/NPCs at max range (50% for depth)
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DistanceSettings, useDistanceLimit, renderDistanceLimit, enablePlayerNpcFade,
                                       playerNpcMinAlpha);

    /**
     * @brief Scaling curve configuration
     * 
     * Controls how entity sizes shrink with distance using the formula:
     * scale = distanceFactor / (distanceFactor + distance^exponent)
     * 
     * The distanceFactor determines where 50% scale occurs.
     * The exponent controls the curve shape (higher = more aggressive at distance).
     */
    struct ScalingSettings {
        // --- Shared Settings (Both Modes) ---
        float scalingStartDistance = 20.0f;     // Distance before scaling begins (mimics game camera-to-player offset)
        float minScale = 0.1f;                  // Minimum scale multiplier (10% - allows extreme shrinking, protected by min sizes)
        float maxScale = 1.0f;                  // Maximum scale multiplier (100% - no magnification for natural feel)
        
        // --- Limit Mode (90m range) ---
        float limitDistanceFactor = 110.0f;     // 50% scale at 110m (just past render limit for meaningful scaling)
        float limitScalingExponent = 1.2f;      // Moderate curve - balanced shrinking over 0-90m range
        
        // --- No Limit Mode (Adaptive range) ---
        float noLimitScalingExponent = 1.2f;    // Balanced curve for long distances (distanceFactor auto-calculated from scene)
        // Note: distanceFactor = adaptiveFarPlane / 2 (automatic 50% scale at midpoint)
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ScalingSettings, scalingStartDistance, minScale, maxScale, limitDistanceFactor,
                                       limitScalingExponent, noLimitScalingExponent);

    /**
     * @brief Base sizes for ESP elements before scaling
     * 
     * These are the "100% scale" sizes. Distance-based scaling multiplies these values.
     * Minimum size protections prevent elements from becoming unusably small.
     */
    struct ElementSizeSettings {
        // --- Text ---
        float baseFontSize = 16.0f;             // Optimal for Bahnschrift (wide letterforms, good balance)
        float minFontSize = 8.0f;               // Absolute minimum font size (readability floor)
        
        // --- Shapes ---
        float baseDotRadius = 3.0f;             // Center dot size (clean, minimal at 2.4px actual with multiplier)
        float baseBoxThickness = 2.0f;          // Bounding box line thickness (optimal visibility)
        float baseBoxHeight = 90.0f;             // Player/NPC box height (realistic proportions)
        float baseBoxWidth = 45.0f;             // Player/NPC box width (2:1 ratio = balanced humanoid shape)
        
        // --- Health Bars ---
        float baseHealthBarWidth = 60.0f;       // Health bar width (33% wider than box, maximum prominence)
        float baseHealthBarHeight = 7.0f;        // Health bar height (~8.5:1 ratio, bold visibility)
        
        // --- Text Display Options ---
        bool enableTextBackgrounds = true;   // Global toggle for text backgrounds (except damage numbers)
        bool enableTextShadows = true;       // Global toggle for text shadows (all text types)
        float globalTextAlpha = 0.8f;        // Global text opacity multiplier (0.5-1.0 range) - 80% matches GW2's subtle UI
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ElementSizeSettings, baseFontSize, minFontSize, baseDotRadius, baseBoxThickness,
                                       baseBoxHeight, baseBoxWidth, baseHealthBarWidth, baseHealthBarHeight,
                                       enableTextBackgrounds, enableTextShadows, globalTextAlpha);

} // namespace kx
