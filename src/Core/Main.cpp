#include <cstdio> // Required for fclose
#include <iostream>
#include <windows.h>

#include "AddressManager.h"
#include "AppState.h"   // Include for g_isVisionWindowOpen, g_isShuttingDown
#include "Console.h"
#include "Hooks.h"

HINSTANCE dll_handle;

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
#ifdef _DEBUG
    kx::SetupConsole(); // Only setup console in Debug builds
#endif // _DEBUG

    // Initialize AddressManager FIRST, so pointers are ready before hooks start
    kx::AddressManager::Initialize();

    if (!kx::InitializeHooks()) {
        std::cerr << "Failed to initialize hooks." << std::endl;
        return 1;
    }

    // Main loop to keep the hook alive
    while (kx::g_isVisionWindowOpen && !(GetAsyncKeyState(VK_DELETE) & 0x8000)) {
        Sleep(100); // Sleep to avoid busy-waiting
    }

    // Signal hooks to stop processing before actual cleanup
    kx::g_isShuttingDown.store(true, std::memory_order_release);

    // Give hooks a moment to recognize the flag before cleanup starts
	// This helps prevent calls into ImGui after it's destroyed.
    Sleep(250);

    // Cleanup hooks and ImGui
    kx::CleanupHooks();

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
        dll_handle = hModule;
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, MainThread, NULL, 0, NULL);
        break;
    case DLL_PROCESS_DETACH:
        // Optional: Handle cleanup if necessary
        break;
    }
    return TRUE;
}
