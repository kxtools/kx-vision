#pragma once

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

#include "../Game/Camera.h"
#include "../Game/MumbleLinkManager.h"

/**
 * @class ImGuiManager
 * @brief Manages ImGui rendering and user interface for KX-Vision
 */
class ImGuiManager {
public:
    // Add a boolean to tell the initializer it's running in GW2AL mode
    // It will skip creating its own context if lib_imgui provides one.
    // For your goal, we'll tell it to create its own context regardless.
    static bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd, bool is_gw2al_mode = false);
    static void NewFrame();
    static void Render(ID3D11DeviceContext* context, ID3D11RenderTargetView* mainRenderTargetView);
    static void RenderUI();
    // Add a shutdown function that knows about the mode
    static void Shutdown(bool is_gw2al_mode = false);

private:
    static void RenderESPWindow();
    
    static void RenderHints();

    static kx::Camera m_camera;
    static kx::MumbleLinkManager m_mumbleLinkManager;
};