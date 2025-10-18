#include "AppLifecycleManager.h"
#include "Config.h"
#include "AppState.h"
#include "SettingsManager.h"
#include "AddressManager.h"
#include "HookManager.h"
#include "Hooks.h"
#include "../Rendering/ImGui/ImGuiManager.h"
#include "../Rendering/Core/ESPRenderer.h"
#include "../Rendering/Core/D3DState.h"
#include "../Hooking/D3DRenderHook.h"
#include "../Utils/DebugLogger.h"
#include "../../libs/ImGui/imgui.h"

namespace kx {

    // Global instance of AppLifecycleManager
    AppLifecycleManager g_App;

    bool AppLifecycleManager::Initialize() {
        LOG_INFO("AppLifecycleManager: Starting initialization");

        // Initialize HookManager (MinHook)
        if (!kx::Hooking::HookManager::Initialize()) {
            LOG_ERROR("AppLifecycleManager: Failed to initialize HookManager");
            return false;
        }
        LOG_INFO("AppLifecycleManager: HookManager initialized");

        // Initialize hooks (this sets up D3D hook which creates ImGuiManager)
        if (!InitializeHooks()) {
            LOG_ERROR("AppLifecycleManager: Failed to initialize hooks");
            return false;
        }

        // Set this lifecycle manager in D3DRenderHook so it can access Camera and MumbleLink
        kx::Hooking::D3DRenderHook::SetLifecycleManager(this);

        LOG_INFO("AppLifecycleManager: Hooks initialized successfully");
        m_currentState = State::WaitingForImGui;

        return true;
    }

    bool AppLifecycleManager::InitializeForGW2AL() {
        LOG_INFO("AppLifecycleManager: Initializing for GW2AL mode");

        // Initialize HookManager (MinHook) - needed for game thread hook later
        if (!kx::Hooking::HookManager::Initialize()) {
            LOG_ERROR("AppLifecycleManager: Failed to initialize HookManager");
            return false;
        }
        LOG_INFO("AppLifecycleManager: HookManager initialized");

        // In GW2AL mode, hooks are managed differently
        // We don't initialize the Present hook here (GW2AL handles that)
        // We just set up the lifecycle manager

        m_currentState = State::WaitingForRenderer;
        LOG_INFO("AppLifecycleManager: Waiting for renderer initialization from GW2AL");

        return true;
    }

    void AppLifecycleManager::OnRendererInitialized() {
        LOG_INFO("AppLifecycleManager: Renderer initialized, waiting for player to be in-game");

        // Set this lifecycle manager in D3DRenderHook so it can access Camera and MumbleLink
        kx::Hooking::D3DRenderHook::SetLifecycleManager(this);

        // Transition to WaitingForGame - we'll check in OnPresent if we should initialize
        m_currentState = State::WaitingForGame;
    }

    void AppLifecycleManager::Update() {
        switch (m_currentState) {
        case State::PreInit:
            HandlePreInitState();
            break;

        case State::WaitingForImGui:
            HandleWaitingForImGuiState();
            break;

        case State::WaitingForRenderer:
            HandleWaitingForRendererState();
            break;

        case State::WaitingForGame:
            HandleWaitingForGameState();
            break;

        case State::InitializingServices:
            HandleInitializingServicesState();
            break;

        case State::Running:
            HandleRunningState();
            break;

        case State::ShuttingDown:
            HandleShuttingDownState();
            break;
        }
    }

    void AppLifecycleManager::SaveSettingsOnExit() {
        // Use the existing shutdown flag in AppState to prevent saving more than once.
        if (AppState::Get().IsShuttingDown()) {
            return; // Shutdown/save has already been initiated.
        }
        
        // Signal that shutdown has begun.
        AppState::Get().SetShuttingDown(true);
    
        LOG_INFO("AppLifecycleManager: Performing final settings save on exit...");
    
        // Check the user's preference and save the settings.
        if (AppState::Get().GetSettings().autoSaveOnExit) {
            SettingsManager::Save(AppState::Get().GetSettings());
        }
    }
    
