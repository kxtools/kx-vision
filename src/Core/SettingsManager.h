#pragma once
#include <filesystem>

namespace kx {
    // Forward-declare the Settings struct to avoid including the full header
    struct Settings; 

    class SettingsManager {
    public:
        // Saves the provided settings object to the config file.
        static void Save(const Settings& settings);

        // Loads settings from the config file into the provided object.
        static void Load(Settings& settings);

    private:
        // Gets the full path to the settings.json file.
        static std::filesystem::path GetConfigFilePath();
    };
}
