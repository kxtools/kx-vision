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
        
        switch (settings.distance.displayMode) {
            case DistanceDisplayMode::Meters:
                return static_cast<size_t>(snprintf(buffer, bufferSize, "%.1fm", meters));
            case DistanceDisplayMode::GW2Units:
                return static_cast<size_t>(snprintf(buffer, bufferSize, "%.0f", units));
            case DistanceDisplayMode::Both:
                return static_cast<size_t>(snprintf(buffer, bufferSize, "%.0f (%.1fm)", units, meters));
        }
        return 0;
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

    char nameBuffer[128];
    char distanceBuffer[64];
    char separatorBuffer[8] = " • ";

    std::string_view nameText;
    if (showName) {
        std::string entityName = std::string(displayName);
        if (entityName.empty() && entity.entityType == EntityTypes::Player) {
            const auto* player = static_cast<const RenderablePlayer*>(&entity);
            if (player) {
                const char* profName = Formatting::GetProfessionName(player->profession);
                if (profName) entityName = profName;
            }
        }
        if (!entityName.empty()) {
            size_t len = entityName.length();
            if (len < sizeof(nameBuffer)) {
                std::copy(entityName.begin(), entityName.end(), nameBuffer);
                nameBuffer[len] = '\0';
                nameText = std::string_view(nameBuffer, len);
            } else {
                nameText = entityName;
            }
        }
    }

    std::string_view distanceText;
    if (showDistance) {
        size_t len = FormatDistance(distanceBuffer, sizeof(distanceBuffer), entity.gameplayDistance, ctx.settings);
        distanceText = std::string_view(distanceBuffer, len);
    }

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
        cursor.Advance(height);
    } else if (showName) {
        style.color = props.style.fadedEntityColor;
        float height = TextRenderer::DrawCentered(ctx.drawList, pos, nameText, style);
        cursor.Advance(height);
    } else if (showDistance) {
        style.color = ESPColors::DEFAULT_TEXT;
        float height = TextRenderer::DrawCentered(ctx.drawList, pos, distanceText, style);
        cursor.Advance(height);
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
    std::string_view damageText(damageBuffer, len > 0 ? static_cast<size_t>(len) : 0);

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
        burstText = std::string_view(burstBuffer, len > 0 ? static_cast<size_t>(len) : 0);
    }
    else {
        len = snprintf(burstBuffer, std::size(burstBuffer), "%.0f", burstDps);
        burstText = std::string_view(burstBuffer, len > 0 ? static_cast<size_t>(len) : 0);
    }

    glm::vec2 anchorPos;
    if (renderHealthBar && healthBarPos.x != 0.0f && healthBarPos.y != 0.0f) {
        float dpsFontSize = props.style.finalFontSize * RenderingLayout::STATUS_TEXT_FONT_SIZE_MULTIPLIER;
        ImFont* font = ImGui::GetFont();
        ImVec2 dpsTextSize = font->CalcTextSizeA(dpsFontSize, FLT_MAX, 0.0f, burstText.data(), burstText.data() + burstText.size());

        float barCenterY = healthBarPos.y + props.style.finalHealthBarHeight / 2.0f;
        
        anchorPos = { 
            healthBarPos.x + props.style.finalHealthBarWidth + RenderingLayout::BURST_DPS_HORIZONTAL_PADDING, 
            barCenterY - (dpsTextSize.y / 2.0f)
        };

        bool shouldRenderHealthPercentage = RenderSettingsHelper::ShouldRenderHealthPercentage(context.settings, entityType);
        if (shouldRenderHealthPercentage && healthPercent >= 0.0f) {
            char hpBuffer[16];
            int hpLen = snprintf(hpBuffer, std::size(hpBuffer), "%d%%", static_cast<int>(healthPercent * 100.0f));
            std::string_view hpText(hpBuffer, hpLen > 0 ? static_cast<size_t>(hpLen) : 0);

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
            glm::vec2 pos = cursor.GetPosition();
            
            FastTextStyle style;
            style.fontSize = props.style.finalFontSize;
            style.shadow = ctx.settings.appearance.enableTextShadows;
            style.background = ctx.settings.appearance.enableTextBackgrounds;
            style.fadeAlpha = props.style.finalAlpha;

            switch (gearDisplayMode) {
                case GearDisplayMode::Compact: {
                    auto summary = InfoBuilder::BuildCompactGearSummary(player);
                    if (!summary.empty()) {
                        char buffer[256];
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
                        for (size_t i = 0; i < summary.size() && count < 32; ++i) {
                            const auto& info = summary[i];
                            char statBuffer[64];
                            int statLen = snprintf(statBuffer, std::size(statBuffer), "%.0f%% %s", info.percentage, info.statName.c_str());
                            std::string_view statText(statBuffer, statLen > 0 ? static_cast<size_t>(statLen) : 0);
                            
                            ImU32 rarityColor = Styling::GetRarityColor(info.highestRarity);
                            append(statText, rarityColor);
                            
                            if (i < summary.size() - 1) {
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
                    auto stats = InfoBuilder::BuildDominantStats(player);
                    if (!stats.empty()) {
                        char buffer[256];
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
                        for (size_t i = 0; i < stats.size() && count < 32; ++i) {
                            const auto& stat = stats[i];
                            char statBuffer[64];
                            int statLen = snprintf(statBuffer, std::size(statBuffer), "%s %.0f%%", stat.name.c_str(), stat.percentage);
                            std::string_view statText(statBuffer, statLen > 0 ? static_cast<size_t>(statLen) : 0);
                            
                            append(statText, stat.color);
                            
                            if (i < stats.size() - 1) {
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
                default: break;
            }
        }
    }

    if (renderDetails && !details.empty()) {
        glm::vec2 pos = cursor.GetPosition();
        float lineSpacing = RenderingLayout::DETAILS_TEXT_LINE_SPACING;
        float totalHeight = 0.0f;

        FastTextStyle style;
        style.fontSize = props.style.finalFontSize;
        style.shadow = ctx.settings.appearance.enableTextShadows;
        style.background = ctx.settings.appearance.enableTextBackgrounds;
        style.fadeAlpha = props.style.finalAlpha;

        for (const auto& detail : details) {
            std::string_view detailText = detail.text;
            style.color = detail.color;
            float height = TextRenderer::DrawCentered(ctx.drawList, pos, detailText, style);
            pos.y += height + lineSpacing;
            totalHeight += height + lineSpacing;
        }

        if (totalHeight > 0.0f) {
            cursor.Advance(totalHeight);
        }
    }
}

} // namespace kx

