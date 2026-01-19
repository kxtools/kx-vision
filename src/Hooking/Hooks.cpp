#include "Hooks.h"

#include <windows.h> // For __try/__except

#include "../Core/Config.h"      // For GW2AL_BUILD define
#include "../Core/AppLifecycleManager.h"
#include "../Core/Architecture/FeatureManager.h"
#include "../Utils/DebugLogger.h"
#include "../Memory/AddressManager.h"
#include "AppState.h"
#include "D3DRenderHook.h"
#include "HookManager.h"

namespace {

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
            int frame_time = a2 ? *reinterpret_cast<int*>(a2 + AddressingConstants::GAME_THREAD_TICK_FRAME_MS_OFFSET) : 0;
            static uint64_t s_gameTimeMs = 0;
            s_gameTimeMs += static_cast<uint64_t>(frame_time > 0 ? frame_time : 0);

            if (AddressManager::GetContextCollectionPtr() == nullptr) {
                using GetContextCollectionFn = void* (*)();
                uintptr_t funcAddr = AddressManager::GetContextCollectionFunc();

                if (funcAddr) {
                    auto getContextCollection = reinterpret_cast<GetContextCollectionFn>(funcAddr);
                    void* pContext = TryGetContextCollection(getContextCollection);

                    if (pContext) {
                        AddressManager::SetContextCollectionPtr(pContext);
                        LOG_INFO("[Hooks] Captured ContextCollection: 0x%p", pContext);
                    }
                }
            }

            kx::g_App.GetEntityManager().Update(s_gameTimeMs);

            kx::g_App.GetFeatureManager().RunGameThreadUpdates();

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
        static bool s_initialized = false;
        if (s_initialized) {
            LOG_WARN("[Hooks] GameThread hook already initialized, skipping.");
            return true;
        }

        uintptr_t gameThreadFuncAddr = AddressManager::GetGameThreadUpdateFunc();
        if (!gameThreadFuncAddr) {
            LOG_WARN("[Hooks] GameThread hook target not found. Character ESP will be disabled.");
            return false;
        }

        if (!Hooking::HookManager::CreateHook(
            reinterpret_cast<LPVOID>(gameThreadFuncAddr),
            reinterpret_cast<LPVOID>(Hooking::DetourGameThread),
            reinterpret_cast<LPVOID*>(&Hooking::pOriginalGameThreadUpdate)
        )) {
            LOG_ERROR("[Hooks] Failed to create GameThread hook.");
            return false;
        }

        if (!Hooking::HookManager::EnableHook(reinterpret_cast<LPVOID>(gameThreadFuncAddr))) {
            LOG_ERROR("[Hooks] Failed to enable GameThread hook.");
            return false;
        }

        s_initialized = true;
        LOG_INFO("[Hooks] GameThread hook created and enabled.");
        return true;
    }

    void CleanupHooks() {
        LOG_INFO("[Hooks] Cleaning up...");

        uintptr_t gameThreadFuncAddr = AddressManager::GetGameThreadUpdateFunc();
        if (gameThreadFuncAddr && Hooking::pOriginalGameThreadUpdate) {
            Hooking::HookManager::DisableHook(reinterpret_cast<LPVOID>(gameThreadFuncAddr));
            Hooking::HookManager::RemoveHook(reinterpret_cast<LPVOID>(gameThreadFuncAddr));
            LOG_INFO("[Hooks] GameThread hook cleaned up.");
        }

#ifndef GW2AL_BUILD
        // D3DRenderHook::Shutdown() now handles Present hook cleanup internally
        Hooking::D3DRenderHook::Shutdown();
#endif

        Hooking::HookManager::Shutdown();

        LOG_INFO("[Hooks] Cleanup finished.");
    }

} // namespace kx
