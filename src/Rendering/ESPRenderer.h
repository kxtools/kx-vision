#pragma once

#include <string>
#include <vector>
#include <map>

#pragma comment(lib, "d3d11.lib")
#include "../Game/Camera.h" // Include Camera header
#include "../Game/MumbleLink.h" // Include MumbleLink for the parameter

// Forward declarations for external types
struct ImDrawList; // Forward declare ImGui's ImDrawList (not in kx namespace)

namespace kx {

class Agent; // Forward declare Agent class in kx namespace
namespace ReClass { class ChCliCharacter; }

class ESPRenderer {
public:
    static void Initialize(Camera& camera);
    static void Render(float screenWidth, float screenHeight, const MumbleLinkData* mumbleData);

private:
    static void RenderAllEntities(ImDrawList* drawList, float screenWidth, float screenHeight);
    static void RenderPlayer(ImDrawList* drawList, float screenWidth, float screenHeight, kx::ReClass::ChCliCharacter& character, const std::map<void*, const wchar_t*>& characterNameToPlayerName);
    static void RenderNpc(ImDrawList* drawList, float screenWidth, float screenHeight, kx::ReClass::ChCliCharacter& character);
    static void RenderObject(ImDrawList* drawList, float screenWidth, float screenHeight, Agent& agent);

    static void RenderEntity(ImDrawList* drawList, const glm::vec3& worldPos, float distance, float screenWidth, float screenHeight, unsigned int color, const std::vector<std::string>& details, float healthPercent, bool renderBox, bool renderDistance, bool renderDot, bool renderDetails);
    static bool ShouldHideESP(const MumbleLinkData* mumbleData);

    static Camera* s_camera; // Camera reference for world-to-screen projections
};

} // namespace kx
