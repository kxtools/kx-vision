#include "HookManager.h"
#include "../Utils/DebugLogger.h"
#include "../../libs/MinHook/MinHook.h"

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <vector>

namespace {
struct HookRecord {
    LPVOID target = nullptr;
    bool enabled = false;
};

std::mutex g_hookMutex;
std::vector<HookRecord> g_hooks;
std::atomic<bool> g_initialized{false};
std::atomic<bool> g_shuttingDown{false};
std::atomic<uint32_t> g_activeCallbacks{0};
std::mutex g_rundownMutex;
std::condition_variable g_rundownCv;
}

namespace kx::Hooking {

    bool HookManager::Initialize() {
        std::lock_guard<std::mutex> lock(g_hookMutex);
        if (g_initialized.load(std::memory_order_acquire)) return true;
        MH_STATUS status = MH_Initialize();
        if (status != MH_OK) {
        	LOG_ERROR("[HookManager] Failed to initialize MinHook: %s",
        	MH_StatusToString(status));
            return false;
        }
        g_hooks.clear();
        g_shuttingDown.store(false, std::memory_order_release);
        g_initialized.store(true, std::memory_order_release);
        LOG_INFO("[HookManager] MinHook initialized.");
        return true;
    }

    void HookManager::Shutdown() {
        BeginShutdown();
        DisableAll();
        WaitForCallbacks();
        RemoveAll();
        std::lock_guard<std::mutex> lock(g_hookMutex);
        if (!g_initialized.exchange(false, std::memory_order_acq_rel)) return;
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
        std::lock_guard<std::mutex> lock(g_hookMutex);
        if (!g_initialized.load(std::memory_order_acquire) || g_shuttingDown.load(std::memory_order_acquire)) {
            LOG_WARN("[HookManager] CreateHook rejected during shutdown or before initialization.");
            return false;
        }
        MH_STATUS status = MH_CreateHook(pTarget, pDetour, ppOriginal);
        if (status != MH_OK) {
            LOG_ERROR("[HookManager] Failed to create hook for target %p: %s", pTarget,
                MH_StatusToString(status));
            return false;
        }
        g_hooks.push_back({pTarget, false});
        return true;
    }

    bool HookManager::RemoveHook(LPVOID pTarget) {
        if (!pTarget) return false;
        std::lock_guard<std::mutex> lock(g_hookMutex);
        MH_STATUS status = MH_RemoveHook(pTarget);
        if (status != MH_OK && status != MH_ERROR_NOT_CREATED) {
            LOG_WARN("[HookManager] Failed to remove hook for target %p: %s", pTarget,
                MH_StatusToString(status));
            return false;
        }
        g_hooks.erase(std::remove_if(g_hooks.begin(), g_hooks.end(),
            [pTarget](const HookRecord& record) { return record.target == pTarget; }), g_hooks.end());
        return true;
    }

    bool HookManager::EnableHook(LPVOID pTarget) {
        if (!pTarget) return false;
        std::lock_guard<std::mutex> lock(g_hookMutex);
        if (g_shuttingDown.load(std::memory_order_acquire)) return false;
        MH_STATUS status = MH_EnableHook(pTarget);
        if (status != MH_OK && status != MH_ERROR_ENABLED) {
            LOG_ERROR("[HookManager] Failed to enable hook for target %p: %s", pTarget,
                MH_StatusToString(status));
            return false;
        }
        for (auto& record : g_hooks) if (record.target == pTarget) record.enabled = true;
        return true;
    }

    bool HookManager::DisableHook(LPVOID pTarget) {
        if (!pTarget) return false;
        std::lock_guard<std::mutex> lock(g_hookMutex);
        MH_STATUS status = MH_DisableHook(pTarget);
        if (status != MH_OK && status != MH_ERROR_DISABLED) {
            LOG_WARN("[HookManager] Failed to disable hook for target %p: %s", pTarget,
                MH_StatusToString(status));
            return false;
        }
        for (auto& record : g_hooks) if (record.target == pTarget) record.enabled = false;
        return true;
    }

    bool HookManager::CreateAndEnable(HookHandle& handle, LPVOID target, LPVOID detour, LPVOID* outOriginal) {
        if (handle.created || !target || !detour) return false;
        LPVOID original = nullptr;
        if (!CreateHook(target, detour, &original)) return false;
        handle = {target, original, true, false};
        if (outOriginal) *outOriginal = original;
        if (!EnableHook(target)) {
            RemoveHook(target);
            if (outOriginal) *outOriginal = nullptr;
            handle = {};
            return false;
        }
        handle.enabled = true;
        return true;
    }

    void HookManager::DisableAndRemove(HookHandle& handle) {
        if (handle.enabled && handle.target) DisableHook(handle.target);
        if (handle.created && handle.target) RemoveHook(handle.target);
        handle = {};
    }

    void HookManager::BeginShutdown() {
        g_shuttingDown.store(true, std::memory_order_release);
    }

    bool HookManager::IsShuttingDown() {
        return g_shuttingDown.load(std::memory_order_acquire);
    }

    void HookManager::DisableAll() {
        std::lock_guard<std::mutex> lock(g_hookMutex);
        if (!g_initialized.load(std::memory_order_acquire)) return;
        for (auto& record : g_hooks) {
            if (!record.enabled) continue;
            const MH_STATUS status = MH_DisableHook(record.target);
            if (status == MH_OK || status == MH_ERROR_DISABLED) record.enabled = false;
        }
    }

    void HookManager::WaitForCallbacks() {
        std::unique_lock<std::mutex> lock(g_rundownMutex);
        g_rundownCv.wait(lock, [] { return g_activeCallbacks.load(std::memory_order_acquire) == 0; });
    }

    void HookManager::RemoveAll() {
        std::lock_guard<std::mutex> lock(g_hookMutex);
        if (!g_initialized.load(std::memory_order_acquire)) return;
        for (auto& record : g_hooks) MH_RemoveHook(record.target);
        g_hooks.clear();
    }

    void HookManager::EnterCallback() noexcept {
        g_activeCallbacks.fetch_add(1, std::memory_order_acq_rel);
    }

    void HookManager::LeaveCallback() noexcept {
        if (g_activeCallbacks.fetch_sub(1, std::memory_order_acq_rel) == 1) g_rundownCv.notify_all();
    }

    HookCallbackGuard::HookCallbackGuard() noexcept { HookManager::EnterCallback(); }
    HookCallbackGuard::~HookCallbackGuard() { HookManager::LeaveCallback(); }


} // namespace kx::Hooking
