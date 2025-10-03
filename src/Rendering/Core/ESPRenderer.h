#pragma once

#include "../../Game/Camera.h"
#include "../../Game/MumbleLink.h"
#include "../Data/RenderableData.h"
#include <unordered_map>

namespace kx {

class ESPRenderer {
public:
    static void Initialize(Camera& camera);
    static void Render(float screenWidth, float screenHeight, const MumbleLinkData* mumbleData);

    // Access to persistent entity state for interpolation
    static std::unordered_map<const void*, RenderablePlayer>& GetPlayerData() { return s_playerData; }
    static std::unordered_map<const void*, RenderableNpc>& GetNpcData() { return s_npcData; }
    static std::unordered_map<const void*, RenderableGadget>& GetGadgetData() { return s_gadgetData; }

private:
    static bool ShouldHideESP(const MumbleLinkData* mumbleData);
    
    /**
     * @brief Clean up stale entities from persistent maps
     * Removes entities that haven't been updated in the last 5 seconds
     */
    static void CleanupStaleEntities();
    
    static Camera* s_camera; // Camera reference for world-to-screen projections
    
    // Persistent entity state for interpolation (maps entity address to data)
    static std::unordered_map<const void*, RenderablePlayer> s_playerData;
    static std::unordered_map<const void*, RenderableNpc> s_npcData;
    static std::unordered_map<const void*, RenderableGadget> s_gadgetData;
};

} // namespace kx

