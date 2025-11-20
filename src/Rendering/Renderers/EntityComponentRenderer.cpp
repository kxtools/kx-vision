#include "EntityComponentRenderer.h"
#include "ShapeRenderer.h"
#include "HealthBarRenderer.h"
#include "EnergyBarRenderer.h"

#include "TextRenderer.h"

#include "../Data/FrameData.h"
#include "../Shared/LayoutConstants.h"
#include "../../../libs/ImGui/imgui.h"
#include "../Data/RenderableData.h"
#include <format>
#include <string>

#include "Presentation/Styling.h"
#include "Presentation/InfoBuilder.h"
#include "Presentation/TextElementFactory.h"
#include "Combat/CombatConstants.h"
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

void EntityComponentRenderer::RenderGeometry(const FrameContext& ctx, const RenderableEntity& entity, const VisualProperties& props) {
    float globalOpacity = GetGlobalOpacity(ctx);

    bool shouldRenderBox = RenderSettingsHelper::ShouldRenderBox(ctx.settings, entity.entityType);
    float entityHeight = entity.hasPhysicsDimensions ? entity.physicsHeight : 0.0f;
    bool sizeAllowed = RenderSettingsHelper::IsBoxAllowedForSize(ctx.settings, entity.entityType, entityHeight);

    if (shouldRenderBox && sizeAllowed) {
        ShapeRenderer::RenderBoundingBox(ctx.drawList, props.geometry.boxMin, props.geometry.boxMax, props.style.fadedEntityColor, props.style.finalBoxThickness, globalOpacity);
    }

    bool shouldRenderWireframe = RenderSettingsHelper::ShouldRenderWireframe(ctx.settings, entity.entityType);
    if (shouldRenderWireframe && sizeAllowed) {
        ShapeRenderer::RenderWireframeBox(ctx.drawList, props, props.style.fadedEntityColor, props.style.finalBoxThickness, globalOpacity);
    }

    if (entity.entityType == EntityTypes::Gadget || entity.entityType == EntityTypes::AttackTarget) {
        if (RenderSettingsHelper::ShouldRenderGadgetSphere(ctx.settings, entity.entityType)) {
            ShapeRenderer::RenderGyroscopicOverlay(
                ctx.drawList, 
                entity.position,          
                entity.gameplayDistance,  
                ctx.camera, 
                ctx.screenWidth, 
                ctx.screenHeight, 
                props.style.finalAlpha, 
                props.style.fadedEntityColor, 
                props.style.scale, 
                globalOpacity
            );
        }
        if (RenderSettingsHelper::ShouldRenderGadgetCircle(ctx.settings, entity.entityType)) {
            ShapeRenderer::RenderGadgetCircle(ctx.drawList, props.geometry.screenPos, props.geometry.circleRadius, props.style.fadedEntityColor, props.style.finalBoxThickness, globalOpacity);
        }
    }

    if (RenderSettingsHelper::ShouldRenderDot(ctx.settings, entity.entityType)) {
        if (entity.entityType == EntityTypes::Gadget || entity.entityType == EntityTypes::AttackTarget) {
            ShapeRenderer::RenderNaturalWhiteDot(ctx.drawList, props.geometry.screenPos, props.style.finalAlpha, props.style.finalDotRadius, globalOpacity);
        } else {
            ShapeRenderer::RenderColoredDot(ctx.drawList, props.geometry.screenPos, props.style.fadedEntityColor, props.style.finalDotRadius, globalOpacity);
        }
    }
}

void EntityComponentRenderer::RenderIdentity(const FrameContext& ctx,
                                             const RenderableEntity& entity,
                                             std::string_view displayName,
                                             const VisualProperties& props,
                                             LayoutCursor& cursor) {
    bool showName = RenderSettingsHelper::ShouldRenderName(ctx.settings, entity.entityType);
    bool showDistance = RenderSettingsHelper::ShouldRenderDistance(ctx.settings, entity.entityType);

    if (showName || showDistance) {
        LayoutRequest tempRequest = { entity, displayName, entity.gameplayDistance, props, ctx };
        TextElement identity = TextElementFactory::CreateIdentityLine(tempRequest, showName, showDistance);
        
        identity.SetAnchor(cursor.GetPosition());
        identity.SetPositioning(TextAnchor::Below);
        identity.SetAlignment(TextAlignment::Center);
        
        ImVec2 size = TextRenderer::Render(ctx.drawList, identity);
        cursor.Advance(size.y);
    }
}

