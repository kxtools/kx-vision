#include "AppState.h"

namespace kx {

    AppState::AppState() {
        // SettingsManager overlays persisted data after features are registered.
    }

    AppState& AppState::Get() {
        static AppState instance;
        return instance;
    }

} // namespace kx
