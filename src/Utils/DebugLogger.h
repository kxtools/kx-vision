#pragma once

#include <string>
#include <memory>
#include <atomic>
#include <Windows.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include "../Core/AppState.h"
#include "MemorySafety.h"

namespace kx {
namespace Debug {

/**
 * @brief spdlog-based logging system with configurable levels and sinks
 * 
 * Features:
 * - Thread-safe operation (handled by spdlog)
 * - Console output with colors (Debug builds)
 * - Rotating file output for persistent logs
 * - Configurable log levels
 * - Exception-safe operations
 * - Async logging support for better performance
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
    // spdlog logger instance
    static std::shared_ptr<spdlog::logger> s_logger;
    static std::atomic<Level> s_minLogLevel;
    
    // Helper methods
    static std::string GetLogFilePath() noexcept;
    static spdlog::level::level_enum ConvertLevel(Level level) noexcept;

public:
    // Initialize logger (call once at startup for guaranteed proper initialization)
    static void Initialize() noexcept;
    
    // Reinitialize logger (call after console setup to enable console output)
    static void Reinitialize() noexcept;
    
    // Cleanup logger (call at shutdown to properly close files)
    static void Cleanup() noexcept;
    
    // Thread-safe log level management
    static void SetMinLogLevel(Level level) noexcept;
    static Level GetMinLogLevel() noexcept;

    // Convenience methods for setting common log levels
    static void SetLogLevelDebug() noexcept { SetMinLogLevel(DEBUG); }
    static void SetLogLevelInfo() noexcept { SetMinLogLevel(INFO); }
    static void SetLogLevelWarning() noexcept { SetMinLogLevel(WARNING); }
    static void SetLogLevelError() noexcept { SetMinLogLevel(ERR); }
    static void SetLogLevelCritical() noexcept { SetMinLogLevel(CRITICAL); }
    
    // Get the underlying spdlog logger for direct access
    static std::shared_ptr<spdlog::logger> GetLogger() noexcept { return s_logger; }
    
    // Helper function to check if a level should be logged (used by macros)
    static bool ShouldLog(Level level) noexcept;
    
    // Debug function to verify current log level (useful for troubleshooting)
    static void PrintCurrentLogLevel() noexcept;
    
    // Disable all logging by setting an impossibly high log level
    static void DisableLogging() noexcept { 
        SetMinLogLevel(static_cast<Level>(999)); 
        if (s_logger) s_logger->set_level(spdlog::level::off);
    }

    // Main logging function - thread-safe and exception-safe
    static void Log(Level level, const std::string& message) noexcept;
    
    // Helper function to format printf-style strings
    template<typename... Args>
    static std::string FormatPrintf(const char* format, Args&&... args) noexcept {
        try {
            char buffer[1024];
            int result = snprintf(buffer, sizeof(buffer), format, std::forward<Args>(args)...);
            
            if (result > 0 && result < static_cast<int>(sizeof(buffer))) {
                return std::string(buffer);
            } else {
                // Fallback for formatting errors
                return std::string("LOG FORMAT ERROR: ") + format;
            }
        }
        catch (...) {
            return std::string("LOG FORMAT ERROR");
        }
    }
    
    // Formatted logging functions (printf-style) - accepts const char*
    template<typename... Args>
    static void LogFormatted(Level level, const char* format, Args&&... args) noexcept {
        try {
            if (!s_logger) return;
            
            // Quick level check first
            if (level < s_minLogLevel.load(std::memory_order_acquire)) {
                return;
            }
            
            if constexpr (sizeof...(args) == 0) {
                // No additional arguments, treat as simple string
                s_logger->log(ConvertLevel(level), format);
            } else {
                // Format the message with arguments using printf
                std::string formatted = FormatPrintf(format, std::forward<Args>(args)...);
                s_logger->log(ConvertLevel(level), formatted);
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
            if (!s_logger) return;
            
            // Quick level check first
            if (level < s_minLogLevel.load(std::memory_order_acquire)) {
                return;
            }
            
            if constexpr (sizeof...(args) == 0) {
                // No additional arguments, just log the string directly
                s_logger->log(ConvertLevel(level), format);
            } else {
                // Format the message with arguments using printf
                std::string formatted = FormatPrintf(format.c_str(), std::forward<Args>(args)...);
                s_logger->log(ConvertLevel(level), formatted);
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
// Support both string and printf-style formatting using custom printf formatting
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

// Convenience macro to reinitialize logger (after console setup)
#define LOG_REINIT() kx::Debug::Logger::Reinitialize()

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
 * @brief Safe memory read with offset calculation
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
 * @brief Safe memory read with offset calculation (const overload)
 * @tparam T Type to read from memory
 * @param basePtr Const base pointer to read from
 * @param offset Offset from base pointer
 * @param result Reference to store the result
 * @return true if read was successful, false otherwise
 */
template<typename T>
inline bool SafeRead(const void* basePtr, uintptr_t offset, T& result) noexcept {
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

