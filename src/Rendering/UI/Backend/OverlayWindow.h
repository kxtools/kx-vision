#pragma once

#include <d3d11.h>
#include <chrono>
#include <memory>
#pragma comment(lib, "d3d11.lib")

// Forward declarations to avoid circular dependencies
namespace kx {
    class Camera;
    class MumbleLinkManager;
    struct MumbleLinkData;
    class FeatureManager;
    struct Settings;
    struct ServiceContext;
    struct EntityFrameSnapshot;
    using FrameSnapshotHandle = std::shared_ptr<const EntityFrameSnapshot>;
}

/**
 * @class OverlayWindow
 * @brief Manages ImGui rendering and user interface for KX-Vision
 * 
 * This class is responsible for UI rendering only. It does not own or manage
 * game state (Camera, MumbleLinkManager). Game state is passed as parameters.
 */
class OverlayWindow {
public:
    static bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd,
                           const kx::Settings& settings);
    static void NewFrame();
    static void Render(ID3D11DeviceContext* context, ID3D11RenderTargetView* mainRenderTargetView);
    
    /**
     * @brief Render the UI with provided game state
     * @param camera Reference to the camera for ESP rendering
     * @param mumbleLinkManager Reference to MumbleLink manager for connection status
     * @param mumbleLinkData Pointer to current MumbleLink data (can be nullptr)
     * @param windowHandle Window handle for input handling
     * @param displayWidth Display width for rendering
     * @param displayHeight Display height for rendering
     * @param featureManager Reference to the feature manager for feature rendering
     * @param ctx Service context for dependency injection
     */
    static void RenderUI(kx::Camera& camera, 
                        kx::MumbleLinkManager& mumbleLinkManager,
                        const kx::MumbleLinkData* mumbleLinkData,
                        HWND windowHandle,
                        float displayWidth,
                        float displayHeight,
                        kx::FeatureManager& featureManager,
                        kx::Settings& settings,
                        const kx::FrameSnapshotHandle& snapshot,
                        const kx::ServiceContext& ctx);
    
    static void Shutdown();
    
    static bool IsImGuiInitialized() { return m_isInitialized; }
    static bool IsVisible() { return m_isVisible; }
    static void ToggleVisibility() { m_isVisible = !m_isVisible; }

private:
    static void RenderESPWindow(kx::MumbleLinkManager& mumbleLinkManager, 
                               const kx::MumbleLinkData* mumbleLinkData,
                               kx::FeatureManager& featureManager,
                               kx::Settings& settings);
    static void RenderHints();

    static bool m_isInitialized;
    static bool m_isVisible;

    // NEW MEMBERS for UI-side timeout logic
    static std::chrono::steady_clock::time_point m_connectingStartTime;
    static bool m_isWaitingForConnection;
};
