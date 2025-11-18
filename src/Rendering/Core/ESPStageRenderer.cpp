#include "ESPStageRenderer.h"
#include "../Layout/LayoutCursor.h"
#include "../../Core/AppState.h"
#include "../../Game/Camera.h"
#include "../Utils/ESPMath.h"
#include "../Utils/ESPStyling.h"
#include "../Utils/CombatConstants.h"
#include "../Utils/LayoutConstants.h"
#include "../Renderers/ESPShapeRenderer.h"
#include "../Renderers/ESPHealthBarRenderer.h"
#include "../Renderers/ESPTrailRenderer.h"
#include "../Data/EntityRenderContext.h"
#include "../../../libs/ImGui/imgui.h"
#include <sstream>
#include <iomanip>
#include "../Utils/TextElementFactory.h"
#include "../Utils/EntityVisualsCalculator.h"
#include "../Utils/ESPFormatting.h"
#include "../Renderers/TextRenderer.h"
#include <optional>
#include <vector>
#include <string>
#include <map>
#include "../Utils/ESPInfoBuilder.h"
#include "../Utils/RenderSettingsHelper.h"

namespace kx {

namespace {
    // Helper function to calculate energy percentage for players
    float CalculateEnergyPercent(const RenderablePlayer* player, EnergyDisplayType displayType) {
        if (displayType == EnergyDisplayType::Endurance) {
            if (player->maxEndurance > 0) {
                return player->currentEndurance / player->maxEndurance;
            }
        } else { // Special
            if (player->maxEnergy > 0) {
                return player->currentEnergy / player->maxEnergy;
            }
        }
        return -1.0f;
    }
}

std::optional<VisualProperties> ESPStageRenderer::CalculateLiveVisuals(const FinalizedRenderable& item, const FrameContext& context) {
    // 1. Start with the pre-calculated, non-geometric properties from the low-frequency update.
    VisualProperties liveVisuals = item.visuals;

    // 2. Determine the entity's 3D world-space dimensions for the bounding box.
    float worldWidth, worldDepth, worldHeight;
    if (item.entity->hasPhysicsDimensions) {
        worldWidth = item.entity->physicsWidth;
        worldDepth = item.entity->physicsDepth;
        worldHeight = item.entity->physicsHeight;
    } else {
        // Fallback to constants for entities without physics data.
        EntityVisualsCalculator::GetWorldBoundsForEntity(item.entity->entityType, worldWidth, worldDepth, worldHeight);
    }
    
    // 3. Project the 8 corners of the 3D world-space box into 2D screen space.
    // This is the core logic that ensures perspective correctness.
    bool isProjectionValid = false;
    EntityVisualsCalculator::Calculate3DBoundingBox(
        item.entity->position,
        worldWidth,
        worldDepth,
        worldHeight,
        context.camera,
        context.screenWidth,
        context.screenHeight,
        liveVisuals,
        isProjectionValid
    );

    // If no corners could be validly projected (e.g., entity is entirely behind the camera), cull it.
    if (!isProjectionValid) {
        return std::nullopt;
    }

    // 4. Perform the final, high-frequency culling check.
    // An entity is visible if its TRUE projected 2D box overlaps the screen area.
    bool overlapsX = liveVisuals.boxMin.x < context.screenWidth && liveVisuals.boxMax.x > 0;
    bool overlapsY = liveVisuals.boxMin.y < context.screenHeight && liveVisuals.boxMax.y > 0;

    if (!overlapsX || !overlapsY) {
        return std::nullopt;
    }

    // 5. Finalize remaining screen-space properties based on the new, correct box.
    // Project the origin point separately for anchoring UI elements.
    if (!ESPMath::ProjectToScreen(item.entity->position, context.camera, context.screenWidth, context.screenHeight, liveVisuals.screenPos)) {
        // If origin point is behind camera, use center of projected box
        liveVisuals.screenPos.x = (liveVisuals.boxMin.x + liveVisuals.boxMax.x) * 0.5f;
        liveVisuals.screenPos.y = (liveVisuals.boxMin.y + liveVisuals.boxMax.y) * 0.5f;
    }
    
    // Calculate the visual center of the final projected box.
    liveVisuals.center = ImVec2(
        (liveVisuals.boxMin.x + liveVisuals.boxMax.x) * 0.5f,
        (liveVisuals.boxMin.y + liveVisuals.boxMax.y) * 0.5f
    );
    
    // 6. Calculate gadget circle radius if needed (scale-based, so can be calculated here)
    if (item.entity->entityType == ESPEntityType::Gadget || item.entity->entityType == ESPEntityType::AttackTarget) {
        const auto& settings = AppState::Get().GetSettings();
        float baseRadius = settings.sizes.baseBoxWidth * EntitySizeRatios::GADGET_CIRCLE_RADIUS_RATIO;
        liveVisuals.circleRadius = (std::max)(MinimumSizes::GADGET_MIN_WIDTH / 2.0f, baseRadius * liveVisuals.scale);
    } else {
        liveVisuals.circleRadius = 0.0f; // No circle for players/NPCs
    }
    
    // Return the complete, correct, and visible properties for this frame.
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
            
            // Render movement trail for players
            if (entityContext.entityType == ESPEntityType::Player) {
                ESPTrailRenderer::RenderPlayerTrail(context, entityContext, *liveVisualsOpt);
            }
        }
    }
}

