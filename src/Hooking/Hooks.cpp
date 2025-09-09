#include "Hooks.h"

#include <iostream>

#include "AppState.h"
#include "D3DRenderHook.h"
#include "HookManager.h"

namespace kx {

    bool InitializeHooks() {
        g_presentHookStatus = HookStatus::Unknown;

        if (!kx::Hooking::HookManager::Initialize()) {
            return false;
        }

        if (!kx::Hooking::D3DRenderHook::Initialize()) {
            kx::Hooking::HookManager::Shutdown();
            g_presentHookStatus = HookStatus::Failed;
            return false;
        }

        std::cout << "[Hooks] All hooks initialized successfully." << std::endl;
        return true;
    }

    void CleanupHooks() {
        std::cout << "[Hooks] Cleaning up..." << std::endl;

        kx::Hooking::D3DRenderHook::Shutdown();
        kx::Hooking::HookManager::Shutdown();

        std::cout << "[Hooks] Cleanup finished." << std::endl;
    }

} // namespace kx
