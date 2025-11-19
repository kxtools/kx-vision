#include "EntityComponentRenderer.h"
#include "ESPShapeRenderer.h"
#include "HealthBarRenderer.h"
#include "ESPEnergyBarRenderer.h"

#include "TextRenderer.h"

#include "../Data/FrameData.h"
#include "../Data/EntityRenderContext.h"

#include "../Shared/LayoutConstants.h"
#include "../../../libs/ImGui/imgui.h"
#include "../Data/RenderableData.h"
#include <sstream>
#include <iomanip>
#include <string>

#include "Presentation/Styling.h"
#include "Presentation/InfoBuilder.h"
#include "Presentation/TextElementFactory.h"
#include "Shared/RenderSettingsHelper.h"

namespace kx {

namespace {
    inline float GetGlobalOpacity(const FrameContext& context) {
        return context.settings.appearance.globalOpacity;
    }

    float CalculateEnergyPercent(const RenderablePlayer* player, EnergyDisplayType displayType) {
        if (displayType == EnergyDisplayType::Endurance) {
            if (player->maxEndurance > 0) {
                return player->currentEndurance / player->maxEndurance;
            }
        } else {
            if (player->maxEnergy > 0) {
                return player->currentEnergy / player->maxEnergy;
            }
        }
        return -1.0f;
    }
}

void EntityComponentRenderer::RenderGeometry(const FrameContext& ctx, const EntityRenderContext& eCtx, const VisualProperties& props) {
    float globalOpacity = GetGlobalOpacity(ctx);

    bool shouldRenderBox = RenderSettingsHelper::ShouldRenderBox(ctx.settings, eCtx.entityType);
    float entityHeight = eCtx.entity->hasPhysicsDimensions ? eCtx.entity->physicsHeight : 0.0f;
    bool sizeAllowed = RenderSettingsHelper::IsBoxAllowedForSize(ctx.settings, eCtx.entityType, entityHeight);

    if (shouldRenderBox && sizeAllowed) {
        ESPShapeRenderer::RenderBoundingBox(ctx.drawList, props.boxMin, props.boxMax, props.fadedEntityColor, props.finalBoxThickness, globalOpacity);
    }

    bool shouldRenderWireframe = RenderSettingsHelper::ShouldRenderWireframe(ctx.settings, eCtx.entityType);
    if (shouldRenderWireframe && sizeAllowed) {
        ESPShapeRenderer::RenderWireframeBox(ctx.drawList, props, props.fadedEntityColor, props.finalBoxThickness, globalOpacity);
    }

    if (eCtx.entityType == ESPEntityType::Gadget || eCtx.entityType == ESPEntityType::AttackTarget) {
        if (RenderSettingsHelper::ShouldRenderGadgetSphere(ctx.settings, eCtx.entityType)) {
            ESPShapeRenderer::RenderGyroscopicOverlay(
                ctx.drawList, 
                eCtx.position,          
                eCtx.gameplayDistance,  
                ctx.camera, 
                ctx.screenWidth, 
                ctx.screenHeight, 
                props.finalAlpha, 
                props.fadedEntityColor, 
                props.scale, 
                globalOpacity
            );
        }
        if (RenderSettingsHelper::ShouldRenderGadgetCircle(ctx.settings, eCtx.entityType)) {
            ESPShapeRenderer::RenderGadgetCircle(ctx.drawList, props.screenPos, props.circleRadius, props.fadedEntityColor, props.finalBoxThickness, globalOpacity);
        }
    }

    if (RenderSettingsHelper::ShouldRenderDot(ctx.settings, eCtx.entityType)) {
        if (eCtx.entityType == ESPEntityType::Gadget || eCtx.entityType == ESPEntityType::AttackTarget) {
            ESPShapeRenderer::RenderNaturalWhiteDot(ctx.drawList, props.screenPos, props.finalAlpha, props.finalDotRadius, globalOpacity);
        } else {
            ESPShapeRenderer::RenderColoredDot(ctx.drawList, props.screenPos, props.fadedEntityColor, props.finalDotRadius, globalOpacity);
        }
    }
}

void EntityComponentRenderer::RenderIdentity(const FrameContext& ctx, const EntityRenderContext& eCtx, const VisualProperties& props, LayoutCursor& cursor) {
    bool showName = RenderSettingsHelper::ShouldRenderName(ctx.settings, eCtx.entityType);
    bool showDistance = RenderSettingsHelper::ShouldRenderDistance(ctx.settings, eCtx.entityType);

    if (showName || showDistance) {
        LayoutRequest tempRequest = { eCtx, props, ctx };
        TextElement identity = TextElementFactory::CreateIdentityLine(tempRequest, showName, showDistance);
        
        identity.SetAnchor(cursor.GetPosition());
        identity.SetPositioning(TextAnchor::Below);
        identity.SetAlignment(TextAlignment::Center);
        
        ImVec2 size = TextRenderer::Render(ctx.drawList, identity);
        cursor.Advance(size.y);
    }
}

static void RenderDamageNumbers(const FrameContext& context, const EntityRenderContext& entityContext, const VisualProperties& props, const glm::vec2& healthBarPos) {
    bool shouldShowDamageNumbers = RenderSettingsHelper::ShouldShowDamageNumbers(context.settings, entityContext.entityType);
    if (!entityContext.showCombatUI || !shouldShowDamageNumbers || entityContext.healthBarAnim.damageNumberAlpha <= 0.0f) {
        return;
    }

    glm::vec2 anchorPos;
    if (entityContext.renderHealthBar && healthBarPos.x != 0.0f && healthBarPos.y != 0.0f) {
        anchorPos = { healthBarPos.x + props.finalHealthBarWidth / 2.0f, healthBarPos.y - entityContext.healthBarAnim.damageNumberYOffset };
    } else {
        anchorPos = { props.center.x, props.center.y - entityContext.healthBarAnim.damageNumberYOffset };
    }

    std::stringstream ss;
    ss << std::fixed << std::setprecision(0) << entityContext.healthBarAnim.damageNumberToDisplay;
    float finalFontSize = props.finalFontSize * Styling::GetDamageNumberFontSizeMultiplier(entityContext.healthBarAnim.damageNumberToDisplay);
    TextElement element = TextElementFactory::CreateDamageNumber(ss.str(), anchorPos, entityContext.healthBarAnim.damageNumberAlpha, finalFontSize, context.settings);
    TextRenderer::Render(context.drawList, element);
}

static void RenderBurstDps(const FrameContext& context, const EntityRenderContext& entityContext, const VisualProperties& props, const glm::vec2& healthBarPos) {
    if (!ImGui::GetCurrentContext()) return;
    
    bool shouldShowBurstDps = RenderSettingsHelper::ShouldShowBurstDps(context.settings, entityContext.entityType);
    if (!entityContext.showCombatUI || !shouldShowBurstDps || entityContext.burstDPS <= 0.0f || entityContext.healthBarAnim.healthBarFadeAlpha <= 0.0f) {
        return;
    }

    float healthPercent = entityContext.entity->maxHealth > 0 ? (entityContext.entity->currentHealth / entityContext.entity->maxHealth) : -1.0f;

    std::stringstream ss;
    if (entityContext.burstDPS >= CombatEffects::DPS_FORMATTING_THRESHOLD) {
        ss << std::fixed << std::setprecision(1) << (entityContext.burstDPS / CombatEffects::DPS_FORMATTING_THRESHOLD) << "k";
    }
    else {
        ss << std::fixed << std::setprecision(0) << entityContext.burstDPS;
    }

    glm::vec2 anchorPos;
    if (entityContext.renderHealthBar && healthBarPos.x != 0.0f && healthBarPos.y != 0.0f) {
        float dpsFontSize = props.finalFontSize * RenderingLayout::STATUS_TEXT_FONT_SIZE_MULTIPLIER;
        ImFont* font = ImGui::GetFont();
        ImVec2 dpsTextSize = font->CalcTextSizeA(dpsFontSize, FLT_MAX, 0.0f, ss.str().c_str());

        float barCenterY = healthBarPos.y + props.finalHealthBarHeight / 2.0f;
        
        anchorPos = { 
            healthBarPos.x + props.finalHealthBarWidth + RenderingLayout::BURST_DPS_HORIZONTAL_PADDING, 
            barCenterY - (dpsTextSize.y / 2.0f)
        };

        bool shouldRenderHealthPercentage = RenderSettingsHelper::ShouldRenderHealthPercentage(context.settings, entityContext.entityType);
        if (shouldRenderHealthPercentage && healthPercent >= 0.0f) {
            std::string hpText = std::to_string(static_cast<int>(healthPercent * 100.0f)) + "%";

            float hpFontSize = props.finalFontSize * RenderingLayout::STATUS_TEXT_FONT_SIZE_MULTIPLIER;
            ImVec2 hpTextSize = font->CalcTextSizeA(hpFontSize, FLT_MAX, 0.0f, hpText.c_str());

            anchorPos.x += hpTextSize.x + RenderingLayout::BURST_DPS_HORIZONTAL_PADDING;
        }

    }
    else {
        anchorPos = { props.screenPos.x, props.screenPos.y + RenderingLayout::BURST_DPS_FALLBACK_Y_OFFSET };
    }

    TextElement element(ss.str(), anchorPos, TextAnchor::Custom);
    element.SetAlignment(TextAlignment::Left);
    TextStyle style = TextElementFactory::GetDistanceStyle(entityContext.healthBarAnim.healthBarFadeAlpha, props.finalFontSize * RenderingLayout::STATUS_TEXT_FONT_SIZE_MULTIPLIER, context.settings);
    style.enableBackground = false;
    style.textColor = ESPBarColors::BURST_DPS_TEXT;
    element.SetStyle(style);
    TextRenderer::Render(context.drawList, element);
}

void EntityComponentRenderer::RenderStatusBars(const FrameContext& ctx, const EntityRenderContext& eCtx, const VisualProperties& props, LayoutCursor& cursor) {
    bool isLiving = (eCtx.entityType == ESPEntityType::Player || eCtx.entityType == ESPEntityType::NPC);
    bool isGadget = (eCtx.entityType == ESPEntityType::Gadget || eCtx.entityType == ESPEntityType::AttackTarget);
    
    if ((isLiving || isGadget) && eCtx.renderHealthBar) {
        float healthPercent = eCtx.entity->maxHealth > 0 ? (eCtx.entity->currentHealth / eCtx.entity->maxHealth) : -1.0f;
        
        if (healthPercent >= 0.0f) {
            glm::vec2 healthBarPos = cursor.GetTopLeftForBar(props.finalHealthBarWidth, props.finalHealthBarHeight);
            
            // REFACTORED CALL: Pass props directly instead of individual fields
            HealthBarRenderer::RenderStandaloneHealthBar(ctx.drawList, healthBarPos, eCtx, props, ctx.settings);

            RenderDamageNumbers(ctx, eCtx, props, healthBarPos);
            RenderBurstDps(ctx, eCtx, props, healthBarPos);

            cursor.Advance(props.finalHealthBarHeight);
        }
    } else {
        glm::vec2 centerPos(props.center.x, props.center.y);
        RenderDamageNumbers(ctx, eCtx, props, centerPos);
        RenderBurstDps(ctx, eCtx, props, centerPos);
    }

    if (eCtx.entityType == ESPEntityType::Player) {
        const auto* player = static_cast<const RenderablePlayer*>(eCtx.entity);
        EnergyDisplayType energyDisplayType = RenderSettingsHelper::GetPlayerEnergyDisplayType(ctx.settings);
        float energyPercent = CalculateEnergyPercent(player, energyDisplayType);
        if (energyPercent >= 0.0f && eCtx.renderEnergyBar) {
            glm::vec2 barPos = cursor.GetTopLeftForBar(props.finalHealthBarWidth, props.finalHealthBarHeight);
            ESPEnergyBarRenderer::Render(ctx.settings, ctx.drawList, barPos, energyPercent,
                props.finalAlpha, props.finalHealthBarWidth, props.finalHealthBarHeight);
            cursor.Advance(props.finalHealthBarHeight);
        }
    }
}

void EntityComponentRenderer::RenderDetails(const FrameContext& ctx, const EntityRenderContext& eCtx, const VisualProperties& props, LayoutCursor& cursor) {
    if (eCtx.entityType == ESPEntityType::Player) {
        const auto* player = static_cast<const RenderablePlayer*>(eCtx.entity);
        if (player != nullptr) {
            GearDisplayMode gearDisplayMode = RenderSettingsHelper::GetPlayerGearDisplayMode(ctx.settings);
            switch (gearDisplayMode) {
                case GearDisplayMode::Compact: {
                    auto summary = InfoBuilder::BuildCompactGearSummary(player);
                    if (!summary.empty()) {
                        TextElement gearElement = TextElementFactory::CreateGearSummaryAt(summary, cursor.GetPosition(), props.finalAlpha, props.finalFontSize, ctx.settings);
                        ImVec2 size = TextRenderer::Render(ctx.drawList, gearElement);
                        cursor.Advance(size.y);
                    }
                    break;
                }
                case GearDisplayMode::Attributes: {
                    auto stats = InfoBuilder::BuildDominantStats(player);
                    auto rarity = InfoBuilder::GetHighestRarity(player);
                    if (!stats.empty()) {
                        TextElement statsElement = TextElementFactory::CreateDominantStatsAt(stats, rarity, cursor.GetPosition(), props.finalAlpha, props.finalFontSize, ctx.settings);
                        ImVec2 size = TextRenderer::Render(ctx.drawList, statsElement);
                        cursor.Advance(size.y);
                    }
                    break;
                }
                default: break;
            }
        }
    }

    if (eCtx.renderDetails && !eCtx.details.empty()) {
        TextElement detailsElement = TextElementFactory::CreateDetailsTextAt(eCtx.details, cursor.GetPosition(), props.finalAlpha, props.finalFontSize, ctx.settings);
        ImVec2 size = TextRenderer::Render(ctx.drawList, detailsElement);
        cursor.Advance(size.y);
    }
}

} // namespace kx

