#pragma once

#include "../../libs/nlohmann/json.hpp"

namespace kx {

    /**
     * @brief Distance display format options
     */
    enum class DistanceDisplayMode {
        Meters,      // "30.5m"
        GW2Units,    // "1200"
        Both         // "1200 (30.5m)"
    };

/**
 * @brief Distance-based rendering configuration
 *
 * Controls how entities are culled and faded based on distance.
 * Two modes with different philosophies:
 * - Limit Mode: Natural Integration - mimics game's native behavior
 * - No Limit Mode: Maximum Information Clarity - adaptive to scene
 * 
 * Distance Measurement:
 *   - All distances use real meters (Mumble Link standard)
 *   - Display can show meters, GW2 units, or both
 *   - 1 GW2 unit = 1 inch = 0.0254 meters
 */
    struct DistanceSettings {
        // --- Distance Limiting ---
        bool useDistanceLimit = true;           // Enable/disable distance-based culling
        float renderDistanceLimit = 90.0f;      // Hard cutoff distance (mimics game's native culling range)
        
        // --- Distance Display Format ---
        DistanceDisplayMode displayMode = DistanceDisplayMode::Meters;  // How to display distances
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DistanceSettings, useDistanceLimit, renderDistanceLimit, displayMode);

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
     * @brief Visual appearance and styling options
     * 
     * Controls the visual styling of ESP elements (not their sizes).
     * These settings affect how elements look, not their dimensions.
     */
    struct AppearanceSettings {
        // --- Global Controls ---
        float globalOpacity = 0.8f;             // Global opacity multiplier for ALL ESP elements (50-100%, default 80%)
        
        // --- Text Styling ---
        bool enableTextBackgrounds = true;      // Add dark backgrounds behind text (except damage numbers)
        bool enableTextShadows = true;          // Add shadows behind text for better contrast
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AppearanceSettings, globalOpacity, enableTextBackgrounds, enableTextShadows);

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
        float baseHealthBarHeight = 7.0f;       // Health bar height (~8.5:1 ratio, bold visibility)
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ElementSizeSettings, baseFontSize, minFontSize, baseDotRadius, baseBoxThickness,
                                       baseBoxHeight, baseBoxWidth, baseHealthBarWidth, baseHealthBarHeight);

} // namespace kx
