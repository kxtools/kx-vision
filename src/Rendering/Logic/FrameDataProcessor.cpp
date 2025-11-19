#include "FrameDataProcessor.h"
#include <vector>

#include "Logic/StyleCalculator.h"
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

        auto styleOpt = Logic::StyleCalculator::Calculate(*entity, context);
        
        if (styleOpt) {
            VisualProperties props;
            props.style = *styleOpt;
            EntityRenderContext renderContext = ContextFactory::CreateEntityRenderContextForRendering(entity, context);
            outData.finalizedEntities.emplace_back(FinalizedRenderable{entity, props, renderContext});
        }
    }

    // Process NPCs
    for (const auto* entity : filteredData.npcs) {
        if (!entity) continue;

        auto styleOpt = Logic::StyleCalculator::Calculate(*entity, context);
        
        if (styleOpt) {
            VisualProperties props;
            props.style = *styleOpt;
            EntityRenderContext renderContext = ContextFactory::CreateEntityRenderContextForRendering(entity, context);
            outData.finalizedEntities.emplace_back(FinalizedRenderable{entity, props, renderContext});
        }
    }

    // Process gadgets
    for (const auto* entity : filteredData.gadgets) {
        if (!entity) continue;

        auto styleOpt = Logic::StyleCalculator::Calculate(*entity, context);
        
        if (styleOpt) {
            VisualProperties props;
            props.style = *styleOpt;
            EntityRenderContext renderContext = ContextFactory::CreateEntityRenderContextForRendering(entity, context);
            outData.finalizedEntities.emplace_back(FinalizedRenderable{entity, props, renderContext});
        }
    }

    // Process attack targets
    for (const auto* entity : filteredData.attackTargets) {
        if (!entity) continue;

        auto styleOpt = Logic::StyleCalculator::Calculate(*entity, context);
        
        if (styleOpt) {
            VisualProperties props;
            props.style = *styleOpt;
            EntityRenderContext renderContext = ContextFactory::CreateEntityRenderContextForRendering(entity, context);
            outData.finalizedEntities.emplace_back(FinalizedRenderable{entity, props, renderContext});
        }
    }
}

} // namespace kx
