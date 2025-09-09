#pragma once

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")
#include "Camera.h" // Include Camera header

// Forward declarations for external types
struct ImDrawList; // Forward declare ImGui's ImDrawList (not in kx namespace)

namespace kx {

class Agent; // Forward declare Agent class in kx namespace

class ESPRenderer {
public:
    /**
     * @brief Initialize the ESP renderer with necessary dependencies
     * @param camera Reference to the camera object used for world-to-screen projection
     */
    static void Initialize(Camera& camera);
    
    /**
     * @brief Render ESP elements on screen
     * @param screenWidth Width of the screen
     * @param screenHeight Height of the screen
     */
    static void Render(float screenWidth, float screenHeight);

private:
    /**
     * @brief Renders a single agent on the screen
     * @param drawList ImGui draw list for rendering
     * @param agent Agent object to render
     * @param screenWidth Width of the screen
     * @param screenHeight Height of the screen
     */
    static void RenderAgent(ImDrawList* drawList, Agent& agent, float screenWidth, float screenHeight);

    /**
     * @brief Checks if ESP should be hidden (e.g., when map is open)
     * @return true if ESP should be hidden, false otherwise
     */
    static bool ShouldHideESP();

    static Camera* s_camera; // Camera reference for world-to-screen projections
};

} // namespace kx