static void RenderDamageNumbers(const FrameContext& context,
                                const RenderableEntity& entity,
                                EntityTypes entityType,
                                bool showCombatUI,
                                bool renderHealthBar,
                                const HealthBarAnimationState& animState,
                                const VisualProperties& props,
                                const glm::vec2& healthBarPos) {
    bool shouldShowDamageNumbers = RenderSettingsHelper::ShouldShowDamageNumbers(context.settings, entityType);
    if (!showCombatUI || !shouldShowDamageNumbers || animState.damageNumberAlpha <= 0.0f) {
        return;
    }

    glm::vec2 anchorPos;
    if (renderHealthBar && healthBarPos.x != 0.0f && healthBarPos.y != 0.0f) {
        anchorPos = { healthBarPos.x + props.style.finalHealthBarWidth / 2.0f, healthBarPos.y - animState.damageNumberYOffset };
    } else {
        anchorPos = { props.geometry.center.x, props.geometry.center.y - animState.damageNumberYOffset };
    }

    std::string damageText = std::format("{:.0f}", animState.damageNumberToDisplay);
    float finalFontSize = props.style.finalFontSize * Styling::GetDamageNumberFontSizeMultiplier(animState.damageNumberToDisplay);
    TextElement element = TextElementFactory::CreateDamageNumber(damageText, anchorPos, animState.damageNumberAlpha, finalFontSize, context.settings);
    TextRenderer::Render(context.drawList, element);
}

static void RenderBurstDps(const FrameContext& context,
                           const RenderableEntity& entity,
                           EntityTypes entityType,
                           bool showCombatUI,
                           bool renderHealthBar,
                           float burstDps,
                           const HealthBarAnimationState& animState,
                           const VisualProperties& props,
                           const glm::vec2& healthBarPos) {
    if (!ImGui::GetCurrentContext()) return;
    
    bool shouldShowBurstDps = RenderSettingsHelper::ShouldShowBurstDps(context.settings, entityType);
    if (!showCombatUI || !shouldShowBurstDps || burstDps <= 0.0f || animState.healthBarFadeAlpha <= 0.0f) {
        return;
    }

    float healthPercent = entity.maxHealth > 0 ? (entity.currentHealth / entity.maxHealth) : -1.0f;

    std::string burstText;
    if (burstDps >= CombatEffects::DPS_FORMATTING_THRESHOLD) {
        burstText = std::format("{:.1f}k", burstDps / CombatEffects::DPS_FORMATTING_THRESHOLD);
    }
    else {
        burstText = std::format("{:.0f}", burstDps);
    }

    glm::vec2 anchorPos;
    if (renderHealthBar && healthBarPos.x != 0.0f && healthBarPos.y != 0.0f) {
        float dpsFontSize = props.style.finalFontSize * RenderingLayout::STATUS_TEXT_FONT_SIZE_MULTIPLIER;
        ImFont* font = ImGui::GetFont();
        ImVec2 dpsTextSize = font->CalcTextSizeA(dpsFontSize, FLT_MAX, 0.0f, burstText.c_str());

        float barCenterY = healthBarPos.y + props.style.finalHealthBarHeight / 2.0f;
        
        anchorPos = { 
            healthBarPos.x + props.style.finalHealthBarWidth + RenderingLayout::BURST_DPS_HORIZONTAL_PADDING, 
            barCenterY - (dpsTextSize.y / 2.0f)
        };

        bool shouldRenderHealthPercentage = RenderSettingsHelper::ShouldRenderHealthPercentage(context.settings, entityType);
        if (shouldRenderHealthPercentage && healthPercent >= 0.0f) {
            std::string hpText = std::to_string(static_cast<int>(healthPercent * 100.0f)) + "%";

            float hpFontSize = props.style.finalFontSize * RenderingLayout::STATUS_TEXT_FONT_SIZE_MULTIPLIER;
            ImVec2 hpTextSize = font->CalcTextSizeA(hpFontSize, FLT_MAX, 0.0f, hpText.c_str());

            anchorPos.x += hpTextSize.x + RenderingLayout::BURST_DPS_HORIZONTAL_PADDING;
        }

    }
    else {
        anchorPos = { props.geometry.screenPos.x, props.geometry.screenPos.y + RenderingLayout::BURST_DPS_FALLBACK_Y_OFFSET };
    }

    TextElement element(burstText, anchorPos, TextAnchor::Custom);
    element.SetAlignment(TextAlignment::Left);
    TextStyle style = TextElementFactory::GetDistanceStyle(animState.healthBarFadeAlpha, props.style.finalFontSize * RenderingLayout::STATUS_TEXT_FONT_SIZE_MULTIPLIER, context.settings);
    style.enableBackground = false;
    style.textColor = ESPBarColors::BURST_DPS_TEXT;
    element.SetStyle(style);
    TextRenderer::Render(context.drawList, element);
}

