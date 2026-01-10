#pragma once

#include "../../../Game/GameEnums.h"

namespace kx {

struct FrameContext;
struct PlayerEntity;
struct VisualProperties;

class TrailRenderer {
public:
    static void RenderPlayerTrail(
        const FrameContext& context,
        const PlayerEntity& player,
        Game::Attitude attitude,
        const VisualProperties& props);
};

} // namespace kx

