#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <chrono>
#include <Windows.h>
#include "../Core/AppState.h"
#include "MemorySafety.h"

namespace kx {
namespace Debug {

class Logger {
public:
    enum Level {
        DEBUG,      // Most verbose - detailed debugging info
        INFO,       // General information
        WARNING,    // Warnings
        ERR,        // Errors
        CRITICAL    // Critical errors
    };

private:
    // Default minimum log level (only show errors and critical by default)
    static Level s_minLogLevel;
    
    // Rate limiting for repetitive messages
    static std::unordered_map<std::string, std::chrono::steady_clock::time_point> s_lastLogTime;
    static constexpr std::chrono::milliseconds RATE_LIMIT_INTERVAL{1000}; // 1 second between identical messages

public:
    // Set the minimum log level
    static void SetMinLogLevel(Level level) {
        s_minLogLevel = level;
    }

    // Get the current minimum log level
    static Level GetMinLogLevel() {
        return s_minLogLevel;
    }

    // Convenience methods for setting common log levels
    static void SetLogLevelDebug() { SetMinLogLevel(DEBUG); }    // Show all logs
    static void SetLogLevelInfo() { SetMinLogLevel(INFO); }      // Show info and above
    static void SetLogLevelWarning() { SetMinLogLevel(WARNING); } // Show warnings and above
    static void SetLogLevelError() { SetMinLogLevel(ERR); }      // Show only errors and critical (default)
    static void SetLogLevelCritical() { SetMinLogLevel(CRITICAL); } // Show only critical

    static void Log(Level level, const std::string& message) {
        // Check if debug logging is enabled and level meets minimum threshold
        if (!kx::AppState::Get().IsDebugLoggingEnabled() || level < s_minLogLevel) return;
        
        // Rate limiting for repetitive messages
        auto now = std::chrono::steady_clock::now();
        auto it = s_lastLogTime.find(message);
        if (it != s_lastLogTime.end()) {
            if (now - it->second < RATE_LIMIT_INTERVAL) {
                return; // Skip this message due to rate limiting
            }
        }
        s_lastLogTime[message] = now;
        
        std::string levelStr;
        switch (level) {
            case DEBUG: levelStr = "[DEBUG]"; break;
            case INFO: levelStr = "[INFO]"; break;
            case WARNING: levelStr = "[WARN]"; break;
            case ERR: levelStr = "[ERROR]"; break;
            case CRITICAL: levelStr = "[CRITICAL]"; break;
        }
        
        std::string logMsg = levelStr + " " + message;
        
        // Log to console
        std::cout << logMsg << std::endl;
        
        // Log to file with thread safety
        try {
            std::ofstream logFile("kx_debug.log", std::ios::app);
            if (logFile.is_open()) {
                logFile << logMsg << std::endl;
                logFile.flush();
                logFile.close();
            }
        } catch (...) {
            // Ignore file logging errors to prevent crashes
        }
    }

    static void LogPointer(const std::string& name, void* ptr) {
        std::stringstream ss;
        ss << name << " = 0x" << std::hex << reinterpret_cast<uintptr_t>(ptr);
        Log(WARNING, ss.str()); // Changed from INFO to WARNING so it's visible
    }

    static void LogMemoryAccess(const std::string& className, const std::string& method, void* ptr, uintptr_t offset) {
        std::stringstream ss;
        ss << className << "::" << method << " accessing 0x" << std::hex << reinterpret_cast<uintptr_t>(ptr) 
           << " + 0x" << offset << " = 0x" << (reinterpret_cast<uintptr_t>(ptr) + offset);
        Log(WARNING, ss.str()); // Changed from INFO to WARNING so it's visible
    }

