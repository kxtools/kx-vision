#pragma once

#include "../Game/Services/Camera/Camera.h"
#include "../../Game/Services/Mumble/MumbleLink.h"
#include "../Data/RenderableData.h"
#include "../../Utils/ObjectPool.h"
#include "../../Features/Combat/CombatStateManager.h"
#include "Data/FrameData.h"
#include "../Shared/LayoutConstants.h"
#include <ankerl/unordered_dense.h>
#include <vector>

namespace kx {

class MasterRenderer {
public:
    MasterRenderer();
    ~MasterRenderer() = default;

    void Render(float screenWidth, float screenHeight, const MumbleLinkData* mumbleData, Camera& camera);

    void Reset();

private:
    bool ShouldHideESP(const MumbleLinkData* mumbleData);
    
    /**
     * @brief Executes the low-frequency data processing pipeline if the update interval has passed.
     * This includes data extraction, combat state updates, filtering, and visual processing.
     * @param context The current frame's context.
     * @param currentTimeSeconds The current time in seconds, used to check the update interval.
     */
    void UpdateESPData(const FrameContext& context, float currentTimeSeconds);

    ObjectPool<RenderablePlayer> m_playerPool{EntityLimits::MAX_PLAYERS};
    ObjectPool<RenderableNpc> m_npcPool{EntityLimits::MAX_NPCS};
    ObjectPool<RenderableGadget> m_gadgetPool{EntityLimits::MAX_GADGETS};
    ObjectPool<RenderableAttackTarget> m_attackTargetPool{EntityLimits::MAX_ATTACK_TARGETS};
    ObjectPool<RenderableItem> m_itemPool{EntityLimits::MAX_ITEMS};

    CombatStateManager m_combatStateManager;
    PooledFrameRenderData m_processedRenderData;
    
    PooledFrameRenderData m_extractionData;
    
    float m_lastUpdateTime = 0.0f;
    ankerl::unordered_dense::set<CombatStateKey, CombatStateKeyHash> m_activeKeys;
    std::vector<RenderableEntity*> m_allEntitiesBuffer;
};

} // namespace kx

