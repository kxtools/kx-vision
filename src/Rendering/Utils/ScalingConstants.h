#pragma once

namespace kx {

/**
 * @brief Adaptive scaling system constants
 * 
 * Constants used for the intelligent adaptive far plane and distance-based scaling system.
 * These values control how the system adapts to different map sizes and entity distributions.
 */
namespace AdaptiveScaling {
    // Adaptive far plane bounds (applies to gadgets/objects only)
    constexpr float FAR_PLANE_MIN = 100.0f;   // Minimum far plane for small instances/dungeons
    constexpr float FAR_PLANE_MAX = 3000.0f;  // Maximum far plane to prevent extreme outliers
    constexpr float FAR_PLANE_DEFAULT = 800.0f; // Fallback when no gadgets present (mid-range)
    constexpr float FAR_PLANE_INITIAL = 1500.0f; // Initial startup value
    
    // Minimum sample size for percentile calculation
    constexpr size_t MIN_ENTITIES_FOR_PERCENTILE = 10; // Need at least 10 entities for meaningful statistics
    
    // Adaptive far plane calculation parameters
    constexpr float PERCENTILE_THRESHOLD = 0.95f;      // 95th percentile for scene depth
    constexpr float SMOOTHING_FACTOR = 0.5f;           // LERP factor for temporal smoothing
    constexpr int RECALC_INTERVAL_SECONDS = 1;         // Update frequency in seconds
    
    // Distance factors for scaling calculation (50% scale point)
    constexpr float PLAYER_NPC_DISTANCE_FACTOR = 150.0f;  // Fixed factor for players/NPCs (they're limited to ~200m)
    constexpr float GADGET_MIN_DISTANCE_FACTOR = 150.0f;  // Minimum factor for gadgets (matches player/NPC baseline)
    
    // Alpha fading constants (Gadgets - adaptive long-range fade)
    constexpr float FADE_START_DISTANCE = 90.0f; // Start fading beyond game's natural entity culling range
    constexpr float MIN_ALPHA = 0.3f;            // Minimum opacity for gadgets at extreme distances (30%)
    
    // Alpha fading constants (Players/NPCs - subtle fixed-range fade)
    constexpr float PLAYER_NPC_FADE_START = 80.0f;  // Start fade at same point as Limit Mode (consistent feel)
    constexpr float PLAYER_NPC_FADE_END = 120.0f;   // End fade at earliest culling distance (game culls at 120-200m)
    constexpr float PLAYER_NPC_MIN_ALPHA = 0.5f;     // 50% minimum (higher than gadgets for combat clarity)
}

/**
 * @brief Scaling limits for ESP elements
 * 
 * Maximum and minimum bounds for scaled ESP elements to prevent extreme sizes
 * at very close or very distant ranges. These limits ensure visual quality and
 * prevent performance issues from oversized elements.
 */
namespace ScalingLimits {
    // Font size limits (base: 16px)
    constexpr float MAX_FONT_SIZE = 40.0f;           // Maximum font size (2.5x base, prevents giant text)
    // Min font size defined in Settings (9px) for user configurability
    
    // Box thickness limits (base: 2px)
    constexpr float MIN_BOX_THICKNESS = 1.0f;        // Minimum line thickness (prevents invisible lines)
    constexpr float MAX_BOX_THICKNESS = 10.0f;       // Maximum line thickness (5x base, prevents chunky borders)
    
    // Dot radius limits (base: 3px)
    constexpr float MIN_DOT_RADIUS = 1.0f;           // Minimum dot size (visible pixel)
    constexpr float MAX_DOT_RADIUS = 15.0f;          // Maximum dot size (5x base, prevents giant dots)
    
    // Health bar limits (base: 60w × 7h)
    constexpr float MIN_HEALTH_BAR_WIDTH = 10.0f;    // Minimum readable width
    constexpr float MAX_HEALTH_BAR_WIDTH = 200.0f;   // Maximum width (3.33x base, allows for rank multipliers)
    constexpr float MIN_HEALTH_BAR_HEIGHT = 2.0f;    // Minimum visible height
    constexpr float MAX_HEALTH_BAR_HEIGHT = 25.0f;   // Maximum height (3.57x base, allows for rank multipliers)
}

/**
 * @brief Rendering effect constants
 * 
 * Constants that control visual effects and rendering behaviors in the ESP system.
 * These values tune the visual experience and performance characteristics.
 */
namespace RenderingEffects {
    // Distance fade zone - entities fade out in the last 11% of their distance limit
    // e.g., with 90m limit: fade starts at 80m (90 * 0.89), ends at 90m
    constexpr float FADE_ZONE_PERCENTAGE = 0.11f;
    
    // Hostile player visibility enhancement (PvP combat awareness)
    // This multiplier is applied to COMBAT-CRITICAL hostile player elements:
    // - Player name font size (for quick identification)
    // - Health bar width/height (for health tracking)
    // NOTE: Box thickness intentionally NOT multiplied to reduce visual clutter
    constexpr float HOSTILE_PLAYER_VISUAL_MULTIPLIER = 2.0f;
}

/**
 * @brief Gadget health scaling constants
 * 
 * Constants for dynamic scaling of gadget health bars based on maximum health.
 * Provides smooth visual progression from normal gadgets to epic structures.
 */
namespace GadgetHealthScaling {
    constexpr float MIN_MULTIPLIER = 1.0f;    // Normal gadgets
    constexpr float MAX_MULTIPLIER = 2.0f;    // Epic structures (matches legendary rank)
    constexpr float HP_TO_REACH_MAX = 1000000.0f; // 1M HP = max emphasis
}

/**
 * @brief Damage number scaling constants
 * 
 * Constants for dynamic scaling of damage numbers based on burst DPS totals.
 * Tuned for GW2 2025 burst ranges:
 * - Normal rotation: 50-83K (top builds)
 * - Big crits: 220K (Bladesworn)
 * - Defiance burst: 660K (broken defiance bar)
 * - Group spike: 400K+ (coordinated)
 */
namespace DamageNumberScaling {
    constexpr float MIN_MULTIPLIER = 2.0f;       // All damage emphasized (GW2 numbers are large)
    constexpr float MAX_MULTIPLIER = 5.0f;       // Increased from 4.0f for better visual range
    constexpr float DAMAGE_TO_REACH_MAX = 400000.0f; // Covers Bladesworn crits + group spikes
}

/**
 * @brief NPC rank multiplier constants
 * 
 * Health bar size multipliers for different NPC rank tiers.
 * Creates visual hierarchy matching threat level.
 * Max of 2.0x maintains consistency with gadget health and hostile player systems.
 */
namespace RankMultipliers {
    constexpr float NORMAL = 1.0f;      // Baseline
    constexpr float VETERAN = 1.25f;    // +25%
    constexpr float ELITE = 1.5f;       // +50%
    constexpr float CHAMPION = 1.75f;   // +75%
    constexpr float LEGENDARY = 2.0f;   // +100% (matches unified max)
}

} // namespace kx