    void AppLifecycleManager::Shutdown() {
        LOG_INFO("AppLifecycleManager: Full shutdown requested");
    
        // Perform the save FIRST. This function handles the atomic flag.
        SaveSettingsOnExit(); 
    
        m_currentState = State::ShuttingDown;
    
        // Give hooks a moment to recognize the flag before cleanup starts
        // This helps prevent calls into ImGui after it's destroyed
        Sleep(250);
    
        CleanupServices();
    
        LOG_INFO("AppLifecycleManager: Shutdown complete");
    }
    bool AppLifecycleManager::IsShutdownRequested() const {
        // Only DELETE key triggers shutdown (not the window close button)
        // Closing the window (X button) just hides it - user can press INSERT to show again
        return (GetAsyncKeyState(VK_DELETE) & 0x8000);
    }

    const char* AppLifecycleManager::GetCurrentStateName() const {
        switch (m_currentState) {
        case State::PreInit:              return "PreInit";
        case State::WaitingForImGui:      return "WaitingForImGui";
        case State::WaitingForRenderer:   return "WaitingForRenderer";
        case State::WaitingForGame:       return "WaitingForGame";
        case State::InitializingServices: return "InitializingServices";
        case State::Running:              return "Running";
        case State::ShuttingDown:         return "ShuttingDown";
        default:                          return "Unknown";
        }
    }

    // ===== Private State Handlers =====

    void AppLifecycleManager::HandlePreInitState() {
        // This state is handled by Initialize()
        // Should never reach here in normal flow
        LOG_WARN("AppLifecycleManager: HandlePreInitState called - unexpected");
    }

    void AppLifecycleManager::HandleWaitingForImGuiState() {
        if (IsImGuiReady()) {
            LOG_INFO("AppLifecycleManager: ImGui is ready, transitioning to WaitingForGame");
            m_currentState = State::WaitingForGame;
    } else {
            // ImGui not initialized yet (waiting for first Present call)
            Sleep(500);
        }
    }

    void AppLifecycleManager::HandleWaitingForRendererState() {
        // In GW2AL mode, we just wait here until OnRendererInitialized() is called
        // This state is transitioned by the GW2AL event callback
        Sleep(100);
    }

    void AppLifecycleManager::HandleWaitingForGameState() {
        // Initialize MumbleLink manager (needs to be done before checking if player is in-game)
        if (!m_mumbleLinkManager.IsInitialized()) {
            // MumbleLink manager will keep trying until it connects
            m_mumbleLinkManager.Update();
        }

        if (IsPlayerInGame()) {
            LOG_INFO("AppLifecycleManager: Player is in-game, transitioning to InitializingServices");
            m_currentState = State::InitializingServices;
    } else {
            // Not in-game yet, wait before checking again
            Sleep(500);
        }
    }

    void AppLifecycleManager::CheckAndInitializeServices() {
        // Only run this check if we're in WaitingForGame state
        if (m_currentState != State::WaitingForGame) {
            return;
        }

        // Check if MumbleLink is connected and player is in-game (GW2AL mode only)
        // In DLL mode, this is handled by the Update() loop via HandleWaitingForGameState()
        if (m_mumbleLinkManager.IsInitialized() && IsPlayerInGame()) {
            LOG_INFO("AppLifecycleManager: Player is in-game, initializing services");

            if (InitializeGameServices()) {
                LOG_INFO("AppLifecycleManager: Services initialized, transitioning to Running");
                m_currentState = State::Running;
                m_servicesInitialized = true;
        } else {
                LOG_ERROR("AppLifecycleManager: Service initialization failed");
                m_currentState = State::ShuttingDown;
            }
        }
    }

