#include <cstdio> // Required for fclose
#include <windows.h>

#include "AddressManager.h"
#include "AppState.h"   // Include for AppState singleton
#include "Console.h"
#include "Hooks.h"
#include "../Utils/DebugLogger.h" // Include for logger initialization

HINSTANCE dll_handle;
static HANDLE g_hSingleInstanceMutex = NULL;

// Eject thread to free the DLL
DWORD WINAPI EjectThread(LPVOID lpParameter) {
#ifdef _DEBUG
    // In Debug builds, close standard streams and free the console
    if (stdout) fclose(stdout);
    if (stderr) fclose(stderr);
    if (stdin) fclose(stdin);

    if (!FreeConsole()) {
        OutputDebugStringA("kx-vision: FreeConsole() failed.\n");
    }
#endif // _DEBUG
    Sleep(100);
    FreeLibraryAndExitThread(dll_handle, 0);
    return 0;
}

// Main function that runs in a separate thread
DWORD WINAPI MainThread(LPVOID lpParameter) {
    // Initialize debug logger first for early logging capability
    LOG_INIT();
    
#ifdef _DEBUG
    kx::SetupConsole(); // Only setup console in Debug builds
#endif // _DEBUG

    LOG_INFO("KX Vision starting up...");
    
    // Initialize AddressManager FIRST, so pointers are ready before hooks start
    kx::AddressManager::Initialize();

    if (!kx::InitializeHooks()) {
        LOG_ERROR("Failed to initialize hooks.");
        return 1;
    }

    LOG_INFO("KX Vision hooks initialized successfully");

    // Main loop to keep the hook alive
    while (kx::AppState::Get().IsVisionWindowOpen() && !(GetAsyncKeyState(VK_DELETE) & 0x8000)) {
        Sleep(100); // Sleep to avoid busy-waiting
    }

    // Signal hooks to stop processing before actual cleanup
    kx::AppState::Get().SetShuttingDown(true);

    // Give hooks a moment to recognize the flag before cleanup starts
	// This helps prevent calls into ImGui after it's destroyed.
    Sleep(250);

    // Cleanup hooks and ImGui
    kx::CleanupHooks();

    LOG_INFO("KX Vision shutting down...");
    
    // Cleanup logger (close log file)
    LOG_CLEANUP();

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
        // It's good practice to release and close the mutex on shutdown.
        // This ensures it's cleaned up properly if the DLL is ever unloaded
        // and then reloaded without the process restarting.
        if (g_hSingleInstanceMutex) {
            ReleaseMutex(g_hSingleInstanceMutex);
            CloseHandle(g_hSingleInstanceMutex);
        }
    }
    break;
    }
    return TRUE;
}
