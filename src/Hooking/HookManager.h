#pragma once

#include <windows.h>
#include <cstdint>

#if _WIN64
#pragma comment(lib, "libs/MinHook/libMinHook.x64.lib")
#else
#pragma comment(lib, "libs/MinHook/libMinHook.x86.lib")
#endif

namespace kx::Hooking {

    struct HookHandle {
        LPVOID target = nullptr;
        LPVOID original = nullptr;
        bool created = false;
        bool enabled = false;
    };

    class HookCallbackGuard {
    public:
        HookCallbackGuard() noexcept;
        ~HookCallbackGuard();
        HookCallbackGuard(const HookCallbackGuard&) = delete;
        HookCallbackGuard& operator=(const HookCallbackGuard&) = delete;
    };

    class HookManager {
    public:
        static bool Initialize();
        static void Shutdown();
        static bool CreateHook(LPVOID pTarget, LPVOID pDetour, LPVOID* ppOriginal);
        static bool RemoveHook(LPVOID pTarget);
        static bool EnableHook(LPVOID pTarget);
        static bool DisableHook(LPVOID pTarget);
        static bool CreateAndEnable(HookHandle& handle, LPVOID target, LPVOID detour, LPVOID* outOriginal = nullptr);
        static void DisableAndRemove(HookHandle& handle);

        static void BeginShutdown();
        static bool IsShuttingDown();
        static void DisableAll();
        static void WaitForCallbacks();
        static void RemoveAll();

        static void EnterCallback() noexcept;
        static void LeaveCallback() noexcept;

    private:
        HookManager() = delete;
        ~HookManager() = delete;
    };

} // namespace kx::Hooking
