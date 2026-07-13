#include "GameEngineService.h"

#include <atomic>

#include "../Architecture/FeatureManager.h"
#include "../../Memory/AddressManager.h"
#include "../../Memory/Safety.h"
#include "../../Utils/DebugLogger.h"

namespace {

std::atomic<kx::GameEngineService*> g_activeGameEngineService{nullptr};

void* TryGetContextCollection(void* (*fn)()) {
    void* context = nullptr;
    __try {
        context = fn();
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        context = nullptr;
    }
    return context;
}

} // namespace

namespace kx {

GameEngineService::~GameEngineService() {
    Shutdown();
}

void GameEngineService::Initialize(FeatureManager& featureManager) {
    if (m_initialized.exchange(true, std::memory_order_acq_rel)) {
        return;
    }

    LOG_INFO("GameEngineService: Initializing game runtime");
    AddressManager::Initialize();
    LOG_INFO("GameEngineService: AddressManager initialized");

    m_featureManager = &featureManager;
    if (InitializeGameThreadHook()) {
        LOG_INFO("GameEngineService: Game thread hook initialized successfully");
    } else {
        LOG_WARN("GameEngineService: Game thread hook initialization failed - ESP may not work");
    }
}

void GameEngineService::Shutdown() {
    if (!m_initialized.exchange(false, std::memory_order_acq_rel)) {
        return;
    }

    // Guardrail: lifecycle drains every active detour before this call. Remove hook
    // before clearing callback targets so no game-thread callback observes torn owners.
    if (m_gameThreadHook.created) {
        Hooking::HookManager::DisableAndRemove(m_gameThreadHook);
        LOG_INFO("GameEngineService: Game thread hook removed");
    }

    g_activeGameEngineService.store(nullptr, std::memory_order_release);
    m_originalGameThreadUpdate = nullptr;
    m_featureManager = nullptr;
}

bool GameEngineService::PollPlayerInGame() {
    if (!m_mumbleLinkManager.IsInitialized()) {
        m_mumbleLinkManager.Update();
        return false;
    }

    const MumbleLinkData* data = m_mumbleLinkManager.GetData();
    if (data && data->context.mapId != 0) {
        LOG_INFO("GameEngineService: Player is in-map (Map ID: %u)", data->context.mapId);
        return true;
    }

    return false;
}

const MumbleLinkData* GameEngineService::UpdateRenderState(HWND windowHandle) {
    m_mumbleLinkManager.Update();
    m_camera.Update(m_mumbleLinkManager, windowHandle);
    return m_mumbleLinkManager.GetData();
}

bool GameEngineService::InitializeGameThreadHook() {
    const uintptr_t target = AddressManager::GetGameThreadUpdateFunc();
    if (!target) {
        LOG_WARN("GameEngineService: Game thread hook target not found. Character ESP will be disabled.");
        return false;
    }

    // Guardrail: detour may run during EnableHook. Publish complete callback target
    // before enable; shutdown clears it only after global callback drain.
    g_activeGameEngineService.store(this, std::memory_order_release);
    if (!Hooking::HookManager::CreateAndEnable(
            m_gameThreadHook,
            reinterpret_cast<LPVOID>(target),
            reinterpret_cast<LPVOID>(DetourGameThread),
            reinterpret_cast<LPVOID*>(&m_originalGameThreadUpdate))) {
        g_activeGameEngineService.store(nullptr, std::memory_order_release);
        m_originalGameThreadUpdate = nullptr;
        LOG_ERROR("GameEngineService: Failed to create and enable game thread hook");
        return false;
    }

    return true;
}

uintptr_t __fastcall GameEngineService::DetourGameThread(uintptr_t a1, uintptr_t a2) {
    Hooking::HookCallbackGuard callbackGuard;
    GameEngineService* service = g_activeGameEngineService.load(std::memory_order_acquire);
    if (!service) {
        return 0;
    }

    GameThreadUpdateFunc original = service->m_originalGameThreadUpdate;
    if (Hooking::HookManager::IsShuttingDown()) {
        return original ? original(a1, a2) : 0;
    }

    int rawFrameTime = 0;
    if (a2) {
        SafeAccess::TryRead<int>(
            a2 + AddressingConstants::GAME_THREAD_TICK_FRAME_MS_OFFSET,
            rawFrameTime);
    }
    (void)rawFrameTime;

    if (AddressManager::GetContextCollectionPtr() == nullptr) {
        using GetContextCollectionFn = void* (*)();
        const uintptr_t functionAddress = AddressManager::GetContextCollectionFunc();
        if (functionAddress) {
            auto getContextCollection = reinterpret_cast<GetContextCollectionFn>(functionAddress);
            if (void* context = TryGetContextCollection(getContextCollection)) {
                AddressManager::SetContextCollectionPtr(context);
                LOG_INFO("GameEngineService: Captured ContextCollection: 0x%p", context);
            }
        }
    }

    service->m_entityManager.Update(GetTickCount64());
    if (service->m_featureManager) {
        service->m_featureManager->RunGameThreadUpdates();
    }

    return original ? original(a1, a2) : 0;
}

} // namespace kx
