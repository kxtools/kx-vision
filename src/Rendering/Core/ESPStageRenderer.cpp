#include "ESPStageRenderer.h"
#include "../Layout/LayoutCalculator.h"
#include "../Layout/LayoutElementKeys.h"
#include "../../Core/AppState.h"
#include "../../Game/Camera.h"
#include "../Utils/ESPMath.h"
#include "../Utils/ESPStyling.h"
#include "../Utils/CombatConstants.h"
#include "../Utils/LayoutConstants.h"
#include "../Renderers/ESPShapeRenderer.h"
#include "../Renderers/ESPTextRenderer.h"
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
#include "../Utils/ESPPlayerDetailsBuilder.h"

namespace kx {

namespace {
    // Helper function to calculate energy percentage for players
    float CalculateEnergyPercent(const RenderablePlayer* player, EnergyDisplayType displayType) {
        if (displayType == EnergyDisplayType::Dodge) {
            if (player->maxEnergy > 0) {
                return player->currentEnergy / player->maxEnergy;
            }
        } else { // Special
            if (player->maxSpecialEnergy > 0) {
                return player->currentSpecialEnergy / player->maxSpecialEnergy;
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
    if (entityContext.renderDistance && layout.HasElement(LayoutElementKey::Distance)) {
        glm::vec2 position = layout.GetElementPosition(LayoutElementKey::Distance);
        ESPTextRenderer::RenderDistanceTextAt(context.drawList, position, entityContext.gameplayDistance, props.finalAlpha, props.finalFontSize);
    }

    // RENDER BELOW BOX ELEMENTS using helper functions
    RenderStatusBars(context, entityContext, props, layout);
    RenderPlayerIdentity(context, entityContext, props, layout);
    RenderEntityDetails(context, entityContext, props, layout);

    // RENDER DEPENDENT ELEMENTS
    // These are not part of the stack, but are anchored to layout elements (like the health bar).
    RenderDamageNumbers(context, entityContext, props, layout);
    RenderBurstDps(context, entityContext, props, layout);
}

void ESPStageRenderer::RenderStatusBars(
    const FrameContext& context,
    EntityRenderContext& entityContext,
    const VisualProperties& props,
    const LayoutResult& layout)
{
    // Health Bar
    bool isLivingEntity = (entityContext.entityType == ESPEntityType::Player || entityContext.entityType == ESPEntityType::NPC);
    bool isGadget = (entityContext.entityType == ESPEntityType::Gadget || entityContext.entityType == ESPEntityType::AttackTarget);
    float healthPercent = entityContext.entity->maxHealth > 0 ? (entityContext.entity->currentHealth / entityContext.entity->maxHealth) : -1.0f;
    if ((isLivingEntity || isGadget) && healthPercent >= 0.0f && entityContext.renderHealthBar && layout.HasElement(LayoutElementKey::HealthBar)) {
        glm::vec2 position = layout.GetElementPosition(LayoutElementKey::HealthBar);
        glm::vec2 topLeft = { position.x - props.finalHealthBarWidth / 2.0f, position.y };
        ESPHealthBarRenderer::RenderStandaloneHealthBar(context.drawList, topLeft, entityContext,
            props.fadedEntityColor, props.finalHealthBarWidth, props.finalHealthBarHeight, props.finalFontSize);
    }

    // Energy Bar (Players only)
    if (entityContext.entityType == ESPEntityType::Player) {
        const auto* player = static_cast<const RenderablePlayer*>(entityContext.entity);
        float energyPercent = CalculateEnergyPercent(player, entityContext.playerEnergyDisplayType);
        if (energyPercent >= 0.0f && entityContext.renderEnergyBar && layout.HasElement(LayoutElementKey::EnergyBar)) {
            glm::vec2 position = layout.GetElementPosition(LayoutElementKey::EnergyBar);
            glm::vec2 topLeft = { position.x - props.finalHealthBarWidth / 2.0f, position.y };
            ESPHealthBarRenderer::RenderStandaloneEnergyBar(context.drawList, topLeft, energyPercent,
                props.finalAlpha, props.finalHealthBarWidth, props.finalHealthBarHeight, props.finalHealthBarHeight);
        }
    }
}

void ESPStageRenderer::RenderPlayerIdentity(
    const FrameContext& context,
    const EntityRenderContext& entityContext,
    const VisualProperties& props,
    const LayoutResult& layout)
{
    // Player Name (fallback to profession if name is empty)
    if (entityContext.renderPlayerName) {
        std::string displayName = entityContext.playerName;
        
        // Fallback to profession for hostile players without names (WvW)
        if (displayName.empty() && entityContext.entityType == ESPEntityType::Player) {
            const auto* player = static_cast<const RenderablePlayer*>(entityContext.entity);
            if (player != nullptr) {
                const char* profName = ESPFormatting::GetProfessionName(player->profession);
                if (profName != nullptr) {
                    displayName = profName;
                }
            }
        }
        
        if (!displayName.empty() && layout.HasElement(LayoutElementKey::PlayerName)) {
            glm::vec2 position = layout.GetElementPosition(LayoutElementKey::PlayerName);
            ESPTextRenderer::RenderPlayerNameAt(context.drawList, position, displayName, props.fadedEntityColor, props.finalFontSize);
        }
    }

    // Player Gear (Players only)
    if (entityContext.entityType == ESPEntityType::Player) {
        const auto* player = static_cast<const RenderablePlayer*>(entityContext.entity);
        if (player != nullptr) {
            switch (entityContext.playerGearDisplayMode) {
                case GearDisplayMode::Compact: {
                    if (layout.HasElement(LayoutElementKey::GearSummary)) {
                        glm::vec2 position = layout.GetElementPosition(LayoutElementKey::GearSummary);
                        auto summary = ESPPlayerDetailsBuilder::BuildCompactGearSummary(player);
                        ESPTextRenderer::RenderGearSummaryAt(context.drawList, position, summary, props.finalAlpha, props.finalFontSize);
                    }
                    break;
                }
                case GearDisplayMode::Attributes: {
                    if (layout.HasElement(LayoutElementKey::DominantStats)) {
                        glm::vec2 position = layout.GetElementPosition(LayoutElementKey::DominantStats);
                        auto stats = ESPPlayerDetailsBuilder::BuildDominantStats(player);
                        auto rarity = ESPPlayerDetailsBuilder::GetHighestRarity(player);
                        ESPTextRenderer::RenderDominantStatsAt(context.drawList, position, stats, rarity, props.finalAlpha, props.finalFontSize);
                    }
                    break;
                }
                default: break;
            }
        }
    }
}

void ESPStageRenderer::RenderEntityDetails(
    const FrameContext& context,
    const EntityRenderContext& entityContext,
    const VisualProperties& props,
    const LayoutResult& layout)
{
    // Entity Details
    if (entityContext.renderDetails && !entityContext.details.empty() && layout.HasElement(LayoutElementKey::Details)) {
        glm::vec2 position = layout.GetElementPosition(LayoutElementKey::Details);
        ESPTextRenderer::RenderDetailsTextAt(context.drawList, position, entityContext.details, props.finalAlpha, props.finalFontSize);
    }
}

void ESPStageRenderer::RenderStaticElements(
    const FrameContext& context,
    const EntityRenderContext& entityContext,
    const VisualProperties& props)
{
    // Bounding Box
    if (entityContext.renderBox) {
        ESPShapeRenderer::RenderBoundingBox(context.drawList, props.boxMin, props.boxMax, props.fadedEntityColor, props.finalBoxThickness);
    }

    // 3D Wireframe Box
    if (entityContext.renderWireframe) {
        ESPShapeRenderer::RenderWireframeBox(context.drawList, props, props.fadedEntityColor, props.finalBoxThickness);
    }

    // Gadget Visuals (Sphere/Circle)
    if (entityContext.entityType == ESPEntityType::Gadget || entityContext.entityType == ESPEntityType::AttackTarget) {
        if (entityContext.renderGadgetSphere) {
            ESPShapeRenderer::RenderGadgetSphere(context.drawList, entityContext, context.camera, props.screenPos, props.finalAlpha, props.fadedEntityColor, props.scale, context.screenWidth, context.screenHeight);
        }
        if (entityContext.renderGadgetCircle) {
            ESPShapeRenderer::RenderGadgetCircle(context.drawList, props.screenPos, props.circleRadius, props.fadedEntityColor, props.finalBoxThickness);
        }
    }

    // Center Dot
    if (entityContext.renderDot) {
        if (entityContext.entityType == ESPEntityType::Gadget || entityContext.entityType == ESPEntityType::AttackTarget) {
            ESPShapeRenderer::RenderNaturalWhiteDot(context.drawList, props.screenPos, props.finalAlpha, props.finalDotRadius);
        } else {
            ESPShapeRenderer::RenderColoredDot(context.drawList, props.screenPos, props.fadedEntityColor, props.finalDotRadius);
        }
    }
}

void ESPStageRenderer::RenderDamageNumbers(const FrameContext& context, const EntityRenderContext& entityContext, const VisualProperties& props, const LayoutResult& layout) {
    // Check if combat UI should be shown and damage numbers are enabled
    if (!entityContext.showCombatUI || !entityContext.showDamageNumbers || entityContext.healthBarAnim.damageNumberAlpha <= 0.0f) {
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
    // Critical: Check if ImGui context is still valid before any ImGui operations
    if (!ImGui::GetCurrentContext()) return;
    
    // Check if combat UI should be shown and burst DPS is enabled
    if (!entityContext.showCombatUI || !entityContext.showBurstDps || entityContext.burstDPS <= 0.0f || entityContext.healthBarAnim.healthBarFadeAlpha <= 0.0f) {
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
    if (entityContext.renderHealthBar) {
        // Base anchor is vertically centered on the bar.
        anchorPos = { layout.healthBarAnchor.x + props.finalHealthBarWidth + RenderingLayout::BURST_DPS_HORIZONTAL_PADDING, layout.healthBarAnchor.y + props.finalHealthBarHeight / 2.0f };

        // If the HP% text is also being rendered, calculate its width and add it to our offset.
        if (entityContext.renderHealthPercentage && healthPercent >= 0.0f) {
            std::string hpText = std::to_string(static_cast<int>(healthPercent * 100.0f));

            // Calculate the size of the HP text using the same font size it will be rendered with.
            float hpFontSize = props.finalFontSize * RenderingLayout::HP_PERCENT_FONT_SIZE_MULTIPLIER;
            ImFont* font = ImGui::GetFont();
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
    TextStyle style = TextElementFactory::GetDistanceStyle(entityContext.healthBarAnim.healthBarFadeAlpha, props.finalFontSize * CombatEffects::DPS_FONT_SIZE_MULTIPLIER);
    style.enableBackground = false;
    style.textColor = ESPBarColors::BURST_DPS_TEXT;
    element.SetStyle(style);
    TextRenderer::Render(context.drawList, element);
}

} // namespace kx
