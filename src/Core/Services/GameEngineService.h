#pragma once

#include <atomic>
#include <cstdint>
#include <windows.h>

#include "../../Game/Services/Camera/Camera.h"
#include "../../Game/Services/Mumble/MumbleLinkManager.h"
#include "../../Hooking/HookManager.h"
#include "EntityManager.h"

namespace kx {

class FeatureManager;

class GameEngineService {
public:
    GameEngineService() = default;
    ~GameEngineService();

    GameEngineService(const GameEngineService&) = delete;
    GameEngineService& operator=(const GameEngineService&) = delete;

    void Initialize(FeatureManager& featureManager);
    void Shutdown();

    bool PollPlayerInGame();
    const MumbleLinkData* UpdateRenderState(HWND windowHandle);

    Camera& GetCamera() { return m_camera; }
    MumbleLinkManager& GetMumbleLinkManager() { return m_mumbleLinkManager; }
    EntityManager& GetEntityManager() { return m_entityManager; }

private:
    using GameThreadUpdateFunc = uintptr_t(__fastcall*)(uintptr_t, uintptr_t);

    static uintptr_t __fastcall DetourGameThread(uintptr_t a1, uintptr_t a2);
    bool InitializeGameThreadHook();

    Camera m_camera;
    MumbleLinkManager m_mumbleLinkManager;
    EntityManager m_entityManager;
    Hooking::HookHandle m_gameThreadHook;
    GameThreadUpdateFunc m_originalGameThreadUpdate = nullptr;
    FeatureManager* m_featureManager = nullptr;
    std::atomic<bool> m_initialized{false};
};

} // namespace kx