void ESPStageRenderer::RenderEntityComponents(const FrameContext& context, EntityRenderContext& entityContext, const VisualProperties& props) {
    // 1. Render static elements that don't affect the layout first.
    // Bounding Box
    bool shouldRenderBox = RenderSettingsHelper::ShouldRenderBox(context.settings, entityContext.entityType);
    float entityHeight = entityContext.entity->hasPhysicsDimensions ? entityContext.entity->physicsHeight : 0.0f;
    if (shouldRenderBox && RenderSettingsHelper::IsBoxAllowedForSize(context.settings, entityContext.entityType, entityHeight)) {
        ESPShapeRenderer::RenderBoundingBox(context.drawList, props.boxMin, props.boxMax, props.fadedEntityColor, props.finalBoxThickness);
    }

    // 3D Wireframe Box
    bool shouldRenderWireframe = RenderSettingsHelper::ShouldRenderWireframe(context.settings, entityContext.entityType);
    if (shouldRenderWireframe && RenderSettingsHelper::IsBoxAllowedForSize(context.settings, entityContext.entityType, entityHeight)) {
        ESPShapeRenderer::RenderWireframeBox(context.drawList, props, props.fadedEntityColor, props.finalBoxThickness);
    }

    // Gadget Visuals (Sphere/Circle)
    if (entityContext.entityType == ESPEntityType::Gadget || entityContext.entityType == ESPEntityType::AttackTarget) {
        if (RenderSettingsHelper::ShouldRenderGadgetSphere(context.settings, entityContext.entityType)) {
            ESPShapeRenderer::RenderGadgetSphere(context.drawList, entityContext, context.camera, props.screenPos, props.finalAlpha, props.fadedEntityColor, props.scale, context.screenWidth, context.screenHeight);
        }
        if (RenderSettingsHelper::ShouldRenderGadgetCircle(context.settings, entityContext.entityType)) {
            ESPShapeRenderer::RenderGadgetCircle(context.drawList, props.screenPos, props.circleRadius, props.fadedEntityColor, props.finalBoxThickness);
        }
    }

    // Center Dot
    if (RenderSettingsHelper::ShouldRenderDot(context.settings, entityContext.entityType)) {
        if (entityContext.entityType == ESPEntityType::Gadget || entityContext.entityType == ESPEntityType::AttackTarget) {
            ESPShapeRenderer::RenderNaturalWhiteDot(context.drawList, props.screenPos, props.finalAlpha, props.finalDotRadius);
        } else {
            ESPShapeRenderer::RenderColoredDot(context.drawList, props.screenPos, props.fadedEntityColor, props.finalDotRadius);
        }
    }

    // 2. Initialize Bottom Stack Cursor
    // Bottom Stack: Starts at Box Max (Feet), grows Downwards (+1)
    LayoutCursor bottomStack({props.center.x, props.boxMax.y}, 1.0f);
    
    // Handle Gadgets (Center Anchor fallback)
    if (entityContext.entityType == ESPEntityType::Gadget && !shouldRenderBox) {
        // If no box, stack everything below the center point
        bottomStack = LayoutCursor(glm::vec2(props.screenPos.x, props.screenPos.y), 1.0f);
    }

    // ------------------------------------------------
    // BOTTOM STACK (Name/Distance, Bars, Gear, Details)
    // ------------------------------------------------
    
    glm::vec2 healthBarPos(0.0f, 0.0f);
    bool hasHealthBar = false;

    // 0. Name/Distance (first in bottom stack)
    bool showName = RenderSettingsHelper::ShouldRenderName(context.settings, entityContext.entityType);
    bool showDistance = RenderSettingsHelper::ShouldRenderDistance(context.settings, entityContext.entityType);

    if (showName || showDistance) {
        // Create temporary request struct just for the factory
        LayoutRequest tempRequest = { entityContext, props, context };
        TextElement identity = TextElementFactory::CreateIdentityLine(tempRequest, showName, showDistance);
        
        // Update Anchor/Alignment based on cursor (Below positioning for bottom stack)
        identity.SetAnchor(bottomStack.GetPosition());
        identity.SetPositioning(TextAnchor::Below);
        identity.SetAlignment(TextAlignment::Center);
        
        // Render & Advance
        ImVec2 size = TextRenderer::Render(context.drawList, identity);
        bottomStack.Advance(size.y);
    }

    // 1. Health Bar
    bool isLiving = (entityContext.entityType == ESPEntityType::Player || entityContext.entityType == ESPEntityType::NPC);
    bool isGadget = (entityContext.entityType == ESPEntityType::Gadget || entityContext.entityType == ESPEntityType::AttackTarget);
    
    if ((isLiving || isGadget) && entityContext.renderHealthBar) {
        float healthPercent = entityContext.entity->maxHealth > 0 ? (entityContext.entity->currentHealth / entityContext.entity->maxHealth) : -1.0f;
        if (healthPercent >= 0.0f) {
            // Calculate Top-Left position using the cursor
            healthBarPos = bottomStack.GetTopLeftForBar(props.finalHealthBarWidth, props.finalHealthBarHeight);
            hasHealthBar = true;
            
            ESPHealthBarRenderer::RenderStandaloneHealthBar(context.drawList, healthBarPos, entityContext,
                props.fadedEntityColor, props.finalHealthBarWidth, props.finalHealthBarHeight, props.finalFontSize);

            // Render Damage Numbers / Burst DPS relative to the bar we just drew
            RenderDamageNumbers(context, entityContext, props, healthBarPos);
            RenderBurstDps(context, entityContext, props, healthBarPos);

            bottomStack.Advance(props.finalHealthBarHeight);
        }
    } else {
        // If no HP bar, render DPS floating near center
        glm::vec2 centerPos(props.center.x, props.center.y);
        RenderDamageNumbers(context, entityContext, props, centerPos);
        RenderBurstDps(context, entityContext, props, centerPos);
    }

    // 2. Energy Bar
    if (entityContext.entityType == ESPEntityType::Player) {
        const auto* player = static_cast<const RenderablePlayer*>(entityContext.entity);
        EnergyDisplayType energyDisplayType = RenderSettingsHelper::GetPlayerEnergyDisplayType(context.settings);
        float energyPercent = CalculateEnergyPercent(player, energyDisplayType);
        if (energyPercent >= 0.0f && entityContext.renderEnergyBar) {
            glm::vec2 barPos = bottomStack.GetTopLeftForBar(props.finalHealthBarWidth, props.finalHealthBarHeight);
            ESPHealthBarRenderer::RenderStandaloneEnergyBar(context.drawList, barPos, energyPercent,
                props.finalAlpha, props.finalHealthBarWidth, props.finalHealthBarHeight, props.finalHealthBarHeight);
            bottomStack.Advance(props.finalHealthBarHeight);
        }
    }

    // 3. Gear / Details
    // Player Gear (Players only)
    if (entityContext.entityType == ESPEntityType::Player) {
        const auto* player = static_cast<const RenderablePlayer*>(entityContext.entity);
        if (player != nullptr) {
            GearDisplayMode gearDisplayMode = RenderSettingsHelper::GetPlayerGearDisplayMode(context.settings);
            switch (gearDisplayMode) {
                case GearDisplayMode::Compact: {
                    auto summary = ESPInfoBuilder::BuildCompactGearSummary(player);
                    if (!summary.empty()) {
                        TextElement gearElement = TextElementFactory::CreateGearSummaryAt(summary, bottomStack.GetPosition(), props.finalAlpha, props.finalFontSize);
                        ImVec2 size = TextRenderer::Render(context.drawList, gearElement);
                        bottomStack.Advance(size.y);
                    }
                    break;
                }
                case GearDisplayMode::Attributes: {
                    auto stats = ESPInfoBuilder::BuildDominantStats(player);
                    auto rarity = ESPInfoBuilder::GetHighestRarity(player);
                    if (!stats.empty()) {
                        TextElement statsElement = TextElementFactory::CreateDominantStatsAt(stats, rarity, bottomStack.GetPosition(), props.finalAlpha, props.finalFontSize);
                        ImVec2 size = TextRenderer::Render(context.drawList, statsElement);
                        bottomStack.Advance(size.y);
                    }
                    break;
                }
                default: break;
            }
        }
    }

    // Entity Details
    if (entityContext.renderDetails && !entityContext.details.empty()) {
        TextElement detailsElement = TextElementFactory::CreateDetailsTextAt(entityContext.details, bottomStack.GetPosition(), props.finalAlpha, props.finalFontSize);
        ImVec2 size = TextRenderer::Render(context.drawList, detailsElement);
        bottomStack.Advance(size.y);
    }
}

