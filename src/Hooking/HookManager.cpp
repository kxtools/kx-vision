#include "HookManager.h"

#include <iostream> // Replace with proper logging later

#include "../../libs/MinHook/MinHook.h"

namespace kx::Hooking {

    bool HookManager::Initialize() {
        MH_STATUS status = MH_Initialize();
        if (status != MH_OK) {
            std::cerr << "[HookManager] ERROR: Failed to initialize MinHook: "
                << MH_StatusToString(status) << std::endl;
            return false;
        }
        std::cout << "[HookManager] MinHook initialized." << std::endl;
        return true;
    }

    void HookManager::Shutdown() {
        // It's often sufficient to just uninitialize, which disables/removes all hooks.
        MH_STATUS status = MH_Uninitialize();
        if (status != MH_OK) {
            std::cerr << "[HookManager] Failed to uninitialize MinHook: "
                << MH_StatusToString(status) << std::endl;
        }
        else {
            std::cout << "[HookManager] MinHook uninitialized." << std::endl;
        }
    }

    bool HookManager::CreateHook(LPVOID pTarget, LPVOID pDetour, LPVOID* ppOriginal) {
        if (!pTarget) {
            std::cerr << "[HookManager] CreateHook failed: pTarget is null." << std::endl;
            return false;
        }
        MH_STATUS status = MH_CreateHook(pTarget, pDetour, ppOriginal);
        if (status != MH_OK) {
            std::cerr << "[HookManager] Failed to create hook for target " << pTarget
                << ": " << MH_StatusToString(status) << std::endl;
            return false;
        }
        return true;
    }

    bool HookManager::RemoveHook(LPVOID pTarget) {
        if (!pTarget) return false; // Or log error
        MH_STATUS status = MH_RemoveHook(pTarget);
        if (status != MH_OK) {
            // This can happen if the hook wasn't created or already removed. Might not be a critical error.
            std::cerr << "[HookManager] Failed to remove hook for target " << pTarget
                << ": " << MH_StatusToString(status) << std::endl;
            return false;
        }
        return true;
    }

    bool HookManager::EnableHook(LPVOID pTarget) {
        if (!pTarget) return false; // Or log error
        MH_STATUS status = MH_EnableHook(pTarget);
        if (status != MH_OK) {
            std::cerr << "[HookManager] Failed to enable hook for target " << pTarget
                << ": " << MH_StatusToString(status) << std::endl;
            return false;
        }
        return true;
    }

    bool HookManager::DisableHook(LPVOID pTarget) {
        if (!pTarget) return false; // Or log error
        MH_STATUS status = MH_DisableHook(pTarget);
        if (status != MH_OK) {
            std::cerr << "[HookManager] Failed to disable hook for target " << pTarget
                << ": " << MH_StatusToString(status) << std::endl;
            return false;
        }
        return true;
    }


} // namespace kx::Hooking