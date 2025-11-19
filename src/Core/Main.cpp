#include "Config.h"
#ifndef GW2AL_BUILD // This entire file is only compiled for standalone DLL mode

#include <windows.h>

#include "AppLifecycleManager.h"
#include "Bootstrap.h"
#include "../Utils/DebugLogger.h"

HINSTANCE dll_handle;
static HANDLE g_hSingleInstanceMutex = NULL;

// Eject thread to free the DLL
DWORD WINAPI EjectThread(LPVOID lpParameter) {
    kx::Bootstrap::Cleanup();
    Sleep(500); // Increased from 100ms to ensure all in-flight Present calls complete
    FreeLibraryAndExitThread(dll_handle, 0);
    return 0;
}

// Main function that runs in a separate thread
DWORD WINAPI MainThread(LPVOID lpParameter) {
    kx::Bootstrap::Initialize("DLL");

    // Initialize the global application lifecycle manager
    if (!kx::g_App.Initialize()) {
        LOG_ERROR("Failed to initialize application - HookManager or D3D hooks setup failed");
        kx::Bootstrap::Cleanup();
        CreateThread(0, 0, EjectThread, 0, 0, 0);
        return 1;
    }
    
    LOG_INFO("KX Vision initialized successfully");
    
    // Main loop - drive the state machine
    while (!kx::g_App.IsShutdownRequested()) {
        kx::g_App.Update();
    }
    
    LOG_INFO("Shutdown requested, cleaning up...");
    
    // Perform shutdown
    kx::g_App.Shutdown();
    
    LOG_INFO("KX Vision shut down successfully");
    
    // Cleanup common resources
    kx::Bootstrap::Cleanup();

    // Eject the DLL and exit the thread
    CreateThread(0, 0, EjectThread, 0, 0, 0);

    return 0;
}

// DLL entry point
BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        // Create a uniquely named mutex.
        // Using a GUID is a good way to ensure the name is unique.
        const wchar_t* mutexName = L"kx-vision-instance-mutex-9A8B7C6D";

        g_hSingleInstanceMutex = CreateMutexW(NULL, TRUE, mutexName);

        // Check if the mutex already exists.
        if (g_hSingleInstanceMutex == NULL || GetLastError() == ERROR_ALREADY_EXISTS) {
            // Another instance is already running. Close the handle we just tried to create
            // and signal the loader to NOT load this DLL.
            if (g_hSingleInstanceMutex) {
                CloseHandle(g_hSingleInstanceMutex);
            }
            return FALSE; // Prevents the DLL from being loaded.
        }

        // If we're here, we are the first and only instance. Proceed with initialization.
        dll_handle = hModule;
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, MainThread, NULL, 0, NULL);
    }
    break;

    case DLL_PROCESS_DETACH:
    {
        // The guaranteed fallback save
        kx::g_App.SaveSettingsOnExit();

        // It's good practice to release and close the mutex on shutdown.
        if (g_hSingleInstanceMutex) {
            ReleaseMutex(g_hSingleInstanceMutex);
            CloseHandle(g_hSingleInstanceMutex);
        }
    }
    break;
    }
    return TRUE;
}

#endif // !GW2AL_BUILD