void ESPStageRenderer::RenderDamageNumbers(const FrameContext& context, const EntityRenderContext& entityContext, const VisualProperties& props, const glm::vec2& healthBarPos) {
    // Check if combat UI should be shown and damage numbers are enabled
    bool shouldShowDamageNumbers = RenderSettingsHelper::ShouldShowDamageNumbers(context.settings, entityContext.entityType);
    if (!entityContext.showCombatUI || !shouldShowDamageNumbers || entityContext.healthBarAnim.damageNumberAlpha <= 0.0f) {
        return;
    }

    // --- ANCHORING LOGIC ---
    glm::vec2 anchorPos;
    if (entityContext.renderHealthBar && healthBarPos.x != 0.0f && healthBarPos.y != 0.0f) {
        // If HP bar is on, anchor above it for perfect alignment.
        anchorPos = { healthBarPos.x + props.finalHealthBarWidth / 2.0f, healthBarPos.y - entityContext.healthBarAnim.damageNumberYOffset };
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

void ESPStageRenderer::RenderBurstDps(const FrameContext& context, const EntityRenderContext& entityContext, const VisualProperties& props, const glm::vec2& healthBarPos) {
    // Critical: Check if ImGui context is still valid before any ImGui operations
    if (!ImGui::GetCurrentContext()) return;
    
    // Check if combat UI should be shown and burst DPS is enabled
    bool shouldShowBurstDps = RenderSettingsHelper::ShouldShowBurstDps(context.settings, entityContext.entityType);
    if (!entityContext.showCombatUI || !shouldShowBurstDps || entityContext.burstDPS <= 0.0f || entityContext.healthBarAnim.healthBarFadeAlpha <= 0.0f) {
        return;
    }

    // Calculate health percentage for positioning logic
    float healthPercent = entityContext.entity->maxHealth > 0 ? (entityContext.entity->currentHealth / entityContext.entity->maxHealth) : -1.0f;

    // --- FORMATTING ---
    std::stringstream ss;
    if (entityContext.burstDPS >= CombatEffects::DPS_FORMATTING_THRESHOLD) {
        ss << std::fixed << std::setprecision(1) << (entityContext.burstDPS / CombatEffects::DPS_FORMATTING_THRESHOLD) << "k";
    }
    else {
        ss << std::fixed << std::setprecision(0) << entityContext.burstDPS;
    }

    // --- ANCHORING LOGIC ---
    glm::vec2 anchorPos;
    if (entityContext.renderHealthBar && healthBarPos.x != 0.0f && healthBarPos.y != 0.0f) {
        // Calculate the DPS text height for proper vertical centering
        float dpsFontSize = props.finalFontSize * RenderingLayout::STATUS_TEXT_FONT_SIZE_MULTIPLIER;
        ImFont* font = ImGui::GetFont();
        ImVec2 dpsTextSize = font->CalcTextSizeA(dpsFontSize, FLT_MAX, 0.0f, ss.str().c_str());

        // Calculate the bar's center Y position
        float barCenterY = healthBarPos.y + props.finalHealthBarHeight / 2.0f;
        
        // Base anchor is horizontally positioned, vertically centered on the bar
        anchorPos = { 
            healthBarPos.x + props.finalHealthBarWidth + RenderingLayout::BURST_DPS_HORIZONTAL_PADDING, 
            barCenterY - (dpsTextSize.y / 2.0f) // Adjust for text height to achieve perfect centering
        };

        // If the HP% text is also being rendered, calculate its width and add it to our offset.
        bool shouldRenderHealthPercentage = RenderSettingsHelper::ShouldRenderHealthPercentage(context.settings, entityContext.entityType);
        if (shouldRenderHealthPercentage && healthPercent >= 0.0f) {
            std::string hpText = std::to_string(static_cast<int>(healthPercent * 100.0f)) + "%";

            // Calculate the size of the HP text using the same font size it will be rendered with.
            float hpFontSize = props.finalFontSize * RenderingLayout::STATUS_TEXT_FONT_SIZE_MULTIPLIER;
            ImVec2 hpTextSize = font->CalcTextSizeA(hpFontSize, FLT_MAX, 0.0f, hpText.c_str());

            // Add the width of the HP text plus another padding amount to the total offset.
            anchorPos.x += hpTextSize.x + RenderingLayout::BURST_DPS_HORIZONTAL_PADDING; // HP Text Width + padding
        }

    }
    else {
        // FALLBACK: If HP bar is off, anchor below the entity's name/details.
        anchorPos = { props.screenPos.x, props.screenPos.y + RenderingLayout::BURST_DPS_FALLBACK_Y_OFFSET };
    }

    // --- RENDER LOGIC ---
    TextElement element(ss.str(), anchorPos, TextAnchor::Custom);
    element.SetAlignment(TextAlignment::Left);
    TextStyle style = TextElementFactory::GetDistanceStyle(entityContext.healthBarAnim.healthBarFadeAlpha, props.finalFontSize * RenderingLayout::STATUS_TEXT_FONT_SIZE_MULTIPLIER);
    style.enableBackground = false;
    style.textColor = ESPBarColors::BURST_DPS_TEXT;
    element.SetStyle(style);
    TextRenderer::Render(context.drawList, element);
}

} // namespace kx
