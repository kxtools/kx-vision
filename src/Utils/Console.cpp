#include "Console.h"

#include <iostream>
#include <windows.h>

#include "DebugLogger.h"

namespace kx {

    void SetupConsole() {
#ifdef _DEBUG
        // 1. Check if a console already exists. If so, do nothing.
        if (GetConsoleWindow() != NULL) {
            LOG_INFO("[Console] A console is already attached.");
            return;
        }

        // 2. Try to allocate a new console.
        if (!AllocConsole()) {
            // Log the failure using Windows' GetLastError() for a specific reason.
            DWORD error = GetLastError();
            char errorMsg[256];
            sprintf_s(errorMsg, "[Console] AllocConsole failed with error %lu\n", error);
            OutputDebugStringA(errorMsg);  // Use OutputDebugString as fallback
            LOG_ERROR("[Console] Failed to allocate console. Error code: %d", error);
            return;
        }

        // 3. Set a title for the console window for easy identification.
        SetConsoleTitle(L"KX Vision - Debug Console");

        // 4. Redirect standard C++ streams to the new console.
        //    Using FILE* streams is fine, but this is the idiomatic C++ way.
        FILE* dummyFile;
        freopen_s(&dummyFile, "CONIN$", "r", stdin);
        freopen_s(&dummyFile, "CONOUT$", "w", stderr);
        freopen_s(&dummyFile, "CONOUT$", "w", stdout);

        // Optional: Sync C++ streams with the C streams after redirection.
        std::ios_base::sync_with_stdio(true);

        // 5. Disable the close button to prevent accidental process termination.
        HWND consoleWindow = GetConsoleWindow();
        if (consoleWindow != NULL) {
            HMENU hMenu = GetSystemMenu(consoleWindow, FALSE);
            if (hMenu != NULL) {
                DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
            }
        }

        LOG_INFO("[Console] Debug console initialized successfully!");
#endif // _DEBUG
    }

} // namespace kx