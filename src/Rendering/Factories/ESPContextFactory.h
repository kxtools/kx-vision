#pragma once

#include <vector>
#include "../Data/RenderableData.h"
#include "../Data/EntityRenderContext.h"
#include "../Data/ESPData.h"

namespace kx {

class CombatStateManager; // Forward declaration

class ESPContextFactory {
public:
    static EntityRenderContext CreateContextForPlayer(const RenderablePlayer* player, const std::vector<ColoredDetail>& details, const FrameContext& context);
    static EntityRenderContext CreateContextForNpc(const RenderableNpc* npc, const std::vector<ColoredDetail>& details, const FrameContext& context);
    static EntityRenderContext CreateContextForGadget(const RenderableGadget* gadget, const std::vector<ColoredDetail>& details, const FrameContext& context);
    static EntityRenderContext CreateContextForAttackTarget(const RenderableAttackTarget* attackTarget, const std::vector<ColoredDetail>& details, const FrameContext& context);
    
    // Helper function to build the render context with details
    static EntityRenderContext CreateEntityRenderContextForRendering(const RenderableEntity* entity, const FrameContext& context);
};

} // namespace kx