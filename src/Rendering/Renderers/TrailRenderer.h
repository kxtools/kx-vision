#pragma once

#include "../../../libs/ImGui/imgui.h"
#include "../../Features/Combat/CombatState.h"
#include "../../Game/GameEnums.h"

namespace kx {

struct FrameContext;
struct RenderablePlayer;
struct VisualProperties;

class TrailRenderer {
public:
    static void RenderPlayerTrail(
        const FrameContext& context,
        const RenderablePlayer& player,
        Game::Attitude attitude,
        const VisualProperties& props);
};

} // namespace kx

