#include "AppState.h"
#include "SettingsManager.h"
#include "../Rendering/Core/MasterRenderer.h"

namespace kx {

    AppState::AppState() : m_masterRenderer(std::make_unique<MasterRenderer>()) {
        // Constructor initializes default values (already done in member initializer list)
        SettingsManager::Load(m_settings); // Load settings from file
    }

    AppState& AppState::Get() {
        static AppState instance;
        return instance;
    }

} // namespace kx