    void AppLifecycleManager::RenderTick(HWND windowHandle, float displayWidth, float displayHeight,
        ID3D11DeviceContext* context, ID3D11RenderTargetView* renderTargetView) {

        if (m_currentState == State::ShuttingDown) {
            return;
        }

        if (!context || !renderTargetView) {
            return;
        }

        // Ensure ImGui is aware of the correct display size every single frame.
        // This is robust against all resize timing issues.
        if (ImGui::GetCurrentContext()) {
            ImGuiIO& io = ImGui::GetIO();
            io.DisplaySize = ImVec2(displayWidth, displayHeight);
        }

        // Handle input for UI toggle
        static bool lastToggleKeyState = false;
        bool currentToggleKeyState = (GetAsyncKeyState(VK_INSERT) & 0x8000) != 0;

        if (currentToggleKeyState && !lastToggleKeyState) {
            bool isOpen = AppState::Get().IsVisionWindowOpen();
            AppState::Get().SetVisionWindowOpen(!isOpen);
        }
        lastToggleKeyState = currentToggleKeyState;

        // Declare state backup here to ensure it's available in the catch block
        StateBackupD3D11 d3dState;

        try {
            // CRITICAL: Backup D3D state before rendering
            BackupD3D11State(context, d3dState);

            m_mumbleLinkManager.Update();
            const MumbleLinkData* mumbleLinkData = m_mumbleLinkManager.GetData();

            CheckAndInitializeServices();

            if (mumbleLinkData) {
                m_camera.Update(mumbleLinkData, windowHandle);
            }

            ImGuiManager::NewFrame();
            ImGuiManager::RenderUI(m_camera, m_mumbleLinkManager, mumbleLinkData,
                windowHandle, displayWidth, displayHeight);
            ImGuiManager::Render(context, renderTargetView);

            // CRITICAL: Restore D3D state after rendering
            RestoreD3D11State(context, d3dState);
        }
        catch (...) {
            LOG_ERROR("Exception caught in RenderTick");
            // Attempt to restore the original D3D state to prevent a game crash
            RestoreD3D11State(context, d3dState);
        }
    }

    void AppLifecycleManager::HandleInitializingServicesState() {
        if (InitializeGameServices()) {
            LOG_INFO("AppLifecycleManager: Services initialized, transitioning to Running");
            m_currentState = State::Running;
            m_servicesInitialized = true;
    } else {
            LOG_ERROR("AppLifecycleManager: Service initialization failed, shutting down");
            m_currentState = State::ShuttingDown;
        }
    }

    void AppLifecycleManager::HandleRunningState() {
        // Normal operation - just sleep to avoid busy-waiting
        // Note: Camera and MumbleLink are updated per-frame in D3DRenderHook::RenderFrame
        Sleep(100);
    }

    void AppLifecycleManager::HandleShuttingDownState() {
        // Shutdown is handled by Shutdown() method
        // This state just prevents further updates
    }

    // ===== Private Helper Methods =====

    bool AppLifecycleManager::IsImGuiReady() const {
        return ImGuiManager::IsImGuiInitialized();
    }

    bool AppLifecycleManager::IsPlayerInGame() const {
        if (!IsImGuiReady()) {
            return false;
        }

        const MumbleLinkData* data = m_mumbleLinkManager.GetData();

        // Check if MumbleLink is connected and player is in a map (mapId != 0)
        if (data && m_mumbleLinkManager.IsInitialized() && data->context.mapId != 0) {
            LOG_INFO("AppLifecycleManager: Player is in-map (Map ID: %u)", data->context.mapId);
            return true;
        }

        return false;
    }

    bool AppLifecycleManager::InitializeGameServices() {
        LOG_INFO("AppLifecycleManager: Initializing game services");

        // Initialize AddressManager
        AddressManager::Initialize();
        LOG_INFO("AppLifecycleManager: AddressManager initialized");

        // Initialize ESPRenderer with Camera reference
        ESPRenderer::Initialize(m_camera);
        LOG_INFO("AppLifecycleManager: ESPRenderer initialized");

        // Note: HookManager was initialized earlier in Initialize() or InitializeForGW2AL()

        // Initialize the game thread hook (both DLL and GW2AL modes)
        if (InitializeGameThreadHook()) {
            LOG_INFO("AppLifecycleManager: Game thread hook initialized successfully");
    } else {
            LOG_WARN("AppLifecycleManager: Game thread hook initialization failed - ESP may not work");
        }

        return true;
    }

    void AppLifecycleManager::CleanupServices() {
        if (m_servicesInitialized) {
            LOG_INFO("AppLifecycleManager: Cleaning up services");

            // Clear lifecycle manager pointer in D3DRenderHook
            kx::Hooking::D3DRenderHook::SetLifecycleManager(nullptr);

            // Cleanup hooks and ImGui
            CleanupHooks();

            m_servicesInitialized = false;
        }
    }

    ID3D11Device* AppLifecycleManager::GetDevice() const {
        return kx::Hooking::D3DRenderHook::GetDevice();
    }

} // namespace kx
