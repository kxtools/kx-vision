#pragma once

#include "../../../Rendering/Renderers/LayoutCursor.h"
#include "../../../Rendering/Data/FrameData.h"
#include "../../../Rendering/Data/HealthBarAnimationState.h"
#include "../../../Game/GameEnums.h"
#include <string_view>

namespace kx {

struct RenderableEntity;

class EntityComponentRenderer {
public:
    static void RenderGeometry(const FrameContext& ctx, const RenderableEntity& entity, const VisualProperties& props);

    static void RenderIdentity(const FrameContext& ctx,
                               const RenderableEntity& entity,
                               std::string_view displayName,
                               const VisualProperties& props,
                               LayoutCursor& cursor);

    static void RenderStatusBars(const FrameContext& ctx,
                                 const RenderableEntity& entity,
                                 bool showCombatUI,
                                 bool renderHealthBar,
                                 bool renderEnergyBar,
                                 float burstDps,
                                 Game::Attitude attitude,
                                 const HealthBarAnimationState& animState,
                                 const VisualProperties& props,
                                 LayoutCursor& cursor);

    static void RenderEntityDetails(const FrameContext& ctx,
                                    const RenderableEntity& entity,
                                    const VisualProperties& props,
                                    LayoutCursor& cursor);
};

} // namespace kx

