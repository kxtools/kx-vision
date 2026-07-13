#include "FrameCoordinator.h"
#include "AppLifecycleManager.h"
#include "AppState.h"
#include "Config.h"
#include "Architecture/FeatureManager.h"
#include "Architecture/IFeature.h"
#include "../Utils/DebugLogger.h"
#include "../Memory/Safety.h"
#include "../Game/Data/FrameData.h"
#include "../../libs/ImGui/imgui.h"
#include "UI/Backend/D3DState.h"
#include "UI/Backend/OverlayWindow.h"
#include "../Hooking/D3DRenderHook.h"
#include <mutex>

void FrameCoordinator::Execute(kx::AppLifecycleManager& lifecycleManager,
                               HWND windowHandle, 
                               float displayWidth, 
                               float displayHeight,
                               ID3D11DeviceContext* context, 
                               ID3D11RenderTargetView* renderTargetView) {

    // Guardrail: render-thread product setup must run before any frame work and
    // exactly once. Running it on entry/init thread breaks thread-local setup.
    static std::once_flag s_renderThreadStartFlag;
    std::call_once(s_renderThreadStartFlag, [&lifecycleManager]() {
        lifecycleManager.OnRenderThreadStart();
    });

    if (!CanExecute(lifecycleManager, context, renderTargetView)) {
        return;
    }

    // Declare state backup here to ensure it's available in the catch block
    kx::StateBackupD3D11 d3dState;
    
    // Track if we successfully started an ImGui frame
    bool isImGuiFrameActive = false;

    try {
        // CRITICAL: Backup D3D state before rendering
        BackupD3D11State(context, d3dState);

        const kx::MumbleLinkData* mumbleLinkData =
            PrepareGameState(lifecycleManager, windowHandle, displayWidth, displayHeight);

        OverlayWindow::NewFrame();
        isImGuiFrameActive = true;

        if (lifecycleManager.RequiresExclusiveOverlay()) {
            // Guardrail: exclusive product overlays preempt feature execution.
            // Moving this later lets locked features execute off-screen.
            RenderExclusiveOverlayFrame(lifecycleManager, context, renderTargetView);
        } else {
            RenderFeatureFrame(lifecycleManager, mumbleLinkData, windowHandle,
                displayWidth, displayHeight, context, renderTargetView);
        }
        isImGuiFrameActive = false;

        // CRITICAL: Restore D3D state after rendering
        RestoreD3D11State(context, d3dState);
    }
    catch (...) {
        RecoverFailedFrame(context, d3dState, isImGuiFrameActive);
    }
}

bool FrameCoordinator::CanExecute(const kx::AppLifecycleManager& lifecycleManager,
                                  ID3D11DeviceContext* context,
                                  ID3D11RenderTargetView* renderTargetView) {
    return !lifecycleManager.IsShuttingDown() &&
           context && renderTargetView && ImGui::GetCurrentContext();
}

const kx::MumbleLinkData* FrameCoordinator::PrepareGameState(
    kx::AppLifecycleManager& lifecycleManager,
    HWND windowHandle,
    float displayWidth,
    float displayHeight) {
    UpdateImGuiDisplaySize(displayWidth, displayHeight);
    HandleInput(windowHandle);
    kx::Hooking::D3DRenderHook::SyncOverlayInputState();

    kx::MumbleLinkManager& mumbleLinkManager = lifecycleManager.GetMumbleLinkManager();
    mumbleLinkManager.Update();

#ifdef GW2AL_BUILD
    lifecycleManager.CheckStateTransitions();
#endif

    lifecycleManager.GetCamera().Update(mumbleLinkManager, windowHandle);
    return mumbleLinkManager.GetData();
}

void FrameCoordinator::RenderExclusiveOverlayFrame(kx::AppLifecycleManager& lifecycleManager,
                                                   ID3D11DeviceContext* context,
                                                   ID3D11RenderTargetView* renderTargetView) {
    lifecycleManager.RenderRuntimeOverlay();
    kx::Hooking::D3DRenderHook::CaptureOverlayRouting();
    OverlayWindow::Render(context, renderTargetView);
}

void FrameCoordinator::RenderFeatureFrame(kx::AppLifecycleManager& lifecycleManager,
                                          const kx::MumbleLinkData* mumbleLinkData,
                                          HWND windowHandle,
                                          float displayWidth,
                                          float displayHeight,
                                          ID3D11DeviceContext* context,
                                          ID3D11RenderTargetView* renderTargetView) {
    kx::FeatureManager& featureManager = lifecycleManager.GetFeatureManager();
    // Guardrail: acquire once per render frame and forward same handle through update
    // and draw. A second acquisition can mix entity generations inside one frame.
    const kx::FrameSnapshotHandle snapshot = lifecycleManager.GetEntityManager().AcquireFrameSnapshot();
    const kx::ServiceContext& ctx = lifecycleManager.GetServiceContext();

    const auto now = std::chrono::steady_clock::now();
    static auto lastFrameTime = now;
    const std::chrono::duration<float> elapsed = now - lastFrameTime;
    lastFrameTime = now;
    // Guardrail: general features receive real monotonic elapsed time so deadlines
    // survive stalls. Clamp only at movement/physics integration call sites.
    const float realDeltaTime = (std::max)(0.0f, elapsed.count());
    featureManager.UpdateAll(realDeltaTime, snapshot, ctx);

    kx::MumbleLinkManager& mumbleLinkManager = lifecycleManager.GetMumbleLinkManager();
    OverlayWindow::RenderUI(lifecycleManager.GetCamera(), mumbleLinkManager, mumbleLinkData,
        windowHandle, displayWidth, displayHeight, featureManager, snapshot, ctx);
    lifecycleManager.RenderRuntimeOverlay();
    kx::Hooking::D3DRenderHook::CaptureOverlayRouting();
    OverlayWindow::Render(context, renderTargetView);
}

void FrameCoordinator::RecoverFailedFrame(ID3D11DeviceContext* context,
                                          kx::StateBackupD3D11& d3dState,
                                          bool isImGuiFrameActive) {
    LOG_ERROR("Exception caught in FrameCoordinator::Execute");
    if (isImGuiFrameActive && ImGui::GetCurrentContext()) {
        ImGui::EndFrame();
    }
    RestoreD3D11State(context, d3dState);
}

void FrameCoordinator::HandleInput(HWND windowHandle) {
    // Handle input for UI toggle
    static bool lastToggleKeyState = false;
    bool currentToggleKeyState = (GetAsyncKeyState(kx::Hotkeys::TOGGLE_OVERLAY) & 0x8000) != 0;

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
