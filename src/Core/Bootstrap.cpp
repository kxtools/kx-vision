#include "Bootstrap.h"
#include "../Utils/DebugLogger.h"
#include "../Utils/Console.h"

#include <cstdio>

#include <Windows.h>

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
        InitializeConsole();
#ifdef _DEBUG
        LOG_INFO("[Bootstrap] Console/logger reinitialized. Starting common bootstrap in %s mode.", modeName);
#else
        (void)modeName;
#endif

#ifdef _DEBUG
        LOG_INFO("KX Vision starting up in %s mode...", modeName);
#endif
    }

    void Bootstrap::Cleanup() {
        CleanupLogging();
        CleanupConsole();
    }

} // namespace kx
