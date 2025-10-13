#include "ESPStageRenderer.h"
#include "../Renderers/ESPContextFactory.h"
#include "../../Core/AppState.h"
#include "../../Game/Camera.h"
#include "../Utils/ESPMath.h"
#include "../Utils/ESPConstants.h"
#include "../Utils/ESPPlayerDetailsBuilder.h"
#include "../Utils/ESPEntityDetailsBuilder.h"
#include "../Renderers/ESPShapeRenderer.h"
#include "../Renderers/ESPTextRenderer.h"
#include "../Renderers/ESPHealthBarRenderer.h"
#include "../Combat/CombatStateManager.h"
#include "../Data/EntityRenderContext.h"
#include "../Utils/ESPFormatting.h"
#include "../../../libs/ImGui/imgui.h"

namespace kx {

// Helper function to build the render context.
// This consolidates the logic from the old RenderPooledPlayers/Npcs/Gadgets functions.
static EntityRenderContext CreateEntityRenderContextForRendering(const RenderableEntity* entity, const FrameContext& context) {
    std::vector<ColoredDetail> details;
    // Use a switch on entity->entityType to call the correct details builder
    switch(entity->entityType) {
        case ESPEntityType::Player:
        {
            const auto* player = static_cast<const RenderablePlayer*>(entity);
            details = ESPPlayerDetailsBuilder::BuildPlayerDetails(player, context.settings.playerESP, context.settings.showDebugAddresses);
            if (context.settings.playerESP.gearDisplayMode == GearDisplayMode::Detailed) {
                auto gearDetails = ESPPlayerDetailsBuilder::BuildGearDetails(player);
                if (!gearDetails.empty()) {
                    if (!details.empty()) {
                        details.push_back({ "--- Gear Stats ---", ESPColors::DEFAULT_TEXT });
                    }
                    details.insert(details.end(), gearDetails.begin(), gearDetails.end());
                }
            }
            break;
        }
        case ESPEntityType::NPC:
        {
            const auto* npc = static_cast<const RenderableNpc*>(entity);
            details = ESPEntityDetailsBuilder::BuildNpcDetails(npc, context.settings.npcESP, context.settings.showDebugAddresses);
            break;
        }
        case ESPEntityType::Gadget:
        {
            const auto* gadget = static_cast<const RenderableGadget*>(entity);
            details = ESPEntityDetailsBuilder::BuildGadgetDetails(gadget, context.settings.objectESP, context.settings.showDebugAddresses);
            break;
        }
    }

    // Now, create the context using the ESPContextFactory, just like before.
    // We pass the main 'context' directly.
    switch(entity->entityType) {
        case ESPEntityType::Player:
            return ESPContextFactory::CreateContextForPlayer(static_cast<const RenderablePlayer*>(entity), details, context);
        case ESPEntityType::NPC:
            return ESPContextFactory::CreateContextForNpc(static_cast<const RenderableNpc*>(entity), details, context);
        case ESPEntityType::Gadget:
            return ESPContextFactory::CreateContextForGadget(static_cast<const RenderableGadget*>(entity), details, context);
    }
    // This should not be reached, but we need to return something.
    // Returning a gadget context as a fallback.
    return ESPContextFactory::CreateContextForGadget(static_cast<const RenderableGadget*>(entity), details, context);
}


void ESPStageRenderer::RenderFrameData(const FrameContext& context, const PooledFrameRenderData& frameData) {
    // This loop now runs every frame.
    for (const auto& item : frameData.finalizedEntities) {
        
        // --- HIGH-FREQUENCY UPDATE ---
        // 1. Re-project the entity's world position to get a fresh screen position.
        glm::vec2 freshScreenPos;
        if (!ESPMath::WorldToScreen(item.entity->position, context.camera, context.screenWidth, context.screenHeight, freshScreenPos)) {
            continue; // Cull if off-screen this frame.
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
        
        // --- END OF HIGH-FREQUENCY UPDATE ---

        // Create the cheap render context.
        EntityRenderContext entityContext = CreateEntityRenderContextForRendering(item.entity, context);
        
        // Render using the LIVE visual properties, which are perfectly in sync with the camera.
        RenderEntityComponents(context, entityContext, liveVisuals);
    }
}

void ESPStageRenderer::RenderEntityComponents(const FrameContext& context, const EntityRenderContext& entityContext, const VisualProperties& props) {
    // The logic inside this function remains almost identical.
    // It just uses the `props` passed into it instead of calculating them.
    RenderHealthBar(context.drawList, entityContext, props, context.settings);
    RenderEnergyBar(context.drawList, entityContext, props, context.settings);
    RenderBoundingBox(context.drawList, entityContext, props);
    RenderGadgetVisuals(context.drawList, entityContext, context.camera, props, context.settings);
    RenderDistanceText(context.drawList, entityContext, props);
    RenderCenterDot(context.drawList, entityContext, props);
    RenderPlayerName(context.drawList, entityContext, props);
    RenderDetailsText(context.drawList, entityContext, props);
    RenderGearSummary(context.drawList, entityContext, props, context.settings);
}


// Component rendering implementations
void ESPStageRenderer::RenderHealthBar(ImDrawList* drawList, const EntityRenderContext& context, const VisualProperties& props, const Settings& settings) {
    bool isLivingEntity = (context.entityType == ESPEntityType::Player || context.entityType == ESPEntityType::NPC);
    bool isGadget = (context.entityType == ESPEntityType::Gadget);

    // Render standalone health bars for living entities when health is available AND setting is enabled
    if ((isLivingEntity || isGadget) && context.healthPercent >= 0.0f && context.renderHealthBar) {
        ESPHealthBarRenderer::RenderStandaloneHealthBar(drawList, props.screenPos, context,
            props.fadedEntityColor, props.finalHealthBarWidth, props.finalHealthBarHeight);
    }
}

void ESPStageRenderer::RenderEnergyBar(ImDrawList* drawList, const EntityRenderContext& context, const VisualProperties& props, const Settings& settings) {
    // Render energy bar for players
    if (context.entityType == ESPEntityType::Player && context.energyPercent >= 0.0f && context.renderEnergyBar) {
        ESPHealthBarRenderer::RenderStandaloneEnergyBar(drawList, props.screenPos, context.energyPercent,
            props.finalAlpha, props.finalHealthBarWidth, props.finalHealthBarHeight,
            props.finalHealthBarHeight);
    }
}

void ESPStageRenderer::RenderBoundingBox(ImDrawList* drawList, const EntityRenderContext& context, const VisualProperties& props) {
    bool isGadget = (context.entityType == ESPEntityType::Gadget);

    // Render bounding box for players/NPCs
    if (!isGadget && context.renderBox) {
        ESPShapeRenderer::RenderBoundingBox(drawList, props.boxMin, props.boxMax, props.fadedEntityColor, props.finalBoxThickness);
    }
}

void ESPStageRenderer::RenderGadgetVisuals(ImDrawList* drawList, const EntityRenderContext& context, Camera& camera, const VisualProperties& props, const Settings& settings) {
    bool isGadget = (context.entityType == ESPEntityType::Gadget);

    // Render gadget visuals (non-exclusive)
    if (isGadget) {
        if (settings.objectESP.renderSphere) {
            ESPShapeRenderer::RenderGadgetSphere(drawList, context, camera, props.screenPos, props.finalAlpha, props.fadedEntityColor, props.scale);
        }
        if (settings.objectESP.renderCircle) {
            drawList->AddCircle(ImVec2(props.screenPos.x, props.screenPos.y), props.circleRadius, props.fadedEntityColor, 0, props.finalBoxThickness);
        }
    }
}

void ESPStageRenderer::RenderDistanceText(ImDrawList* drawList, const EntityRenderContext& context, const VisualProperties& props) {
    bool isGadget = (context.entityType == ESPEntityType::Gadget);

    // Render distance text
    if (context.renderDistance) {
        if (isGadget) {
            // For gadgets, position distance text above the circle
            ImVec2 textAnchor(props.center.x, props.center.y - props.circleRadius);
            ESPTextRenderer::RenderDistanceText(drawList, props.center, textAnchor, context.gameplayDistance,
                props.finalAlpha, props.finalFontSize);
        }
        else {
            // For players/NPCs, use traditional positioning
            ESPTextRenderer::RenderDistanceText(drawList, props.center, props.boxMin, context.gameplayDistance,
                props.finalAlpha, props.finalFontSize);
        }
    }
}

void ESPStageRenderer::RenderCenterDot(ImDrawList* drawList, const EntityRenderContext& context, const VisualProperties& props) {
    bool isGadget = (context.entityType == ESPEntityType::Gadget);

    // Render center dot
    if (context.renderDot) {
        if (isGadget) {
            ESPShapeRenderer::RenderNaturalWhiteDot(drawList, props.screenPos, props.finalAlpha, props.finalDotRadius);
        }
        else {
            ESPShapeRenderer::RenderColoredDot(drawList, props.screenPos, props.fadedEntityColor, props.finalDotRadius);
        }
    }
}

void ESPStageRenderer::RenderPlayerName(ImDrawList* drawList, const EntityRenderContext& context, const VisualProperties& props) {
    // Render player name for natural identification (players only)
    if (context.entityType == ESPEntityType::Player && context.renderPlayerName) {
        // For hostile players with an empty name, display their profession
        std::string displayName = context.playerName;
        if (displayName.empty() && context.attitude == Game::Attitude::Hostile) {
            if (context.player) {
                const char* prof = ESPFormatting::GetProfessionName(context.player->profession);
                if (prof) {
                    displayName = prof;
                }
            }
        }

        if (!displayName.empty()) {
            // Use entity color directly (already attitude-based from ESPContextFactory)
            ESPTextRenderer::RenderPlayerName(drawList, props.screenPos, displayName, props.fadedEntityColor, props.finalFontSize);
        }
    }
}

void ESPStageRenderer::RenderDetailsText(ImDrawList* drawList, const EntityRenderContext& context, const VisualProperties& props) {
    bool isGadget = (context.entityType == ESPEntityType::Gadget);

    // Render details text (for all entities when enabled)
    if (context.renderDetails && !context.details.empty()) {
        if (isGadget) {
            // For gadgets, position details below the circle
            ImVec2 textAnchor(props.center.x, props.center.y + props.circleRadius);
            ESPTextRenderer::RenderDetailsText(drawList, props.center, textAnchor, context.details, props.finalAlpha, props.finalFontSize);
        }
        else {
            // For players/NPCs, use traditional positioning
            ESPTextRenderer::RenderDetailsText(drawList, props.center, props.boxMax, context.details, props.finalAlpha, props.finalFontSize);
        }
    }
}

void ESPStageRenderer::RenderGearSummary(ImDrawList* drawList, const EntityRenderContext& context, const VisualProperties& props, const Settings& settings) {
    // Specialized Summary Rendering (Players Only)
    if (context.entityType == ESPEntityType::Player && context.player != nullptr) {
        switch (settings.playerESP.gearDisplayMode) {
        case GearDisplayMode::Compact: { // Compact (Stat Names)
            auto compactSummary = ESPPlayerDetailsBuilder::BuildCompactGearSummary(context.player);
            ESPTextRenderer::RenderGearSummary(drawList, props.screenPos, compactSummary, props.finalAlpha, props.finalFontSize);
            break;
        }
        case GearDisplayMode::Attributes: { // Top 3 Attributes
            auto dominantStats = ESPPlayerDetailsBuilder::BuildDominantStats(context.player);
            auto topRarity = ESPPlayerDetailsBuilder::GetHighestRarity(context.player);
            ESPTextRenderer::RenderDominantStats(drawList, props.screenPos, dominantStats, topRarity, props.finalAlpha, props.finalFontSize);
            break;
        }
        default:
            // Modes Off and Detailed do not have a separate summary view
            break;
        }
    }
}

} // namespace kx