    static void LogException(const std::string& className, const std::string& method, const std::string& details = "") {
        std::string msg = "EXCEPTION in " + className + "::" + method;
        if (!details.empty()) {
            msg += " - " + details;
        }
        Log(CRITICAL, msg);
    }
};

// Macros for easier usage
#define LOG_DEBUG(msg) kx::Debug::Logger::Log(kx::Debug::Logger::DEBUG, msg)
#define LOG_INFO(msg) kx::Debug::Logger::Log(kx::Debug::Logger::INFO, msg)
#define LOG_WARN(msg) kx::Debug::Logger::Log(kx::Debug::Logger::WARNING, msg)
#define LOG_ERROR(msg) kx::Debug::Logger::Log(kx::Debug::Logger::ERR, msg)
#define LOG_CRITICAL(msg) kx::Debug::Logger::Log(kx::Debug::Logger::CRITICAL, msg)
#define LOG_PTR(name, ptr) kx::Debug::Logger::LogPointer(name, ptr)
#define LOG_MEMORY(cls, method, ptr, offset) kx::Debug::Logger::LogMemoryAccess(cls, method, ptr, offset)
#define LOG_EXCEPTION(cls, method, details) kx::Debug::Logger::LogException(cls, method, details)

// Convenience macros for setting log levels
#define LOG_SET_LEVEL_DEBUG() kx::Debug::Logger::SetLogLevelDebug()
#define LOG_SET_LEVEL_INFO() kx::Debug::Logger::SetLogLevelInfo()
#define LOG_SET_LEVEL_WARNING() kx::Debug::Logger::SetLogLevelWarning()
#define LOG_SET_LEVEL_ERROR() kx::Debug::Logger::SetLogLevelError()
#define LOG_SET_LEVEL_CRITICAL() kx::Debug::Logger::SetLogLevelCritical()

// Implementation function without C++ objects that need unwinding
template<typename T>
static bool SafeReadImpl(uintptr_t address, T& result) {
    __try {
        result = *reinterpret_cast<T*>(address);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        // Can't use stringstream here due to object unwinding, use simple logging
        return false;
    }
}

// Safe memory access helper (silent version for frequent operations)
template<typename T>
bool SafeRead(void* basePtr, uintptr_t offset, T& result) {
    if (!basePtr) {
        return false; // Don't log null pointers, they're common
    }
    
    uintptr_t address = reinterpret_cast<uintptr_t>(basePtr) + offset;
    
    // Use centralized address validation from SafeAccess
    if (address == 0xFFFFFFFFFFFFFFFF || 
        address < SafeAccess::MIN_VALID_MEMORY_ADDRESS || 
        address > SafeAccess::MAX_VALID_MEMORY_ADDRESS) {
        return false; // Don't log obviously invalid addresses
    }
    
    // Use centralized memory safety check
    if (!SafeAccess::IsMemorySafe(reinterpret_cast<void*>(address), sizeof(T))) {
        return false; // Don't log unreadable memory, it's common in scanning
    }
    
    // Use a separate function for the __try block to avoid object unwinding issues
    return SafeReadImpl(address, result);
}

// Safe memory access helper with logging (for critical reads)
template<typename T>
bool SafeReadWithLogging(void* basePtr, uintptr_t offset, T& result, const std::string& context = "") {
    if (!basePtr) {
        if (!context.empty()) {
            LOG_ERROR("SafeRead: base pointer is null in " + context);
        }
        return false;
    }
    
    uintptr_t address = reinterpret_cast<uintptr_t>(basePtr) + offset;
    
    // Use centralized address validation from SafeAccess
    if (address == 0xFFFFFFFFFFFFFFFF || 
        address < SafeAccess::MIN_VALID_MEMORY_ADDRESS || 
        address > SafeAccess::MAX_VALID_MEMORY_ADDRESS) {
        if (!context.empty()) {
            std::stringstream ss;
            ss << "SafeRead: Invalid address 0x" << std::hex << address 
               << " (base: 0x" << std::hex << reinterpret_cast<uintptr_t>(basePtr) 
               << " + offset: 0x" << std::hex << offset << ") in " << context;
            LOG_ERROR(ss.str());
        }
        return false;
    }
    
    // Use centralized memory safety check
    if (!SafeAccess::IsMemorySafe(reinterpret_cast<void*>(address), sizeof(T))) {
        if (!context.empty()) {
            std::stringstream ss;
            ss << "SafeRead: Memory not readable at 0x" << std::hex << address 
               << " (base: 0x" << std::hex << reinterpret_cast<uintptr_t>(basePtr) 
               << " + offset: 0x" << std::hex << offset << ") in " << context;
            LOG_ERROR(ss.str());
        }
        return false;
    }
    
    // Use a separate function for the __try block to avoid object unwinding issues
    return SafeReadImpl(address, result);
}

} // namespace Debug
} // namespace kx
