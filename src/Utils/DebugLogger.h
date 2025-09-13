#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <memory>
#include <Windows.h>
#include "../Core/AppState.h"
#include "MemorySafety.h"

namespace kx {
namespace Debug {

/**
 * @brief Thread-safe logging system with rate limiting and configurable levels
 * 
 * Features:
 * - Thread-safe operation with mutex protection
 * - Rate limiting to prevent log spam
 * - Configurable log levels
 * - Memory management for rate limiting cache
 * - Exception-safe file operations
 */
class Logger {
public:
    enum Level {
        DEBUG = 0,  // Most verbose - detailed debugging info
        INFO = 1,   // General information
        WARNING = 2,// Warnings
        ERR = 3,    // Errors
        CRITICAL = 4// Critical errors
    };

private:
    // Thread-safe static members
    static std::atomic<Level> s_minLogLevel;
    static std::atomic<size_t> s_rateLimitCacheSize;
    static std::mutex s_rateLimitMutex;
    static std::unordered_map<std::string, std::chrono::steady_clock::time_point> s_lastLogTime;
    static std::mutex s_fileMutex;
    static std::chrono::steady_clock::time_point s_lastCleanup;
    static std::unique_ptr<std::ofstream> s_logFile;
    static std::string s_logFilePath;
    
    // Configuration constants
    static constexpr std::chrono::milliseconds RATE_LIMIT_INTERVAL{500}; // 500ms between identical messages (more aggressive)
    static constexpr std::chrono::milliseconds AGGRESSIVE_RATE_LIMIT{100}; // 100ms for high-frequency patterns
    static constexpr size_t MAX_RATE_LIMIT_ENTRIES = 1000; // Prevent memory growth
    static constexpr std::chrono::minutes RATE_LIMIT_CLEANUP_INTERVAL{5}; // Clean old entries every 5 minutes
    
    // Helper methods
    static std::string GetTimestamp() noexcept;
    static std::string LevelToString(Level level) noexcept;
    static void CleanupRateLimitCache() noexcept; // Assumes caller holds s_rateLimitMutex
    static bool ShouldLogMessage(const std::string& message) noexcept;
    static bool ShouldRateLimit(const std::string& key, std::chrono::milliseconds interval) noexcept;
    static void LogImpl(Level level, const std::string& message, const std::string& context = "") noexcept;
    static bool InitializeLogFile() noexcept;
    static std::string GetLogFilePath() noexcept;

public:
    // Initialize logger (call once at startup for guaranteed proper initialization)
    static void Initialize() noexcept {
        try {
            // Ensure static members are properly initialized
            s_rateLimitCacheSize.store(0, std::memory_order_release);
            
            // Lock and clear any existing data to ensure clean state
            std::lock_guard<std::mutex> lock(s_rateLimitMutex);
            s_lastLogTime.clear();
            s_lastCleanup = std::chrono::steady_clock::now();
            
            // Initialize log file
            InitializeLogFile();
        }
        catch (...) {
            // If initialization fails, logger will work with defaults
        }
    }
    
    // Cleanup logger (call at shutdown to properly close files)
    static void Cleanup() noexcept {
        try {
            std::lock_guard<std::mutex> lock(s_fileMutex);
            if (s_logFile && s_logFile->is_open()) {
                s_logFile->flush();
                s_logFile->close();
            }
            s_logFile.reset();
        }
        catch (...) {
            // Ignore cleanup errors
        }
    }
    
    // Thread-safe log level management
    static void SetMinLogLevel(Level level) noexcept {
        s_minLogLevel.store(level, std::memory_order_release);
    }

    static Level GetMinLogLevel() noexcept {
        return s_minLogLevel.load(std::memory_order_acquire);
    }

    // Convenience methods for setting common log levels
    static void SetLogLevelDebug() noexcept { SetMinLogLevel(DEBUG); }
    static void SetLogLevelInfo() noexcept { SetMinLogLevel(INFO); }
    static void SetLogLevelWarning() noexcept { SetMinLogLevel(WARNING); }
    static void SetLogLevelError() noexcept { SetMinLogLevel(ERR); }
    static void SetLogLevelCritical() noexcept { SetMinLogLevel(CRITICAL); }
    
