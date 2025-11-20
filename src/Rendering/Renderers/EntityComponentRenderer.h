#pragma once

#include "LayoutCursor.h"
#include "../Data/FrameData.h"
#include "../Data/HealthBarAnimationState.h"
#include "../../Game/GameEnums.h"
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

