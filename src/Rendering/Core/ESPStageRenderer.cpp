#include "ESPStageRenderer.h"
#include "../Renderers/ESPContextFactory.h"
#include "../../Core/AppState.h"
#include "../../Game/Camera.h"
#include "../Utils/ESPMath.h"
#include "../Utils/ESPConstants.h"
#include "../Utils/ESPPlayerDetailsBuilder.h"
#include "../Utils/ESPEntityDetailsBuilder.h"
#include "../Utils/EntityVisualsCalculator.h"
#include "../Renderers/ESPShapeRenderer.h"
#include "../Renderers/ESPTextRenderer.h"
#include "../Renderers/ESPHealthBarRenderer.h"
#include "../Combat/CombatStateManager.h"
#include "../Data/EntityRenderContext.h"
#include "../Utils/ESPFormatting.h"
#include "../../../libs/ImGui/imgui.h"

namespace kx {

    void ESPStageRenderer::RenderFrameData(ImDrawList* drawList, float screenWidth, float screenHeight,
        const PooledFrameRenderData& frameData, Camera& camera,
        CombatStateManager& stateManager, uint64_t now) {
        const auto& settings = AppState::Get().GetSettings();
        FrameRenderContext context = { drawList, camera, stateManager, settings, now, screenWidth, screenHeight };

        // Simple rendering - no filtering logic, just draw everything that was passed in
        RenderPooledPlayers(context, frameData.players);
        RenderPooledNpcs(context, frameData.npcs);
        RenderPooledGadgets(context, frameData.gadgets);
    }

    void ESPStageRenderer::RenderEntityComponents(ImDrawList* drawList, const EntityRenderContext& context,
        Camera& camera, const VisualProperties& props) {
        const auto& settings = AppState::Get().GetSettings();
        bool isLivingEntity = (context.entityType == ESPEntityType::Player || context.entityType == ESPEntityType::NPC);
        bool isGadget = (context.entityType == ESPEntityType::Gadget);

        // fadedEntityColor already has all alphas applied from EntityVisualsCalculator
        // No need to apply alpha again here

        // Render standalone health bars for living entities when health is available AND setting is enabled
        if ((isLivingEntity || isGadget) && context.healthPercent >= 0.0f && context.renderHealthBar) {
            ESPHealthBarRenderer::RenderStandaloneHealthBar(drawList, props.screenPos, context,
                props.fadedEntityColor, props.finalHealthBarWidth, props.finalHealthBarHeight);
        }

        // Render energy bar for players
        if (context.entityType == ESPEntityType::Player && context.energyPercent >= 0.0f && context.renderEnergyBar) {
            ESPHealthBarRenderer::RenderStandaloneEnergyBar(drawList, props.screenPos, context.energyPercent,
                props.finalAlpha, props.finalHealthBarWidth, props.finalHealthBarHeight,
                props.finalHealthBarHeight);
        }

        // Render bounding box for players/NPCs
        if (!isGadget && context.renderBox) {
            ESPShapeRenderer::RenderBoundingBox(drawList, props.boxMin, props.boxMax, props.fadedEntityColor, props.finalBoxThickness);
        }

        // Render gadget visuals (non-exclusive)
        if (isGadget) {
            if (settings.objectESP.renderSphere) {
                ESPShapeRenderer::RenderGadgetSphere(drawList, context, camera, props.screenPos, props.finalAlpha, props.fadedEntityColor, props.scale);
            }
            if (settings.objectESP.renderCircle) {
                drawList->AddCircle(ImVec2(props.screenPos.x, props.screenPos.y), props.circleRadius, props.fadedEntityColor, 0, props.finalBoxThickness);
            }
        }

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

        // Render center dot
        if (context.renderDot) {
            if (isGadget) {
                ESPShapeRenderer::RenderNaturalWhiteDot(drawList, props.screenPos, props.finalAlpha, props.finalDotRadius);
            }
            else {
                ESPShapeRenderer::RenderColoredDot(drawList, props.screenPos, props.fadedEntityColor, props.finalDotRadius);
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
                ESPTextRenderer::RenderPlayerName(drawList, props.screenPos, displayName, props.fadedEntityColor, props.finalFontSize);
            }
        }

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

    void ESPStageRenderer::RenderEntity(ImDrawList* drawList, const EntityRenderContext& context, Camera& camera, CombatStateManager& stateManager, uint64_t now) {
        // Calculate all visual properties using the new calculator
        auto visualPropsOpt = EntityVisualsCalculator::Calculate(context, camera, context.screenWidth, context.screenHeight);

        if (!visualPropsOpt) {
            return; // Entity is not visible or fully transparent
        }

        const auto& props = *visualPropsOpt;

        // --- Post-Update for State Manager ---
        // This is where we can run calculations that depend on the final layout, like health bar width.
        if (context.renderHealthBar) {
            stateManager.PostUpdate(context.entity, props.finalHealthBarWidth, now);
        }

        // Now just use the pre-calculated properties to draw all components
        RenderEntityComponents(drawList, context, camera, props);
    }

    void ESPStageRenderer::RenderPooledPlayers(const FrameRenderContext& context, const std::vector<RenderablePlayer*>& players) {
        const auto& settings = AppState::Get().GetSettings(); // Still need settings for details builder

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
            auto entityContext = ESPContextFactory::CreateContextForPlayer(player, context.settings, context.stateManager, details, context.screenWidth, context.screenHeight, context.now);
            RenderEntity(context.drawList, entityContext, context.camera, context.stateManager, context.now);
        }
    }

    void ESPStageRenderer::RenderPooledNpcs(const FrameRenderContext& context, const std::vector<RenderableNpc*>& npcs) {
        const auto& settings = AppState::Get().GetSettings(); // Still need settings for details builder

        for (const auto* npc : npcs) {
            if (!npc) continue; // Safety check

            // Use the builder to prepare NPC details
            std::vector<ColoredDetail> details = ESPEntityDetailsBuilder::BuildNpcDetails(npc, settings.npcESP, settings.showDebugAddresses);

            auto entityContext = ESPContextFactory::CreateContextForNpc(npc, context.settings, context.stateManager, details, context.screenWidth, context.screenHeight, context.now);
            RenderEntity(context.drawList, entityContext, context.camera, context.stateManager, context.now);
        }
    }

    void ESPStageRenderer::RenderPooledGadgets(const FrameRenderContext& context, const std::vector<RenderableGadget*>& gadgets) {
        const auto& settings = AppState::Get().GetSettings(); // Still need settings for details builder

        for (const auto* gadget : gadgets) {
            if (!gadget) continue; // Safety check

            // Use the builder to prepare Gadget details
            std::vector<ColoredDetail> details = ESPEntityDetailsBuilder::BuildGadgetDetails(gadget, settings.objectESP, settings.showDebugAddresses);

            auto entityContext = ESPContextFactory::CreateContextForGadget(gadget, context.settings, context.stateManager, details, context.screenWidth, context.screenHeight, context.now);
            RenderEntity(context.drawList, entityContext, context.camera, context.stateManager, context.now);
        }
    }
}
