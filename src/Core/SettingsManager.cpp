#include "SettingsManager.h"
#include "Settings.h"
#include "AppLifecycleManager.h"
#include "Architecture/FeatureManager.h"
#include "Architecture/IFeature.h"
#include "../Utils/DebugLogger.h"
#include <fstream>

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
            
            // Start with core settings
            nlohmann::json j = settings;
            
            // Save feature-specific settings
            auto& featureManager = g_App.GetFeatureManager();
            for (const auto& feature : featureManager.GetFeatures()) {
                try {
                    feature->SaveSettings(j);
                } catch (const std::exception& e) {
                    LOG_ERROR("Failed to save settings for feature '%s': %s", feature->GetName(), e.what());
                    // Continue saving other features
                }
            }
            
            std::ofstream file(path);
            file << j.dump(4);
            LOG_INFO("Settings saved to %s", path.u8string().c_str());
        }
        catch (const std::exception& e) {
            LOG_ERROR("Failed to save settings: %s", e.what());
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
                LOG_WARN("Settings file version mismatch (file: %d, current: %d). Using default settings.", fileVersion, CURRENT_SETTINGS_VERSION);
                return;
            }

            // Load core settings
            j.get_to(settings);
            
            LOG_INFO("Settings loaded from %s", path.u8string().c_str());
        }
        catch (const std::exception& e) {
            LOG_ERROR("Failed to load settings: %s. Using default settings.", e.what());
            settings = Settings(); // Reset to default
        }
    }

    void SettingsManager::LoadFeatureSettings() {
        auto path = GetConfigFilePath();
        if (path.empty() || !std::filesystem::exists(path)) {
            LOG_INFO("Settings file not found for features, using defaults.");
            return;
        }

        try {
            std::ifstream file(path);
            nlohmann::json j;
            file >> j;

            int fileVersion = j.value("settingsVersion", 0);
            if (fileVersion != CURRENT_SETTINGS_VERSION) {
                LOG_WARN("Settings file version mismatch, skipping feature settings load.");
                return;
            }

            // Load feature-specific settings
            auto& featureManager = g_App.GetFeatureManager();
            for (const auto& feature : featureManager.GetFeatures()) {
                try {
                    feature->LoadSettings(j);
                } catch (const std::exception& e) {
                    LOG_ERROR("Failed to load settings for feature '%s': %s. Using defaults.", feature->GetName(), e.what());
                    // Continue loading other features
                }
            }
            
            LOG_INFO("Feature settings loaded successfully");
        }
        catch (const std::exception& e) {
            LOG_ERROR("Failed to load feature settings: %s. Using defaults.", e.what());
        }
    }

}
