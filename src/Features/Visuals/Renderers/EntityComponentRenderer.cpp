#include "EntityComponentRenderer.h"
#include "ShapeRenderer.h"
#include "HealthBarRenderer.h"
#include "EnergyBarRenderer.h"

#include "../../../Rendering/Renderers/TextRenderer.h"

#include "../../../Game/Data/FrameData.h"
#include "../../../Rendering/Shared/LayoutConstants.h"
#include "../../../../libs/ImGui/imgui.h"
#include "../../../Game/Data/RenderableData.h"
#include <string_view>
#include <span>
#include <format>
#include <array>

#include "../Presentation/Styling.h"
#include "../Presentation/InfoBuilder.h"
#include "../Presentation/Formatting.h"
#include "../../Combat/CombatConstants.h"
#include "../../../Rendering/Shared/RenderSettingsHelper.h"
#include "../../../Utils/UnitConversion.h"
#include "../../../Rendering/Shared/ColorConstants.h"

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

    size_t FormatDistance(char* buffer, size_t bufferSize, float meters, const Settings& settings) {
        float units = UnitConversion::MetersToGW2Units(meters);
        std::format_to_n_result<char*> result;
        
        switch (settings.distance.displayMode) {
            case DistanceDisplayMode::Meters:
                result = std::format_to_n(buffer, bufferSize, "{:.1f}m", meters);
                break;
            case DistanceDisplayMode::GW2Units:
                result = std::format_to_n(buffer, bufferSize, "{:.0f}", units);
                break;
            case DistanceDisplayMode::Both:
                result = std::format_to_n(buffer, bufferSize, "{:.0f} ({:.1f}m)", units, meters);
                break;
            default:
                return 0;
        }

        return (std::min)(static_cast<size_t>(result.size), bufferSize);
    }
}

void EntityComponentRenderer::RenderGeometry(const FrameContext& ctx, const GameEntity& entity, const VisualProperties& props) {
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

    if (RenderSettingsHelper::IsObjectType(entity.entityType)) {
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
        if (RenderSettingsHelper::IsObjectType(entity.entityType) && entity.entityType != EntityTypes::Item) {
            ShapeRenderer::RenderNaturalWhiteDot(ctx.drawList, props.geometry.screenPos, props.style.finalAlpha, props.style.finalDotRadius, globalOpacity);
        } else {
            ShapeRenderer::RenderColoredDot(ctx.drawList, props.geometry.screenPos, props.style.fadedEntityColor, props.style.finalDotRadius, globalOpacity);
        }
    }
}

void EntityComponentRenderer::RenderIdentity(const FrameContext& ctx,
                                             const GameEntity& entity,
                                             std::string_view displayName,
                                             const VisualProperties& props,
                                             LayoutCursor& cursor) {
    bool showName = RenderSettingsHelper::ShouldRenderName(ctx.settings, entity.entityType);
    bool showDistance = RenderSettingsHelper::ShouldRenderDistance(ctx.settings, entity.entityType);

    if (!showName && !showDistance) return;

    glm::vec2 pos = cursor.GetPosition();
    pos.y += RenderingLayout::TEXT_ANCHOR_GAP;

    char distanceBuffer[RenderingLayout::TEXT_BUFFER_SIZE];
    char separatorBuffer[8] = " • ";

    std::string_view nameText = displayName;
    if (showName) {
        if (nameText.empty() && entity.entityType == EntityTypes::Player) {
            const auto* player = static_cast<const RenderablePlayer*>(&entity);
            if (player) {
                const char* profName = Formatting::GetProfessionName(player->profession);
                if (profName) {
                    nameText = profName;
                }
            }
        }
    }

    std::string_view distanceText;
    if (showDistance) {
        size_t len = FormatDistance(distanceBuffer, sizeof(distanceBuffer), entity.gameplayDistance, ctx.settings);
        distanceText = std::string_view(distanceBuffer, len);
    }

    const float identityBottomPadding = 3.0f;

    FastTextStyle style;
    style.fontSize = props.style.finalFontSize;
    style.shadow = ctx.settings.appearance.enableTextShadows;
    style.background = ctx.settings.appearance.enableTextBackgrounds;
    style.fadeAlpha = props.style.finalAlpha;

    if (showName && showDistance) {
        std::string_view texts[] = { nameText, separatorBuffer, distanceText };
        ImU32 colors[] = { 
            props.style.fadedEntityColor, 
            ESPColors::DEFAULT_TEXT, 
            ESPColors::DEFAULT_TEXT 
        };
        float height = TextRenderer::DrawMultiColored(ctx.drawList, pos, texts, colors, style);
        cursor.Advance(height + identityBottomPadding);
    } else if (showName) {
        style.color = props.style.fadedEntityColor;
        float height = TextRenderer::DrawCentered(ctx.drawList, pos, nameText, style);
        cursor.Advance(height + identityBottomPadding);
    } else if (showDistance) {
        style.color = ESPColors::DEFAULT_TEXT;
        float height = TextRenderer::DrawCentered(ctx.drawList, pos, distanceText, style);
        cursor.Advance(height + identityBottomPadding);
    }
}

