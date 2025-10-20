#pragma once

namespace kx {

/**
 * @brief Minimum size constraints for entity visibility
 * 
 * These constants ensure that entities remain visible even at extreme distances
 * by enforcing minimum pixel dimensions for each entity type. Values are optimized
 * based on base sizes (90h×45w) and scaling system (minScale=0.1, ~22% for minimums).
 */
namespace MinimumSizes {
    // Player minimum sizes (maintains 2:1 humanoid aspect ratio)
    constexpr float PLAYER_MIN_HEIGHT = 20.0f;   // ~22% of base 90px height
    constexpr float PLAYER_MIN_WIDTH = 10.0f;    // Proportional: 20÷2.0 (maintains 2:1 ratio)
    
    // NPC minimum sizes (square boxes, larger for better visibility)
    constexpr float NPC_MIN_HEIGHT = 15.0f;      // ~33% of base 45px width (square rendering)
    constexpr float NPC_MIN_WIDTH = 15.0f;       // Square maintains recognizable shape
    
    // Gadget minimum sizes (circles, very small but distinct points)
    constexpr float GADGET_MIN_HEIGHT = 3.0f;    // ~44% of full-scale radius (base 6.75px = 45w × 0.15)
    constexpr float GADGET_MIN_WIDTH = 3.0f;     // Minimum for circle visibility at distance
}

/**
 * @brief World-space bounding box dimensions for 3D projection
 * 
 * These constants define the actual physical dimensions (in meters) of entities
 * in the game world, used for proper 3D bounding box projection to screen space.
 * 
 * Design philosophy:
 * - NPCs: Cube-like dimensions (0.7m all sides) for square appearance from most angles
 * - Players: Rectangular dimensions (narrow width, tall height) for distinct humanoid shape
 */
namespace EntityWorldBounds {
    // Player bounding box in world space (meters) - RECTANGULAR feel
    constexpr float PLAYER_WORLD_WIDTH = 0.6f;   // 0.6m width (wider, less narrow - was 0.4m)
    constexpr float PLAYER_WORLD_DEPTH = 0.6f;   // 0.6m depth (matches width)
    constexpr float PLAYER_WORLD_HEIGHT = 1.4f;  // 1.4m height (tall for distinct rectangle - 2.3:1 ratio)
    
    // NPC bounding box in world space (meters) - SQUARE/CUBE feel
    constexpr float NPC_WORLD_WIDTH = 0.6f;      // 0.6m - smaller cube for less screen clutter
    constexpr float NPC_WORLD_DEPTH = 0.6f;      // 0.6m - matches width
    constexpr float NPC_WORLD_HEIGHT = 0.6f;     // 0.6m - matches width (1:1:1 cube)
}

/**
 * @brief Entity size ratios and proportions
 * 
 * Defines the relative sizing and proportions for different entity types.
 * These ratios determine how entity sizes relate to the base box dimensions.
 */
namespace EntitySizeRatios {
    // Gadget circle sizing (relative to base box width)
    constexpr float GADGET_CIRCLE_RADIUS_RATIO = 0.15f;  // Circle radius = baseBoxWidth × 0.15 (6.75px from 45px)
    // Results in 13.5px diameter (30% of player width) - small enough for "objects" not "characters"
    
    // NPC box sizing (uses base box width directly - see CalculateEntityBoxDimensions)
    // NPCs are 45×45 squares (1.0× player width) for visual consistency
}

/**
 * @brief Coordinate transformation constants
 * 
 * Constants used for converting between game world coordinates and MumbleLink coordinates.
 * The game uses different coordinate systems internally vs what's exposed through MumbleLink.
 */
namespace CoordinateTransform {
    // Scale factor for converting game world coordinates to MumbleLink meter-based units
    constexpr float GAME_TO_MUMBLE_SCALE_FACTOR = 1.23f;
}

/**
 * @brief Data extraction capacity constants
 * 
 * Initial capacity reservations for entity collections to minimize dynamic allocations
 * during frame data extraction. These values are based on actual game data analysis:
 * - PlayerList typically has ~134 players, we extract ~12 valid players
 * - CharacterList capacity ~9728, we extract ~29 NPCs 
 * - GadgetList capacity ~9216, we extract ~457 gadgets
 * - Values include safe buffer for peak scenarios and different map types
 */
namespace ExtractionCapacity {
    constexpr size_t PLAYERS_RESERVE = 64;     // ~12 typical + buffer for busy instances
    constexpr size_t NPCS_RESERVE = 128;       // ~29 typical + buffer for NPC-heavy areas  
    constexpr size_t GADGETS_RESERVE = 1024;   // ~457 typical + buffer for resource-rich zones
}

/**
 * @brief Screen space culling constants
 * 
 * Constants used for determining when entities are visible on screen.
 */
namespace ScreenCulling {
    // Margin for screen bounds culling - allows partially visible entities to render
    constexpr float VISIBILITY_MARGIN = 50.0f;
}

/**
 * @brief ESP layout constants for positioning and sizing visual elements.
 *
 * Optimized values based on base sizes (16px font, 45w×90h boxes, 60w×7h HP bars)
 * and industry-standard UI spacing. All alpha values use 0-255 range.
 */
namespace RenderingLayout {
    // Layout
    constexpr float ELEMENT_MARGIN_VERTICAL = 4.0f; // Spacing between stacked elements
    constexpr float REGION_MARGIN_VERTICAL = 8.0f;  // Spacing between the box and the first element

