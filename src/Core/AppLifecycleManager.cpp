#include "AppLifecycleManager.h"
#include "Config.h"
#include "SettingsManager.h"
#include "FrameCoordinator.h"
#include "Services/GameEngineService.h"
#include "HookManager.h"
#include "Hooks.h"
#include "../Hooking/D3DRenderHook.h"
#include "../Utils/DebugLogger.h"
#include "../../libs/ImGui/imgui.h"
#include <windows.h>

#include "UI/Backend/OverlayWindow.h"
#include "Architecture/FeatureManager.h"

namespace kx {

    namespace {
        void ApplyLoggerSettings(const Settings& settings) {
            Debug::Logger::SetDebugLoggingEnabled(settings.enableDebugLogging);
            Debug::Logger::SetMinLogLevel(static_cast<Debug::Logger::Level>(settings.logLevel));
        }
    }

    // Global instance of AppLifecycleManager
    AppLifecycleManager g_App;

    // Define destructor where FeatureManager's complete type is known
    AppLifecycleManager::~AppLifecycleManager() = default;

    bool AppLifecycleManager::Initialize(AppComposition composition) {
        LOG_INFO("AppLifecycleManager: Starting initialization");

        if (!composition.features.registerAll) {
            LOG_ERROR("AppLifecycleManager: Missing required feature registration callback");
            return false;
        }
        m_featureRegistration = composition.features;
        m_runtimeExtension = std::move(composition.runtimeExtension);
        if (!m_runtimeExtension) {
            m_runtimeExtension = std::make_unique<IRuntimeExtension>();
        }
        m_runtimeExtension->OnEntryThreadStart();
        if (!m_runtimeExtension->Initialize()) {
            m_runtimeExtension->Shutdown();
            LOG_ERROR("AppLifecycleManager: Runtime extension initialization failed");
            return false;
        }
        m_runtimeExtensionInitialized = true;

        m_gameEngineService = std::make_unique<GameEngineService>();

        // Initialize FeatureManager
        m_featureManager = std::make_unique<FeatureManager>();

        // Register features immediately so UI tabs appear in correct order from first frame
        m_featureRegistration.registerAll(*m_featureManager);
        SettingsManager::Load(m_settings, *m_featureManager);
        ApplyLoggerSettings(m_settings);
        m_serviceContext.settings = &m_settings;

        // Guardrail: Present/WndProc callbacks may run as soon as hooks become enabled.
        // Publish their host dependency first so no callback observes a null target.
        Hooking::D3DRenderHook::SetLifecycleManager(this);

        // Initialize host hooks (this sets up D3D hook which creates ImGuiManager)
        if (!InitializeHostHooks()) {
            Hooking::D3DRenderHook::SetLifecycleManager(nullptr);
            LOG_ERROR("AppLifecycleManager: Failed to initialize host hooks");
            return false;
        }

        LOG_INFO("AppLifecycleManager: Hooks initialized successfully");
        m_currentState = State::WaitingForImGui;

        return true;
    }

    bool AppLifecycleManager::InitializeForGW2AL(AppComposition composition) {
        LOG_INFO("AppLifecycleManager: Initializing for GW2AL mode");

        if (!composition.features.registerAll) {
            LOG_ERROR("AppLifecycleManager: Missing required feature registration callback for GW2AL mode");
            return false;
        }
        m_featureRegistration = composition.features;
        m_runtimeExtension = std::move(composition.runtimeExtension);
        if (!m_runtimeExtension) {
            m_runtimeExtension = std::make_unique<IRuntimeExtension>();
        }
        m_runtimeExtension->OnEntryThreadStart();
        if (!m_runtimeExtension->Initialize()) {
            m_runtimeExtension->Shutdown();
            LOG_ERROR("AppLifecycleManager: Runtime extension initialization failed for GW2AL mode");
            return false;
        }
        m_runtimeExtensionInitialized = true;

        m_gameEngineService = std::make_unique<GameEngineService>();

        // Initialize FeatureManager
        m_featureManager = std::make_unique<FeatureManager>();

        // Register features immediately so UI tabs appear in correct order from first frame
        m_featureRegistration.registerAll(*m_featureManager);
        SettingsManager::Load(m_settings, *m_featureManager);
        ApplyLoggerSettings(m_settings);
        m_serviceContext.settings = &m_settings;

        // GW2AL renderer callbacks are enabled later, but their host dependency must exist first.
        Hooking::D3DRenderHook::SetLifecycleManager(this);

        if (!InitializeHostHooks()) {
            Hooking::D3DRenderHook::SetLifecycleManager(nullptr);
            LOG_ERROR("AppLifecycleManager: Failed to initialize host hooks for GW2AL mode");
            return false;
        }

        // In GW2AL mode, hooks are managed differently
        // We don't initialize the Present hook here (GW2AL handles that)
        // We just set up the lifecycle manager

        m_currentState = State::WaitingForRenderer;
        LOG_INFO("AppLifecycleManager: Waiting for renderer initialization from GW2AL");

        return true;
    }

