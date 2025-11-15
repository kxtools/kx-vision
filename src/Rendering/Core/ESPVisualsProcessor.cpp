#include "ESPVisualsProcessor.h"
#include "../Utils/EntityVisualsCalculator.h"
#include "../Factories/ESPContextFactory.h"
#include <vector>

namespace kx {

void ESPVisualsProcessor::Process(const FrameContext& context, 
                                  const PooledFrameRenderData& filteredData,
                                  PooledFrameRenderData& outData) {
    outData.finalizedEntities.clear();
    outData.finalizedEntities.reserve(
        filteredData.players.size() + filteredData.npcs.size() + filteredData.gadgets.size() + filteredData.attackTargets.size()
    );

    // Process players
    for (const auto* entity : filteredData.players) {
        if (!entity) continue;

        auto visualPropsOpt = EntityVisualsCalculator::Calculate(*entity, context);
        
        if (visualPropsOpt) {
            EntityRenderContext renderContext = ESPContextFactory::CreateEntityRenderContextForRendering(entity, context);
            outData.finalizedEntities.emplace_back(FinalizedRenderable{entity, *visualPropsOpt, renderContext});
        }
    }

    // Process NPCs
    for (const auto* entity : filteredData.npcs) {
        if (!entity) continue;

        auto visualPropsOpt = EntityVisualsCalculator::Calculate(*entity, context);
        
        if (visualPropsOpt) {
            EntityRenderContext renderContext = ESPContextFactory::CreateEntityRenderContextForRendering(entity, context);
            outData.finalizedEntities.emplace_back(FinalizedRenderable{entity, *visualPropsOpt, renderContext});
        }
    }

    // Process gadgets
    for (const auto* entity : filteredData.gadgets) {
        if (!entity) continue;

        auto visualPropsOpt = EntityVisualsCalculator::Calculate(*entity, context);
        
        if (visualPropsOpt) {
            EntityRenderContext renderContext = ESPContextFactory::CreateEntityRenderContextForRendering(entity, context);
            outData.finalizedEntities.emplace_back(FinalizedRenderable{entity, *visualPropsOpt, renderContext});
        }
    }

    // Process attack targets
    for (const auto* entity : filteredData.attackTargets) {
        if (!entity) continue;

        auto visualPropsOpt = EntityVisualsCalculator::Calculate(*entity, context);
        
        if (visualPropsOpt) {
            EntityRenderContext renderContext = ESPContextFactory::CreateEntityRenderContextForRendering(entity, context);
            outData.finalizedEntities.emplace_back(FinalizedRenderable{entity, *visualPropsOpt, renderContext});
        }
    }
}

} // namespace kx
