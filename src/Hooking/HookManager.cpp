#include "HookManager.h"
#include "../Utils/DebugLogger.h"
#include "../../libs/MinHook/MinHook.h"

namespace kx::Hooking {

    bool HookManager::Initialize() {
        MH_STATUS status = MH_Initialize();
        if (status != MH_OK) {
        	LOG_ERROR("[HookManager] Failed to initialize MinHook: %s",
        	MH_StatusToString(status));
            return false;
        }
        LOG_INFO("[HookManager] MinHook initialized.");
        return true;
    }

    void HookManager::Shutdown() {
        MH_STATUS status = MH_Uninitialize();
        if (status != MH_OK) {
            LOG_ERROR("[HookManager] Failed to uninitialize MinHook: %s",
                MH_StatusToString(status));
        }
        else {
            LOG_INFO("[HookManager] MinHook uninitialized.");
        }
    }

    bool HookManager::CreateHook(LPVOID pTarget, LPVOID pDetour, LPVOID* ppOriginal) {
        if (!pTarget) {
            LOG_ERROR("[HookManager] CreateHook failed: pTarget is null.");
            return false;
        }
        MH_STATUS status = MH_CreateHook(pTarget, pDetour, ppOriginal);
        if (status != MH_OK) {
            LOG_ERROR("[HookManager] Failed to create hook for target %p: %s", pTarget,
                MH_StatusToString(status));
            return false;
        }
        return true;
    }

    bool HookManager::RemoveHook(LPVOID pTarget) {
        if (!pTarget) return false;
        MH_STATUS status = MH_RemoveHook(pTarget);
        if (status != MH_OK) {
            LOG_WARN("[HookManager] Failed to remove hook for target %p: %s", pTarget,
                MH_StatusToString(status));
            return false;
        }
        return true;
    }

    bool HookManager::EnableHook(LPVOID pTarget) {
        if (!pTarget) return false;
        MH_STATUS status = MH_EnableHook(pTarget);
        if (status != MH_OK) {
            LOG_ERROR("[HookManager] Failed to enable hook for target %p: %s", pTarget,
                MH_StatusToString(status));
            return false;
        }
        return true;
    }

    bool HookManager::DisableHook(LPVOID pTarget) {
        if (!pTarget) return false;
        MH_STATUS status = MH_DisableHook(pTarget);
        if (status != MH_OK) {
            LOG_WARN("[HookManager] Failed to disable hook for target %p: %s", pTarget,
                MH_StatusToString(status));
            return false;
        }
        return true;
    }


} // namespace kx::Hooking