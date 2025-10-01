#include "AppLifecycleManager.h"
#include "AppState.h"
#include "AddressManager.h"
#include "Hooks.h"
#include "../Rendering/ImGuiManager.h"
#include "../Rendering/Core/ESPRenderer.h"
#include "../Hooking/D3DRenderHook.h"
#include "../Utils/DebugLogger.h"

namespace kx {

bool AppLifecycleManager::Initialize() {
    LOG_INFO("AppLifecycleManager: Starting initialization");
    
    // Initialize hooks first (this sets up D3D hook which creates ImGuiManager)
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

void AppLifecycleManager::Update() {
    switch (m_currentState) {
        case State::PreInit:
            HandlePreInitState();
            break;
            
        case State::WaitingForImGui:
            HandleWaitingForImGuiState();
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

void AppLifecycleManager::Shutdown() {
    LOG_INFO("AppLifecycleManager: Shutdown requested");
    m_currentState = State::ShuttingDown;
    
    // Signal hooks to stop processing before actual cleanup
    AppState::Get().SetShuttingDown(true);
    
    // Give hooks a moment to recognize the flag before cleanup starts
    // This helps prevent calls into ImGui after it's destroyed
    Sleep(250);
    
    CleanupServices();
    
    LOG_INFO("AppLifecycleManager: Shutdown complete");
}

bool AppLifecycleManager::IsShutdownRequested() const {
    // Check if user requested shutdown via DELETE key or window closed
    return !AppState::Get().IsVisionWindowOpen() || (GetAsyncKeyState(VK_DELETE) & 0x8000);
}

const char* AppLifecycleManager::GetCurrentStateName() const {
    switch (m_currentState) {
        case State::PreInit:              return "PreInit";
        case State::WaitingForImGui:      return "WaitingForImGui";
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
    
    // Initialize the game thread hook (requires AddressManager)
    if (InitializeGameThreadHook()) {
        LOG_INFO("AppLifecycleManager: Game thread hook initialized successfully");
        return true;
    } else {
        LOG_WARN("AppLifecycleManager: Game thread hook initialization failed - ESP may not work");
        // We still return true as this isn't a fatal error
        return true;
    }
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

} // namespace kx
