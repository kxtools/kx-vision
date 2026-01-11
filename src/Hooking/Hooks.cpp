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

namespace kx {

    namespace Hooking {

        typedef void(__fastcall* GameThreadUpdateFunc)(void*, int);
        GameThreadUpdateFunc pOriginalGameThreadUpdate = nullptr;

        void __fastcall DetourGameThread(void* pInst, int frame_time) {
            using GetContextCollectionFn = void* (*)();

            uintptr_t funcAddr = AddressManager::GetContextCollectionFunc();
            if (funcAddr) {
                auto getContextCollection = reinterpret_cast<GetContextCollectionFn>(funcAddr);

                void* contextCollection = AddressManager::GetContextCollectionPtr();
                bool fetched = false;

                __try {
                    contextCollection = getContextCollection();
                    fetched = true;
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    // Keep the last known pointer if retrieval faults
                }

                if (fetched) {
                    AddressManager::SetContextCollectionPtr(contextCollection);
                }
            }

            // Update EntityManager on Game Thread for safe TLS access
            // Note: Memory safety is handled at the ForeignClass level during extraction
            const uint64_t now = GetTickCount64();
            kx::g_App.GetEntityManager().Update(now);

            // Run game thread updates for all features
            kx::g_App.GetFeatureManager().RunGameThreadUpdates();

            if (pOriginalGameThreadUpdate) {
                pOriginalGameThreadUpdate(pInst, frame_time);
            }
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
