#include "ESPStateFinalizer.h"
#include "AppState.h"
#include "../Combat/CombatStateManager.h"
#include "../Data/EntityRenderContext.h"
#include "../Renderers/ESPContextFactory.h"
#include "../Utils/EntityVisualsCalculator.h"
#include <vector>

namespace kx {

    void ESPStateFinalizer::Finalize(const PooledFrameRenderData& filteredData, Camera& camera, CombatStateManager& stateManager, uint64_t now)
    {
        const auto& settings = AppState::Get().GetSettings();
        const auto& io = ImGui::GetIO();

        // This context contains all the shared information needed for the calculations.
        FactoryContext factoryContext = {
            settings,
            stateManager, // const reference is fine here
            io.DisplaySize.x,
            io.DisplaySize.y,
            now
        };

        // Create an empty details vector to pass to the context factory.
        // It's not needed for visual calculations, only for text rendering.
        static const std::vector<ColoredDetail> emptyDetails;

        // --- Process Players ---
        for (const auto* player : filteredData.players)
        {
            if (!player) continue;

            // Create a context specific to this player.
            EntityRenderContext context = ESPContextFactory::CreateContextForPlayer(player, emptyDetails, factoryContext);
            if (!context.renderHealthBar) continue;

            // Calculate visual properties to get the final health bar width.
            auto visualPropsOpt = EntityVisualsCalculator::Calculate(context, camera, factoryContext.screenWidth, factoryContext.screenHeight);
            if (visualPropsOpt) {
                // This is the call we are moving. It finalizes the damage chunking animation state.
                stateManager.PostUpdate(player, visualPropsOpt->finalHealthBarWidth, now);
            }
        }

        // --- Process NPCs ---
        for (const auto* npc : filteredData.npcs)
        {
            if (!npc) continue;

            // Create a context specific to this NPC.
            EntityRenderContext context = ESPContextFactory::CreateContextForNpc(npc, emptyDetails, factoryContext);
            if (!context.renderHealthBar) continue;

            // Calculate visual properties.
            auto visualPropsOpt = EntityVisualsCalculator::Calculate(context, camera, factoryContext.screenWidth, factoryContext.screenHeight);
            if (visualPropsOpt) {
                stateManager.PostUpdate(npc, visualPropsOpt->finalHealthBarWidth, now);
            }
        }

        // --- Process Gadgets ---
        for (const auto* gadget : filteredData.gadgets)
        {
            if (!gadget) continue;

            // Create a context specific to this gadget.
            EntityRenderContext context = ESPContextFactory::CreateContextForGadget(gadget, emptyDetails, factoryContext);
            if (!context.renderHealthBar) continue;

            // Calculate visual properties.
            auto visualPropsOpt = EntityVisualsCalculator::Calculate(context, camera, factoryContext.screenWidth, factoryContext.screenHeight);
            if (visualPropsOpt) {
                stateManager.PostUpdate(gadget, visualPropsOpt->finalHealthBarWidth, now);
            }
        }
    }

} // namespace kx