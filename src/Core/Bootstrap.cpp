#include "Bootstrap.h"
#include "../Utils/DebugLogger.h"
#include "../Utils/Console.h"
#include "AppState.h"
#include <cstdio>

namespace kx {

    void Bootstrap::InitializeLogging() {
        LOG_INIT();
    }

    void Bootstrap::InitializeConsole() {
#ifdef _DEBUG
        SetupConsole();
        LOG_REINIT(); // Reinitialize logger to enable console output
#endif
    }

    void Bootstrap::CleanupLogging() {
        LOG_CLEANUP();
    }

    void Bootstrap::CleanupConsole() {
#ifdef _DEBUG
        // Close standard streams and free console in debug builds
        if (stdout) fclose(stdout);
        if (stderr) fclose(stderr);
        if (stdin) fclose(stdin);

        if (!FreeConsole()) {
            OutputDebugStringA("kx-vision: FreeConsole() failed.\n");
        }
#endif
    }

    void Bootstrap::Initialize(const char* modeName) {
        InitializeLogging();
        
        // Load settings and apply saved log level early
        AppState::Get();  // Force settings load
        LOG_INIT_WITH_SETTINGS();
        
        InitializeConsole();
        LOG_INFO("KX Vision starting up in %s mode...", modeName);
    }

    void Bootstrap::Cleanup() {
        CleanupLogging();
        CleanupConsole();
    }

} // namespace kx
