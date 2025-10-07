#include "ESPStageRenderer.h"
#include "../Renderers/ESPContextFactory.h"
#include "../../Core/AppState.h"
#include "../../Game/Camera.h"
#include "../Utils/ESPMath.h"
#include "../Utils/ESPConstants.h"
#include "../Utils/ESPPlayerDetailsBuilder.h"
#include "../Utils/ESPEntityDetailsBuilder.h"
#include "../Utils/EntityVisualsCalculator.h"
#include "ESPFilter.h"
#include "../Renderers/ESPShapeRenderer.h"
#include "../Renderers/ESPTextRenderer.h"
#include "../Renderers/ESPHealthBarRenderer.h"
#include "../Data/EntityRenderContext.h"
#include "../Utils/ESPFormatting.h"
#include "../../../libs/ImGui/imgui.h"
#include <algorithm>

namespace kx {

void ESPStageRenderer::RenderFrameData(ImDrawList* drawList, float screenWidth, float screenHeight, 
                                      const PooledFrameRenderData& frameData, Camera& camera) {
    // Simple rendering - no filtering logic, just draw everything that was passed in
    RenderPooledPlayers(drawList, screenWidth, screenHeight, frameData.players, camera);
    RenderPooledNpcs(drawList, screenWidth, screenHeight, frameData.npcs, camera);
    RenderPooledGadgets(drawList, screenWidth, screenHeight, frameData.gadgets, camera);
}

void ESPStageRenderer::RenderEntityComponents(ImDrawList* drawList, const EntityRenderContext& context,
                                             const glm::vec2& screenPos, const ImVec2& boxMin, const ImVec2& boxMax,
                                             const ImVec2& center, unsigned int fadedEntityColor, 
                                             float distanceFadeAlpha, float scale, float circleRadius,
                                             float finalAlpha, float finalFontSize, float finalBoxThickness,
                                             float finalDotRadius, float finalHealthBarWidth, float finalHealthBarHeight) {
    const auto& settings = AppState::Get().GetSettings();
    bool isLivingEntity = (context.entityType == ESPEntityType::Player || context.entityType == ESPEntityType::NPC);
    bool isGadget = (context.entityType == ESPEntityType::Gadget);
    
    // fadedEntityColor already has all alphas applied from EntityVisualsCalculator
    // No need to apply alpha again here
    
    // Render standalone health bars for living entities when health is available AND setting is enabled
    if (isLivingEntity && context.healthPercent >= 0.0f && context.renderHealthBar) {
        ESPHealthBarRenderer::RenderStandaloneHealthBar(drawList, screenPos, context.healthPercent, 
                                                     fadedEntityColor, finalHealthBarWidth, finalHealthBarHeight,
                                                     context.entityType, context.attitude);
    }

    // Render energy bar for players
    if (context.entityType == ESPEntityType::Player && context.energyPercent >= 0.0f && context.renderEnergyBar) {
        ESPHealthBarRenderer::RenderStandaloneEnergyBar(drawList, screenPos, context.energyPercent, 
                                                     finalAlpha, finalHealthBarWidth, finalHealthBarHeight,
                                                     finalHealthBarHeight);
    }

    // Render bounding box OR circle
    if (context.renderBox) {
        if (isGadget) {
            // Render circle for gadgets (centered at screenPos)
            drawList->AddCircle(ImVec2(screenPos.x, screenPos.y), circleRadius, fadedEntityColor, 0, finalBoxThickness);
        } else {
            // Render traditional box for players/NPCs
            ESPShapeRenderer::RenderBoundingBox(drawList, boxMin, boxMax, fadedEntityColor, finalBoxThickness);
        }
    }

    // Render distance text
    if (context.renderDistance) {
        if (isGadget) {
            // For gadgets, position distance text above the circle
            ImVec2 textAnchor(center.x, center.y - circleRadius);
            ESPTextRenderer::RenderDistanceText(drawList, center, textAnchor, context.gameplayDistance, 
                                                  finalAlpha, finalFontSize);
        } else {
            // For players/NPCs, use traditional positioning
            ESPTextRenderer::RenderDistanceText(drawList, center, boxMin, context.gameplayDistance, 
                                                  finalAlpha, finalFontSize);
        }
    }

    // Render center dot
    if (context.renderDot) {
        if (isGadget) {
            ESPShapeRenderer::RenderNaturalWhiteDot(drawList, screenPos, finalAlpha, finalDotRadius);
        } else {
            ESPShapeRenderer::RenderColoredDot(drawList, screenPos, fadedEntityColor, finalDotRadius);
        }
    }

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
            ESPTextRenderer::RenderPlayerName(drawList, screenPos, displayName, fadedEntityColor, finalFontSize);
        }
    }

    // Render details text (for all entities when enabled)
    if (context.renderDetails && !context.details.empty()) {
        if (isGadget) {
            // For gadgets, position details below the circle
            ImVec2 textAnchor(center.x, center.y + circleRadius);
            ESPTextRenderer::RenderDetailsText(drawList, center, textAnchor, context.details, finalAlpha, finalFontSize);
        } else {
            // For players/NPCs, use traditional positioning
            ESPTextRenderer::RenderDetailsText(drawList, center, boxMax, context.details, finalAlpha, finalFontSize);
        }
    }

    // Specialized Summary Rendering (Players Only)
    if (context.entityType == ESPEntityType::Player && context.player != nullptr) {
        switch (settings.playerESP.gearDisplayMode) {
        case GearDisplayMode::Compact: { // Compact (Stat Names)
            auto compactSummary = ESPPlayerDetailsBuilder::BuildCompactGearSummary(context.player);
            ESPTextRenderer::RenderGearSummary(drawList, screenPos, compactSummary, finalAlpha, finalFontSize);
            break;
        }
        case GearDisplayMode::Attributes: { // Top 3 Attributes
            auto dominantStats = ESPPlayerDetailsBuilder::BuildDominantStats(context.player);
            auto topRarity = ESPPlayerDetailsBuilder::GetHighestRarity(context.player);
            ESPTextRenderer::RenderDominantStats(drawList, screenPos, dominantStats, topRarity, finalAlpha, finalFontSize);
            break;
        }
        default:
            // Modes Off and Detailed do not have a separate summary view
            break;
        }
    }
}

