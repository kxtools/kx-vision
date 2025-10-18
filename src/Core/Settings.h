#pragma once

#include "../../libs/nlohmann/json.hpp"
#include "Settings/SettingsConstants.h"
#include "Settings/ESPSettings.h"
#include "Settings/RenderSettings.h"

namespace kx {

    struct Settings {
        int settingsVersion = CURRENT_SETTINGS_VERSION; // This ensures new objects have the current version

        // Category-specific ESP settings
        PlayerEspSettings playerESP;
        NpcEspSettings npcESP;
        ObjectEspSettings objectESP;

        // Organized configuration groups
        DistanceSettings distance;
        ScalingSettings scaling;
        ElementSizeSettings sizes;
        
        // Performance settings
        float espUpdateRate = 60.0f;            // ESP updates per second (60 = smooth, lower = better performance)
        
        // Enhanced filtering options
        bool hideDepletedNodes = true;          // Hide depleted resource nodes (visual clutter reduction)
        
        // New setting for this feature
        bool autoSaveOnExit = true;

        // Debug options
        bool enableDebugLogging = true;

#ifdef _DEBUG
        bool showDebugAddresses = true;         // Show entity memory addresses on ESP
#else
        bool showDebugAddresses = false;
#endif
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Settings, settingsVersion, playerESP, npcESP, objectESP, distance, scaling,
                                       sizes, espUpdateRate, hideDepletedNodes, autoSaveOnExit, enableDebugLogging,
                                       showDebugAddresses);

} // namespace kx