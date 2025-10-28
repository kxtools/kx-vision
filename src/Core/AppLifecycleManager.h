#pragma once

#include <d3d11.h>
#include <windows.h>
#include "../Game/Camera.h"
#include "../Game/MumbleLinkManager.h"

namespace kx {

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
 * - InitializingServices: Initializing AddressManager and game thread hook
 * - Running: Normal operation
 * - ShuttingDown: Cleanup in progress
 */
class AppLifecycleManager {
public:
    /**
     * @brief Initialize the application lifecycle manager (DLL mode)
     * @return true if initialization successful, false otherwise
     */
    bool Initialize();

    /**
     * @brief Initialize for GW2AL mode (event-driven)
     * @return true if initialization successful, false otherwise
     */
    bool InitializeForGW2AL();

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
    bool IsShuttingDown() const { return m_currentState == State::ShuttingDown; }

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
     * @brief Get reference to the Camera (for core game state)
     * @return Reference to Camera instance
     */
    Camera& GetCamera() { return m_camera; }

    /**
     * @brief Get reference to MumbleLinkManager (for core game state)
     * @return Reference to MumbleLinkManager instance
     */
    MumbleLinkManager& GetMumbleLinkManager() { return m_mumbleLinkManager; }

    /**
     * @brief Get const pointer to current MumbleLink data
     * @return Pointer to MumbleLinkData, or nullptr if not available
     */
    const MumbleLinkData* GetMumbleLinkData() const { return m_mumbleLinkManager.GetData(); }

    /**
     * @brief Get the D3D11 device (if initialized)
     * @return Pointer to ID3D11Device, or nullptr if not available
     */
    ID3D11Device* GetDevice() const;

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

    /**
     * @brief Show donation prompt with 30% random chance (if not already shown this session)
     */
    void ShowDonationPromptIfNeeded();

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

    State m_currentState = State::PreInit;
    bool m_servicesInitialized = false;
    bool m_donationPromptShownOnStartup = false;

    // Core game state (owned by lifecycle manager)
    Camera m_camera;
    MumbleLinkManager m_mumbleLinkManager;

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
    bool IsPlayerInGame() const;
    bool InitializeGameServices();
    void CleanupServices();
};

// Global instance of AppLifecycleManager (used by both DLL and GW2AL modes)
extern AppLifecycleManager g_App;

} // namespace kx
