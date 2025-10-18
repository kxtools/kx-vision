// In Core/ESPVisualsProcessor.cpp

#include "ESPVisualsProcessor.h"
#include "../Data/RenderableData.h"
#include "../Utils/EntityVisualsCalculator.h"
#include "../Factories/ESPContextFactory.h"
#include "../Data/ESPEntityTypes.h"
#include <vector>

namespace kx {

// Helper function to process a vector of any RenderableEntity type.
template<typename T>
void ProcessEntityVector(const std::vector<T*>& entities, const FrameContext& context, std::vector<FinalizedRenderable>& outFinalized) {
    for (const auto* entity : entities) {
        if (!entity) continue;

        // The single, expensive calculation happens HERE.
        auto visualPropsOpt = EntityVisualsCalculator::Calculate(*entity, context.camera, context.screenWidth, context.screenHeight);
        
        if (visualPropsOpt) {
            // Build the render context with details
            EntityRenderContext renderContext = ESPContextFactory::CreateEntityRenderContextForRendering(entity, context);
            
            // If visible, add it to our final list with both visuals and context.
            outFinalized.emplace_back(FinalizedRenderable{entity, *visualPropsOpt, renderContext});
        }
    }
}

void ESPVisualsProcessor::Process(const FrameContext& context, 
                                  const PooledFrameRenderData& filteredData,
                                  PooledFrameRenderData& outData) {
    outData.finalizedEntities.clear();
    outData.finalizedEntities.reserve(
        filteredData.players.size() + filteredData.npcs.size() + filteredData.gadgets.size()
    );

    // Process each type of entity and add it to the single finalized list.
    ProcessEntityVector(filteredData.players, context, outData.finalizedEntities);
    ProcessEntityVector(filteredData.npcs, context, outData.finalizedEntities);
    ProcessEntityVector(filteredData.gadgets, context, outData.finalizedEntities);
}

} // namespace kx