static void RenderDamageNumbers(const FrameContext& context,
                                const GameEntity& entity,
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

    char damageBuffer[32];
    auto result = std::format_to_n(damageBuffer, std::size(damageBuffer), "{:.0f}", animState.damageNumberToDisplay);
    std::string_view damageText(damageBuffer, result.size);

    float finalFontSize = props.style.finalFontSize * Styling::GetDamageNumberFontSizeMultiplier(animState.damageNumberToDisplay);
    
    FastTextStyle style;
    style.fontSize = finalFontSize;
    style.color = IM_COL32(255, 255, 255, 255);
    style.shadow = context.settings.appearance.enableTextShadows;
    style.background = false;
    style.fadeAlpha = animState.damageNumberAlpha;

    TextRenderer::DrawCentered(context.drawList, anchorPos, damageText, style);
}

static void RenderBurstDps(const FrameContext& context,
                           const GameEntity& entity,
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

    char burstBuffer[32];
    std::string_view burstText;
    std::format_to_n_result<char*> result;
    if (burstDps >= CombatEffects::DPS_FORMATTING_THRESHOLD) {
        result = std::format_to_n(burstBuffer, std::size(burstBuffer), "{:.1f}k", burstDps / CombatEffects::DPS_FORMATTING_THRESHOLD);
        burstText = std::string_view(burstBuffer, result.size);
    }
    else {
        result = std::format_to_n(burstBuffer, std::size(burstBuffer), "{:.0f}", burstDps);
        burstText = std::string_view(burstBuffer, result.size);
    }

    glm::vec2 anchorPos;
    if (renderHealthBar && healthBarPos.x != 0.0f && healthBarPos.y != 0.0f) {
        float dpsFontSize = props.style.finalFontSize * RenderingLayout::STATUS_TEXT_FONT_SIZE_MULTIPLIER;
        ImFont* font = ImGui::GetFont();
        ImVec2 dpsTextSize = font->CalcTextSizeA(dpsFontSize, FLT_MAX, 0.0f, burstText.data(), burstText.data() + burstText.size());

        float barCenterY = healthBarPos.y + props.style.finalHealthBarHeight / 2.0f;
        
        bool shouldRenderHealthPercentage = RenderSettingsHelper::ShouldRenderHealthPercentage(context.settings, entityType);
        
        float spacingFromBar = shouldRenderHealthPercentage 
            ? RenderingLayout::BURST_DPS_HORIZONTAL_PADDING 
            : RenderingLayout::BURST_DPS_MIN_SPACING_FROM_BAR;
        
        anchorPos = { 
            healthBarPos.x + props.style.finalHealthBarWidth + spacingFromBar, 
            barCenterY - (dpsTextSize.y / 2.0f)
        };

        if (shouldRenderHealthPercentage && healthPercent >= 0.0f) {
            char hpBuffer[16];
            auto hpResult = std::format_to_n(hpBuffer, std::size(hpBuffer), "{:.0f}%", healthPercent * 100.0f);
            std::string_view hpText(hpBuffer, hpResult.size);

            float hpFontSize = props.style.finalFontSize * RenderingLayout::STATUS_TEXT_FONT_SIZE_MULTIPLIER;
            ImVec2 hpTextSize = font->CalcTextSizeA(hpFontSize, FLT_MAX, 0.0f, hpText.data(), hpText.data() + hpText.size());

            anchorPos.x += hpTextSize.x + RenderingLayout::BURST_DPS_SPACING_FROM_HP_PERCENT;
        }

    }
    else {
        anchorPos = { props.geometry.screenPos.x, props.geometry.screenPos.y + RenderingLayout::BURST_DPS_FALLBACK_Y_OFFSET };
    }

    FastTextStyle style;
    style.fontSize = props.style.finalFontSize * RenderingLayout::STATUS_TEXT_FONT_SIZE_MULTIPLIER;
    style.color = ESPBarColors::BURST_DPS_TEXT;
    style.shadow = context.settings.appearance.enableTextShadows;
    style.background = false;
    style.fadeAlpha = animState.healthBarFadeAlpha;

    TextRenderer::DrawCentered(context.drawList, anchorPos, burstText, style);
}

