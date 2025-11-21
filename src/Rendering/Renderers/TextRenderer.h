#pragma once

#include <glm.hpp>
#include <string_view>
#include <span>
#include "../../../libs/ImGui/imgui.h"

struct ImDrawList;

namespace kx {

    struct FastTextStyle {
        float fontSize = 13.0f;
        ImU32 color = 0xFFFFFFFF;
        bool shadow = true;
        bool background = true;
        float fadeAlpha = 1.0f;
    };

    class TextRenderer {
    public:
        static float DrawCentered(ImDrawList* dl, const glm::vec2& pos, std::string_view text, const FastTextStyle& style);
        
        static float DrawMultiColored(ImDrawList* dl, const glm::vec2& pos, 
                                      std::span<const std::string_view> texts, 
                                      std::span<const ImU32> colors, 
                                      const FastTextStyle& style);
    };

}
