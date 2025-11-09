#include "Hooks.h"

#include <windows.h> // For __try/__except
#include <vector>

#include "../Core/Config.h"      // For GW2AL_BUILD define
#include "../Utils/DebugLogger.h"
#include "../Utils/SafeIterators.h"
#include "../Utils/MemorySafety.h"
#include "../Game/AddressManager.h"
#include "../Game/NameResolver.h"
#include "../Game/ReClassStructs.h"
#include "AppState.h"
#include "D3DRenderHook.h"
#include "HookManager.h"

namespace kx {

    namespace Hooking {

        // Define the type of the original function we're hooking
        typedef void(__fastcall* GameThreadUpdateFunc)(void*, int);
        GameThreadUpdateFunc pOriginalGameThreadUpdate = nullptr;

        // Helper function to call the game's GetContextCollection within an SEH block.
        // This isolates the unsafe call and prevents C2712 errors.
        void* GetContextCollection_SEH() {
            // Define the type for GetContextCollection
            using GetContextCollectionFn = void* (*)();

            // Get the function pointer from our AddressManager
            uintptr_t funcAddr = AddressManager::GetContextCollectionFunc();
            if (!funcAddr) {
                return nullptr;
            }

            auto getContextCollection = reinterpret_cast<GetContextCollectionFn>(funcAddr);

            __try {
                return getContextCollection();
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                return nullptr;
            }
        }

        // This is our detour function. It will be executed on the GAME'S LOGIC THREAD.
        void __fastcall DetourGameThread(void* pInst, int frame_time) {
            // Periodically clear old cache entries (every ~300 frames / ~5 seconds at 60fps)
            static int frameCounter = 0;
            if (++frameCounter >= 300) {
                frameCounter = 0;
                NameResolver::ClearNameCache();
            }

            // Safely get the context collection pointer using the SEH-wrapped helper.
            void* pContextCollection = GetContextCollection_SEH();
            AddressManager::SetContextCollectionPtr(pContextCollection);

            // NOW we're on the game thread with valid TLS context!
            // We can safely use C++ objects outside of the __try block.
            if (pContextCollection && kx::SafeAccess::IsMemorySafe(pContextCollection)) {
                std::unordered_map<void*, uint8_t> agentPointers;
                agentPointers.reserve(512); // Reserve space for typical agent count

                kx::ReClass::ContextCollection ctxCollection(pContextCollection);

                // Collect character agents
                kx::ReClass::ChCliContext charContext = ctxCollection.GetChCliContext();
                if (charContext.data()) {
                    kx::SafeAccess::CharacterList charList(charContext);
                    for (const auto& character : charList) {
                        if (character.data()) {
                            agentPointers.emplace((void*)character.data(), 0);
                        }
                    }
                }

                // Collect gadget agents
                kx::ReClass::GdCliContext gadgetContext = ctxCollection.GetGdCliContext();
                if (gadgetContext.data()) {
                    kx::SafeAccess::GadgetList gadgetList(gadgetContext);
                    for (const auto& gadget : gadgetList) {
                        if (gadget.data()) {
                            agentPointers.emplace((void*)gadget.data(), 1);
                        }
                    }
                }

                // Resolve and cache all names (this is safe here on game thread)
                if (!agentPointers.empty()) {
                    NameResolver::CacheNamesForAgents(agentPointers);
                }
            }

            // IMPORTANT: Call the original game function to allow the game to run normally.
            if (pOriginalGameThreadUpdate) {
                pOriginalGameThreadUpdate(pInst, frame_time);
            }
        }

    } // namespace Hooking

    bool InitializeHooks() {
        AppState::Get().SetPresentHookStatus(HookStatus::Unknown);

        // Note: HookManager::Initialize() is now called in AppLifecycleManager::Initialize()
        // or AppLifecycleManager::InitializeForGW2AL() for architectural consistency

#ifndef GW2AL_BUILD
        // Only initialize D3D hook in standalone DLL mode
        // In GW2AL mode, this is handled by GW2AL_Integration.cpp
        if (!kx::Hooking::D3DRenderHook::Initialize()) {
            kx::Hooking::HookManager::Shutdown();
            AppState::Get().SetPresentHookStatus(HookStatus::Failed);
            return false;
        }
#endif

        LOG_INFO("[Hooks] Essential hooks initialized successfully.");
        return true;
    }

    bool InitializeGameThreadHook() {
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

        LOG_INFO("[Hooks] GameThread hook created and enabled.");
        return true;
    }

    void CleanupHooks() {
        LOG_INFO("[Hooks] Cleaning up...");

        // NEW: Disable and remove the game thread hook during shutdown
        uintptr_t gameThreadFuncAddr = AddressManager::GetGameThreadUpdateFunc();
        if (gameThreadFuncAddr && Hooking::pOriginalGameThreadUpdate) {
            Hooking::HookManager::DisableHook(reinterpret_cast<LPVOID>(gameThreadFuncAddr));
            Hooking::HookManager::RemoveHook(reinterpret_cast<LPVOID>(gameThreadFuncAddr));
            LOG_INFO("[Hooks] GameThread hook cleaned up.");
        }

#ifndef GW2AL_BUILD
        // Only shutdown D3D hook in standalone DLL mode
        // In GW2AL mode, this is handled by GW2AL_Integration.cpp
        kx::Hooking::D3DRenderHook::Shutdown();
#endif

        kx::Hooking::HookManager::Shutdown();

        LOG_INFO("[Hooks] Cleanup finished.");
    }

} // namespace kx