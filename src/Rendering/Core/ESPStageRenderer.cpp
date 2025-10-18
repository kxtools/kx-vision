#include "ESPStageRenderer.h"
#include "../Layout/LayoutCalculator.h"
#include "../../Core/AppState.h"
#include "../../Game/Camera.h"
#include "../Utils/ESPMath.h"
#include "../Utils/ESPConstants.h"
#include "../Utils/ESPStyling.h"
#include "../Renderers/ESPShapeRenderer.h"
#include "../Renderers/ESPTextRenderer.h"
#include "../Renderers/ESPHealthBarRenderer.h"
#include "../Data/EntityRenderContext.h"
#include "../Utils/ESPFormatting.h"
#include "../../../libs/ImGui/imgui.h"
#include <sstream>
#include <iomanip>
#include "../Utils/TextElementFactory.h"
#include "../Utils/EntityVisualsCalculator.h"
#include "../Renderers/TextRenderer.h"
#include <optional>
#include <vector>
#include <string>
#include <map>
#include "../Utils/ESPPlayerDetailsBuilder.h"

namespace kx {



std::optional<VisualProperties> ESPStageRenderer::CalculateLiveVisuals(const FinalizedRenderable& item, const FrameContext& context) {
    // 1. Re-project the entity's world position to get a fresh screen position.
    glm::vec2 freshScreenPos;
    if (!ESPMath::WorldToScreen(item.entity->position, context.camera, context.screenWidth, context.screenHeight, freshScreenPos)) {
        return std::nullopt; // Cull if off-screen this frame.
    }

    // 2. Make a mutable copy of the cached visual properties.
    VisualProperties liveVisuals = item.visuals;

    // 3. Overwrite the stale screen-space properties with fresh ones.
    liveVisuals.screenPos = freshScreenPos;
    
    // 4. Recalculate derived screen-space coordinates using the cached dimensions.
    if (item.entity->entityType == ESPEntityType::Gadget) {
        liveVisuals.center = ImVec2(liveVisuals.screenPos.x, liveVisuals.screenPos.y);
        liveVisuals.boxMin = ImVec2(liveVisuals.center.x - liveVisuals.circleRadius, liveVisuals.center.y - liveVisuals.circleRadius);
        liveVisuals.boxMax = ImVec2(liveVisuals.center.x + liveVisuals.circleRadius, liveVisuals.center.y + liveVisuals.circleRadius);
    } else {
        float boxWidth = liveVisuals.boxMax.x - liveVisuals.boxMin.x;
        float boxHeight = liveVisuals.boxMax.y - liveVisuals.boxMin.y;
        
        liveVisuals.boxMin = ImVec2(liveVisuals.screenPos.x - boxWidth / 2.0f, liveVisuals.screenPos.y - boxHeight);
        liveVisuals.boxMax = ImVec2(liveVisuals.screenPos.x + boxWidth / 2.0f, liveVisuals.screenPos.y);
        liveVisuals.center = ImVec2(liveVisuals.screenPos.x, liveVisuals.screenPos.y - boxHeight / 2.0f);
    }
    
    return liveVisuals;
}

void ESPStageRenderer::RenderFrameData(const FrameContext& context, const PooledFrameRenderData& frameData) {
    for (const auto& item : frameData.finalizedEntities) {
        
        // First, perform the high-frequency update to get live visual properties for this frame.
        auto liveVisualsOpt = CalculateLiveVisuals(item, context);
        
        // If the entity is on-screen, proceed with rendering.
        if (liveVisualsOpt) {
            // Use the pre-built context from the finalized renderable.
            EntityRenderContext entityContext = item.context;
            
            // Render using the fresh visual properties.
            RenderEntityComponents(context, entityContext, *liveVisualsOpt);
        }
    }
}

void ESPStageRenderer::RenderEntityComponents(const FrameContext& context, EntityRenderContext& entityContext, const VisualProperties& props) {
    // 1. Render static elements that don't affect the layout first.
    RenderStaticElements(context, entityContext, props);

    // 2. Calculate layout using the new LayoutCalculator
    LayoutRequest request = { entityContext, props, context };
    LayoutResult layout = LayoutCalculator::CalculateLayout(request);

    // 3. Render the dynamic elements at their calculated positions.
    RenderLayoutElements(context, entityContext, props, layout);
}


void ESPStageRenderer::RenderLayoutElements(
    const FrameContext& context,
    EntityRenderContext& entityContext,
    const VisualProperties& props,
    const LayoutResult& layout)
{
    // RENDER ABOVE BOX ELEMENTS
    if (entityContext.renderDistance) {
        auto it = layout.elementPositions.find("distance");
        if (it != layout.elementPositions.end()) {
            ESPTextRenderer::RenderDistanceTextAt(context.drawList, it->second, entityContext.gameplayDistance, props.finalAlpha, props.finalFontSize);
        }
    }

    // RENDER BELOW BOX ELEMENTS
    bool isLivingEntity = (entityContext.entityType == ESPEntityType::Player || entityContext.entityType == ESPEntityType::NPC);
    bool isGadget = (entityContext.entityType == ESPEntityType::Gadget);
    float healthPercent = entityContext.entity->maxHealth > 0 ? (entityContext.entity->currentHealth / entityContext.entity->maxHealth) : -1.0f;
    if ((isLivingEntity || isGadget) && healthPercent >= 0.0f && entityContext.renderHealthBar) {
        auto it = layout.elementPositions.find("healthBar");
        if (it != layout.elementPositions.end()) {
            glm::vec2 topLeft = { it->second.x - props.finalHealthBarWidth / 2.0f, it->second.y };
            ESPHealthBarRenderer::RenderStandaloneHealthBar(context.drawList, topLeft, entityContext,
                props.fadedEntityColor, props.finalHealthBarWidth, props.finalHealthBarHeight, props.finalFontSize);
            // healthBarAnchor is already set by LayoutCalculator
        }
    }
    if (entityContext.entityType == ESPEntityType::Player) {
        const auto* player = static_cast<const RenderablePlayer*>(entityContext.entity);
        float energyPercent = -1.0f;
        if (context.settings.playerESP.energyDisplayType == EnergyDisplayType::Dodge) {
            if (player->maxEnergy > 0) {
                energyPercent = player->currentEnergy / player->maxEnergy;
            }
        } else { // Special
            if (player->maxSpecialEnergy > 0) {
                energyPercent = player->currentSpecialEnergy / player->maxSpecialEnergy;
            }
        }
        if (energyPercent >= 0.0f && entityContext.renderEnergyBar) {
        auto it = layout.elementPositions.find("energyBar");
        if (it != layout.elementPositions.end()) {
            glm::vec2 topLeft = { it->second.x - props.finalHealthBarWidth / 2.0f, it->second.y };
            ESPHealthBarRenderer::RenderStandaloneEnergyBar(context.drawList, topLeft, energyPercent,
                props.finalAlpha, props.finalHealthBarWidth, props.finalHealthBarHeight, props.finalHealthBarHeight);
        }
        }
    }

    if (entityContext.renderPlayerName && !entityContext.playerName.empty()) {
        auto it = layout.elementPositions.find("playerName");
        if (it != layout.elementPositions.end()) {
            ESPTextRenderer::RenderPlayerNameAt(context.drawList, it->second, entityContext.playerName, props.fadedEntityColor, props.finalFontSize);
        }
    }

    if (entityContext.entityType == ESPEntityType::Player) {
        const auto* player = static_cast<const RenderablePlayer*>(entityContext.entity);
        if (player != nullptr) {
        switch (context.settings.playerESP.gearDisplayMode) {
            case GearDisplayMode::Compact: {
                auto it = layout.elementPositions.find("gearSummary");
                if (it != layout.elementPositions.end()) {
                    auto summary = ESPPlayerDetailsBuilder::BuildCompactGearSummary(player);
                    ESPTextRenderer::RenderGearSummaryAt(context.drawList, it->second, summary, props.finalAlpha, props.finalFontSize);
                }
                break;
            }
            case GearDisplayMode::Attributes: {
                auto it = layout.elementPositions.find("dominantStats");
                if (it != layout.elementPositions.end()) {
                    auto stats = ESPPlayerDetailsBuilder::BuildDominantStats(player);
                    auto rarity = ESPPlayerDetailsBuilder::GetHighestRarity(player);
                    ESPTextRenderer::RenderDominantStatsAt(context.drawList, it->second, stats, rarity, props.finalAlpha, props.finalFontSize);
                }
                break;
            }
            default: break;
        }
        }
    }

    if (entityContext.renderDetails && !entityContext.details.empty()) {
        auto it = layout.elementPositions.find("details");
        if (it != layout.elementPositions.end()) {
            ESPTextRenderer::RenderDetailsTextAt(context.drawList, it->second, entityContext.details, props.finalAlpha, props.finalFontSize);
        }
    }

    // RENDER DEPENDENT ELEMENTS
    // These are not part of the stack, but are anchored to layout elements (like the health bar).
    RenderDamageNumbers(context, entityContext, props, layout);
    RenderBurstDps(context, entityContext, props, layout);
}

void ESPStageRenderer::RenderStaticElements(
    const FrameContext& context,
    const EntityRenderContext& entityContext,
    const VisualProperties& props)
{
    // Bounding Box
    if (entityContext.entityType != ESPEntityType::Gadget && entityContext.renderBox) {
        ESPShapeRenderer::RenderBoundingBox(context.drawList, props.boxMin, props.boxMax, props.fadedEntityColor, props.finalBoxThickness);
    }

    // Gadget Visuals (Sphere/Circle)
    if (entityContext.entityType == ESPEntityType::Gadget) {
        if (context.settings.objectESP.renderSphere) {
            ESPShapeRenderer::RenderGadgetSphere(context.drawList, entityContext, context.camera, props.screenPos, props.finalAlpha, props.fadedEntityColor, props.scale, context.screenWidth, context.screenHeight);
        }
        if (context.settings.objectESP.renderCircle) {
            context.drawList->AddCircle(ImVec2(props.screenPos.x, props.screenPos.y), props.circleRadius, props.fadedEntityColor, 0, props.finalBoxThickness);
        }
    }

    // Center Dot
    if (entityContext.renderDot) {
        if (entityContext.entityType == ESPEntityType::Gadget) {
            ESPShapeRenderer::RenderNaturalWhiteDot(context.drawList, props.screenPos, props.finalAlpha, props.finalDotRadius);
        } else {
            ESPShapeRenderer::RenderColoredDot(context.drawList, props.screenPos, props.fadedEntityColor, props.finalDotRadius);
        }
    }
}


void ESPStageRenderer::RenderDamageNumbers(const FrameContext& context, const EntityRenderContext& entityContext, const VisualProperties& props, const LayoutResult& layout) {
    // For gadgets, check if combat UI should be hidden for this type
    if (entityContext.entityType == ESPEntityType::Gadget) {
        const auto* gadget = static_cast<const RenderableGadget*>(entityContext.entity);
        if (gadget && ESPStyling::ShouldHideCombatUIForGadget(gadget->type)) {
            return;
        }
    }

    // Check if the setting is enabled for the specific entity type
    bool isEnabled = false;
    if (entityContext.entityType == ESPEntityType::Player) isEnabled = context.settings.playerESP.showDamageNumbers;
    else if (entityContext.entityType == ESPEntityType::NPC) isEnabled = context.settings.npcESP.showDamageNumbers;
    else if (entityContext.entityType == ESPEntityType::Gadget) isEnabled = context.settings.objectESP.showDamageNumbers;

    if (!isEnabled || entityContext.healthBarAnim.damageNumberAlpha <= 0.0f) {
        return;
    }

    // --- ANCHORING LOGIC ---
    glm::vec2 anchorPos;
    if (entityContext.renderHealthBar) {
        // If HP bar is on, anchor above it for perfect alignment.
        anchorPos = { layout.healthBarAnchor.x + props.finalHealthBarWidth / 2.0f, layout.healthBarAnchor.y - entityContext.healthBarAnim.damageNumberYOffset };
    } else {
        // FALLBACK: If HP bar is off, anchor to the entity's visual center.
        anchorPos = { props.center.x, props.center.y - entityContext.healthBarAnim.damageNumberYOffset };
    }

    // --- RENDER LOGIC (moved from ESPHealthBarRenderer) ---
    std::stringstream ss;
    ss << std::fixed << std::setprecision(0) << entityContext.healthBarAnim.damageNumberToDisplay;
    float finalFontSize = props.finalFontSize * EntityVisualsCalculator::GetDamageNumberFontSizeMultiplier(entityContext.healthBarAnim.damageNumberToDisplay);
    TextElement element = TextElementFactory::CreateDamageNumber(ss.str(), anchorPos, entityContext.healthBarAnim.damageNumberAlpha, finalFontSize);
    TextRenderer::Render(context.drawList, element);
}

void ESPStageRenderer::RenderBurstDps(const FrameContext& context, const EntityRenderContext& entityContext, const VisualProperties& props, const LayoutResult& layout) {
    // For gadgets, check if combat UI should be hidden for this type
    if (entityContext.entityType == ESPEntityType::Gadget) {
        const auto* gadget = static_cast<const RenderableGadget*>(entityContext.entity);
        if (gadget && ESPStyling::ShouldHideCombatUIForGadget(gadget->type)) {
            return;
        }
    }

    // Calculate health percentage for positioning logic
    float healthPercent = entityContext.entity->maxHealth > 0 ? (entityContext.entity->currentHealth / entityContext.entity->maxHealth) : -1.0f;

    // Check if the setting is enabled
    bool isEnabled = false;
    if (entityContext.entityType == ESPEntityType::Player) isEnabled = context.settings.playerESP.showBurstDps;
    else if (entityContext.entityType == ESPEntityType::NPC) isEnabled = context.settings.npcESP.showBurstDps;
    else if (entityContext.entityType == ESPEntityType::Gadget) isEnabled = context.settings.objectESP.showBurstDps;

    if (!isEnabled || entityContext.burstDPS <= 0.0f || entityContext.healthBarAnim.healthBarFadeAlpha <= 0.0f) {
        return;
    }

    // --- FORMATTING ---
    std::stringstream ss;
    if (entityContext.burstDPS >= 1000.0f) {
        ss << std::fixed << std::setprecision(1) << (entityContext.burstDPS / 1000.0f) << "k";
    }
    else {
        ss << std::fixed << std::setprecision(0) << entityContext.burstDPS;
    }

    // --- ANCHORING LOGIC ---
    glm::vec2 anchorPos;
    if (entityContext.renderHealthBar) {
        // Base anchor is vertically centered on the bar.
        anchorPos = { layout.healthBarAnchor.x + props.finalHealthBarWidth + 5.0f, layout.healthBarAnchor.y + props.finalHealthBarHeight / 2.0f };

        // If the HP% text is also being rendered, calculate its width and add it to our offset.
        if (entityContext.renderHealthPercentage && healthPercent >= 0.0f) {
            std::string hpText = std::to_string(static_cast<int>(healthPercent * 100.0f));

            // Calculate the size of the HP text using the same font size it will be rendered with.
            float hpFontSize = props.finalFontSize * 0.8f;
            ImFont* font = ImGui::GetFont();
            ImVec2 hpTextSize = font->CalcTextSizeA(hpFontSize, FLT_MAX, 0.0f, hpText.c_str());

            // Add the width of the HP text plus another padding amount to the total offset.
            anchorPos.x += hpTextSize.x + 5.0f; // HP Text Width + 5px padding
        }

    }
    else {
        // FALLBACK: If HP bar is off, anchor below the entity's name/details.
        anchorPos = { props.screenPos.x, props.screenPos.y + 20.0f }; // Adjust Y-offset as needed
    }

    // --- RENDER LOGIC ---
    TextElement element(ss.str(), anchorPos, TextAnchor::Custom);
    element.SetAlignment(TextAlignment::Left);
    TextStyle style = TextElementFactory::GetDistanceStyle(entityContext.healthBarAnim.healthBarFadeAlpha, props.finalFontSize * 0.9f);
    style.enableBackground = false;
    style.textColor = IM_COL32(255, 200, 50, 255); // A distinct gold/yellow for DPS
    element.SetStyle(style);
    TextRenderer::Render(context.drawList, element);
}

} // namespace kx