void ESPStageRenderer::RenderEntity(ImDrawList* drawList, const EntityRenderContext& context, Camera& camera) {
    // Calculate all visual properties using the new calculator
    auto visualPropsOpt = EntityVisualsCalculator::Calculate(context, camera, context.screenWidth, context.screenHeight);
    
    if (!visualPropsOpt) {
        return; // Entity is not visible or fully transparent
    }
    
    const auto& props = *visualPropsOpt;

    // Now just use the pre-calculated properties to draw all components
    RenderEntityComponents(drawList, context, props.screenPos, props.boxMin, props.boxMax, props.center,
                          props.fadedEntityColor, props.distanceFadeAlpha, props.scale, props.circleRadius,
                          props.finalAlpha, props.finalFontSize, props.finalBoxThickness, props.finalDotRadius,
                          props.finalHealthBarWidth, props.finalHealthBarHeight);
}

void ESPStageRenderer::RenderPooledPlayers(ImDrawList* drawList, float screenWidth, float screenHeight,
    const std::vector<RenderablePlayer*>& players, Camera& camera) {
    const auto& settings = AppState::Get().GetSettings();

    for (const auto* player : players) {
        if (!player) continue;

        // --- 1. DATA PREPARATION ---
        // Use the builder to prepare all text details. This includes general info and detailed gear view if selected.
        std::vector<ColoredDetail> details = ESPPlayerDetailsBuilder::BuildPlayerDetails(player, settings.playerESP, settings.showDebugAddresses);

        // If "Detailed" mode is selected, build the full gear list and add it to the details.
        if (settings.playerESP.gearDisplayMode == GearDisplayMode::Detailed) {
            auto gearDetails = ESPPlayerDetailsBuilder::BuildGearDetails(player);
            if (!gearDetails.empty()) {
                if (!details.empty()) {
                    details.push_back({ "--- Gear Stats ---", ESPColors::DEFAULT_TEXT });
                }
                details.insert(details.end(), gearDetails.begin(), gearDetails.end());
            }
        }

        // --- 2. CORE RENDERING ---
        // Use factory to create context and render
        auto context = ESPContextFactory::CreateContextForPlayer(player, settings, details, screenWidth, screenHeight);
        RenderEntity(drawList, context, camera);
    }
}

void ESPStageRenderer::RenderPooledNpcs(ImDrawList* drawList, float screenWidth, float screenHeight, 
                                       const std::vector<RenderableNpc*>& npcs, Camera& camera) {
    const auto& settings = AppState::Get().GetSettings();
    
    for (const auto* npc : npcs) {
        if (!npc) continue; // Safety check

        // Use the builder to prepare NPC details
        std::vector<ColoredDetail> details = ESPEntityDetailsBuilder::BuildNpcDetails(npc, settings.npcESP, settings.showDebugAddresses);

        auto context = ESPContextFactory::CreateContextForNpc(npc, settings, details, screenWidth, screenHeight);
        RenderEntity(drawList, context, camera);
    }
}

void ESPStageRenderer::RenderPooledGadgets(ImDrawList* drawList, float screenWidth, float screenHeight, 
                                          const std::vector<RenderableGadget*>& gadgets, Camera& camera) {
    const auto& settings = AppState::Get().GetSettings();
    
    for (const auto* gadget : gadgets) {
        if (!gadget) continue; // Safety check

        // Use the builder to prepare Gadget details
        std::vector<ColoredDetail> details = ESPEntityDetailsBuilder::BuildGadgetDetails(gadget, settings.objectESP, settings.showDebugAddresses);

        auto context = ESPContextFactory::CreateContextForGadget(gadget, settings, details, screenWidth, screenHeight);
        RenderEntity(drawList, context, camera);
    }
}

float ESPStageRenderer::CalculateEntityDistanceFadeAlpha(float distance, bool useDistanceLimit, float distanceLimit) {
    // Use the same logic as ESPFilter for consistency
    return ESPFilter::CalculateDistanceFadeAlpha(distance, useDistanceLimit, distanceLimit);
}

} // namespace kx