    // Common (base rendering elements)
    constexpr float TEXT_SHADOW_OFFSET = 1.0f;       // 1px shadow for crisp text readability
    constexpr float DOT_RADIUS_MULTIPLIER = 0.8f;    // 80% of base radius (2.4px from 3px base)

    // Standalone Health Bar (horizontal bars below entities)
    constexpr float STANDALONE_HEALTH_BAR_Y_OFFSET = 12.0f;          // 12px below entity (compact spacing)
    constexpr float STANDALONE_HEALTH_BAR_BG_ROUNDING = 1.0f;        // Subtle 1px rounding
    constexpr float STANDALONE_HEALTH_BAR_BORDER_ROUNDING = 1.0f;    // Matches background
    constexpr float STANDALONE_HEALTH_BAR_BORDER_THICKNESS = 1.0f;   // crisp border
    constexpr float STANDALONE_HEALTH_BAR_BG_ALPHA = 180.0f;         // ~71% opacity (readable)
    constexpr float STANDALONE_HEALTH_BAR_HEALTH_ALPHA = 220.0f;     // ~86% opacity (prominent)
    constexpr float STANDALONE_HEALTH_BAR_BORDER_ALPHA = 100.0f;     // ~39% opacity (subtle)

    // Player Name (above entity, prominent label)
    constexpr float PLAYER_NAME_BG_PADDING_X = 4.0f;          // 4px horizontal padding
    constexpr float PLAYER_NAME_BG_PADDING_Y = 2.0f;          // 2px vertical padding
    constexpr float PLAYER_NAME_BG_ROUNDING = 3.0f;           // 3px rounded corners
    constexpr float PLAYER_NAME_BORDER_THICKNESS = 1.0f;      // 1px crisp border
    constexpr float PLAYER_NAME_BG_ALPHA = 60.0f;             // ~24% opacity (very subtle)
    constexpr float PLAYER_NAME_BORDER_ALPHA = 120.0f;        // ~47% opacity (visible outline)
    constexpr float PLAYER_NAME_SHADOW_ALPHA = 180.0f;        // ~71% opacity (strong shadow)
    constexpr float PLAYER_NAME_TEXT_ALPHA = 220.0f;          // ~86% opacity (clear text)

    // Bounding Box (corner-style box rendering)
    constexpr float BOX_CORNER_SIZE_MULTIPLIER = 4.0f;        // Corner length = thickness × 4 (8px at 2px thick)

    // Distance Text (compact metric label)
    constexpr float DISTANCE_TEXT_BG_PADDING_X = 3.0f;        // 3px horizontal padding (increased from 2px)
    constexpr float DISTANCE_TEXT_BG_PADDING_Y = 1.0f;        // 1px vertical padding (compact)
    constexpr float DISTANCE_TEXT_BG_ROUNDING = 2.0f;         // 2px rounded corners
    constexpr float DISTANCE_TEXT_BG_ALPHA = 60.0f;           // ~24% opacity (very subtle)
    constexpr float DISTANCE_TEXT_SHADOW_ALPHA = 180.0f;      // ~71% opacity (strong shadow)
    constexpr float DISTANCE_TEXT_TEXT_ALPHA = 220.0f;        // ~86% opacity (clear text)

    // Details Text (multi-line entity information)
    constexpr float DETAILS_TEXT_BG_PADDING_X = 4.0f;         // 4px horizontal padding (increased from 3px)
    constexpr float DETAILS_TEXT_BG_PADDING_Y = 2.0f;         // 2px vertical padding (increased from 1px)
    constexpr float DETAILS_TEXT_BG_ROUNDING = 2.0f;          // 2px rounded corners (increased from 1px)
    constexpr float DETAILS_TEXT_LINE_SPACING = 2.0f;         // 2px between lines (reduced from 3px)
    constexpr float DETAILS_TEXT_BG_ALPHA = 60.0f;            // ~24% opacity (very subtle)
    constexpr float DETAILS_TEXT_SHADOW_ALPHA = 180.0f;       // ~71% opacity (strong shadow)

