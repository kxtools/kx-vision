#include "AppState.h"
#include "SettingsManager.h"
#include "../Utils/DebugLogger.h"

namespace kx {

    AppState::AppState() {
        // Constructor initializes default values (already done in member initializer list)
        SettingsManager::Load(m_settings); // Load settings from file
    }

    AppState& AppState::Get() {
        static AppState instance;
        return instance;
    }

} // namespace kx