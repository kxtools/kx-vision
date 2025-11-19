#define NOMINMAX

#include "ESPHealthBarRenderer.h"
#include <iomanip>
#include "../Utils/ESPConstants.h"
#include "../../../libs/ImGui/imgui.h"
#include <Windows.h>

#include "../Data/EntityRenderContext.h"
#include <algorithm>
#include "../Utils/EntityVisualsCalculator.h"
#include "../Data/TextElement.h"
#include "TextRenderer.h"
#include "../Utils/LayoutConstants.h"
#include "../../Core/Settings.h"
#include "../Utils/RenderSettingsHelper.h"

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
        float fadeAlpha,
        const Settings& settings) {
        float hpWidth = barWidth * Clamp01(healthPercent);
        ImVec2 hMin = barMin;
        ImVec2 hMax(barMin.x + hpWidth, barMax.y);

        // Apply global opacity to health bar fill
        unsigned int healthAlpha =
	        static_cast<unsigned int>(RenderingLayout::STANDALONE_HEALTH_BAR_HEALTH_ALPHA * fadeAlpha * settings.appearance.globalOpacity + 0.5f);
        unsigned int baseColorNoA = (entityColor & 0x00FFFFFF);
        ImU32 baseHealthColor = (baseColorNoA) | (ClampAlpha(healthAlpha) << 24);

        DrawFilledRect(dl, hMin, hMax, baseHealthColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
    }

    void ESPHealthBarRenderer::DrawHealOverlay(ImDrawList* dl, const EntityRenderContext& context, const ImVec2& barMin, float barWidth, float barHeight,
	    float fadeAlpha, const Settings& settings)
    {
        const auto& anim = context.healthBarAnim;
        if (anim.healOverlayAlpha <= 0.0f) return;

        float startPercent = anim.healOverlayStartPercent;
        float currentPercent = anim.healOverlayEndPercent;
        if (currentPercent <= startPercent) return;

        ImVec2 oMin(barMin.x + barWidth * startPercent, barMin.y);
        ImVec2 oMax(barMin.x + barWidth * currentPercent, barMin.y + barHeight);

        // Apply global opacity to heal overlay
        ImU32 color = ApplyAlphaToColor(ESPBarColors::HEAL_OVERLAY, anim.healOverlayAlpha * fadeAlpha * settings.appearance.globalOpacity);
        DrawFilledRect(dl, oMin, oMax, color, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
    }

    void ESPHealthBarRenderer::DrawHealFlash(ImDrawList* dl,
        const EntityRenderContext& context,
        const ImVec2& barMin,
        float barWidth,
        float barHeight,
        float fadeAlpha,
        const Settings& settings) {
        const auto& anim = context.healthBarAnim;
        if (anim.healFlashAlpha <= 0.0f) return;

        float startPercent = anim.healOverlayStartPercent; // Reuse from heal overlay
        float currentPercent = anim.healOverlayEndPercent;
        if (currentPercent <= startPercent) return;

        ImVec2 fMin(barMin.x + barWidth * startPercent, barMin.y);
        ImVec2 fMax(barMin.x + barWidth * currentPercent, barMin.y + barHeight);

        // Apply global opacity to heal flash
        ImU32 flashColor = IM_COL32( // keep runtime alpha because it varies per frame
            255, 255, 255, static_cast<int>(anim.healFlashAlpha * 255 * fadeAlpha * settings.appearance.globalOpacity));
        flashColor = (ESPBarColors::HEAL_FLASH & 0x00FFFFFF) | (flashColor & 0xFF000000);
        DrawFilledRect(dl, fMin, fMax, flashColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
    }

    void ESPHealthBarRenderer::DrawAccumulatedDamage(ImDrawList* dl,
        const EntityRenderContext& context,
        const ImVec2& barMin,
        float barWidth,
        float barHeight,
        float fadeAlpha,
        const Settings& settings) {
        const auto& anim = context.healthBarAnim;
        // Exit if there's nothing to draw OR if the fade animation is complete
        if (anim.damageAccumulatorPercent <= 0.0f || anim.damageAccumulatorAlpha <= 0.0f) return;

        float startPercent = context.entity->maxHealth > 0 ? (context.entity->currentHealth / context.entity->maxHealth) : 0.0f;
        float endPercent = (anim.damageAccumulatorPercent > 1.0f) ? 1.0f : anim.damageAccumulatorPercent;

        if (endPercent <= startPercent) return;

        ImVec2 oMin(barMin.x + barWidth * startPercent, barMin.y);
        ImVec2 oMax(barMin.x + barWidth * endPercent, barMin.y + barHeight);

        ImU32 base = ESPBarColors::DAMAGE_ACCUM;
        unsigned int a = (base >> 24) & 0xFF;
        // --- THIS IS THE FIX ---
        // Multiply by the overall bar fade AND the specific accumulator fade animation alpha AND global opacity
        unsigned int finalA = static_cast<unsigned int>(a * fadeAlpha * anim.damageAccumulatorAlpha * settings.appearance.globalOpacity + 0.5f);
        base = (base & 0x00FFFFFF) | (ClampAlpha(finalA) << 24);
        DrawFilledRect(dl, oMin, oMax, base, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
    }

    void ESPHealthBarRenderer::DrawDamageFlash(ImDrawList* dl,
        const EntityRenderContext& context,
        const ImVec2& barMin,
        float barWidth,
        float barHeight,
        float fadeAlpha,
        const Settings& settings) {
        const auto& anim = context.healthBarAnim;
        if (anim.damageFlashAlpha <= 0.0f) return;

        float currentPercent = context.entity->maxHealth > 0 ? (context.entity->currentHealth / context.entity->maxHealth) : 0.0f;
        float previousPercent = anim.damageFlashStartPercent;
        if (previousPercent > 1.f) previousPercent = 1.f;
        if (previousPercent <= currentPercent) return;

        ImVec2 fMin(barMin.x + barWidth * currentPercent, barMin.y);
        ImVec2 fMax(barMin.x + barWidth * previousPercent, barMin.y + barHeight);

        ImU32 flashColor = ESPBarColors::DAMAGE_FLASH;
        // Apply global opacity to damage flash
        unsigned int a = static_cast<unsigned int>(255 * anim.damageFlashAlpha * fadeAlpha * settings.appearance.globalOpacity);
        flashColor = (flashColor & 0x00FFFFFF) | (ClampAlpha(a) << 24);
        DrawFilledRect(dl, fMin, fMax, flashColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
    }



    void ESPHealthBarRenderer::DrawBarrierOverlay(ImDrawList* dl,
        const EntityRenderContext& context,
        const ImVec2& barMin,
        const ImVec2& barMax,
        float barWidth,
        float barHeight,
        float fadeAlpha,
        const Settings& settings)
    {
        const RenderableEntity* entity = context.entity;
        if (!entity || entity->maxHealth <= 0) return;

        const float animatedBarrier = context.healthBarAnim.animatedBarrier;
        if (animatedBarrier <= 0.0f) return;

        const float healthPercent = entity->currentHealth / entity->maxHealth;
        const float barrierPercent = animatedBarrier / entity->maxHealth;

        // Apply global opacity to barrier overlay
        const ImU32 barrierColor = ApplyAlphaToColor(ESPBarColors::BARRIER_FILL, fadeAlpha * settings.appearance.globalOpacity);
        const ImU32 overflowOutlineColor = ApplyAlphaToColor(ESPBarColors::BARRIER_SEPARATOR, fadeAlpha * settings.appearance.globalOpacity);

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
        const glm::vec2& barTopLeftPosition,
        const EntityRenderContext& context,
        const VisualProperties& props,
        const Settings& settings) 
    { 
        const auto& anim = context.healthBarAnim;
        
        // Use properties from VisualProperties directly
        float fadeAlpha = ((props.fadedEntityColor >> 24) & 0xFF) / 255.0f;
        fadeAlpha *= anim.healthBarFadeAlpha;

        if (fadeAlpha <= 0.f) return; // Exit if NOTHING is visible

        // Geometry
        ImVec2 barMin(barTopLeftPosition.x, barTopLeftPosition.y);
        ImVec2 barMax(barTopLeftPosition.x + props.finalHealthBarWidth, barTopLeftPosition.y + props.finalHealthBarHeight);

        // Background
        unsigned int bgAlpha =
            static_cast<unsigned int>(RenderingLayout::STANDALONE_HEALTH_BAR_BG_ALPHA * fadeAlpha * settings.appearance.globalOpacity + 0.5f);
        drawList->AddRectFilled(barMin,
            barMax,
            IM_COL32(0, 0, 0, ClampAlpha(bgAlpha)),
            RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);

        // Alive vs Dead specialized rendering
        if (context.entity->currentHealth > 0) {
            RenderAliveState(drawList, context, barMin, barMax, props, fadeAlpha, settings);
        }
        else {
            RenderDeadState(drawList, context, barMin, barMax, props.finalHealthBarWidth, fadeAlpha);
        }

		// Outer stroke settings
        const float outset = 1.0f; // 1 px outside, feels "harder" and more separated
        unsigned int outerA = static_cast<unsigned int>(RenderingLayout::STANDALONE_HEALTH_BAR_BORDER_ALPHA * fadeAlpha * settings.appearance.globalOpacity + 0.5f);
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
        const VisualProperties& props,
        float fadeAlpha,
        const Settings& settings) 
    {
        const RenderableEntity* entity = context.entity;
        if (!entity || entity->maxHealth <= 0) return;

        float barWidth = props.finalHealthBarWidth;
        float barHeight = props.finalHealthBarHeight;

        // 1. Base health fill
        DrawHealthBase(drawList, barMin, barMax, barWidth, 
            context.entity->maxHealth > 0 ? (context.entity->currentHealth / context.entity->maxHealth) : 0.0f, 
            props.fadedEntityColor, fadeAlpha, settings);

        // 2. Healing overlays
        DrawHealOverlay(drawList, context, barMin, barWidth, barHeight, fadeAlpha, settings);
        DrawHealFlash(drawList, context, barMin, barWidth, barHeight, fadeAlpha, settings);

        // 3. Accumulated damage
        DrawAccumulatedDamage(drawList, context, barMin, barWidth, barHeight, fadeAlpha, settings);

        // 4. Damage flash
        DrawDamageFlash(drawList, context, barMin, barWidth, barHeight, fadeAlpha, settings);

        // 5. Barrier overlay (drawn last, on top of everything)
        DrawBarrierOverlay(drawList, context, barMin, barMax, barWidth, barHeight, fadeAlpha, settings);

        // 6. Health Percentage Text (drawn last, on top of everything)
        bool shouldRenderHealthPercentage = RenderSettingsHelper::ShouldRenderHealthPercentage(settings, context.entityType);
        if (shouldRenderHealthPercentage && context.entity->maxHealth > 0) {
            float healthPercent = context.entity->currentHealth / context.entity->maxHealth;
            DrawHealthPercentageText(drawList, barMin, barMax, healthPercent, props.finalFontSize, fadeAlpha);
        }
    }

    void ESPHealthBarRenderer::DrawHealthPercentageText(ImDrawList* dl, const ImVec2& barMin, const ImVec2& barMax, float healthPercent, float fontSize, float fadeAlpha)
    {
        // 1. Format the percentage string
        int percent = static_cast<int>(healthPercent * 100.0f);
        std::string text = std::to_string(percent) + "%";

        // 2. Calculate the text height for proper vertical centering
        float finalFontSize = fontSize * RenderingLayout::STATUS_TEXT_FONT_SIZE_MULTIPLIER;
        ImFont* font = ImGui::GetFont();
        ImVec2 textSize = font->CalcTextSizeA(finalFontSize, FLT_MAX, 0.0f, text.c_str());

        // 3. Define the anchor point: to the right of the bar, vertically centered
        const float padding = 5.0f; // 5px gap between the bar and the text
        float barCenterY = barMin.y + (barMax.y - barMin.y) * 0.5f;
        glm::vec2 anchor(
            barMax.x + padding,
            barCenterY - (textSize.y / 2.0f) // Adjust for text height to achieve perfect centering
        );

        // 4. Create the TextElement. We must align it to the left.
        TextElement element(text, anchor, TextAnchor::Custom);
        element.SetAlignment(TextAlignment::Left);

        // 5. Define a style for high-contrast text
        TextStyle style;
        style.fontSize = finalFontSize;
        style.textColor = IM_COL32(255, 255, 255, 255); // Pure white
        style.enableBackground = false; // No background box
        style.enableShadow = true;
        style.shadowAlpha = 0.9f; // Strong shadow for readability
        style.fadeAlpha = fadeAlpha; // Respect the overall bar fade

        element.SetStyle(style);

        // 6. Render using the global TextRenderer
        TextRenderer::Render(dl, element);
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

} // namespace kx