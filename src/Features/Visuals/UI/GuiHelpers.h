#pragma once
#include "../Settings/VisualsSettings.h"

namespace kx {
    namespace GUI {

        /**
         * @brief Renders a consistent set of style settings for Player ESP.
         * @param settings A reference to the PlayerEspSettings struct.
         */
        void RenderPlayerStyleSettings(PlayerEspSettings& settings);

        /**
         * @brief Renders a consistent set of style settings for NPC ESP.
         * @param settings A reference to the NpcEspSettings struct.
         */
        void RenderNpcStyleSettings(NpcEspSettings& settings);

        /**
         * @brief Renders a consistent set of style settings for Object ESP.
         * @param settings A reference to the ObjectEspSettings struct.
         */
        void RenderObjectStyleSettings(ObjectEspSettings& settings);

    } // namespace GUI
} // namespace kx
