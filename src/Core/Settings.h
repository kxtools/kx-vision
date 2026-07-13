#pragma once

#include "Config.h"
#include "../../libs/nlohmann/json.hpp"
#include "Settings/SettingsConstants.h"
#include "Settings/SettingsFields.h"

namespace kx {

#define KX_GUI_SETTINGS(X) \
    X(float, uiScale, 1.0f) \
    X(float, menuOpacity, 0.90f)
    KX_DEFINE_SETTINGS_TYPE(GuiSettings, KX_GUI_SETTINGS)

#define KX_CORE_SETTINGS(X) \
    X(bool, autoSaveOnExit, true) \
    X(bool, enableDebugLogging, true) \
    X(int, logLevel, AppConfig::DEFAULT_LOG_LEVEL) \
    X(GuiSettings, gui, {})
    KX_DEFINE_SETTINGS_TYPE(Settings, KX_CORE_SETTINGS)

} // namespace kx
