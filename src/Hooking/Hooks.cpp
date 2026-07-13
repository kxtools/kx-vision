#include "Hooks.h"

#include "../Core/Config.h"
#include "../Utils/DebugLogger.h"
#include "D3DRenderHook.h"
#include "HookManager.h"

namespace kx {

bool InitializeHostHooks() {
    if (!Hooking::HookManager::Initialize()) {
        LOG_ERROR("[Hooks] Failed to initialize HookManager");
        return false;
    }

#ifndef GW2AL_BUILD
    if (!Hooking::D3DRenderHook::Initialize()) {
        Hooking::HookManager::Shutdown();
        return false;
    }
#endif

    LOG_INFO("[Hooks] Host hooks initialized successfully");
    return true;
}

void ShutdownHostHooks() {
    LOG_INFO("[Hooks] Cleaning up host hooks");

#ifndef GW2AL_BUILD
    Hooking::D3DRenderHook::Shutdown();
#endif

    Hooking::HookManager::Shutdown();
    LOG_INFO("[Hooks] Host hook cleanup finished");
}

} // namespace kx
