#pragma once

#include "glm.hpp"
#include "../../../libs/ImGui/imgui.h"
#include "../../Game/GameEnums.h"
#include "Data/RenderableData.h"

namespace kx {

    // Forward declarations
    class CombatStateManager;
    struct EntityRenderContext;
    struct EntityCombatState;

    /**
     * @brief Utility functions for rendering health & energy bars with combat effect overlays.
     */
    class ESPHealthBarRenderer {
    public:
        static void RenderStandaloneHealthBar(ImDrawList* drawList,
            const glm::vec2& centerPos,
            const EntityRenderContext& context,
            unsigned int entityColor,
            float barWidth,
            float barHeight,
            CombatStateManager& stateManager);

        static void RenderStandaloneEnergyBar(ImDrawList* drawList,
            const glm::vec2& centerPos,
            float energyPercent,
            float fadeAlpha,
            float barWidth,
            float barHeight,
            float healthBarHeight);

    private:
        // --- Internal Specializations ---
        static void RenderAliveState(ImDrawList* drawList,
            const EntityRenderContext& context,
            EntityCombatState* state,
            const ImVec2& barMin,
            const ImVec2& barMax,
            float barWidth,
            unsigned int entityColor,
            float fadeAlpha);

        static void RenderDeadState(ImDrawList* drawList,
            const EntityCombatState* state,
            const ImVec2& barMin,
            const ImVec2& barMax,
            float barWidth,
            float fadeAlpha);

        // --- Small Utilities ---
        static inline unsigned int ClampAlpha(unsigned int alpha) { return (alpha < 255u ? alpha : 255u); }
        static inline float Clamp01(float v) { return v < 0.f ? 0.f : (v > 1.f ? 1.f : v); }
        static inline ImU32 ApplyAlphaToColor(ImU32 color, float alphaMul);
        static inline uint64_t NowMs();

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
            float fadeAlpha);

        static void DrawHealOverlay(ImDrawList* dl,
            const EntityCombatState* state,
            const RenderableEntity* entity,
            uint64_t now,
            const ImVec2& barMin,
            float barWidth,
            float barHeight);

        static void DrawHealFlash(ImDrawList* dl,
            const EntityCombatState* state,
            const RenderableEntity* entity,
            uint64_t now,
            const ImVec2& barMin,
            float barWidth,
            float barHeight,
            float fadeAlpha);

        static void DrawAccumulatedDamage(ImDrawList* dl,
            const EntityCombatState* state,
            const RenderableEntity* entity,
            const ImVec2& barMin,
            float barWidth,
            float barHeight,
            float fadeAlpha);

        static void DrawDamageFlash(ImDrawList* dl,
            const EntityCombatState* state,
            const RenderableEntity* entity,
            uint64_t now,
            const ImVec2& barMin,
            float barWidth,
            float barHeight,
            float fadeAlpha);
    };

} // namespace kx