void EntityComponentRenderer::RenderStatusBars(const FrameContext& ctx,
                                               const GameEntity& entity,
                                               bool showCombatUI,
                                               bool renderHealthBar,
                                               bool renderEnergyBar,
                                               float burstDps,
                                               Game::Attitude attitude,
                                               const HealthBarAnimationState& animState,
                                               const VisualProperties& props,
                                               LayoutCursor& cursor) {
    bool isLiving = (entity.entityType == EntityTypes::Player || entity.entityType == EntityTypes::NPC);
    bool isGadget = RenderSettingsHelper::IsObjectType(entity.entityType);
    
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

void EntityComponentRenderer::RenderEntityDetails(const FrameContext& ctx,
                                                  const GameEntity& entity,
                                                  const VisualProperties& props,
                                                  LayoutCursor& cursor) {
    if (entity.entityType == EntityTypes::Player) {
        const auto* player = static_cast<const RenderablePlayer*>(&entity);
        if (player != nullptr && ctx.settings.playerESP.enableGearDisplay) {
            GearDisplayMode gearDisplayMode = RenderSettingsHelper::GetPlayerGearDisplayMode(ctx.settings);
            glm::vec2 pos = cursor.GetPosition();
            
            FastTextStyle style;
            style.fontSize = props.style.finalFontSize;
            style.shadow = ctx.settings.appearance.enableTextShadows;
            style.background = ctx.settings.appearance.enableTextBackgrounds;
            style.fadeAlpha = props.style.finalAlpha;

            switch (gearDisplayMode) {
                case GearDisplayMode::Compact: {
                    CompactStatInfo summaryBuffer[3];
                    size_t summaryCount = InfoBuilder::BuildCompactGearSummary(player, summaryBuffer, 3);
                    if (summaryCount > 0) {
                        char buffer[RenderingLayout::GEAR_BUFFER_SIZE];
                        char* current = buffer;
                        size_t remaining = sizeof(buffer);
                        
                        std::string_view texts[32];
                        ImU32 colors[32];
                        int count = 0;

                        auto append = [&](std::string_view text, ImU32 color) {
                            if (count >= 32 || remaining < text.size() + 1) return false;
                            texts[count] = std::string_view(current, text.size());
                            colors[count] = color;
                            std::copy(text.begin(), text.end(), current);
                            current += text.size();
                            *current = '\0';
                            current++;
                            remaining -= text.size() + 1;
                            count++;
                            return true;
                        };

                        append("Stats: ", ESPColors::SUMMARY_TEXT_RGB);
                        for (size_t i = 0; i < summaryCount && count < 32; ++i) {
                            const auto& info = summaryBuffer[i];
                            char statBuffer[64];
                            auto result = std::format_to_n(statBuffer, std::size(statBuffer), "{:.0f}% {}", info.percentage, info.statName);
                            std::string_view statText(statBuffer, result.size);
                            
                            ImU32 rarityColor = Styling::GetRarityColor(info.highestRarity);
                            append(statText, rarityColor);
                            
                            if (i < summaryCount - 1) {
                                append(", ", ESPColors::SUMMARY_TEXT_RGB);
                            }
                        }

                        if (count > 0) {
                            float height = TextRenderer::DrawMultiColored(ctx.drawList, pos, std::span(texts, count), std::span(colors, count), style);
                            cursor.Advance(height);
                        }
                    }
                    break;
                }
                case GearDisplayMode::Attributes: {
                    DominantStat statsBuffer[3];
                    size_t statsCount = InfoBuilder::BuildDominantStats(player, statsBuffer, 3);
                    if (statsCount > 0) {
                        char buffer[RenderingLayout::GEAR_BUFFER_SIZE];
                        char* current = buffer;
                        size_t remaining = sizeof(buffer);
                        
                        std::string_view texts[32];
                        ImU32 colors[32];
                        int count = 0;

                        auto append = [&](std::string_view text, ImU32 color) {
                            if (count >= 32 || remaining < text.size() + 1) return false;
                            texts[count] = std::string_view(current, text.size());
                            colors[count] = color;
                            std::copy(text.begin(), text.end(), current);
                            current += text.size();
                            *current = '\0';
                            current++;
                            remaining -= text.size() + 1;
                            count++;
                            return true;
                        };

                        append("[", ESPColors::SUMMARY_TEXT_RGB);
                        for (size_t i = 0; i < statsCount && count < 32; ++i) {
                            const auto& stat = statsBuffer[i];
                            char statBuffer[64];
                            auto result = std::format_to_n(statBuffer, std::size(statBuffer), "{} {:.0f}%", stat.name, stat.percentage);
                            std::string_view statText(statBuffer, result.size);
                            
                            append(statText, stat.color);
                            
                            if (i < statsCount - 1) {
                                append(" | ", ESPColors::SUMMARY_TEXT_RGB);
                            }
                        }
                        append("]", ESPColors::SUMMARY_TEXT_RGB);

                        if (count > 0) {
                            float height = TextRenderer::DrawMultiColored(ctx.drawList, pos, std::span(texts, count), std::span(colors, count), style);
                            cursor.Advance(height);
                        }
                    }
                    break;
                }
                case GearDisplayMode::Detailed: {
                    InfoBuilder::RenderGearDetails(ctx.drawList, cursor, props, player, ctx.settings.appearance);
                    break;
                }
            }
        }
    }

    switch (entity.entityType) {
        case EntityTypes::Player: {
            const auto* player = static_cast<const RenderablePlayer*>(&entity);
            InfoBuilder::RenderPlayerDetails(ctx.drawList, cursor, props, player, ctx.settings.playerESP, ctx.settings.appearance, ctx.settings.showDebugAddresses);
            break;
        }
        case EntityTypes::NPC: {
            const auto* npc = static_cast<const RenderableNpc*>(&entity);
            InfoBuilder::RenderNpcDetails(ctx.drawList, cursor, props, npc, ctx.settings.npcESP, ctx.settings.appearance, ctx.settings.showDebugAddresses);
            break;
        }
        case EntityTypes::Gadget: {
            const auto* gadget = static_cast<const RenderableGadget*>(&entity);
            InfoBuilder::RenderGadgetDetails(ctx.drawList, cursor, props, gadget, ctx.settings.objectESP, ctx.settings.appearance, ctx.settings.showDebugAddresses);
            break;
        }
        case EntityTypes::AttackTarget: {
            const auto* attackTarget = static_cast<const RenderableAttackTarget*>(&entity);
            InfoBuilder::RenderAttackTargetDetails(ctx.drawList, cursor, props, attackTarget, ctx.settings.objectESP, ctx.settings.appearance, ctx.settings.showDebugAddresses);
            break;
        }
        case EntityTypes::Item: {
            const auto* item = static_cast<const RenderableItem*>(&entity);
            InfoBuilder::RenderItemDetails(ctx.drawList, cursor, props, item, ctx.settings.objectESP, ctx.settings.appearance, ctx.settings.showDebugAddresses);
            break;
        }
    }
}

} // namespace kx

