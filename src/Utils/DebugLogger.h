#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <Windows.h>

// Forward declaration
namespace kx {
    bool IsDebugLoggingEnabled();
}

namespace kx {
namespace Debug {

class Logger {
public:
    enum Level {
        INFO,
        WARNING,
        ERR,
        CRITICAL
    };

    static void Log(Level level, const std::string& message) {
        // Check if debug logging is enabled
        if (!IsDebugLoggingEnabled()) return;
        
        // Skip INFO messages even when debug logging is enabled (too verbose)
        if (level == INFO) return;
        
        std::string levelStr;
        switch (level) {
            case INFO: levelStr = "[INFO]"; break;
            case WARNING: levelStr = "[WARN]"; break;
            case ERR: levelStr = "[ERROR]"; break;
            case CRITICAL: levelStr = "[CRITICAL]"; break;
        }
        
        std::string logMsg = levelStr + " " + message;
        
        // Log to console
        std::cout << logMsg << std::endl;
        
        // Log to file (optional)
        static std::ofstream logFile("kx_debug.log", std::ios::app);
        if (logFile.is_open()) {
            logFile << logMsg << std::endl;
            logFile.flush();
        }
    }

    static void LogPointer(const std::string& name, void* ptr) {
        std::stringstream ss;
        ss << name << " = 0x" << std::hex << reinterpret_cast<uintptr_t>(ptr);
        Log(INFO, ss.str());
    }

    static void LogMemoryAccess(const std::string& className, const std::string& method, void* ptr, uintptr_t offset) {
        std::stringstream ss;
        ss << className << "::" << method << " accessing 0x" << std::hex << reinterpret_cast<uintptr_t>(ptr) 
           << " + 0x" << offset << " = 0x" << (reinterpret_cast<uintptr_t>(ptr) + offset);
        Log(INFO, ss.str());
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
#define LOG_INFO(msg) kx::Debug::Logger::Log(kx::Debug::Logger::INFO, msg)
#define LOG_WARN(msg) kx::Debug::Logger::Log(kx::Debug::Logger::WARNING, msg)
#define LOG_ERROR(msg) kx::Debug::Logger::Log(kx::Debug::Logger::ERR, msg)
#define LOG_CRITICAL(msg) kx::Debug::Logger::Log(kx::Debug::Logger::CRITICAL, msg)
#define LOG_PTR(name, ptr) kx::Debug::Logger::LogPointer(name, ptr)
#define LOG_MEMORY(cls, method, ptr, offset) kx::Debug::Logger::LogMemoryAccess(cls, method, ptr, offset)
#define LOG_EXCEPTION(cls, method, details) kx::Debug::Logger::LogException(cls, method, details)

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

// Check if memory address is readable
static bool IsMemoryReadable(void* ptr, size_t size = sizeof(void*)) {
    if (!ptr) return false;
    
    uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
    
    // Quick validation for obviously invalid addresses
    // Valid user-mode addresses on x64 are typically < 0x7FFFFFFFFFFF
    if (address < 0x1000 || address > 0x7FFFFFFFFFFF) {
        return false;
    }
    
    MEMORY_BASIC_INFORMATION mbi = {};
    SIZE_T result = VirtualQuery(ptr, &mbi, sizeof(mbi));
    
    if (result == 0) return false;
    
    // Check if memory is committed and readable
    if (mbi.State != MEM_COMMIT) return false;
    if (!(mbi.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE))) return false;
    if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) return false;
    
    return true;
}

// Safe memory access helper (silent version for frequent operations)
template<typename T>
bool SafeRead(void* basePtr, uintptr_t offset, T& result) {
    if (!basePtr) {
        return false; // Don't log null pointers, they're common
    }
    
    uintptr_t address = reinterpret_cast<uintptr_t>(basePtr) + offset;
    
    // Check for obviously invalid addresses
    if (address == 0xFFFFFFFFFFFFFFFF || address < 0x1000 || address > 0x7FFFFFFFFFFF) {
        return false; // Don't log obviously invalid addresses
    }
    
    // Check if the memory is actually readable
    if (!IsMemoryReadable(reinterpret_cast<void*>(address), sizeof(T))) {
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
    
    // Check for obviously invalid addresses
    if (address == 0xFFFFFFFFFFFFFFFF || address < 0x1000 || address > 0x7FFFFFFFFFFF) {
        if (!context.empty()) {
            std::string errorMsg = "SafeRead: Invalid address 0x" + std::to_string(address) + 
                                  " (base: 0x" + std::to_string(reinterpret_cast<uintptr_t>(basePtr)) + 
                                  " + offset: 0x" + std::to_string(offset) + ") in " + context;
            LOG_ERROR(errorMsg);
        }
        return false;
    }
    
    // Check if the memory is actually readable
    if (!IsMemoryReadable(reinterpret_cast<void*>(address), sizeof(T))) {
        if (!context.empty()) {
            std::string errorMsg = "SafeRead: Memory not readable at 0x" + std::to_string(address) + 
                                  " (base: 0x" + std::to_string(reinterpret_cast<uintptr_t>(basePtr)) + 
                                  " + offset: 0x" + std::to_string(offset) + ") in " + context;
            LOG_ERROR(errorMsg);
        }
        return false;
    }
    
    // Use a separate function for the __try block to avoid object unwinding issues
    return SafeReadImpl(address, result);
}

} // namespace Debug
} // namespace kx
