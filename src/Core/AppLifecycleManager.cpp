#include "AppLifecycleManager.h"
#include "Config.h"
#include "AppState.h"
#include "SettingsManager.h"
#include "FrameCoordinator.h"
#include "AddressManager.h"
#include "HookManager.h"
#include "Hooks.h"
#include "../Rendering/Core/MasterRenderer.h"
#include "../Hooking/D3DRenderHook.h"
#include "../Utils/DebugLogger.h"
#include "../../libs/ImGui/imgui.h"
#include <windows.h>
#include <shellapi.h>

#include "GUI/Backend/OverlayWindow.h"

namespace kx {

    // Global instance of AppLifecycleManager
    AppLifecycleManager g_App;

    bool AppLifecycleManager::Initialize() {
        LOG_INFO("AppLifecycleManager: Starting initialization");

        // Initialize HookManager (MinHook)
        if (!Hooking::HookManager::Initialize()) {
            LOG_ERROR("AppLifecycleManager: Failed to initialize HookManager - MinHook initialization failed");
            return false;
        }
        LOG_INFO("AppLifecycleManager: HookManager initialized");

        // Initialize hooks (this sets up D3D hook which creates ImGuiManager)
        if (!InitializeHooks()) {
            LOG_ERROR("AppLifecycleManager: Failed to initialize hooks - D3D Present hook setup failed");
            return false;
        }

        // Set this lifecycle manager in D3DRenderHook so it can access Camera and MumbleLink
        Hooking::D3DRenderHook::SetLifecycleManager(this);

        LOG_INFO("AppLifecycleManager: Hooks initialized successfully");
        m_currentState = State::WaitingForImGui;

        return true;
    }