    // Helper function to check if a level should be logged (used by macros)
    static bool ShouldLog(Level level) noexcept {
        try {
            // First check the internal log level
            if (level < s_minLogLevel.load(std::memory_order_acquire)) {
                return false;
            }
            
            // Then check the AppState debug logging setting
            // Only check for DEBUG level logs - errors should always be allowed
            if (level == DEBUG) {
                return kx::AppState::Get().IsDebugLoggingEnabled();
            }
            
            // For non-debug levels (INFO, WARNING, ERROR, CRITICAL), always allow if level passes
            return true;
        }
        catch (...) {
            // If we can't check AppState for any reason, fall back to level-only check
            return level >= s_minLogLevel.load(std::memory_order_acquire);
        }
    }
    
    // Debug function to verify current log level (useful for troubleshooting)
    static void PrintCurrentLogLevel() noexcept {
        try {
            Level current = GetMinLogLevel();
            const char* levelName = "UNKNOWN";
            switch (current) {
                case DEBUG: levelName = "DEBUG"; break;
                case INFO: levelName = "INFO"; break;
                case WARNING: levelName = "WARNING"; break;
                case ERR: levelName = "ERROR"; break;
                case CRITICAL: levelName = "CRITICAL"; break;
                default: 
                    if (current >= 999) levelName = "DISABLED";
                    break;
            }
            
            bool guiDebugEnabled = kx::AppState::Get().IsDebugLoggingEnabled();
            
            // Force output to console regardless of current level
            std::cout << "[LOGGER] Internal log level: " << levelName 
                      << " (" << static_cast<int>(current) << ")" << std::endl;
            std::cout << "[LOGGER] GUI Debug Logging: " << (guiDebugEnabled ? "ENABLED" : "DISABLED") << std::endl;
            std::cout << "[LOGGER] DEBUG logs will " << (ShouldLog(DEBUG) ? "BE SHOWN" : "BE HIDDEN") << std::endl;
            std::cout.flush();
        }
        catch (...) {
            std::cout << "[LOGGER] Error checking log status" << std::endl;
        }
    }
    
    // Disable all logging by setting an impossibly high log level
    static void DisableLogging() noexcept { 
        SetMinLogLevel(static_cast<Level>(999)); 
    }

    // Main logging function - thread-safe and exception-safe
    static void Log(Level level, const std::string& message) noexcept;
    
    // Formatted logging functions (printf-style) - accepts const char*
    template<typename... Args>
    static void LogFormatted(Level level, const char* format, Args&&... args) noexcept {
        try {
            // Quick level check first with proper memory ordering
            if (level < s_minLogLevel.load(std::memory_order_acquire)) {
                return;
            }
            
            if constexpr (sizeof...(args) == 0) {
                // No additional arguments, treat as simple string
                Log(level, std::string(format));
            } else {
                // Format the message with arguments
                char buffer[1024];
                int result = snprintf(buffer, sizeof(buffer), format, std::forward<Args>(args)...);
                
                if (result > 0 && result < static_cast<int>(sizeof(buffer))) {
                    Log(level, std::string(buffer));
                } else {
                    // Fallback for formatting errors
                    Log(level, std::string("LOG FORMAT ERROR: ") + format);
                }
            }
        }
        catch (...) {
            // Ignore formatting errors
        }
    }
    
    // Overload for std::string format parameter
    template<typename... Args>
    static void LogFormatted(Level level, const std::string& format, Args&&... args) noexcept {
        try {
            // Quick level check first with proper memory ordering
            if (level < s_minLogLevel.load(std::memory_order_acquire)) {
                return;
            }
            
            if constexpr (sizeof...(args) == 0) {
                // No additional arguments, just log the string directly
                Log(level, format);
            } else {
                // Format the message with arguments using c_str()
                char buffer[1024];
                int result = snprintf(buffer, sizeof(buffer), format.c_str(), std::forward<Args>(args)...);
                
                if (result > 0 && result < static_cast<int>(sizeof(buffer))) {
                    Log(level, std::string(buffer));
                } else {
                    // Fallback for formatting errors
                    Log(level, std::string("LOG FORMAT ERROR: ") + format);
                }
            }
        }
        catch (...) {
            // Ignore formatting errors
        }
    }
    
