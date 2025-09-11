#include "Hooks.h"

#include <iostream>
#include <windows.h> // For __try/__except

#include "AddressManager.h"
#include "AppState.h"
#include "D3DRenderHook.h"
#include "HookManager.h"

namespace kx {

    namespace Hooking {

        // Define the type of the original function we're hooking
        typedef void(__fastcall* GameThreadUpdateFunc)(void*, int);
        GameThreadUpdateFunc pOriginalGameThreadUpdate = nullptr;

        // This is our detour function. It will be executed on the GAME'S LOGIC THREAD.
        void __fastcall DetourGameThread(void* pInst, int frame_time) {
            // Define the type for GetContextCollection
            using GetContextCollectionFn = void* (*)();

            // Get the function pointer from our AddressManager
            uintptr_t funcAddr = AddressManager::GetContextCollectionFunc();
            if (funcAddr) {
                auto getContextCollection = reinterpret_cast<GetContextCollectionFn>(funcAddr);

                // CAPTURE the pointer and store it in our shared static variable.
                // This is a call into game code, so we wrap it in a __try/__except block
                // to prevent a crash in the game's function from crashing our tool.
                __try {
                    AddressManager::SetContextCollectionPtr(getContextCollection());
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    // If the game function crashes, we'll just get a nullptr this frame.
                    AddressManager::SetContextCollectionPtr(nullptr);
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

        if (!kx::Hooking::HookManager::Initialize()) {
            return false;
        }

        if (!kx::Hooking::D3DRenderHook::Initialize()) {
            kx::Hooking::HookManager::Shutdown();
            AppState::Get().SetPresentHookStatus(HookStatus::Failed);
            return false;
        }

        uintptr_t gameThreadFuncAddr = AddressManager::GetGameThreadUpdateFunc();
        if (gameThreadFuncAddr) {
            if (!Hooking::HookManager::CreateHook(
                reinterpret_cast<LPVOID>(gameThreadFuncAddr),
                reinterpret_cast<LPVOID>(Hooking::DetourGameThread),
                reinterpret_cast<LPVOID*>(&Hooking::pOriginalGameThreadUpdate)
            )) {
                std::cerr << "[Hooks] Failed to create GameThread hook." << std::endl;
            }
            else {
                if (Hooking::HookManager::EnableHook(reinterpret_cast<LPVOID>(gameThreadFuncAddr))) {
                    std::cout << "[Hooks] GameThread hook created and enabled." << std::endl;
                }
                else {
                    std::cerr << "[Hooks] Failed to enable GameThread hook." << std::endl;
                }
            }
        }
        else {
            std::cerr << "[Hooks] GameThread hook target not found. Character ESP will be disabled." << std::endl;
        }

        std::cout << "[Hooks] All hooks initialized successfully." << std::endl;
        return true;
    }

    void CleanupHooks() {
        std::cout << "[Hooks] Cleaning up..." << std::endl;

        // NEW: Disable and remove the game thread hook during shutdown
        uintptr_t gameThreadFuncAddr = AddressManager::GetGameThreadUpdateFunc();
        if (gameThreadFuncAddr && Hooking::pOriginalGameThreadUpdate) {
            Hooking::HookManager::DisableHook(reinterpret_cast<LPVOID>(gameThreadFuncAddr));
            Hooking::HookManager::RemoveHook(reinterpret_cast<LPVOID>(gameThreadFuncAddr));
            std::cout << "[Hooks] GameThread hook cleaned up." << std::endl;
        }

        kx::Hooking::D3DRenderHook::Shutdown();
        kx::Hooking::HookManager::Shutdown();

        std::cout << "[Hooks] Cleanup finished." << std::endl;
    }

} // namespace kx
