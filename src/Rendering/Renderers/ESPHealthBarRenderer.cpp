#define NOMINMAX

#include "ESPHealthBarRenderer.h"

#include "../Utils/ESPConstants.h"
#include "../../../libs/ImGui/imgui.h"
#include <Windows.h>

#include "../Data/EntityRenderContext.h"
#include <algorithm>
#include "../Utils/AnimationHelpers.h"

namespace kx {

    // -----------------------------------------------------------------------------
    // Small Utilities
    // -----------------------------------------------------------------------------
    ImU32 ESPHealthBarRenderer::ApplyAlphaToColor(ImU32 color, float alphaMul) {
        alphaMul = (alphaMul < 0.f ? 0.f : (alphaMul > 1.f ? 1.f : alphaMul));
        unsigned int a = (color >> 24) & 0xFF;
        unsigned int finalA = ClampAlpha(static_cast<unsigned int>(a * alphaMul + 0.5f));
        return (color & 0x00FFFFFF) | (finalA << 24);
    }

    void ESPHealthBarRenderer::DrawFilledRect(ImDrawList* dl,
        const ImVec2& min,
        const ImVec2& max,
        ImU32 color,
        float rounding) {
        if (min.x < max.x && min.y < max.y) {
            dl->AddRectFilled(min, max, color, rounding);
        }
    }

