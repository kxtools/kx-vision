#include "SettingsManager.h"
#include "AppLifecycleManager.h"
#include "Architecture/FeatureManager.h"
#include "Architecture/IFeature.h"
#include "Settings/SettingsConstants.h"
#include "../Utils/DebugLogger.h"
#include <fstream>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <windows.h>

namespace kx {
namespace {

nlohmann::json g_loadedDocument = nlohmann::json::object();

void NormalizeCore(Settings& s) {
    const auto finiteOr = [](float value, float fallback) { return (std::isfinite)(value) ? value : fallback; };
    s.gui.uiScale = (std::clamp)(finiteOr(s.gui.uiScale, 1.0f), 0.8f, 1.5f);
    s.gui.menuOpacity = (std::clamp)(finiteOr(s.gui.menuOpacity, 0.9f), 0.5f, 1.0f);
    s.logLevel = (std::clamp)(s.logLevel, 0, 4);
}

} // namespace

std::filesystem::path SettingsManager::GetConfigFilePath() {
    char* appdata = nullptr;
    size_t len = 0;
    _dupenv_s(&appdata, &len, "APPDATA");
    if (!appdata) return {};
    std::filesystem::path path = appdata;
    free(appdata);
    return path / "kx-vision" / "settings.json";
}

void SettingsManager::Load(Settings& settings) {
    settings = Settings{};
    g_loadedDocument = nlohmann::json::object();
    const auto path = GetConfigFilePath();

    try {
        if (!path.empty() && std::filesystem::exists(path)) {
            std::ifstream file(path);
            nlohmann::json document;
            file >> document;
            if (!document.is_object() || document.value(SettingsSections::Version, 0) != CURRENT_SETTINGS_VERSION) {
                throw std::runtime_error("unsupported settings schema");
            }
            g_loadedDocument = std::move(document);
            const auto core = g_loadedDocument.find(SettingsSections::Core);
            if (core != g_loadedDocument.end()) core->get_to(settings);
        }
    } catch (const std::exception& e) {
        LOG_WARN("Settings load failed: %s. Using defaults.", e.what());
        settings = Settings{};
        g_loadedDocument = nlohmann::json::object();
    }

    NormalizeCore(settings);
    g_App.GetFeatureManager().LoadAllSettings(g_loadedDocument);
}

void SettingsManager::LoadPendingFeatureSettings() {
    g_App.GetFeatureManager().LoadPendingSettings(g_loadedDocument);
}

void SettingsManager::ResetAll(Settings& settings) {
    settings = Settings{};
    NormalizeCore(settings);
    g_App.GetFeatureManager().ResetAllSettings();
}

void SettingsManager::Save(const Settings& source) {
    const auto path = GetConfigFilePath();
    if (path.empty()) return;

    try {
        Settings settings = source;
        NormalizeCore(settings);
        nlohmann::json document = g_loadedDocument.is_object() ? g_loadedDocument : nlohmann::json::object();
        document[SettingsSections::Version] = CURRENT_SETTINGS_VERSION;
        document[SettingsSections::Core] = settings;
        g_App.GetFeatureManager().ForEachFeature([&](IFeature& feature) {
            try {
                feature.SaveSettings(document);
            } catch (const std::exception& e) {
                LOG_ERROR("Failed to save settings for feature '%s': %s", feature.GetName(), e.what());
            }
        });

        std::filesystem::create_directories(path.parent_path());
        const auto temporary = path.wstring() + L".tmp";
        {
            std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
            if (!file) throw std::runtime_error("could not open temporary settings file");
            file << document.dump(4);
            file.flush();
            if (!file) throw std::runtime_error("could not write temporary settings file");
        }

        // Guardrail: never damage last valid config. Replacement happens only after
        // complete temporary write has closed successfully.
        if (!MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
            std::filesystem::remove(temporary);
            throw std::runtime_error("could not replace settings file");
        }
        g_loadedDocument = std::move(document);
        LOG_INFO("Settings saved to %s", path.u8string().c_str());
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to save settings: %s", e.what());
    }
}

} // namespace kx
