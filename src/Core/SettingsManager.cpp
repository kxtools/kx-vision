#include "SettingsManager.h"
#include "Settings.h"
#include "../Utils/DebugLogger.h"
#include <fstream>
#include <cstdlib>

namespace kx {

    std::filesystem::path SettingsManager::GetConfigFilePath() {
        char* appdata = nullptr;
        size_t len;
        _dupenv_s(&appdata, &len, "APPDATA");
        if (appdata == nullptr) {
            LOG_ERROR("Could not get APPDATA environment variable.");
            return "";
        }
        std::filesystem::path path = appdata;
        free(appdata);
        path /= "kx-vision";
        return path / "settings.json";
    }

    void SettingsManager::Save(const Settings& settings) {
        auto path = GetConfigFilePath();
        if (path.empty()) return;

        try {
            std::filesystem::create_directories(path.parent_path());
            std::ofstream file(path);
            nlohmann::json j = settings;
            file << j.dump(4);
            LOG_INFO("Settings saved to {}", path.string());
        }
        catch (const std::exception& e) {
            LOG_ERROR("Failed to save settings: {}", e.what());
        }
    }

    void SettingsManager::Load(Settings& settings) {
        auto path = GetConfigFilePath();
        if (path.empty() || !std::filesystem::exists(path)) {
            LOG_INFO("Settings file not found, using defaults.");
            return;
        }

        try {
            std::ifstream file(path);
            nlohmann::json j;
            file >> j;

            int fileVersion = j.value("settingsVersion", 0);
            if (fileVersion != CURRENT_SETTINGS_VERSION) {
                LOG_WARN("Settings file version mismatch (file: {}, current: {}). Using default settings.", fileVersion, CURRENT_SETTINGS_VERSION);
                return;
            }

            j.get_to(settings);
            LOG_INFO("Settings loaded from {}", path.string());
        }
        catch (const std::exception& e) {
            LOG_ERROR("Failed to load settings: {}. Using default settings.", e.what());
            settings = Settings(); // Reset to default
        }
    }

}