    void ESPHealthBarRenderer::DrawHealthBase(ImDrawList* dl,
        const ImVec2& barMin,
        const ImVec2& barMax,
        float barWidth,
        float healthPercent,
        unsigned int entityColor,
        float fadeAlpha) {
        float hpWidth = barWidth * Clamp01(healthPercent);
        ImVec2 hMin = barMin;
        ImVec2 hMax(barMin.x + hpWidth, barMax.y);

        unsigned int healthAlpha =
            static_cast<unsigned int>(RenderingLayout::STANDALONE_HEALTH_BAR_HEALTH_ALPHA * fadeAlpha + 0.5f);
        unsigned int baseColorNoA = (entityColor & 0x00FFFFFF);
        ImU32 baseHealthColor = (baseColorNoA) | (ClampAlpha(healthAlpha) << 24);

        DrawFilledRect(dl, hMin, hMax, baseHealthColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
    }

    void ESPHealthBarRenderer::DrawHealOverlay(ImDrawList* dl, const EntityRenderContext& context, const ImVec2& barMin, float barWidth, float barHeight,
	    float fadeAlpha)
    {
        const auto& anim = context.healthBarAnim;
        if (anim.healOverlayAlpha <= 0.0f) return;

        float startPercent = anim.healOverlayStartPercent;
        float currentPercent = anim.healOverlayEndPercent;
        if (currentPercent <= startPercent) return;

        ImVec2 oMin(barMin.x + barWidth * startPercent, barMin.y);
        ImVec2 oMax(barMin.x + barWidth * currentPercent, barMin.y + barHeight);

        ImU32 color = ApplyAlphaToColor(ESPBarColors::HEAL_OVERLAY, anim.healOverlayAlpha * fadeAlpha);
        DrawFilledRect(dl, oMin, oMax, color, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
    }

    void ESPHealthBarRenderer::DrawHealFlash(ImDrawList* dl,
        const EntityRenderContext& context,
        const ImVec2& barMin,
        float barWidth,
        float barHeight,
        float fadeAlpha) {
        const auto& anim = context.healthBarAnim;
        if (anim.healFlashAlpha <= 0.0f) return;

        float startPercent = anim.healOverlayStartPercent; // Reuse from heal overlay
        float currentPercent = anim.healOverlayEndPercent;
        if (currentPercent <= startPercent) return;

        ImVec2 fMin(barMin.x + barWidth * startPercent, barMin.y);
        ImVec2 fMax(barMin.x + barWidth * currentPercent, barMin.y + barHeight);

        ImU32 flashColor = IM_COL32( // keep runtime alpha because it varies per frame
            255, 255, 255, static_cast<int>(anim.healFlashAlpha * 255 * fadeAlpha));
        flashColor = (ESPBarColors::HEAL_FLASH & 0x00FFFFFF) | (flashColor & 0xFF000000);
        DrawFilledRect(dl, fMin, fMax, flashColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
    }

    void ESPHealthBarRenderer::DrawAccumulatedDamage(ImDrawList* dl,
        const EntityRenderContext& context,
        const ImVec2& barMin,
        float barWidth,
        float barHeight,
        float fadeAlpha) {
        const auto& anim = context.healthBarAnim;
        if (anim.damageAccumulatorPercent <= 0.0f) return;

        float startPercent = context.healthPercent;
        float endPercent = anim.damageAccumulatorPercent;
        if (endPercent > 1.f) endPercent = 1.f;
        if (endPercent <= startPercent) return;

        ImVec2 oMin(barMin.x + barWidth * startPercent, barMin.y);
        ImVec2 oMax(barMin.x + barWidth * endPercent, barMin.y + barHeight);

        // Alpha is baked into the damageAccumulatorPercent by the factory
        ImU32 base = ESPBarColors::DAMAGE_ACCUM;
        unsigned int a = (base >> 24) & 0xFF;
        unsigned int finalA = static_cast<unsigned int>(a * fadeAlpha + 0.5f);
        base = (base & 0x00FFFFFF) | (ClampAlpha(finalA) << 24);
        DrawFilledRect(dl, oMin, oMax, base, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
    }

    void ESPHealthBarRenderer::DrawDamageFlash(ImDrawList* dl,
        const EntityRenderContext& context,
        const ImVec2& barMin,
        float barWidth,
        float barHeight,
        float fadeAlpha) {
        const auto& anim = context.healthBarAnim;
        if (anim.damageFlashAlpha <= 0.0f) return;

        float currentPercent = context.healthPercent;
        float previousPercent = anim.damageFlashStartPercent;
        if (previousPercent > 1.f) previousPercent = 1.f;
        if (previousPercent <= currentPercent) return;

        ImVec2 fMin(barMin.x + barWidth * currentPercent, barMin.y);
        ImVec2 fMax(barMin.x + barWidth * previousPercent, barMin.y + barHeight);

        ImU32 flashColor = ESPBarColors::DAMAGE_FLASH;
        unsigned int a = static_cast<unsigned int>(255 * anim.damageFlashAlpha * fadeAlpha);
        flashColor = (flashColor & 0x00FFFFFF) | (ClampAlpha(a) << 24);
        DrawFilledRect(dl, fMin, fMax, flashColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
    }

    void ESPHealthBarRenderer::DrawBarrierOverlay(ImDrawList* dl,
        const EntityRenderContext& context,
        const ImVec2& barMin,
        const ImVec2& barMax,
        float barWidth,
        float barHeight,
        float fadeAlpha)
    {
        const RenderableEntity* entity = context.entity;
        if (!entity || entity->maxHealth <= 0) return;

        const float animatedBarrier = context.healthBarAnim.animatedBarrier;
        if (animatedBarrier <= 0.0f) return;

        const float healthPercent = entity->currentHealth / entity->maxHealth;
        const float barrierPercent = animatedBarrier / entity->maxHealth;

        const ImU32 barrierColor = ApplyAlphaToColor(ESPBarColors::BARRIER_FILL, fadeAlpha);
        const ImU32 overflowOutlineColor = ApplyAlphaToColor(ESPBarColors::BARRIER_SEPARATOR, fadeAlpha);

        // 1) Barrier inside the remaining health segment, left to right
        if (healthPercent < 1.0f) {
            const float startP = healthPercent;
            const float endP = (std::min)(1.0f, healthPercent + barrierPercent);
            if (endP > startP) {
                ImVec2 fillP0(barMin.x + barWidth * startP, barMin.y);
                ImVec2 fillP1(barMin.x + barWidth * endP, barMax.y);
                DrawFilledRect(dl, fillP0, fillP1, barrierColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
            }
        }

        // 2) Barrier overflow, anchored to the right edge
        if (healthPercent + barrierPercent > 1.0f) {
            const float overflowAmount = (healthPercent + barrierPercent) - 1.0f;
            if (overflowAmount > 0.0f) {
                const float ow = barWidth * (std::min)(1.0f, overflowAmount);

                ImVec2 ovrP0(barMax.x - ow, barMin.y);
                ImVec2 ovrP1(barMax.x, barMax.y);

                DrawFilledRect(dl, ovrP0, ovrP1, barrierColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);

                // Outline only, no extra line to avoid a thicker seam
                dl->AddRect(
                    ovrP0,
                    ovrP1,
                    overflowOutlineColor,
                    RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING,
                    0,
                    RenderingLayout::STANDALONE_HEALTH_BAR_BORDER_THICKNESS
                );
            }
        }
    }

    // -----------------------------------------------------------------------------
    // Public API
    // -----------------------------------------------------------------------------
    void ESPHealthBarRenderer::RenderStandaloneHealthBar(ImDrawList* drawList,
        const glm::vec2& centerPos,
        const EntityRenderContext& context,
        unsigned int entityColor,
        float barWidth,
        float barHeight) { // Removed const
        if (context.healthPercent < -1.0f) return; // allow exactly 0 for dead

        const auto& anim = context.healthBarAnim;
        float fadeAlpha = ((entityColor >> 24) & 0xFF) / 255.0f;
        fadeAlpha *= anim.healthBarFadeAlpha;

        if (fadeAlpha <= 0.f) return;

        // Geometry
        const float yOffset = RenderingLayout::STANDALONE_HEALTH_BAR_Y_OFFSET;
        ImVec2 barMin(centerPos.x - barWidth * 0.5f, centerPos.y + yOffset);
        ImVec2 barMax(centerPos.x + barWidth * 0.5f, barMin.y + barHeight);

        // Background
        unsigned int bgAlpha =
            static_cast<unsigned int>(RenderingLayout::STANDALONE_HEALTH_BAR_BG_ALPHA * fadeAlpha + 0.5f);
        drawList->AddRectFilled(barMin,
            barMax,
            IM_COL32(0, 0, 0, ClampAlpha(bgAlpha)),
            RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);

        // Alive vs Dead specialized rendering
        if (context.entity->currentHealth > 0) {
            RenderAliveState(drawList, context, barMin, barMax, barWidth, entityColor, fadeAlpha);
        }
        else {
            RenderDeadState(drawList, context, barMin, barMax, barWidth, fadeAlpha);
        }

		// Outer stroke settings
        const float outset = 1.0f; // 1 px outside, feels "harder" and more separated
        unsigned int outerA = static_cast<unsigned int>(RenderingLayout::STANDALONE_HEALTH_BAR_BORDER_ALPHA * fadeAlpha + 0.5f);
        ImU32 outerDark = IM_COL32(0, 0, 0, ClampAlpha(outerA));

        // Hostile
        if (context.attitude == Game::Attitude::Hostile) {
            // existing inside stroke
            unsigned int borderAlpha =
                static_cast<unsigned int>(RenderingLayout::STANDALONE_HEALTH_BAR_BORDER_ALPHA * fadeAlpha + 0.5f);
            drawList->AddRect(barMin, barMax,
                IM_COL32(0, 0, 0, ClampAlpha(borderAlpha)),
                RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING,
                0,
                RenderingLayout::STANDALONE_HEALTH_BAR_BORDER_THICKNESS); // inside stroke
            // add a subtle outer stroke to harden the edge
            ImVec2 oMin(barMin.x - outset, barMin.y - outset);
            ImVec2 oMax(barMax.x + outset, barMax.y + outset);
            drawList->AddRect(oMin, oMax, outerDark,
                RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING + outset,
                0,
                1.0f);
        }
        // Others, skip inside stroke, use only the outer stroke
        else {
            ImVec2 oMin(barMin.x - outset, barMin.y - outset);
            ImVec2 oMax(barMax.x + outset, barMax.y + outset);
            drawList->AddRect(oMin, oMax, outerDark,
                RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING + outset,
                0,
                1.0f);
        }
    }

    void ESPHealthBarRenderer::RenderAliveState(ImDrawList* drawList,
        const EntityRenderContext& context,
        const ImVec2& barMin,
        const ImVec2& barMax,
        float barWidth,
        unsigned int entityColor,
        float fadeAlpha) {
        const RenderableEntity* entity = context.entity;
        if (!entity || entity->maxHealth <= 0) return;

        const auto& anim = context.healthBarAnim;
        float barHeight = barMax.y - barMin.y;

        // 1. Base health fill
        DrawHealthBase(drawList, barMin, barMax, barWidth, context.healthPercent, entityColor, fadeAlpha);

        // 2. Healing overlays
        DrawHealOverlay(drawList, context, barMin, barWidth, barHeight, fadeAlpha);
        DrawHealFlash(drawList, context, barMin, barWidth, barHeight, fadeAlpha);

        // 3. Accumulated damage
        DrawAccumulatedDamage(drawList, context, barMin, barWidth, barHeight, fadeAlpha);

        // 4. Damage flash
        DrawDamageFlash(drawList, context, barMin, barWidth, barHeight, fadeAlpha);

        // 5. Barrier overlay (drawn last, on top of everything)
        DrawBarrierOverlay(drawList, context, barMin, barMax, barWidth, barHeight, fadeAlpha);
    }

    void ESPHealthBarRenderer::RenderDeadState(ImDrawList* drawList,
        const EntityRenderContext& context,
        const ImVec2& barMin,
        const ImVec2& barMax,
        float barWidth,
        float fadeAlpha) {
        const auto& anim = context.healthBarAnim;
        if (anim.deathBurstAlpha <= 0.0f) return;

        // Invert the animation: start wide and shrink to center for an "impact" feel.
        float width = barWidth * anim.deathBurstWidth;
        ImVec2 center(barMin.x + barWidth * 0.5f, (barMin.y + barMax.y) * 0.5f);
        ImVec2 burstMin(center.x - width * 0.5f, barMin.y);
        ImVec2 burstMax(center.x + width * 0.5f, barMax.y);

        ImU32 burstColor = ESPBarColors::DEATH_BURST;
        unsigned int a = static_cast<unsigned int>(255 * anim.deathBurstAlpha * fadeAlpha);
        burstColor = (burstColor & 0x00FFFFFF) | (ClampAlpha(a) << 24);
        DrawFilledRect(drawList, burstMin, burstMax, burstColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
    }

    void ESPHealthBarRenderer::RenderStandaloneEnergyBar(ImDrawList* drawList,
        const glm::vec2& centerPos,
        float energyPercent,
        float fadeAlpha,
        float barWidth,
        float barHeight,
        float healthBarHeight) {
        if (energyPercent < 0.0f || energyPercent > 1.0f) return;

        const float yOffset =
            RenderingLayout::STANDALONE_HEALTH_BAR_Y_OFFSET + healthBarHeight + 2.0f; // 2px gap below health bar
        ImVec2 barMin(centerPos.x - barWidth * 0.5f, centerPos.y + yOffset);
        ImVec2 barMax(centerPos.x + barWidth * 0.5f, barMin.y + barHeight);

        // Background
        unsigned int bgAlpha =
            ClampAlpha(static_cast<unsigned int>(RenderingLayout::STANDALONE_HEALTH_BAR_BG_ALPHA * fadeAlpha + 0.5f));
        drawList->AddRectFilled(barMin,
            barMax,
            IM_COL32(0, 0, 0, bgAlpha),
            RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);

        // Energy fill
        float fillWidth = barWidth * energyPercent;
        ImVec2 eMin(barMin.x, barMin.y);
        ImVec2 eMax(barMin.x + fillWidth, barMax.y);

        ImU32 energyColor = ESPColors::ENERGY_BAR;
        float colorA = ((energyColor >> 24) & 0xFF) / 255.0f;
        ImU32 finalColor = ApplyAlphaToColor(energyColor, colorA * fadeAlpha);

        DrawFilledRect(drawList, eMin, eMax, finalColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
    }

} // namespace kx