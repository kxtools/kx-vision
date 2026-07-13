#pragma once

namespace kx {
    class FeatureManager;
    struct Settings;

    namespace GUI {
        void RenderSettingsTab(Settings& settings, FeatureManager& featureManager);
    }
}
