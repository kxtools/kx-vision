#include "ESPStateFinalizer.h"
#include "../Data/ESPData.h"
#include "../Combat/CombatStateManager.h"

namespace kx {

    void ESPStateFinalizer::Finalize(const FrameContext& context, const PooledFrameRenderData& finalizedData)
    {
        for (const auto& item : finalizedData.finalizedEntities)
        {
            // We directly access the pre-calculated width. No more recalculations!
            float healthBarWidth = item.visuals.finalHealthBarWidth;
            
            // Only call PostUpdate if a health bar would actually be rendered.
            // A width of 0 can be a good proxy for this.
            if (healthBarWidth > 0.0f) {
                context.stateManager.PostUpdate(item.entity, healthBarWidth, context.now);
            }
        }
    }

} // namespace kx