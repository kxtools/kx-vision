#pragma once

#include "LayoutCursor.h"
#include "../../../Game/Data/FrameData.h"
#include "../../../Rendering/Data/HealthBarAnimationState.h"
#include "../../../Game/GameEnums.h"
#include <string_view>

namespace kx {

struct GameEntity;
struct VisualsConfiguration;

class EntityComponentRenderer {
public:
    static void RenderGeometry(const FrameContext& ctx, const GameEntity& entity, const VisualProperties& props, const VisualsConfiguration& visualsConfig);

    static void RenderIdentity(const FrameContext& ctx,
                               const GameEntity& entity,
                               std::string_view displayName,
                               const VisualProperties& props,
                               LayoutCursor& cursor,
                               const VisualsConfiguration& visualsConfig);

    static void RenderStatusBars(const FrameContext& ctx,
                                 const GameEntity& entity,
                                 bool showCombatUI,
                                 bool renderHealthBar,
                                 bool renderEnergyBar,
                                 float burstDps,
                                 Game::Attitude attitude,
                                 const HealthBarAnimationState& animState,
                                 const VisualProperties& props,
                                 LayoutCursor& cursor,
                                 const VisualsConfiguration& visualsConfig);

    static void RenderEntityDetails(const FrameContext& ctx,
                                    const GameEntity& entity,
                                    const VisualProperties& props,
                                    LayoutCursor& cursor,
                                    const VisualsConfiguration& visualsConfig);
};

} // namespace kx

