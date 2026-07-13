#pragma once
#include <glm/vec2.hpp>

struct ImDrawList;

namespace kx {
    struct VisualsConfiguration;

    class EnergyBarRenderer {
    public:
        static void Render(
            const VisualsConfiguration& settings,
            ImDrawList* drawList,
            const glm::vec2& barTopLeftPosition,
            float energyPercent,
            float fadeAlpha,
            float barWidth,
            float barHeight
        );
    };
}

