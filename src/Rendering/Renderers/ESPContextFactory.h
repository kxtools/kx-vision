#pragma once

#include <vector>
#include "../Data/RenderableData.h"
#include "../Data/EntityRenderContext.h"
#include "../../Core/Settings.h"

namespace kx {

class CombatStateManager; // Forward declaration

class ESPContextFactory {
public:
    static EntityRenderContext CreateContextForPlayer(const RenderablePlayer* player, const std::vector<ColoredDetail>& details, const FrameContext& context);
    static EntityRenderContext CreateContextForNpc(const RenderableNpc* npc, const std::vector<ColoredDetail>& details, const FrameContext& context);
    static EntityRenderContext CreateContextForGadget(const RenderableGadget* gadget, const std::vector<ColoredDetail>& details, const FrameContext& context);
};

} // namespace kx