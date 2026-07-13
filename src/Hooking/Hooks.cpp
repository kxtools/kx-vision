#include "Hooks.h"

#include <windows.h> // For __try/__except
#include <thread>
#include <chrono>
#include <atomic>

#include "../Core/Config.h"      // For GW2AL_BUILD define
#include "../Core/AppLifecycleManager.h"
#include "../Core/Architecture/FeatureManager.h"
#include "../Utils/DebugLogger.h"
#include "../Memory/AddressManager.h"
#include "../Memory/Safety.h"
#include "AppState.h"
#include "D3DRenderHook.h"
#include "HookManager.h"

namespace {
    std::atomic<bool> g_gameThreadHookInitialized{false};
    kx::Hooking::HookHandle g_gameThreadHook;

    void* TryGetContextCollection(void* (*fn)()) {
        void* p = nullptr;
        __try {
            p = fn();
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            p = nullptr;
        }
        return p;
    }

} // namespace

namespace kx {

    namespace Hooking {

        typedef uintptr_t(__fastcall* GameThreadUpdateFunc)(uintptr_t, uintptr_t);
        GameThreadUpdateFunc pOriginalGameThreadUpdate = nullptr;

        uintptr_t __fastcall DetourGameThread(uintptr_t a1, uintptr_t a2) {
            HookCallbackGuard callbackGuard;
            if (HookManager::IsShuttingDown()) {
                return pOriginalGameThreadUpdate ? pOriginalGameThreadUpdate(a1, a2) : 0;
            }
            // Native game delta is FPS-quantized and may pause. Keep it diagnostic-only;
            // behavioral clocks use monotonic real elapsed time.
            int rawFrameTime = 0;
            if (a2) {
                SafeAccess::TryRead<int>(a2 + AddressingConstants::GAME_THREAD_TICK_FRAME_MS_OFFSET, rawFrameTime);
            }
            (void)rawFrameTime;

            // 3. Lazy Initialization of ContextCollection
            // This runs only once. After that, it's just a null check (near-zero overhead).
            if (AddressManager::GetContextCollectionPtr() == nullptr) {
                using GetContextCollectionFn = void* (*)();
                uintptr_t funcAddr = AddressManager::GetContextCollectionFunc();

                if (funcAddr) {
                    auto getContextCollection = reinterpret_cast<GetContextCollectionFn>(funcAddr);
                    // Protected call in case engine returns garbage during early init
                    void* pContext = TryGetContextCollection(getContextCollection);

                    if (pContext) {
                        AddressManager::SetContextCollectionPtr(pContext);
                        LOG_INFO("[Hooks] Captured ContextCollection: 0x%p", pContext);
                    }
                }
            }

            // 4. Update Features using Synced Game Time
            kx::g_App.GetEntityManager().Update(GetTickCount64());
            kx::g_App.GetFeatureManager().RunGameThreadUpdates();

            // 5. Pass execution back to the engine
            if (pOriginalGameThreadUpdate) {
                return pOriginalGameThreadUpdate(a1, a2);
            }
            return 0;
        }

    } // namespace Hooking

    bool InitializeHooks() {
        AppState::Get().SetPresentHookStatus(HookStatus::Unknown);

#ifndef GW2AL_BUILD
        if (!Hooking::D3DRenderHook::Initialize()) {
            Hooking::HookManager::Shutdown();
            AppState::Get().SetPresentHookStatus(HookStatus::Failed);
            return false;
        }
#endif

        LOG_INFO("[Hooks] Essential hooks initialized successfully.");
        return true;
    }

    bool InitializeGameThreadHook() {
        if (g_gameThreadHookInitialized.load(std::memory_order_acquire)) {
            LOG_WARN("[Hooks] GameThread hook already initialized, skipping.");
            return true;
        }

        uintptr_t gameThreadFuncAddr = AddressManager::GetGameThreadUpdateFunc();
        if (!gameThreadFuncAddr) {
            LOG_WARN("[Hooks] GameThread hook target not found. Character ESP will be disabled.");
            return false;
        }

        if (!Hooking::HookManager::CreateAndEnable(
                g_gameThreadHook,
                reinterpret_cast<LPVOID>(gameThreadFuncAddr),
                reinterpret_cast<LPVOID>(Hooking::DetourGameThread),
                reinterpret_cast<LPVOID*>(&Hooking::pOriginalGameThreadUpdate))) {
            LOG_ERROR("[Hooks] Failed to create and enable GameThread hook.");
            return false;
        }
        g_gameThreadHookInitialized.store(true, std::memory_order_release);
        LOG_INFO("[Hooks] GameThread hook created and enabled.");
        return true;
    }

    void CleanupHooks() {
        LOG_INFO("[Hooks] Cleaning up...");

        if (g_gameThreadHook.created) {
            Hooking::HookManager::DisableAndRemove(g_gameThreadHook);
            LOG_INFO("[Hooks] GameThread hook disabled.");
            LOG_INFO("[Hooks] GameThread hook removed.");
        }
        Hooking::pOriginalGameThreadUpdate = nullptr;
        g_gameThreadHookInitialized.store(false, std::memory_order_release);

#ifndef GW2AL_BUILD
        // D3DRenderHook::Shutdown() now handles Present hook cleanup internally
        Hooking::D3DRenderHook::Shutdown();
#endif

        Hooking::HookManager::Shutdown();

        LOG_INFO("[Hooks] Cleanup finished.");
    }

} // namespace kx
