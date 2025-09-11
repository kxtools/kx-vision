#pragma once

#include <string>
#include <vector>
#include <map>

#pragma comment(lib, "d3d11.lib")
#include "../Game/Camera.h" // Include Camera header
#include "../Game/MumbleLink.h" // Include MumbleLink for the parameter

// Forward declarations for external types
struct ImDrawList; // Forward declare ImGui's ImDrawList (not in kx namespace)
struct ImVec2;

namespace kx {

enum class ESPEntityType {
    Player,
    NPC,
    Gadget
};

class Agent; // Forward declare Agent class in kx namespace
namespace ReClass { class ChCliCharacter; }
namespace ReClass { class GdCliGadget; }

class ESPRenderer {
public:
    static void Initialize(Camera& camera);
    static void Render(float screenWidth, float screenHeight, const MumbleLinkData* mumbleData);

private:
    static void RenderGadgets(ImDrawList* drawList, float screenWidth, float screenHeight);
    static void RenderAllEntities(ImDrawList* drawList, float screenWidth, float screenHeight);
    static void RenderPlayer(ImDrawList* drawList, float screenWidth, float screenHeight, kx::ReClass::ChCliCharacter& character, const std::map<void*, const wchar_t*>& characterNameToPlayerName);
    static void RenderNpc(ImDrawList* drawList, float screenWidth, float screenHeight, kx::ReClass::ChCliCharacter& character);
    static void RenderObject(ImDrawList* drawList, float screenWidth, float screenHeight, kx::ReClass::GdCliGadget& gadget);

    static void RenderEntity(ImDrawList* drawList, const glm::vec3& worldPos, float distance, float screenWidth, float screenHeight, unsigned int color, const std::vector<std::string>& details, float healthPercent, bool renderBox, bool renderDistance, bool renderDot, bool renderDetails, bool renderHealthBar, ESPEntityType entityType = ESPEntityType::Player);
    static bool ShouldHideESP(const MumbleLinkData* mumbleData);

    // Universal ESP rendering helpers
    static void RenderHealthBar(ImDrawList* drawList, const ImVec2& boxMin, const ImVec2& boxMax, float healthPercent);
    static void RenderBoundingBox(ImDrawList* drawList, const ImVec2& boxMin, const ImVec2& boxMax, unsigned int color);
    static void RenderDistanceText(ImDrawList* drawList, const ImVec2& center, const ImVec2& boxMin, float distance);
    static void RenderCenterDot(ImDrawList* drawList, const glm::vec2& feetPos);
    static void RenderDetailsText(ImDrawList* drawList, const ImVec2& center, const ImVec2& boxMax, const std::vector<std::string>& details);

    static Camera* s_camera; // Camera reference for world-to-screen projections
};

} // namespace kx
