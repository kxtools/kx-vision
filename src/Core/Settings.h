#pragma once

#include "Config.h"
#include "../../libs/nlohmann/json.hpp"
#include "Settings/SettingsConstants.h"
#include "../Features/Visuals/Settings/VisualsSettings.h"
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
        AppearanceSettings appearance;
        
        // Performance settings
        float espUpdateRate = 60.0f;            // ESP updates per second (30-360 FPS range, 60 = smooth, lower = better performance)
        
        // Enhanced filtering options
        bool hideDepletedNodes = true;          // Hide depleted resource nodes (visual clutter reduction)
        
        // New setting for this feature
        bool autoSaveOnExit = true;

        // Debug options
        bool enableDebugLogging = true;
        int logLevel = AppConfig::DEFAULT_LOG_LEVEL;  // Log level (0=DEBUG, 1=INFO, 2=WARNING, 3=ERROR, 4=CRITICAL)

#ifdef _DEBUG
        bool showDebugAddresses = true;         // Show entity memory addresses on ESP
#else
        bool showDebugAddresses = false;
#endif

        // GUI appearance settings
        struct GuiSettings {
            float uiScale = 1.0f;               // Menu UI scale (0.8 - 1.5)
            float menuOpacity = 0.90f;          // Menu window opacity (0.5 - 1.0), 90% matches current style
        } gui;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Settings::GuiSettings, uiScale, menuOpacity);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Settings, settingsVersion, playerESP, npcESP, objectESP, distance, scaling,
                                       sizes, appearance, espUpdateRate, hideDepletedNodes, autoSaveOnExit, enableDebugLogging,
                                       logLevel, gui);

} // namespace kx