    bool AppLifecycleManager::InitializeForGW2AL() {
        LOG_INFO("AppLifecycleManager: Initializing for GW2AL mode");

        // Initialize HookManager (MinHook) - needed for game thread hook later
        if (!Hooking::HookManager::Initialize()) {
            LOG_ERROR("AppLifecycleManager: Failed to initialize HookManager for GW2AL mode - MinHook initialization failed");
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
        Hooking::D3DRenderHook::SetLifecycleManager(this);

        // Transition to WaitingForGame - services will be initialized when player enters game
        m_currentState = State::WaitingForGame;
    }

    void AppLifecycleManager::Update() {
#ifdef GW2AL_BUILD
        LOG_ERROR("AppLifecycleManager::Update() should not be called in GW2AL mode - state transitions handled by render thread");
        return;
#endif

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
    
    void AppLifecycleManager::ShowDonationPromptIfNeeded() {
        // Don't show in debug builds
#ifdef _DEBUG
        return;
#endif
        
        // If already shown this session, skip
        if (AppState::Get().IsDonationPromptShown()) {
            return;
        }
        
        // Use QueryPerformanceCounter for better randomness (no seeding needed)
        LARGE_INTEGER counter;
        QueryPerformanceCounter(&counter);
        int randomValue = static_cast<int>(counter.QuadPart % 100);
        
        // 20% chance to show
        if (randomValue < 20) {
            AppState::Get().SetDonationPromptShown(true);
            
            // Bring window to foreground
            HWND hwnd = GetForegroundWindow();
            if (hwnd) {
                SetForegroundWindow(hwnd);
            }
            
        const wchar_t* title = L"Thank You for Using KX Vision!";
        const wchar_t* message = 
            L"Thank you for using KX Vision!\r\n\r\n"
            L"This is free, open-source software, but it still costs money to write, support, and distribute it.\r\n\r\n"
            L"If you enjoy using it, please consider a donation to help:\r\n"
            L"• Build new features and fix bugs\r\n"
            L"• Keep it 100% free and ad-free forever\r\n\r\n"
            L"Click Yes to visit my GitHub Sponsors page.";
        
            int result = MessageBoxW(NULL, message, title, MB_ICONINFORMATION | MB_YESNO | MB_TOPMOST | MB_SETFOREGROUND);
            
            if (result == IDYES) {
                ShellExecuteW(NULL, L"open", L"https://github.com/sponsors/Krixx1337", NULL, NULL, SW_SHOWNORMAL);
                Sleep(500);
            }
        } else {
            // Still set the flag even if we don't show, so we don't check again
            AppState::Get().SetDonationPromptShown(true);
        }
    }

    void AppLifecycleManager::Shutdown() {
        LOG_INFO("AppLifecycleManager: Full shutdown requested");
        
        // Perform the save FIRST. This function handles the atomic flag.
        SaveSettingsOnExit(); 
        
        m_currentState = State::ShuttingDown;
        
        // Give hooks a moment to recognize the flag before cleanup starts
        // This helps prevent calls into ImGui after it's destroyed
        Sleep(Timing::SHUTDOWN_GRACE_MS);
        
        CleanupServices();
        
        LOG_INFO("AppLifecycleManager: Shutdown complete");
    }

    bool AppLifecycleManager::IsShutdownRequested() const {
        // Only DELETE key triggers shutdown (not the window close button)
        // Closing the window (X button) just hides it - user can press INSERT to show again
        return (GetAsyncKeyState(Hotkeys::EXIT_APPLICATION) & 0x8000);
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
            Sleep(Timing::INIT_POLL_INTERVAL_MS);
        }
    }

    void AppLifecycleManager::HandleWaitingForRendererState() {
        // In GW2AL mode, we just wait here until OnRendererInitialized() is called
        // This state is transitioned by the GW2AL event callback
        Sleep(Timing::RUNNING_POLL_INTERVAL_MS);
    }

    void AppLifecycleManager::CheckStateTransitions() {
        // Handle state transitions and state processing for GW2AL mode
        switch (m_currentState) {
        case State::WaitingForGame:
            // Initialize MumbleLink manager if needed
            if (!m_mumbleLinkManager.IsInitialized()) {
                m_mumbleLinkManager.Update();
                return;
            }

            // Check if player is in-game and transition to InitializingServices
            if (IsPlayerInGame()) {
                LOG_INFO("AppLifecycleManager: Player is in-game, transitioning to InitializingServices");
                m_currentState = State::InitializingServices;
            }
            break;

        case State::InitializingServices:
            // Process the InitializingServices state (GW2AL mode)
            HandleInitializingServicesState();
            break;

        case State::Running:
            // Show donation prompt once after entering Running state (game fully loaded)
            if (!m_donationPromptShownOnStartup) {
                m_donationPromptShownOnStartup = true;
                ShowDonationPromptIfNeeded();
            }
            break;

        default:
            // Other states don't need processing in GW2AL mode
            break;
        }
    }

    void AppLifecycleManager::HandleWaitingForGameState() {
        // Use the same logic as CheckStateTransitions but with sleep for DLL mode
        CheckStateTransitions();
        
        // If still in WaitingForGame state, sleep before next check (DLL mode)
        if (m_currentState == State::WaitingForGame) {
            Sleep(Timing::INIT_POLL_INTERVAL_MS);
        }
    }

    void AppLifecycleManager::RenderTick(HWND windowHandle, float displayWidth, float displayHeight,
        ID3D11DeviceContext* context, ID3D11RenderTargetView* renderTargetView) {

        // Delegate to FrameCoordinator for the actual rendering logic
        FrameCoordinator::Execute(*this, windowHandle, displayWidth, displayHeight, context, renderTargetView);
    }

    void AppLifecycleManager::HandleInitializingServicesState() {
        if (InitializeGameServices()) {
            LOG_INFO("AppLifecycleManager: Services initialized, transitioning to Running");
            m_currentState = State::Running;
            m_servicesInitialized = true;
        } else {
            LOG_ERROR("AppLifecycleManager: Service initialization failed - AddressManager or ESPRenderer setup failed, shutting down");
            m_currentState = State::ShuttingDown;
        }
    }

    void AppLifecycleManager::HandleRunningState() {
        // Show donation prompt once after entering Running state (game fully loaded)
        if (!m_donationPromptShownOnStartup) {
            m_donationPromptShownOnStartup = true;
            ShowDonationPromptIfNeeded();
        }
        
        // Normal operation - just sleep to avoid busy-waiting
        // Note: Camera and MumbleLink are updated per-frame in D3DRenderHook::RenderFrame
        Sleep(Timing::RUNNING_POLL_INTERVAL_MS);
    }

    void AppLifecycleManager::HandleShuttingDownState() {
        // Shutdown is handled by Shutdown() method
        // This state just prevents further updates
    }

    // ===== Private Helper Methods =====

    bool AppLifecycleManager::IsImGuiReady() const {
        return OverlayWindow::IsImGuiInitialized();
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

        // Initialize AddressManager (both DLL and GW2AL modes)
        AddressManager::Initialize();
        LOG_INFO("AppLifecycleManager: AddressManager initialized");

        // Initialize ESPRenderer with Camera reference (both DLL and GW2AL modes)
        MasterRenderer::Initialize(m_camera);
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
            Hooking::D3DRenderHook::SetLifecycleManager(nullptr);

            // Cleanup hooks and ImGui
            CleanupHooks();

            m_servicesInitialized = false;
        }
    }

    ID3D11Device* AppLifecycleManager::GetDevice() const {
        return Hooking::D3DRenderHook::GetDevice();
    }

} // namespace kx
