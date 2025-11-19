#pragma once

namespace kx {

    /**
     * @brief Common initialization and cleanup utilities for entry points
     * 
     * Provides shared functionality for both DLL and GW2AL entry points,
     * reducing code duplication and ensuring consistent behavior.
     */
    class Bootstrap {
    public:
        /**
         * @brief Initialize logging system
         * 
         * Sets up the debug logger for early logging capability.
         * Must be called before any LOG_* macros.
         */
        static void InitializeLogging();

        /**
         * @brief Initialize console for debug builds
         * 
         * Sets up console output for debug builds only.
         * Safe to call multiple times.
         */
        static void InitializeConsole();

        /**
         * @brief Cleanup logging system
         * 
         * Closes log files and cleans up logging resources.
         * Should be called before process exit.
         */
        static void CleanupLogging();

        /**
         * @brief Cleanup console resources
         * 
         * Closes standard streams and frees console in debug builds.
         * Safe to call multiple times.
         */
        static void CleanupConsole();

        /**
         * @brief Perform complete initialization sequence
         * 
         * Calls InitializeLogging() and InitializeConsole() in the correct order.
         * This is the recommended way to initialize common resources.
         * 
         * @param modeName Human-readable mode name for logging (e.g., "DLL", "GW2AL")
         */
        static void Initialize(const char* modeName);

        /**
         * @brief Perform complete cleanup sequence
         * 
         * Calls CleanupLogging() and CleanupConsole() in the correct order.
         * This is the recommended way to cleanup common resources.
         */
        static void Cleanup();
    };

} // namespace kx
