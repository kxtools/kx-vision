#pragma once

#include <windows.h>

#if _WIN64
#pragma comment(lib, "libs/MinHook/libMinHook.x64.lib")
#else
#pragma comment(lib, "libs/MinHook/libMinHook.x86.lib")
#endif

namespace kx::Hooking {

    class HookManager {
    public:
        static bool Initialize();
        static void Shutdown();
        static bool CreateHook(LPVOID pTarget, LPVOID pDetour, LPVOID* ppOriginal);
        static bool RemoveHook(LPVOID pTarget);
        static bool EnableHook(LPVOID pTarget);
        static bool DisableHook(LPVOID pTarget);

    private:
        HookManager() = delete;
        ~HookManager() = delete;
    };

} // namespace kx::Hooking