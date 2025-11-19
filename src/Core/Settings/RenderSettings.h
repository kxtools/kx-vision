#pragma once

#include "../../libs/nlohmann/json.hpp"
#include "../../Rendering/Data/EntityTypes.h"
#include "../../Rendering/UI/UIConstants.h"

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
     * @brief Distance culling mode options
     */
    enum class DistanceCullingMode {
        Natural,      // Locked 90m global limit
        CombatFocus,  // Unlimited Players/NPCs, limited Objects
        Unlimited,    // No limit for anything
        Custom        // Granular user-defined limits
    };

/**
 * @brief Distance-based rendering configuration
 *
 * Controls how entities are culled and faded based on distance.
 * Uses intent-driven modes for clearer user experience:
 * - Natural: Mimics game's native 90m culling range
 * - CombatFocus: Unlimited Players/NPCs, limited Objects (ideal for PvP/WvW)
 * - Unlimited: No distance limits for maximum information
 * - Custom: Granular control per entity type
 * 
 * Distance Measurement:
 *   - All distances use real meters (Mumble Link standard)
 *   - Display can show meters, GW2 units, or both
 *   - 1 GW2 unit = 1 inch = 0.0254 meters
 */
    struct DistanceSettings {
        // --- Culling Mode & Value ---
        DistanceCullingMode mode = DistanceCullingMode::Natural; // The primary control
        float renderDistanceLimit = 90.0f;                      // Value used by Natural, CombatFocus, and Custom modes

        // --- Custom Mode Toggles ---
        // These are ONLY used when mode is DistanceCullingMode::Custom
        bool customLimitPlayers = false;
        bool customLimitNpcs = false;
        bool customLimitObjects = true; // Default Custom mode to only limiting objects
        
        // --- Distance Display Format ---
        DistanceDisplayMode displayMode = DistanceDisplayMode::Meters;  // How to display distances

        /**
         * @brief Helper to determine if the current mode uses a fixed distance limit for scaling.
         * This encapsulates the complex logic, cleaning up the UI code.
         * @return True if a limit-based scaling curve should be used.
         */
        bool IsInDistanceLimitMode() const {
            switch (mode) {
                case DistanceCullingMode::Natural:
                case DistanceCullingMode::CombatFocus:
                    return true;

                case DistanceCullingMode::Custom:
                    // In custom mode, it's a limit mode if any of the checkboxes are ticked.
                    return customLimitPlayers || customLimitNpcs || customLimitObjects;

                case DistanceCullingMode::Unlimited:
                default:
                    return false;
            }
        }

        /**
         * @brief Gets the active distance limit for a specific entity type, considering the current mode and game context.
         * This is the single source of truth for all distance culling logic.
         * @param entityType The type of the entity being checked.
         * @param isInWvW True if the player is currently on a WvW map.
         * @return The distance limit in meters, or a value <= 0 if there is no limit.
         */
        float GetActiveDistanceLimit(EntityTypes entityType, bool isInWvW) const {
            switch (mode) {
                case DistanceCullingMode::Natural:
                    return isInWvW ? GUI::UIConstants::WVW_NATURAL_LIMIT : GUI::UIConstants::PVE_PVP_NATURAL_LIMIT;

                case DistanceCullingMode::CombatFocus:
                    // In Combat Focus, only Objects are limited. Players/NPCs are unlimited.
                    if (entityType == EntityTypes::Gadget || entityType == EntityTypes::AttackTarget) {
                        return renderDistanceLimit;
                    }
                    return -1.0f; // No limit for Players/NPCs

                case DistanceCullingMode::Unlimited:
                    return -1.0f; // No limit for anything

                case DistanceCullingMode::Custom:
                    switch (entityType) {
                        case EntityTypes::Player:
                            return customLimitPlayers ? renderDistanceLimit : -1.0f;
                        case EntityTypes::NPC:
                            return customLimitNpcs ? renderDistanceLimit : -1.0f;
                        case EntityTypes::Gadget:
                        case EntityTypes::AttackTarget:
                            return customLimitObjects ? renderDistanceLimit : -1.0f;
                        default:
                            return -1.0f;
                    }
            }
            return -1.0f; // Default to no limit
        }
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DistanceSettings, mode, renderDistanceLimit, customLimitPlayers, customLimitNpcs, customLimitObjects, displayMode);

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
