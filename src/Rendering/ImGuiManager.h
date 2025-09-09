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
    static bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd);
    static void NewFrame();
    static void Render(ID3D11DeviceContext* context, ID3D11RenderTargetView* mainRenderTargetView);
    static void RenderUI();
    static void Shutdown();

private:
    static void RenderESPWindow();
    static void RenderInfoSection();
    static void RenderHints();

    static kx::Camera m_camera;
    static kx::MumbleLinkManager m_mumbleLinkManager;
};