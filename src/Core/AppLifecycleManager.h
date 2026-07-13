#pragma once

#include <d3d11.h>
#include <windows.h>
#include <memory>
#include <atomic>
#include "Settings.h"
#include "Architecture/AppComposition.h"
#include "Architecture/ServiceContext.h"

namespace kx {

// Forward declarations
class FeatureManager;
class GameEngineService;

/**
 * @brief Manages the application lifecycle state machine
 * 
 * This class encapsulates the initialization, runtime, and shutdown logic
 * of the KX Vision application. It provides a clean separation between
 * the main thread loop and the complex state management.
 * 
 * States:
 * - PreInit: Initial state, waiting to start initialization
 * - WaitingForImGui: Waiting for ImGui to be initialized by the Present hook
 * - WaitingForRenderer: (GW2AL mode) Waiting for GW2AL to provide D3D device
 * - WaitingForGame: Waiting for player to be in-game (map loaded)
 * - InitializingServices: Initializing game runtime and features
 * - Running: Normal operation
 * - ShuttingDown: Cleanup in progress
 */
class AppLifecycleManager {
public:
    /**
     * @brief Initialize the application lifecycle manager (DLL mode)
     * @return true if initialization successful, false otherwise
     */
    bool Initialize(AppComposition composition);

    /**
     * @brief Initialize for GW2AL mode (event-driven)
     * @return true if initialization successful, false otherwise
     */
    bool InitializeForGW2AL(AppComposition composition);

    /**
     * @brief Destructor
     */
    ~AppLifecycleManager();

    /**
     * @brief Called when the renderer is initialized (GW2AL mode)
     * 
     * This transitions the state machine from WaitingForRenderer to the next state.
     */
    void OnRendererInitialized();

    /**
     * @brief Update the state machine (called every frame)
     */
    void Update();

    /**
     * @brief Begin the shutdown process
     */
    void Shutdown();

    /**
     * @brief Performs a minimal, safe-ish save of settings.
     * This is designed to be called from DllMain on process detach.
     */
    void SaveSettingsOnExit();

    /**
     * @brief Check if the application is currently shutting down
     * @return true if shutting down, false otherwise
     */
    bool IsShuttingDown() const { return m_currentState.load(std::memory_order_acquire) == State::ShuttingDown; }

    /**
     * @brief Check if shutdown has been requested
     * @return true if user requested shutdown (DELETE key or window closed)
     */
    bool IsShutdownRequested() const;

    /**
     * @brief Get current state as string for debugging
     */
    const char* GetCurrentStateName() const;

    /**
     * @brief Get reference to the FeatureManager
     * @return Reference to FeatureManager instance
     */
    FeatureManager& GetFeatureManager() { return *m_featureManager; }

    Settings& GetSettings() { return m_settings; }
    const Settings& GetSettings() const { return m_settings; }

    /**
     * @brief Get the D3D11 device (if initialized)
     * @return Pointer to ID3D11Device, or nullptr if not available
     */
    ID3D11Device* GetDevice() const;

    /**
     * @brief Get the service context for dependency injection
     * @return Const reference to ServiceContext
     */
    const ServiceContext& GetServiceContext() const { return m_serviceContext; }

    /**
     * @brief Check for state transitions (for GW2AL mode)
     * 
     * This method allows external code (like GW2AL OnPresent) to trigger
     * state machine transitions when conditions are met. This replaces
     * the old CheckAndInitializeServices method.
     */
    void CheckStateTransitions();

    /**
     * @brief Centralized per-frame tick logic for rendering
     * 
     * This function contains all the shared per-frame logic that needs to
     * happen before ImGui rendering in both DLL and GW2AL modes:
     * - Update MumbleLink data
     * - Update camera
     * - Check and initialize services (if not already done)
     * 
     * @param windowHandle The HWND of the game window
     * @param displayWidth The width of the display/viewport
     * @param displayHeight The height of the display/viewport
     * @param context The D3D11 device context (for rendering)
     * @param renderTargetView The render target view to render to
     */
    void RenderTick(HWND windowHandle, float displayWidth, float displayHeight, 
                    ID3D11DeviceContext* context, ID3D11RenderTargetView* renderTargetView);

    void SynchronizeFeatures();

    void OnRenderThreadStart();
    bool RequiresExclusiveOverlay() const;
    bool WantsOverlayInput() const;
    void RenderRuntimeOverlay();

private:
    /**
     * @brief Application lifecycle states
     */
    enum class State {
        PreInit,              // Initial state before any initialization
        WaitingForImGui,      // Waiting for ImGui to be initialized
        WaitingForRenderer,   // (GW2AL mode) Waiting for renderer initialization
        WaitingForGame,       // Waiting for player to enter a map
        InitializingServices, // Initializing game services
        Running,              // Normal operation
        ShuttingDown          // Cleanup in progress
    };

    // Guardrail: render, init, and shutdown threads observe this state. Atomic visibility
    // prevents render from entering services after shutdown transition begins.
    std::atomic<State> m_currentState{State::PreInit};
    std::atomic<bool> m_servicesInitialized{false};
    std::atomic<bool> m_shutdownStarted{false};
    std::atomic<bool> m_settingsSaveStarted{false};

    // Host state and orchestration owned by lifecycle manager.
    Settings m_settings;
    std::unique_ptr<GameEngineService> m_gameEngineService;
    std::unique_ptr<FeatureManager> m_featureManager;
    FeatureRegistration m_featureRegistration;
    std::unique_ptr<IRuntimeExtension> m_runtimeExtension;
    bool m_runtimeExtensionInitialized = false;

    // Service context for dependency injection (populated during initialization)
    ServiceContext m_serviceContext;

    // State transition handlers
    void HandlePreInitState();
    void HandleWaitingForImGuiState();
    void HandleWaitingForRendererState();
    void HandleWaitingForGameState();
    void HandleInitializingServicesState();
    void HandleRunningState();
    void HandleShuttingDownState();

    // Helper methods
    bool IsImGuiReady() const;
    bool InitializeFeatures();
    void CleanupServices();
};

// Global instance of AppLifecycleManager (used by both DLL and GW2AL modes)
extern AppLifecycleManager g_App;

} // namespace kx
