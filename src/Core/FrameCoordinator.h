#pragma once

#include <d3d11.h>
#include <windows.h>

// Forward declarations to avoid circular dependencies
namespace kx {
    class Camera;
    class GameEngineService;
    class MumbleLinkManager;
    class AppLifecycleManager;
    struct MumbleLinkData;
    struct StateBackupD3D11;
}

/**
 * @brief Coordinates per-frame rendering operations
 * 
 * Extracts the rendering logic from AppLifecycleManager to improve
 * separation of concerns. Handles D3D state management, input processing,
 * and coordinates between game state updates and UI rendering.
 */
class FrameCoordinator {
public:
    /**
     * @brief Execute a complete frame rendering cycle
     * 
     * This method contains all the shared per-frame logic that needs to
     * happen before ImGui rendering in both DLL and GW2AL modes:
     * - Update MumbleLink data
     * - Update camera
     * - Handle UI input
     * - Render ImGui UI
     * 
     * @param lifecycleManager Reference to the app lifecycle manager
     * @param gameEngineService Reference to game-runtime state and snapshots
     * @param windowHandle The HWND of the game window
     * @param displayWidth The width of the display/viewport
     * @param displayHeight The height of the display/viewport
     * @param context The D3D11 device context (for rendering)
     * @param renderTargetView The render target view to render to
     */
    static void Execute(kx::AppLifecycleManager& lifecycleManager,
                       kx::GameEngineService& gameEngineService,
                       HWND windowHandle, 
                       float displayWidth, 
                       float displayHeight,
                       ID3D11DeviceContext* context, 
                       ID3D11RenderTargetView* renderTargetView);

private:
    static bool CanExecute(const kx::AppLifecycleManager& lifecycleManager,
                           ID3D11DeviceContext* context,
                           ID3D11RenderTargetView* renderTargetView);

    static const kx::MumbleLinkData* PrepareGameState(kx::AppLifecycleManager& lifecycleManager,
                                                       kx::GameEngineService& gameEngineService,
                                                       HWND windowHandle,
                                                       float displayWidth,
                                                       float displayHeight);

    static void RenderExclusiveOverlayFrame(kx::AppLifecycleManager& lifecycleManager,
                                            ID3D11DeviceContext* context,
                                            ID3D11RenderTargetView* renderTargetView);

    static void RenderFeatureFrame(kx::AppLifecycleManager& lifecycleManager,
                                   kx::GameEngineService& gameEngineService,
                                   const kx::MumbleLinkData* mumbleLinkData,
                                   HWND windowHandle,
                                   float displayWidth,
                                   float displayHeight,
                                   ID3D11DeviceContext* context,
                                   ID3D11RenderTargetView* renderTargetView);

    static void RecoverFailedFrame(ID3D11DeviceContext* context,
                                   kx::StateBackupD3D11& d3dState,
                                   bool isImGuiFrameActive);

    /**
     * @brief Handle UI input (toggle keys, etc.)
     * @param windowHandle Window handle for input context
     */
    static void HandleInput(HWND windowHandle);

    /**
     * @brief Update ImGui display size
     * @param displayWidth Display width
     * @param displayHeight Display height
     */
    static void UpdateImGuiDisplaySize(float displayWidth, float displayHeight);
};
