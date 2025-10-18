#include "FrameCoordinator.h"
#include "AppLifecycleManager.h"
#include "AppState.h"
#include "../Rendering/ImGui/ImGuiManager.h"
#include "../Rendering/Utils/D3DState.h"
#include "../Utils/DebugLogger.h"
#include "../../libs/ImGui/imgui.h"

void FrameCoordinator::Execute(kx::AppLifecycleManager& lifecycleManager,
                               HWND windowHandle, 
                               float displayWidth, 
                               float displayHeight,
                               ID3D11DeviceContext* context, 
                               ID3D11RenderTargetView* renderTargetView) {

    // Early exit checks
    if (lifecycleManager.IsShuttingDown()) {
        return;
    }

    if (!context || !renderTargetView) {
        return;
    }

    // Update ImGui display size
    UpdateImGuiDisplaySize(displayWidth, displayHeight);

    // Handle input for UI toggle
    HandleInput(windowHandle);

    // Declare state backup here to ensure it's available in the catch block
    kx::StateBackupD3D11 d3dState;

    try {
        // CRITICAL: Backup D3D state before rendering
        BackupD3D11State(context, d3dState);

        // Update game state
        kx::MumbleLinkManager& mumbleLinkManager = lifecycleManager.GetMumbleLinkManager();
        mumbleLinkManager.Update();
        const kx::MumbleLinkData* mumbleLinkData = mumbleLinkManager.GetData();

        // Check for state transitions (GW2AL mode)
        lifecycleManager.CheckStateTransitions();

        // Update camera with current game state
        kx::Camera& camera = lifecycleManager.GetCamera();
        camera.Update(mumbleLinkManager, windowHandle);

        // Render ImGui UI
        ImGuiManager::NewFrame();
        ImGuiManager::RenderUI(camera, mumbleLinkManager, mumbleLinkData,
            windowHandle, displayWidth, displayHeight);
        ImGuiManager::Render(context, renderTargetView);

        // CRITICAL: Restore D3D state after rendering
        RestoreD3D11State(context, d3dState);
    }
    catch (...) {
        LOG_ERROR("Exception caught in FrameCoordinator::Execute");
        // Attempt to restore the original D3D state to prevent a game crash
        RestoreD3D11State(context, d3dState);
    }
}

void FrameCoordinator::HandleInput(HWND windowHandle) {
    // Handle input for UI toggle
    static bool lastToggleKeyState = false;
    bool currentToggleKeyState = (GetAsyncKeyState(VK_INSERT) & 0x8000) != 0;

    if (currentToggleKeyState && !lastToggleKeyState) {
        bool isOpen = kx::AppState::Get().IsVisionWindowOpen();
        kx::AppState::Get().SetVisionWindowOpen(!isOpen);
    }
    lastToggleKeyState = currentToggleKeyState;
}

void FrameCoordinator::UpdateImGuiDisplaySize(float displayWidth, float displayHeight) {
    // Ensure ImGui is aware of the correct display size every single frame.
    // This is robust against all resize timing issues.
    if (ImGui::GetCurrentContext()) {
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(displayWidth, displayHeight);
    }
}
