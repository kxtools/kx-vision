#pragma once
#include <glm/vec2.hpp>

struct ImDrawList;

namespace kx {
    struct Settings;

    class ESPEnergyBarRenderer {
    public:
        static void Render(
            const Settings& settings, 
            ImDrawList* drawList,
            const glm::vec2& barTopLeftPosition,
            float energyPercent,
            float fadeAlpha,
            float barWidth,
            float barHeight
        );
    };
}

