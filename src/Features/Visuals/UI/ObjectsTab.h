#pragma once

namespace kx {
    struct ObjectEspSettings;
    struct VisualsConfiguration;

    namespace GUI {
        void RenderObjectsTab(VisualsConfiguration& config);
        void RenderObjectTypeFilters(ObjectEspSettings& settings);
        void RenderSpecialFilters(VisualsConfiguration& config);
        void RenderAttackTargetListSettings(ObjectEspSettings& settings);
        void RenderItemListSettings(ObjectEspSettings& settings);
        void RenderDetailedInformationSettings(ObjectEspSettings& settings);
    }
}
