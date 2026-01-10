#pragma once
#include <filesystem>
#include "Settings.h"

namespace kx { 

    class SettingsManager {
    public:
        // Saves the provided settings object to the config file.
        static void Save(const Settings& settings);

        // Loads settings from the config file into the provided object.
        static void Load(Settings& settings);

        // Loads feature-specific settings. Call this after features are registered.
        static void LoadFeatureSettings();

    private:
        // Gets the full path to the settings.json file.
        static std::filesystem::path GetConfigFilePath();
    };
}