    // Specialized logging functions
    static void LogPointer(const std::string& name, const void* ptr) noexcept;
    static void LogMemoryAccess(const std::string& className, const std::string& method, 
                               const void* ptr, uintptr_t offset) noexcept;
    static void LogException(const std::string& className, const std::string& method, 
                           const std::string& details = "") noexcept;
};

// Macros for easier usage - all wrapped for exception safety with runtime level checks
// Support both string and printf-style formatting
#define LOG_DEBUG(...) do { \
    if (kx::Debug::Logger::ShouldLog(kx::Debug::Logger::DEBUG)) { \
        try { \
            kx::Debug::Logger::LogFormatted(kx::Debug::Logger::DEBUG, __VA_ARGS__); \
        } \
        catch (...) { /* Ignore logging errors */ } \
    } \
} while(0)

#define LOG_INFO(...) do { \
    if (kx::Debug::Logger::ShouldLog(kx::Debug::Logger::INFO)) { \
        try { \
            kx::Debug::Logger::LogFormatted(kx::Debug::Logger::INFO, __VA_ARGS__); \
        } \
        catch (...) { /* Ignore logging errors */ } \
    } \
} while(0)

#define LOG_WARN(...) do { \
    if (kx::Debug::Logger::ShouldLog(kx::Debug::Logger::WARNING)) { \
        try { \
            kx::Debug::Logger::LogFormatted(kx::Debug::Logger::WARNING, __VA_ARGS__); \
        } \
        catch (...) { /* Ignore logging errors */ } \
    } \
} while(0)

#define LOG_ERROR(...) do { \
    if (kx::Debug::Logger::ShouldLog(kx::Debug::Logger::ERR)) { \
        try { \
            kx::Debug::Logger::LogFormatted(kx::Debug::Logger::ERR, __VA_ARGS__); \
        } \
        catch (...) { /* Ignore logging errors */ } \
    } \
} while(0)

#define LOG_CRITICAL(...) do { \
    if (kx::Debug::Logger::ShouldLog(kx::Debug::Logger::CRITICAL)) { \
        try { \
            kx::Debug::Logger::LogFormatted(kx::Debug::Logger::CRITICAL, __VA_ARGS__); \
        } \
        catch (...) { /* Ignore logging errors */ } \
    } \
} while(0)

#define LOG_PTR(name, ptr) do { \
    if (kx::Debug::Logger::ShouldLog(kx::Debug::Logger::DEBUG)) { \
        try { kx::Debug::Logger::LogPointer(name, ptr); } \
        catch (...) { /* Ignore logging errors */ } \
    } \
} while(0)

#define LOG_MEMORY(cls, method, ptr, offset) do { \
    if (kx::Debug::Logger::ShouldLog(kx::Debug::Logger::DEBUG)) { \
        try { kx::Debug::Logger::LogMemoryAccess(cls, method, ptr, offset); } \
        catch (...) { /* Ignore logging errors */ } \
    } \
} while(0)

#define LOG_EXCEPTION(cls, method, details) do { \
    if (kx::Debug::Logger::ShouldLog(kx::Debug::Logger::ERR)) { \
        try { kx::Debug::Logger::LogException(cls, method, details); } \
        catch (...) { /* Ignore logging errors */ } \
    } \
} while(0)

// Convenience macros for setting log levels
#define LOG_SET_LEVEL_DEBUG() kx::Debug::Logger::SetLogLevelDebug()
#define LOG_SET_LEVEL_INFO() kx::Debug::Logger::SetLogLevelInfo()
#define LOG_SET_LEVEL_WARNING() kx::Debug::Logger::SetLogLevelWarning()
#define LOG_SET_LEVEL_ERROR() kx::Debug::Logger::SetLogLevelError()
#define LOG_SET_LEVEL_CRITICAL() kx::Debug::Logger::SetLogLevelCritical()

