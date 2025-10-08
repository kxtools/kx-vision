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
#include "../Combat/CombatStateManager.h"
#include "../Data/EntityRenderContext.h"
#include "../Utils/ESPFormatting.h"
#include "../../../libs/ImGui/imgui.h"
#include <algorithm>

namespace kx {

void ESPStageRenderer::RenderFrameData(ImDrawList* drawList, float screenWidth, float screenHeight, 
                                      const PooledFrameRenderData& frameData, Camera& camera, 
                                      const CombatStateManager& stateManager) {
    // Simple rendering - no filtering logic, just draw everything that was passed in
    RenderPooledPlayers(drawList, screenWidth, screenHeight, frameData.players, camera, stateManager);
    RenderPooledNpcs(drawList, screenWidth, screenHeight, frameData.npcs, camera, stateManager);
    RenderPooledGadgets(drawList, screenWidth, screenHeight, frameData.gadgets, camera, stateManager);
}

void ESPStageRenderer::RenderEntityComponents(ImDrawList* drawList, const EntityRenderContext& context,
                                             Camera& camera,
                                             const glm::vec2& screenPos, const ImVec2& boxMin, const ImVec2& boxMax,
                                             const ImVec2& center, unsigned int fadedEntityColor, 
                                             float distanceFadeAlpha, float scale, float circleRadius,
                                             float finalAlpha, float finalFontSize, float finalBoxThickness,
                                             float finalDotRadius, float finalHealthBarWidth, float finalHealthBarHeight,
                                             const CombatStateManager& stateManager) {
    const auto& settings = AppState::Get().GetSettings();
    bool isLivingEntity = (context.entityType == ESPEntityType::Player || context.entityType == ESPEntityType::NPC);
    bool isGadget = (context.entityType == ESPEntityType::Gadget);
    
    // fadedEntityColor already has all alphas applied from EntityVisualsCalculator
    // No need to apply alpha again here
    
    // Render standalone health bars for living entities when health is available AND setting is enabled
    if (isLivingEntity && context.healthPercent >= 0.0f && context.renderHealthBar) {
        ESPHealthBarRenderer::RenderStandaloneHealthBar(drawList, screenPos, context, 
                                                     fadedEntityColor, finalHealthBarWidth, finalHealthBarHeight,
                                                     stateManager);
    }

    // Render energy bar for players
    if (context.entityType == ESPEntityType::Player && context.energyPercent >= 0.0f && context.renderEnergyBar) {
        ESPHealthBarRenderer::RenderStandaloneEnergyBar(drawList, screenPos, context.energyPercent, 
                                                     finalAlpha, finalHealthBarWidth, finalHealthBarHeight,
                                                     finalHealthBarHeight);
    }

    // Render bounding box for players/NPCs
    if (!isGadget && context.renderBox) {
        ESPShapeRenderer::RenderBoundingBox(drawList, boxMin, boxMax, fadedEntityColor, finalBoxThickness);
    }

    // Render gadget visuals (non-exclusive)
    if (isGadget) {
        if (settings.objectESP.renderSphere) {
            RenderGadgetSphere(drawList, context, camera, screenPos, finalAlpha, fadedEntityColor, scale);
        }
        if (settings.objectESP.renderCircle) {
            drawList->AddCircle(ImVec2(screenPos.x, screenPos.y), circleRadius, fadedEntityColor, 0, finalBoxThickness);
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

void ESPStageRenderer::RenderEntity(ImDrawList* drawList, const EntityRenderContext& context, Camera& camera, const CombatStateManager& stateManager) {
    // Calculate all visual properties using the new calculator
    auto visualPropsOpt = EntityVisualsCalculator::Calculate(context, camera, context.screenWidth, context.screenHeight);
    
    if (!visualPropsOpt) {
        return; // Entity is not visible or fully transparent
    }
    
    const auto& props = *visualPropsOpt;

    // Now just use the pre-calculated properties to draw all components
    RenderEntityComponents(drawList, context, camera, props.screenPos, props.boxMin, props.boxMax, props.center,
                          props.fadedEntityColor, props.distanceFadeAlpha, props.scale, props.circleRadius,
                          props.finalAlpha, props.finalFontSize, props.finalBoxThickness, props.finalDotRadius,
                          props.finalHealthBarWidth, props.finalHealthBarHeight, stateManager);
}

void ESPStageRenderer::RenderPooledPlayers(ImDrawList* drawList, float screenWidth, float screenHeight,
    const std::vector<RenderablePlayer*>& players, Camera& camera, const CombatStateManager& stateManager) {
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
        RenderEntity(drawList, context, camera, stateManager);
    }
}

void ESPStageRenderer::RenderPooledNpcs(ImDrawList* drawList, float screenWidth, float screenHeight, 
                                       const std::vector<RenderableNpc*>& npcs, Camera& camera, 
                                       const CombatStateManager& stateManager) {
    const auto& settings = AppState::Get().GetSettings();
    
    for (const auto* npc : npcs) {
        if (!npc) continue; // Safety check

        // Use the builder to prepare NPC details
        std::vector<ColoredDetail> details = ESPEntityDetailsBuilder::BuildNpcDetails(npc, settings.npcESP, settings.showDebugAddresses);

        auto context = ESPContextFactory::CreateContextForNpc(npc, settings, details, screenWidth, screenHeight);
        RenderEntity(drawList, context, camera, stateManager);
    }
}

void ESPStageRenderer::RenderPooledGadgets(ImDrawList* drawList, float screenWidth, float screenHeight, 
                                          const std::vector<RenderableGadget*>& gadgets, Camera& camera, 
                                          const CombatStateManager& stateManager) {
    const auto& settings = AppState::Get().GetSettings();
    
    for (const auto* gadget : gadgets) {
        if (!gadget) continue; // Safety check

        // Use the builder to prepare Gadget details
        std::vector<ColoredDetail> details = ESPEntityDetailsBuilder::BuildGadgetDetails(gadget, settings.objectESP, settings.showDebugAddresses);

        auto context = ESPContextFactory::CreateContextForGadget(gadget, settings, details, screenWidth, screenHeight);
        RenderEntity(drawList, context, camera, stateManager);
    }
}

void ESPStageRenderer::RenderGadgetSphere(ImDrawList* drawList, const EntityRenderContext& context, Camera& camera,
    const glm::vec2& screenPos, float finalAlpha, unsigned int fadedEntityColor, float scale) {
    // --- Final 3D Gyroscope with a Robust LOD to a 2D Circle ---

    // --- 1. LOD (Level of Detail) Calculation ---
    const float LOD_TRANSITION_START = 180.0f;
    const float LOD_TRANSITION_END = 200.0f;

    float gyroscopeAlpha = 1.0f;
    float circleAlpha = 0.0f;

    if (context.gameplayDistance > LOD_TRANSITION_START) {
        // We are in or past the transition zone.
        float range = LOD_TRANSITION_END - LOD_TRANSITION_START;
        float progress = std::clamp((context.gameplayDistance - LOD_TRANSITION_START) / range, 0.0f, 1.0f);

        gyroscopeAlpha = 1.0f - progress; // Fades out from 1.0 to 0.0
        circleAlpha = progress;          // Fades in from 0.0 to 1.0
    }

    // --- RENDER THE 3D GYROSCOPE (if it's visible) ---
    if (gyroscopeAlpha > 0.0f) {
        // --- Define the 3D sphere's properties ---
        const int NUM_RING_POINTS = 16;
        const float PI = 3.14159265359f;
        const float baseThickness = 2.5f;
        const float finalLineThickness = std::clamp(baseThickness * scale, 1.0f, 5.0f);
        const float VERTICAL_RADIUS = 0.35f;
        const float HORIZONTAL_RADIUS = VERTICAL_RADIUS * 0.9f;

        // --- Define vertices ---
        std::vector<glm::vec3> localRingXY, localRingXZ, localRingYZ;
        localRingXY.reserve(NUM_RING_POINTS + 1);
        localRingXZ.reserve(NUM_RING_POINTS + 1);
        localRingYZ.reserve(NUM_RING_POINTS + 1);
        for (int i = 0; i <= NUM_RING_POINTS; ++i) {
            float angle = 2.0f * PI * i / NUM_RING_POINTS;
            float s = sin(angle), c = cos(angle);
            localRingXY.emplace_back(c * HORIZONTAL_RADIUS, s * HORIZONTAL_RADIUS, 0);
            localRingXZ.emplace_back(c * VERTICAL_RADIUS, 0, s * VERTICAL_RADIUS);
            localRingYZ.emplace_back(0, c * VERTICAL_RADIUS, s * VERTICAL_RADIUS);
        }

        // --- Project points ---
        std::vector<ImVec2> screenRingXY, screenRingXZ, screenRingYZ;
        bool projection_ok = true;
        auto project_ring = [&](const std::vector<glm::vec3>& local_points, std::vector<ImVec2>& screen_points) {
            if (!projection_ok) return;
            screen_points.reserve(local_points.size());
            for (const auto& point : local_points) {
                glm::vec2 sp;
                if (ESPMath::WorldToScreen(context.position + point, camera, context.screenWidth, context.screenHeight, sp)) {
                    screen_points.push_back(ImVec2(sp.x, sp.y));
                }
                else { projection_ok = false; screen_points.clear(); return; }
            }
            };
        project_ring(localRingXY, screenRingXY);
        project_ring(localRingXZ, screenRingXZ);
        project_ring(localRingYZ, screenRingYZ);

        // --- Draw the 3D sphere ---
        if (projection_ok) {
            // Apply the master LOD fade alpha to all colors
            unsigned int masterAlpha = (fadedEntityColor >> 24) & 0xFF;
            unsigned int finalLODAlpha = static_cast<unsigned int>(masterAlpha * gyroscopeAlpha);

            ImU32 brightColor = (fadedEntityColor & 0x00FFFFFF) | (finalLODAlpha << 24);
            ImVec4 dimColorVec = ImGui::ColorConvertU32ToFloat4(brightColor);
            dimColorVec.x *= 0.7f; dimColorVec.y *= 0.7f; dimColorVec.z *= 0.7f;
            ImU32 dimColor = ImGui::ColorConvertFloat4ToU32(dimColorVec);

            const float equatorThickness = finalLineThickness;
            const float verticalThickness = finalLineThickness * 0.7f;

            if (!screenRingXZ.empty()) drawList->AddPolyline(screenRingXZ.data(), screenRingXZ.size(), dimColor, false, verticalThickness);
            if (!screenRingYZ.empty()) drawList->AddPolyline(screenRingYZ.data(), screenRingYZ.size(), dimColor, false, verticalThickness);
            if (!screenRingXY.empty()) drawList->AddPolyline(screenRingXY.data(), screenRingXY.size(), brightColor, false, equatorThickness);
        }
    }

    // --- RENDER THE 2D CIRCLE (if it's visible) ---
    if (circleAlpha > 0.0f) {
        // Calculate the radius for the 2D circle based on scale
        float circleRadius = std::clamp(10.0f * scale, 2.0f, 15.0f);

        // Create the color with the calculated LOD alpha
        unsigned int masterAlpha = ((fadedEntityColor & 0xFF000000) >> 24);
        unsigned int finalLODAlpha = static_cast<unsigned int>(masterAlpha * circleAlpha);
        ImU32 circleColor = (fadedEntityColor & 0x00FFFFFF) | (finalLODAlpha << 24);

        // Use the simple "Holographic Disc" from our earlier discussion for a nice look
        ImU32 glowColor = (circleColor & 0x00FFFFFF) | (static_cast<unsigned int>(finalLODAlpha * 0.3f) << 24);
        ImU32 coreColor = (circleColor & 0x00FFFFFF) | (static_cast<unsigned int>(finalLODAlpha * 0.7f) << 24);
        ImU32 hotspotColor = IM_COL32(255, 255, 255, static_cast<unsigned int>(255 * circleAlpha * finalAlpha));

        drawList->AddCircleFilled(ImVec2(screenPos.x, screenPos.y), circleRadius, glowColor);
        drawList->AddCircleFilled(ImVec2(screenPos.x, screenPos.y), circleRadius * 0.7f, coreColor);
        drawList->AddCircleFilled(ImVec2(screenPos.x, screenPos.y), circleRadius * 0.2f, hotspotColor);
    }
}

} // namespace kx