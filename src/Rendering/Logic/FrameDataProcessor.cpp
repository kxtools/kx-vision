#include "FrameDataProcessor.h"
#include <vector>

#include "Logic/VisualsCalculator.h"
#include "Presentation/ContextFactory.h"

namespace kx {

void FrameDataProcessor::Process(const FrameContext& context, 
                                  const PooledFrameRenderData& filteredData,
                                  PooledFrameRenderData& outData) {
    // This processor acts as a Data Preparer, not a Culler.
    // It calculates abstract visual properties (alpha, color, scale, sizes) but does NOT
    // perform geometric projection or frustum culling. Entities are added to the output
    // even if they're currently off-screen, as ESPStageRenderer will handle geometric culling
    // using the live camera in the render thread.
    
    outData.finalizedEntities.clear();
    outData.finalizedEntities.reserve(
        filteredData.players.size() + filteredData.npcs.size() + filteredData.gadgets.size() + filteredData.attackTargets.size()
    );

    // Process players
    for (const auto* entity : filteredData.players) {
        if (!entity) continue;

        // This now ONLY calculates colors, scales, and alphas.
        // It returns nullopt ONLY if distance culling (logic) fails.
        // It does NOT return nullopt if the entity is simply off-screen (geometry).
        auto visualPropsOpt = VisualsCalculator::Calculate(*entity, context);
        
        if (visualPropsOpt) {
            EntityRenderContext renderContext = ContextFactory::CreateEntityRenderContextForRendering(entity, context);
            // We add it to the list. Even if it's behind the camera right now.
            // ESPStageRenderer will check the camera later.
            outData.finalizedEntities.emplace_back(FinalizedRenderable{entity, *visualPropsOpt, renderContext});
        }
    }

    // Process NPCs
    for (const auto* entity : filteredData.npcs) {
        if (!entity) continue;

        auto visualPropsOpt = VisualsCalculator::Calculate(*entity, context);
        
        if (visualPropsOpt) {
            EntityRenderContext renderContext = ContextFactory::CreateEntityRenderContextForRendering(entity, context);
            outData.finalizedEntities.emplace_back(FinalizedRenderable{entity, *visualPropsOpt, renderContext});
        }
    }

    // Process gadgets
    for (const auto* entity : filteredData.gadgets) {
        if (!entity) continue;

        auto visualPropsOpt = VisualsCalculator::Calculate(*entity, context);
        
        if (visualPropsOpt) {
            EntityRenderContext renderContext = ContextFactory::CreateEntityRenderContextForRendering(entity, context);
            outData.finalizedEntities.emplace_back(FinalizedRenderable{entity, *visualPropsOpt, renderContext});
        }
    }

    // Process attack targets
    for (const auto* entity : filteredData.attackTargets) {
        if (!entity) continue;

        auto visualPropsOpt = VisualsCalculator::Calculate(*entity, context);
        
        if (visualPropsOpt) {
            EntityRenderContext renderContext = ContextFactory::CreateEntityRenderContextForRendering(entity, context);
            outData.finalizedEntities.emplace_back(FinalizedRenderable{entity, *visualPropsOpt, renderContext});
        }
    }
}

} // namespace kx
