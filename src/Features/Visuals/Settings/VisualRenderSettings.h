#pragma once

#include "../../../../libs/nlohmann/json.hpp"
#include "../../../Game/Data/EntityTypes.h"
#include "../../../Core/Settings/SettingsFields.h"

namespace kx {

namespace VisualDistanceLimits {
    inline constexpr float PVE_PVP_NATURAL = 90.0f;
    inline constexpr float WVW_NATURAL = 130.0f;
}

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
#define KX_DISTANCE_SETTINGS(X) \
        X(DistanceCullingMode, mode, DistanceCullingMode::Natural) \
        X(float, renderDistanceLimit, 90.0f) \
        X(bool, customLimitPlayers, false) \
        X(bool, customLimitNpcs, false) \
        X(bool, customLimitObjects, true) \
        X(DistanceDisplayMode, displayMode, DistanceDisplayMode::Meters)
    struct DistanceSettings {
        KX_DISTANCE_SETTINGS(KX_SETTING_DECLARE)

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
         * @brief Helper to check if an entity type is an object type (Gadget, AttackTarget, or Item)
         */
        static bool IsObjectType(EntityTypes type) {
            return type == EntityTypes::Gadget || 
                   type == EntityTypes::AttackTarget || 
                   type == EntityTypes::Item;
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
                    return isInWvW ? VisualDistanceLimits::WVW_NATURAL : VisualDistanceLimits::PVE_PVP_NATURAL;

                case DistanceCullingMode::CombatFocus:
                    // In Combat Focus, only Objects are limited. Players/NPCs are unlimited.
                    if (IsObjectType(entityType)) {
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
                        case EntityTypes::Item:
                            return customLimitObjects ? renderDistanceLimit : -1.0f;
                        default:
                            return -1.0f;
                    }
            }
            return -1.0f; // Default to no limit
        }
    };

    KX_DEFINE_SETTINGS_JSON(DistanceSettings, KX_DISTANCE_SETTINGS)

    /**
     * @brief Scaling curve configuration
     * 
     * Controls how entity sizes shrink with distance using the formula:
     * scale = distanceFactor / (distanceFactor + distance^exponent)
     * 
     * The distanceFactor determines where 50% scale occurs.
     * The exponent controls the curve shape (higher = more aggressive at distance).
     */
#define KX_SCALING_SETTINGS(X) \
        X(float, scalingStartDistance, 20.0f) \
        X(float, minScale, 0.1f) \
        X(float, maxScale, 1.0f) \
        X(float, limitDistanceFactor, 110.0f) \
        X(float, limitScalingExponent, 1.2f) \
        X(float, noLimitScalingExponent, 1.2f)
    KX_DEFINE_SETTINGS_TYPE(ScalingSettings, KX_SCALING_SETTINGS)

    /**
     * @brief Visual appearance and styling options
     * 
     * Controls the visual styling of ESP elements (not their sizes).
     * These settings affect how elements look, not their dimensions.
     */
#define KX_APPEARANCE_SETTINGS(X) \
        X(float, globalOpacity, 0.8f) \
        X(bool, enableTextBackgrounds, true) \
        X(bool, enableTextShadows, true)
    KX_DEFINE_SETTINGS_TYPE(AppearanceSettings, KX_APPEARANCE_SETTINGS)

    /**
     * @brief Base sizes for ESP elements before scaling
     * 
     * These are the "100% scale" sizes. Distance-based scaling multiplies these values.
     * Minimum size protections prevent elements from becoming unusably small.
     */
#define KX_ELEMENT_SIZE_SETTINGS(X) \
        X(float, baseFontSize, 16.0f) \
        X(float, minFontSize, 8.0f) \
        X(float, baseDotRadius, 3.0f) \
        X(float, baseBoxThickness, 2.0f) \
        X(float, baseBoxHeight, 90.0f) \
        X(float, baseBoxWidth, 45.0f) \
        X(float, baseHealthBarWidth, 60.0f) \
        X(float, baseHealthBarHeight, 7.0f)
    KX_DEFINE_SETTINGS_TYPE(ElementSizeSettings, KX_ELEMENT_SIZE_SETTINGS)

} // namespace kx