void EntityComponentRenderer::RenderStatusBars(const FrameContext& ctx,
                                               const RenderableEntity& entity,
                                               bool showCombatUI,
                                               bool renderHealthBar,
                                               bool renderEnergyBar,
                                               float burstDps,
                                               Game::Attitude attitude,
                                               const HealthBarAnimationState& animState,
                                               const VisualProperties& props,
                                               LayoutCursor& cursor) {
    bool isLiving = (entity.entityType == EntityTypes::Player || entity.entityType == EntityTypes::NPC);
    bool isGadget = (entity.entityType == EntityTypes::Gadget || entity.entityType == EntityTypes::AttackTarget);
    
    if ((isLiving || isGadget) && renderHealthBar) {
        float healthPercent = entity.maxHealth > 0 ? (entity.currentHealth / entity.maxHealth) : -1.0f;
        
        if (healthPercent >= 0.0f) {
            glm::vec2 healthBarPos = cursor.GetTopLeftForBar(props.style.finalHealthBarWidth, props.style.finalHealthBarHeight);
            
            HealthBarRenderer::RenderStandaloneHealthBar(ctx.drawList,
                                                        healthBarPos,
                                                        entity,
                                                        entity.entityType,
                                                        attitude,
                                                        props,
                                                        animState,
                                                        ctx.settings);

            RenderDamageNumbers(ctx, entity, entity.entityType, showCombatUI, renderHealthBar, animState, props, healthBarPos);
            RenderBurstDps(ctx, entity, entity.entityType, showCombatUI, renderHealthBar, burstDps, animState, props, healthBarPos);

            cursor.Advance(props.style.finalHealthBarHeight);
        }
    } else {
        glm::vec2 centerPos(props.geometry.center.x, props.geometry.center.y);
        RenderDamageNumbers(ctx, entity, entity.entityType, showCombatUI, renderHealthBar, animState, props, centerPos);
        RenderBurstDps(ctx, entity, entity.entityType, showCombatUI, renderHealthBar, burstDps, animState, props, centerPos);
    }

    if (entity.entityType == EntityTypes::Player) {
        const auto* player = static_cast<const RenderablePlayer*>(&entity);
        EnergyDisplayType energyDisplayType = RenderSettingsHelper::GetPlayerEnergyDisplayType(ctx.settings);
        float energyPercent = CalculateEnergyPercent(player, energyDisplayType);
        if (energyPercent >= 0.0f && renderEnergyBar) {
            glm::vec2 barPos = cursor.GetTopLeftForBar(props.style.finalHealthBarWidth, props.style.finalHealthBarHeight);
            EnergyBarRenderer::Render(ctx.settings, ctx.drawList, barPos, energyPercent,
                props.style.finalAlpha, props.style.finalHealthBarWidth, props.style.finalHealthBarHeight);
            cursor.Advance(props.style.finalHealthBarHeight);
        }
    }
}

void EntityComponentRenderer::RenderDetails(const FrameContext& ctx,
                                            const RenderableEntity& entity,
                                            bool renderDetails,
                                            const std::vector<ColoredDetail>& details,
                                            const VisualProperties& props,
                                            LayoutCursor& cursor) {
    if (entity.entityType == EntityTypes::Player) {
        const auto* player = static_cast<const RenderablePlayer*>(&entity);
        if (player != nullptr) {
            GearDisplayMode gearDisplayMode = RenderSettingsHelper::GetPlayerGearDisplayMode(ctx.settings);
            switch (gearDisplayMode) {
                case GearDisplayMode::Compact: {
                    auto summary = InfoBuilder::BuildCompactGearSummary(player);
                    if (!summary.empty()) {
                        TextElement gearElement = TextElementFactory::CreateGearSummaryAt(summary, cursor.GetPosition(), props.style.finalAlpha, props.style.finalFontSize, ctx.settings);
                        ImVec2 size = TextRenderer::Render(ctx.drawList, gearElement);
                        cursor.Advance(size.y);
                    }
                    break;
                }
                case GearDisplayMode::Attributes: {
                    auto stats = InfoBuilder::BuildDominantStats(player);
                    auto rarity = InfoBuilder::GetHighestRarity(player);
                    if (!stats.empty()) {
                        TextElement statsElement = TextElementFactory::CreateDominantStatsAt(stats, rarity, cursor.GetPosition(), props.style.finalAlpha, props.style.finalFontSize, ctx.settings);
                        ImVec2 size = TextRenderer::Render(ctx.drawList, statsElement);
                        cursor.Advance(size.y);
                    }
                    break;
                }
                default: break;
            }
        }
    }

    if (renderDetails && !details.empty()) {
        TextElement detailsElement = TextElementFactory::CreateDetailsTextAt(details, cursor.GetPosition(), props.style.finalAlpha, props.style.finalFontSize, ctx.settings);
        ImVec2 size = TextRenderer::Render(ctx.drawList, detailsElement);
        cursor.Advance(size.y);
    }
}

} // namespace kx

