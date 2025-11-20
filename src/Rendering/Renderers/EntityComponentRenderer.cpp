#include "EntityComponentRenderer.h"
#include "ShapeRenderer.h"
#include "HealthBarRenderer.h"
#include "EnergyBarRenderer.h"

#include "TextRenderer.h"

#include "../Data/FrameData.h"
#include "../Shared/LayoutConstants.h"
#include "../../../libs/ImGui/imgui.h"
#include "../Data/RenderableData.h"
#include <string_view>
#include <cstring>
#include <cstdio>

#include "Presentation/Styling.h"
#include "Presentation/InfoBuilder.h"
#include "Presentation/Formatting.h"
#include "Combat/CombatConstants.h"
#include "Shared/RenderSettingsHelper.h"
#include "../../Utils/UnitConversion.h"
#include "../Shared/ColorConstants.h"

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
        int ret = 0;
        
        switch (settings.distance.displayMode) {
            case DistanceDisplayMode::Meters:
                ret = snprintf(buffer, bufferSize, "%.1fm", meters);
                break;
            case DistanceDisplayMode::GW2Units:
                ret = snprintf(buffer, bufferSize, "%.0f", units);
                break;
            case DistanceDisplayMode::Both:
                ret = snprintf(buffer, bufferSize, "%.0f (%.1fm)", units, meters);
                break;
        }

        if (ret < 0) return 0;
        if (static_cast<size_t>(ret) >= bufferSize) return bufferSize - 1;
        return static_cast<size_t>(ret);
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
        float height = TextRenderer::DrawMultiColored(ctx.drawList, pos, 3, texts, colors, style);
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

    char damageBuffer[32];
    int len = snprintf(damageBuffer, std::size(damageBuffer), "%.0f", animState.damageNumberToDisplay);
    if (len < 0) len = 0;
    if (static_cast<size_t>(len) >= std::size(damageBuffer)) len = static_cast<int>(std::size(damageBuffer) - 1);
    std::string_view damageText(damageBuffer, static_cast<size_t>(len));

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

    char burstBuffer[32];
    std::string_view burstText;
    int len;
    if (burstDps >= CombatEffects::DPS_FORMATTING_THRESHOLD) {
        len = snprintf(burstBuffer, std::size(burstBuffer), "%.1fk", burstDps / CombatEffects::DPS_FORMATTING_THRESHOLD);
        if (len < 0) len = 0;
        if (static_cast<size_t>(len) >= std::size(burstBuffer)) len = static_cast<int>(std::size(burstBuffer) - 1);
        burstText = std::string_view(burstBuffer, static_cast<size_t>(len));
    }
    else {
        len = snprintf(burstBuffer, std::size(burstBuffer), "%.0f", burstDps);
        if (len < 0) len = 0;
        if (static_cast<size_t>(len) >= std::size(burstBuffer)) len = static_cast<int>(std::size(burstBuffer) - 1);
        burstText = std::string_view(burstBuffer, static_cast<size_t>(len));
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
            int hpLen = snprintf(hpBuffer, std::size(hpBuffer), "%d%%", static_cast<int>(healthPercent * 100.0f));
            if (hpLen < 0) hpLen = 0;
            if (static_cast<size_t>(hpLen) >= std::size(hpBuffer)) hpLen = static_cast<int>(std::size(hpBuffer) - 1);
            std::string_view hpText(hpBuffer, static_cast<size_t>(hpLen));

            float hpFontSize = props.style.finalFontSize * RenderingLayout::STATUS_TEXT_FONT_SIZE_MULTIPLIER;
            ImVec2 hpTextSize = font->CalcTextSizeA(hpFontSize, FLT_MAX, 0.0f, hpText.data(), hpText.data() + hpText.size());

            anchorPos.x += hpTextSize.x + RenderingLayout::BURST_DPS_HORIZONTAL_PADDING;
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

void EntityComponentRenderer::RenderEntityDetails(const FrameContext& ctx,
                                                  const RenderableEntity& entity,
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
                            int statLen = snprintf(statBuffer, std::size(statBuffer), "%.0f%% %.*s", info.percentage, static_cast<int>(info.statName.size()), info.statName.data());
                            if (statLen < 0) statLen = 0;
                            if (static_cast<size_t>(statLen) >= std::size(statBuffer)) statLen = static_cast<int>(std::size(statBuffer) - 1);
                            std::string_view statText(statBuffer, static_cast<size_t>(statLen));
                            
                            ImU32 rarityColor = Styling::GetRarityColor(info.highestRarity);
                            append(statText, rarityColor);
                            
                            if (i < summaryCount - 1) {
                                append(", ", ESPColors::SUMMARY_TEXT_RGB);
                            }
                        }

                        if (count > 0) {
                            float height = TextRenderer::DrawMultiColored(ctx.drawList, pos, count, texts, colors, style);
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
                            int statLen = snprintf(statBuffer, std::size(statBuffer), "%.*s %.0f%%", static_cast<int>(stat.name.size()), stat.name.data(), stat.percentage);
                            if (statLen < 0) statLen = 0;
                            if (static_cast<size_t>(statLen) >= std::size(statBuffer)) statLen = static_cast<int>(std::size(statBuffer) - 1);
                            std::string_view statText(statBuffer, static_cast<size_t>(statLen));
                            
                            append(statText, stat.color);
                            
                            if (i < statsCount - 1) {
                                append(" | ", ESPColors::SUMMARY_TEXT_RGB);
                            }
                        }
                        append("]", ESPColors::SUMMARY_TEXT_RGB);

                        if (count > 0) {
                            float height = TextRenderer::DrawMultiColored(ctx.drawList, pos, count, texts, colors, style);
                            cursor.Advance(height);
                        }
                    }
                    break;
                }
                case GearDisplayMode::Detailed: {
                    InfoBuilder::RenderGearDetails(ctx.drawList, cursor, props, player);
                    break;
                }
            }
        }
    }

    switch (entity.entityType) {
        case EntityTypes::Player: {
            const auto* player = static_cast<const RenderablePlayer*>(&entity);
            InfoBuilder::RenderPlayerDetails(ctx.drawList, cursor, props, player, ctx.settings.playerESP, ctx.settings.showDebugAddresses);
            break;
        }
        case EntityTypes::NPC: {
            const auto* npc = static_cast<const RenderableNpc*>(&entity);
            InfoBuilder::RenderNpcDetails(ctx.drawList, cursor, props, npc, ctx.settings.npcESP, ctx.settings.showDebugAddresses);
            break;
        }
        case EntityTypes::Gadget: {
            const auto* gadget = static_cast<const RenderableGadget*>(&entity);
            InfoBuilder::RenderGadgetDetails(ctx.drawList, cursor, props, gadget, ctx.settings.objectESP, ctx.settings.showDebugAddresses);
            break;
        }
        case EntityTypes::AttackTarget: {
            const auto* attackTarget = static_cast<const RenderableAttackTarget*>(&entity);
            InfoBuilder::RenderAttackTargetDetails(ctx.drawList, cursor, props, attackTarget, ctx.settings.objectESP, ctx.settings.showDebugAddresses);
            break;
        }
    }
}

} // namespace kx

