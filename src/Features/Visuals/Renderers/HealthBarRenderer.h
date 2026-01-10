#pragma once

#include "glm.hpp"
#include "../../../../libs/ImGui/imgui.h"
#include "../../../Rendering/Data/FrameData.h"
#include "../../../Rendering/Data/HealthBarAnimationState.h"
#include "../../../Rendering/Data/EntityTypes.h"
#include "../../../Game/GameEnums.h"

namespace kx {

    // Forward declarations
    class CombatStateManager;
    struct RenderableEntity;
    struct EntityCombatState;
    struct Settings;

    /**
     * @brief Utility functions for rendering health & energy bars with combat effect overlays.
     */
    class HealthBarRenderer {
    public:
        static void RenderStandaloneHealthBar(ImDrawList* drawList,
            const glm::vec2& barTopLeftPosition,
            const RenderableEntity& entity,
            EntityTypes entityType,
            Game::Attitude attitude,
            const VisualProperties& props,
            const HealthBarAnimationState& animState,
            const Settings& settings);

    private:
        // --- Internal Specializations ---
        static void RenderAliveState(ImDrawList* drawList,
            const RenderableEntity& entity,
            EntityTypes entityType,
            const ImVec2& barMin,
            const ImVec2& barMax,
            const VisualProperties& props,
            float fadeAlpha,
            const HealthBarAnimationState& animState,
            const Settings& settings);

        // Add new helper for drawing text
        static void DrawHealthPercentageText(ImDrawList* dl, const ImVec2& barMin, const ImVec2& barMax, float healthPercent, float fontSize, float fadeAlpha);

        static void RenderDeadState(ImDrawList* drawList,
            const HealthBarAnimationState& animState,
            const ImVec2& barMin,
            const ImVec2& barMax,
            float barWidth,
            float fadeAlpha);

        // --- Small Utilities ---
        static inline unsigned int ClampAlpha(unsigned int alpha) { return (alpha < 255u ? alpha : 255u); }
        static inline float Clamp01(float v) { return v < 0.f ? 0.f : (v > 1.f ? 1.f : v); }
        static inline ImU32 ApplyAlphaToColor(ImU32 color, float alphaMul);


        static void DrawFilledRect(ImDrawList* dl,
            const ImVec2& min,
            const ImVec2& max,
            ImU32 color,
            float rounding);

        static void DrawHealthBase(ImDrawList* dl,
            const ImVec2& barMin,
            const ImVec2& barMax,
            float barWidth,
            float healthPercent,
            unsigned int entityColor,
            float fadeAlpha,
            const Settings& settings);

        static void DrawHealOverlay(ImDrawList* dl, const HealthBarAnimationState& animState, const ImVec2& barMin, float barWidth, float barHeight, float fadeAlpha, const Settings& settings);

        static void DrawHealFlash(ImDrawList* dl,
            const HealthBarAnimationState& animState,
            const ImVec2& barMin,
            float barWidth,
            float barHeight,
            float fadeAlpha,
            const Settings& settings);

        static void DrawAccumulatedDamage(ImDrawList* dl,
			const RenderableEntity& entity,
            const HealthBarAnimationState& animState,
            const ImVec2& barMin,
            float barWidth,
            float barHeight,
            float fadeAlpha,
            const Settings& settings);

        static void DrawDamageFlash(ImDrawList* dl,
            const RenderableEntity& entity,
            const HealthBarAnimationState& animState,
            const ImVec2& barMin,
            float barWidth,
            float barHeight,
            float fadeAlpha,
            const Settings& settings);

        static void DrawBarrierOverlay(ImDrawList* dl,
            const RenderableEntity& entity,
            const HealthBarAnimationState& animState,
            const ImVec2& barMin,
            const ImVec2& barMax,
            float barWidth,
            float barHeight,
            float fadeAlpha,
            const Settings& settings);


    };

} // namespace kx