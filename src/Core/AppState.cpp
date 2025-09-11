#include "AppState.h"

namespace kx {

    AppState::AppState() {
        // Constructor initializes default values (already done in member initializer list)
    }

    AppState& AppState::Get() {
        static AppState instance;
        return instance;
    }

} // namespace kx