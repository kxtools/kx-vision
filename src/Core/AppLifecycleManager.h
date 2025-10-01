#pragma once

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
 * - WaitingForGame: Waiting for player to be in-game (map loaded)
 * - InitializingServices: Initializing AddressManager and game thread hook
 * - Running: Normal operation
 * - ShuttingDown: Cleanup in progress
 */
class AppLifecycleManager {
public:
    /**
     * @brief Initialize the application lifecycle manager
     * @return true if initialization successful, false otherwise
     */
    bool Initialize();

    /**
     * @brief Update the state machine (called every frame)
     */
    void Update();

    /**
     * @brief Begin the shutdown process
     */
    void Shutdown();

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

private:
    /**
     * @brief Application lifecycle states
     */
    enum class State {
        PreInit,              // Initial state before any initialization
        WaitingForImGui,      // Waiting for ImGui to be initialized
        WaitingForGame,       // Waiting for player to enter a map
        InitializingServices, // Initializing game services
        Running,              // Normal operation
        ShuttingDown          // Cleanup in progress
    };

    State m_currentState = State::PreInit;
    bool m_servicesInitialized = false;

    // Core game state (owned by lifecycle manager)
    Camera m_camera;
    MumbleLinkManager m_mumbleLinkManager;

    // State transition handlers
    void HandlePreInitState();
    void HandleWaitingForImGuiState();
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

} // namespace kx
