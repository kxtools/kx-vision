#pragma once

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

#include "Camera.h" // Include Camera header

/**
 * @class ImGuiManager
 * @brief Manages ImGui rendering and user interface for KX-Vision
 */
class ImGuiManager {
public:
    /**
     * @brief Initialize ImGui and related components
     * @param device D3D11 device
     * @param context D3D11 device context
     * @param hwnd Window handle
     * @return True if initialization succeeded, false otherwise
     */
    static bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd);
    
    /**
     * @brief Prepare ImGui for a new frame
     */
    static void NewFrame();
    
    /**
     * @brief Render ImGui contents to the screen
     * @param context D3D11 device context
     * @param mainRenderTargetView Main render target view
     */
    static void Render(ID3D11DeviceContext* context, ID3D11RenderTargetView* mainRenderTargetView);
    
    /**
     * @brief Render all user interface elements
     */
    static void RenderUI();
    
    /**
     * @brief Clean up ImGui resources
     */
    static void Shutdown();

private:
    /**
     * @brief Render the main KX-Vision ESP settings window
     */
    static void RenderESPWindow();
    
    /**
     * @brief Render information section with links
     */
    static void RenderInfoSection();
    
    /**
     * @brief Render keyboard shortcut hints
     */
    static void RenderHints();
    
    /**
     * @brief Camera used for ESP and 3D-2D projections
     */
    static kx::Camera m_camera;
};