// Convenience macro to disable all logging
#define LOG_DISABLE() kx::Debug::Logger::DisableLogging()

// Convenience macro to initialize logger
#define LOG_INIT() kx::Debug::Logger::Initialize()

// Convenience macro to cleanup logger
#define LOG_CLEANUP() kx::Debug::Logger::Cleanup()

// Debug macro to check current log level
#define LOG_PRINT_LEVEL() kx::Debug::Logger::PrintCurrentLogLevel()

/**
 * @brief Safe memory read implementation without C++ objects for exception safety
 * @tparam T Type to read from memory
 * @param address Memory address to read from
 * @param result Reference to store the result
 * @return true if read was successful, false otherwise
 */
template<typename T>
inline bool SafeReadImpl(uintptr_t address, T& result) noexcept {
    __try {
        result = *reinterpret_cast<const T*>(address);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

/**
 * @brief Safe memory access helper (silent version for frequent operations)
 * @tparam T Type to read from memory
 * @param basePtr Base pointer to read from
 * @param offset Offset from base pointer
 * @param result Reference to store the result
 * @return true if read was successful, false otherwise
 */
template<typename T>
inline bool SafeRead(void* basePtr, uintptr_t offset, T& result) noexcept {
    if (!basePtr) {
        return false;
    }
    
    const uintptr_t address = reinterpret_cast<uintptr_t>(basePtr) + offset;
    
    // Use centralized address validation from SafeAccess
    if (address == 0xFFFFFFFFFFFFFFFF || 
        address < SafeAccess::MIN_VALID_MEMORY_ADDRESS || 
        address > SafeAccess::MAX_VALID_MEMORY_ADDRESS) {
        return false;
    }
    
    // Use centralized memory safety check
    if (!SafeAccess::IsMemorySafe(reinterpret_cast<void*>(address), sizeof(T))) {
        return false;
    }
    
    return SafeReadImpl(address, result);
}

/**
 * @brief Safe memory access helper with detailed error logging
 * @tparam T Type to read from memory  
 * @param basePtr Base pointer to read from
 * @param offset Offset from base pointer
 * @param result Reference to store the result
 * @param context Optional context string for debugging
 * @return true if read was successful, false otherwise
 */
template<typename T>
inline bool SafeReadWithLogging(void* basePtr, uintptr_t offset, T& result, 
                               const std::string& context = "") noexcept {
    if (!basePtr) {
        if (!context.empty()) {
            LOG_ERROR("SafeRead: base pointer is null in " + context);
        }
        return false;
    }
    
    const uintptr_t address = reinterpret_cast<uintptr_t>(basePtr) + offset;
    
    // Use centralized address validation from SafeAccess
    if (address == 0xFFFFFFFFFFFFFFFF || 
        address < SafeAccess::MIN_VALID_MEMORY_ADDRESS || 
        address > SafeAccess::MAX_VALID_MEMORY_ADDRESS) {
        if (!context.empty()) {
            try {
                std::stringstream ss;
                ss << "SafeRead: Invalid address 0x" << std::hex << address 
                   << " (base: 0x" << std::hex << reinterpret_cast<uintptr_t>(basePtr) 
                   << " + offset: 0x" << std::hex << offset << ") in " << context;
                LOG_ERROR(ss.str());
            } catch (...) {
                // Ignore string formatting errors
            }
        }
        return false;
    }
    
    // Use centralized memory safety check
    if (!SafeAccess::IsMemorySafe(reinterpret_cast<void*>(address), sizeof(T))) {
        if (!context.empty()) {
            try {
                std::stringstream ss;
                ss << "SafeRead: Memory not readable at 0x" << std::hex << address 
                   << " (base: 0x" << std::hex << reinterpret_cast<uintptr_t>(basePtr) 
                   << " + offset: 0x" << std::hex << offset << ") in " << context;
                LOG_ERROR(ss.str());
            } catch (...) {
                // Ignore string formatting errors
            }
        }
        return false;
    }
    
    return SafeReadImpl(address, result);
}

} // namespace Debug
} // namespace kx
