#pragma once

#include <windows.h>
#include "../libs/MinHook/MinHook.h"

#if _WIN64
#pragma comment(lib, "libs/MinHook/libMinHook.x64.lib")
#else
#pragma comment(lib, "libs/MinHook/libMinHook.x86.lib")
#endif

namespace kx::Hooking {

    /**
     * @brief Manages the global state and operations for MinHook.
     */
    class HookManager {
    public:
        /**
         * @brief Initializes the MinHook library.
         * @return True if successful, false otherwise.
         */
        static bool Initialize();

        /**
         * @brief Uninitializes the MinHook library, disabling and removing all hooks.
         */
        static void Shutdown();

        /**
         * @brief Creates and enables a hook for a target function.
         * @param pTarget Address of the target function.
         * @param pDetour Address of the detour function.
         * @param ppOriginal Pointer to store the address of the original function trampoline.
         * @return True if the hook was successfully created and enabled, false otherwise.
         */
        static bool CreateHook(LPVOID pTarget, LPVOID pDetour, LPVOID* ppOriginal);

        /**
         * @brief Removes a previously created hook.
         * @param pTarget Address of the target function hook to remove.
         * @return True if successful, false otherwise.
         */
        static bool RemoveHook(LPVOID pTarget);

        /**
        * @brief Enables a previously created hook.
        * @param pTarget Address of the target function hook to enable.
        * @return True if successful, false otherwise.
        */
        static bool EnableHook(LPVOID pTarget);

        /**
        * @brief Disables a previously created hook without removing it.
        * @param pTarget Address of the target function hook to disable.
        * @return True if successful, false otherwise.
        */
        static bool DisableHook(LPVOID pTarget);

    private:
        // Prevent instantiation
        HookManager() = delete;
        ~HookManager() = delete;
    };

} // namespace kx::Hooking