    void AppLifecycleManager::OnRendererInitialized() {
        LOG_INFO("AppLifecycleManager: Renderer initialized, waiting for player to be in-game");

        // Transition to WaitingForGame - services will be initialized when player enters game
        m_currentState = State::WaitingForGame;
    }

    void AppLifecycleManager::Update() {
#ifdef GW2AL_BUILD
        LOG_ERROR("AppLifecycleManager::Update() should not be called in GW2AL mode - state transitions handled by render thread");
        return;
#endif

        if (m_runtimeExtension && m_runtimeExtension->Tick()) {
            SynchronizeFeatures();
        }

        switch (m_currentState.load(std::memory_order_acquire)) {
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
        // Guardrail: clean shutdown and DllMain detach can both request persistence.
        // Claim save ownership before touching feature settings so only one path writes.
        if (m_settingsSaveStarted.exchange(true, std::memory_order_acq_rel)) return;
    
        LOG_INFO("AppLifecycleManager: Performing final settings save on exit...");
    
        if (m_settings.autoSaveOnExit && m_featureManager) {
            SettingsManager::Save(m_settings, *m_featureManager);
        }
    }

    void AppLifecycleManager::Shutdown() {
        if (m_shutdownStarted.exchange(true, std::memory_order_acq_rel)) return;
        LOG_INFO("AppLifecycleManager: Full shutdown requested");

        // Publish shutdown before persistence so render callbacks cannot observe
        // feature settings while they are being serialized.
        m_currentState = State::ShuttingDown;

        // Perform the save before service teardown. Separate save guard handles detach too.
        SaveSettingsOnExit();

        // Guardrail: disable callback sources, then drain code already executing in
        // detours before any feature, ImGui, or original-function storage is destroyed.
        Hooking::HookManager::BeginShutdown();
        Hooking::D3DRenderHook::UninstallWndProc();
        Hooking::HookManager::DisableAll();
        Hooking::HookManager::WaitForCallbacks();

        CleanupServices();
        if (m_runtimeExtensionInitialized && m_runtimeExtension) {
            m_runtimeExtension->Shutdown();
            m_runtimeExtensionInitialized = false;
        }
        LOG_INFO("AppLifecycleManager: Shutdown complete");
    }

    bool AppLifecycleManager::IsShutdownRequested() const {
        // Only DELETE key triggers shutdown (not the window close button)
        // Closing the window (X button) just hides it - user can press INSERT to show again
        return (GetAsyncKeyState(Hotkeys::EXIT_APPLICATION) & 0x8000);
    }

    const char* AppLifecycleManager::GetCurrentStateName() const {
        switch (m_currentState.load(std::memory_order_acquire)) {
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
#ifdef GW2AL_BUILD
        if (m_runtimeExtension && m_runtimeExtension->Tick()) {
            SynchronizeFeatures();
        }
#endif

        // Handle state transitions and state processing for GW2AL mode
        switch (m_currentState.load(std::memory_order_acquire)) {
        case State::WaitingForGame:
            // Check if player is in-game and transition to InitializingServices
            if (m_gameEngineService && m_gameEngineService->PollPlayerInGame()) {
                LOG_INFO("AppLifecycleManager: Player is in-game, transitioning to InitializingServices");
                m_currentState = State::InitializingServices;
            }
            break;

        case State::InitializingServices:
            // Process the InitializingServices state (GW2AL mode)
            HandleInitializingServicesState();
            break;

        case State::Running:
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
        if (m_gameEngineService) {
            FrameCoordinator::Execute(*this, *m_gameEngineService, windowHandle,
                displayWidth, displayHeight, context, renderTargetView);
        }
    }

    void AppLifecycleManager::SynchronizeFeatures() {
        if (!m_featureManager || !m_featureRegistration.sync) {
            return;
        }

        if (!m_featureRegistration.sync(*m_featureManager)) {
            return;
        }

        // Guardrail: features registered at runtime still need their persisted
        // settings/default path resolution before Initialize() runs. Startup does this
        // later in InitializeFeatures(), so runtime synchronization must mirror it.
        SettingsManager::LoadPendingFeatureSettings(*m_featureManager);

        // Guardrail: runtime synchronization must initialize only the
        // newly registered features. Reinitializing the full list would rerun
        // hooks and stateful setup for features already in service.
        if (m_servicesInitialized && !m_featureManager->InitializeAll(m_serviceContext)) {
#ifdef _DEBUG
            LOG_ERROR("AppLifecycleManager: Failed to initialize feature(s) added by runtime synchronization");
#else
            LOG_ERROR("AppLifecycleManager: Runtime feature initialization failed");
#endif
        }
    }

    void AppLifecycleManager::OnRenderThreadStart() {
        if (m_runtimeExtension) {
            m_runtimeExtension->OnRenderThreadStart();
        }
    }

    bool AppLifecycleManager::RequiresExclusiveOverlay() const {
        return m_runtimeExtension && m_runtimeExtension->RequiresExclusiveOverlay();
    }

    bool AppLifecycleManager::WantsOverlayInput() const {
        return m_runtimeExtension && m_runtimeExtension->WantsOverlayInput();
    }

    void AppLifecycleManager::RenderRuntimeOverlay() {
        if (m_runtimeExtension) {
            m_runtimeExtension->RenderOverlay();
        }
    }

    void AppLifecycleManager::HandleInitializingServicesState() {
        if (m_gameEngineService) {
            m_gameEngineService->Initialize(*m_featureManager);
        }

        if (m_gameEngineService && InitializeFeatures()) {
            LOG_INFO("AppLifecycleManager: Services and features initialized, transitioning to Running");
            m_currentState = State::Running;
            m_servicesInitialized = true;
        } else {
            LOG_ERROR("AppLifecycleManager: Service or feature initialization failed, shutting down");
            m_currentState = State::ShuttingDown;
        }
    }

    void AppLifecycleManager::HandleRunningState() {
        // Normal operation - just sleep to avoid busy-waiting
        // GameEngineService updates render-facing game state per frame.
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

    bool AppLifecycleManager::InitializeFeatures() {
        LOG_INFO("AppLifecycleManager: Initializing features");

        // Populate ServiceContext with pointers to core services
        m_serviceContext.entities = &m_gameEngineService->GetEntityManager();
        m_serviceContext.camera = &m_gameEngineService->GetCamera();
        m_serviceContext.mumble = &m_gameEngineService->GetMumbleLinkManager();
        m_serviceContext.settings = &m_settings;
        m_serviceContext.device = GetDevice();

        // Features already registered in Initialize(), just initialize them now
        if (!m_featureManager->InitializeAll(m_serviceContext)) {
            LOG_ERROR("AppLifecycleManager: Failed to initialize features");
            return false;
        }

        LOG_INFO("AppLifecycleManager: Features initialized successfully");
        return true;
    }

    void AppLifecycleManager::CleanupServices() {
        LOG_INFO("AppLifecycleManager: Cleaning up owned resources");

        // Partial initialization still owns registered features and MinHook state.
        // Resource flags inside each owner make this repeated cleanup idempotent.
        if (m_featureManager) {
            m_featureManager->ShutdownAll();
        }

        if (m_gameEngineService) {
            m_gameEngineService->Shutdown();
        }

        Hooking::D3DRenderHook::SetLifecycleManager(nullptr);
        ShutdownHostHooks();
        m_servicesInitialized = false;
    }

    ID3D11Device* AppLifecycleManager::GetDevice() const {
        return Hooking::D3DRenderHook::GetDevice();
    }

} // namespace kx