    // Compact Summary Views (gear display, stat summaries)
    constexpr float SUMMARY_BG_PADDING_X = 5.0f;              // 5px horizontal padding (increased from 4px)
    constexpr float SUMMARY_BG_PADDING_Y = 3.0f;              // 3px vertical padding (increased from 2px)
    constexpr float SUMMARY_BG_ROUNDING = 3.0f;               // 3px rounded corners
    constexpr float SUMMARY_BG_ALPHA = 70.0f;                 // ~27% opacity (increased from 60 for visibility)
    constexpr float SUMMARY_SHADOW_ALPHA = 180.0f;            // ~71% opacity (increased from 160)
    constexpr float SUMMARY_TEXT_ALPHA = 220.0f;              // ~86% opacity (increased from 200)

    // Text Rendering - Positioning (generic text system)
    constexpr float TEXT_ANCHOR_GAP = 5.0f;                   // 5px gap between anchor and text
    constexpr float TEXT_LINE_SPACING_EXTRA = 2.0f;           // 2px extra spacing between lines

    // Text Rendering - Default Style Values (fallback when not specified)
    constexpr float TEXT_DEFAULT_FONT_SIZE = 14.0f;           // 14px default (increased from 13px)
    constexpr float TEXT_DEFAULT_SHADOW_OFFSET_X = 1.0f;      // 1px horizontal shadow
    constexpr float TEXT_DEFAULT_SHADOW_OFFSET_Y = 1.0f;      // 1px vertical shadow
    constexpr float TEXT_DEFAULT_SHADOW_ALPHA = 128.0f;       // ~50% opacity (crisp shadow)
    constexpr float TEXT_DEFAULT_BG_PADDING_X = 4.0f;         // 4px horizontal padding
    constexpr float TEXT_DEFAULT_BG_PADDING_Y = 2.0f;         // 2px vertical padding
    constexpr float TEXT_DEFAULT_BG_ALPHA = 180.0f;           // ~71% opacity (visible background)
    constexpr float TEXT_DEFAULT_BG_ROUNDING = 3.0f;          // 3px rounded corners
    constexpr float TEXT_DEFAULT_BORDER_THICKNESS = 1.0f;     // 1px crisp border
    constexpr float TEXT_DEFAULT_LINE_SPACING = 2.0f;

    // Burst DPS Display
    constexpr float BURST_DPS_HORIZONTAL_PADDING = 5.0f;     // Padding between elements
    constexpr float BURST_DPS_FALLBACK_Y_OFFSET = 20.0f;     // Y-offset when HP bar is off
    constexpr float HP_PERCENT_FONT_SIZE_MULTIPLIER = 0.8f;  // 80% of base font size
}

/**
 * @brief Constants for the 3D Gadget Sphere/Gyroscope visual.
 */
namespace GadgetSphere {
    // LOD Transition
    constexpr float LOD_TRANSITION_START = 80.0f;
    constexpr float LOD_TRANSITION_END = 90.0f;

    // Geometry
    constexpr int NUM_RING_POINTS = 16;
    constexpr float VERTICAL_RADIUS = 0.35f;
    constexpr float HORIZONTAL_RADIUS_RATIO = 0.9f; // Horizontal radius = Vertical * this

    // Thickness
    constexpr float BASE_THICKNESS = 2.5f;
    constexpr float MIN_THICKNESS = 1.0f;
    constexpr float MAX_THICKNESS = 5.0f;
    constexpr float VERTICAL_THICKNESS_RATIO = 0.7f; // Vertical rings are thinner

    // Shading
    constexpr float DIM_COLOR_MULTIPLIER = 0.7f;

    // 2D Circle Fallback (Holographic Disc)
    constexpr float CIRCLE_RADIUS_BASE = 10.0f;
    constexpr float CIRCLE_RADIUS_MIN = 2.0f;
    constexpr float CIRCLE_RADIUS_MAX = 15.0f;
    constexpr float GLOW_ALPHA_RATIO = 0.3f;
    constexpr float CORE_ALPHA_RATIO = 0.7f;

    // Camera-facing perception
    constexpr float DEPTH_BRIGHTNESS_MIN = 0.5f;  // Minimum brightness for back-facing segments
    constexpr float DEPTH_BRIGHTNESS_MAX = 1.0f;  // Maximum brightness for front-facing segments
    constexpr float DEPTH_THICKNESS_MIN = 0.8f;   // Thickness multiplier for back-facing segments
    constexpr float DEPTH_THICKNESS_MAX = 1.3f;   // Thickness multiplier for front-facing segments
    constexpr bool ENABLE_PER_SEGMENT_DEPTH = true; // Toggle for facing effects
}

} // namespace kx
