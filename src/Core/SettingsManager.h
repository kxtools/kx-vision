#pragma once
#include <filesystem>
#include "Settings.h"

namespace kx { 

    class FeatureManager;

    class SettingsManager {
    public:
        // Saves the provided settings object to the config file.
        static void Save(const Settings& settings, FeatureManager& featureManager);

        // Loads settings from the config file into the provided object.
        static void Load(Settings& settings, FeatureManager& featureManager);

        static void LoadPendingFeatureSettings(FeatureManager& featureManager);
        static void ResetAll(Settings& settings, FeatureManager& featureManager);

    private:
        // Gets the full path to the settings.json file.
        static std::filesystem::path GetConfigFilePath();
    };
}
