#pragma once

namespace kx {
    struct Settings;
    struct ObjectEspSettings;

    namespace GUI {
        void RenderObjectsTab();
        void RenderObjectTypeFilters(ObjectEspSettings& settings);
        void RenderSpecialFilters(Settings& settings);
        void RenderAttackTargetListSettings(ObjectEspSettings& settings);
        void RenderItemListSettings(ObjectEspSettings& settings);
        void RenderDetailedInformationSettings(ObjectEspSettings& settings);
    }
}
