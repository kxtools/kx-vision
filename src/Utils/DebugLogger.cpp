#include "DebugLogger.h"
#include "../Core/Settings.h"
#include "Config.h"
#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>

namespace kx {
namespace Debug {

// Initialize static atomic members with thread-safe default values
std::atomic<Logger::Level> Logger::s_minLogLevel{static_cast<Level>(AppConfig::DEFAULT_LOG_LEVEL)};
std::atomic<size_t> Logger::s_rateLimitCacheSize{0};

// Initialize rate limiting infrastructure with proper thread safety
std::unordered_map<std::string, std::chrono::steady_clock::time_point> Logger::s_lastLogTime;
std::mutex Logger::s_rateLimitMutex;
std::mutex Logger::s_fileMutex;
std::chrono::steady_clock::time_point Logger::s_lastCleanup = std::chrono::steady_clock::now();
std::unique_ptr<std::ofstream> Logger::s_logFile = nullptr;
std::string Logger::s_logFilePath = "";

/**
 * @brief Get the path for the log file
 * @return Path to the log file in the executable directory
 */
std::string Logger::GetLogFilePath() noexcept {
    try {
        char exePath[MAX_PATH];
        DWORD pathLen = GetModuleFileNameA(NULL, exePath, MAX_PATH);
        if (pathLen == 0 || pathLen >= MAX_PATH) {
            return "kx-vision_debug.log"; // Fallback to current directory
        }
        
        // Find the last backslash to get directory
        std::string executablePath(exePath);
        size_t lastSlash = executablePath.find_last_of("\\/");
        if (lastSlash != std::string::npos) {
            return executablePath.substr(0, lastSlash + 1) + "kx-vision_debug.log";
        }
        
        return "kx-vision_debug.log"; // Fallback
    }
    catch (...) {
        return "kx-vision_debug.log"; // Fallback
    }
}

/**
 * @brief Initialize the log file for output
 * @return true if file was successfully opened, false otherwise
 */
bool Logger::InitializeLogFile() noexcept {
    try {
        std::lock_guard<std::mutex> lock(s_fileMutex);
        
        // Close existing file if open
        if (s_logFile && s_logFile->is_open()) {
            s_logFile->close();
        }
        
        // Get log file path
        s_logFilePath = GetLogFilePath();
        
        // Create new file stream
        s_logFile = std::make_unique<std::ofstream>(s_logFilePath, std::ios::app);
        
        if (s_logFile && s_logFile->is_open()) {
            // Write startup message
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            struct tm timeinfo;
            if (localtime_s(&timeinfo, &time_t) == 0) {
                *s_logFile << "\n=== KX-Vision Debug Logger Started at " 
                          << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S") 
                          << " ===\n" << std::flush;
            }
            return true;
        }
        
        return false;
    }
    catch (...) {
        return false;
    }
}

/**
 * @brief Get current timestamp as formatted string for logging
 * @return Formatted timestamp string in HH:MM:SS format
 */
std::string Logger::GetTimestamp() noexcept {
    try {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        // Use safe localtime_s instead of deprecated localtime
        struct tm timeinfo;
        errno_t result = localtime_s(&timeinfo, &time_t);
        if (result != 0) {
            return "??:??:??.???"; // Return fallback on error
        }
        
        std::stringstream ss;
        ss << std::put_time(&timeinfo, "%H:%M:%S")
           << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }
    catch (...) {
        return "??:??:??.???"; // Fallback timestamp
    }
}

/**
 * @brief Clean up old entries from rate limiting cache to prevent memory growth
 * @note Called automatically when cache size exceeds threshold
 * @note Assumes caller already holds s_rateLimitMutex lock
 */
void Logger::CleanupRateLimitCache() noexcept {
    try {
        // Don't acquire lock here - caller must already hold it
        auto now = std::chrono::steady_clock::now();
        auto cleanupThreshold = now - RATE_LIMIT_CLEANUP_INTERVAL;
        
        auto it = s_lastLogTime.begin();
        while (it != s_lastLogTime.end()) {
            if (it->second < cleanupThreshold) {
                it = s_lastLogTime.erase(it);
            } else {
                ++it;
            }
        }
        
        s_rateLimitCacheSize.store(s_lastLogTime.size(), std::memory_order_release);
        s_lastCleanup = now;
    }
    catch (...) {
        // Ignore cleanup errors to prevent cascading failures
    }
}

/**
 * @brief Check if a message should be rate limited
 * @param key Unique key for rate limiting (typically function name + message hash)
 * @param interval Minimum interval between messages with the same key
 * @return true if message should be logged, false if rate limited
 */
bool Logger::IsRateLimited(const std::string& key, std::chrono::milliseconds interval) noexcept {
    try {
        std::lock_guard<std::mutex> lock(s_rateLimitMutex);
        
        auto now = std::chrono::steady_clock::now();
        auto it = s_lastLogTime.find(key);
        
        // Check if we need cleanup before processing this request
        bool needsCleanup = false;
        if (s_rateLimitCacheSize.load(std::memory_order_acquire) > MAX_RATE_LIMIT_ENTRIES ||
            (now - s_lastCleanup) > RATE_LIMIT_CLEANUP_INTERVAL) {
            needsCleanup = true;
        }
        
        // Check rate limiting
        bool shouldLog = false;
        if (it == s_lastLogTime.end() || (now - it->second) >= interval) {
            s_lastLogTime[key] = now;
            s_rateLimitCacheSize.store(s_lastLogTime.size(), std::memory_order_release);
            shouldLog = true;
        }
        
        // Perform cleanup if needed (while still holding the lock)
        if (needsCleanup) {
            CleanupRateLimitCache();
        }
        
        return !shouldLog; // Return true if should NOT log (is rate limited)
    }
    catch (...) {
        return false; // On error, allow logging (not rate limited)
    }
}

/**
 * @brief Check if a message should be logged based on rate limiting
 * @param message The message to potentially log
 * @return true if message should be logged, false if rate limited
 */
bool Logger::ShouldLogMessage(const std::string& message) noexcept {
    try {
        // For memory access patterns and repetitive debug messages, 
        // create a simplified key by removing dynamic content
        std::string simplifiedMessage = message;
        
        // Remove hex addresses (0x followed by hex digits) to group similar memory access patterns
        size_t pos = 0;
        while ((pos = simplifiedMessage.find("0x", pos)) != std::string::npos) {
            size_t hexEnd = pos + 2;
            while (hexEnd < simplifiedMessage.length() && 
                   ((simplifiedMessage[hexEnd] >= '0' && simplifiedMessage[hexEnd] <= '9') ||
                    (simplifiedMessage[hexEnd] >= 'a' && simplifiedMessage[hexEnd] <= 'f') ||
                    (simplifiedMessage[hexEnd] >= 'A' && simplifiedMessage[hexEnd] <= 'F'))) {
                hexEnd++;
            }
            simplifiedMessage.replace(pos, hexEnd - pos, "0xXXX");
            pos += 5; // Length of "0xXXX"
        }
        
        // Remove floating point numbers to group similar value patterns
        pos = 0;
        while ((pos = simplifiedMessage.find_first_of("0123456789", pos)) != std::string::npos) {
            size_t numEnd = pos;
            bool hasDecimal = false;
            while (numEnd < simplifiedMessage.length() && 
                   (simplifiedMessage[numEnd] >= '0' && simplifiedMessage[numEnd] <= '9' || 
                    (simplifiedMessage[numEnd] == '.' && !hasDecimal))) {
                if (simplifiedMessage[numEnd] == '.') hasDecimal = true;
                numEnd++;
            }
            if (numEnd > pos + 1) { // Only replace if it's more than a single digit
                simplifiedMessage.replace(pos, numEnd - pos, "###");
                pos += 3; // Length of "###"
            } else {
                pos++;
            }
        }
        
        // Create rate limiting key from simplified message
        std::hash<std::string> hasher;
        auto messageHash = hasher(simplifiedMessage);
        std::string key = "pattern_" + std::to_string(messageHash);
        
        // Use aggressive rate limiting for high-frequency patterns
        std::chrono::milliseconds interval = AGGRESSIVE_RATE_LIMIT;
        
        // Identify common high-frequency patterns that need more aggressive limiting
        if (simplifiedMessage.find("accessing") != std::string::npos ||
            simplifiedMessage.find("GetCurrent") != std::string::npos ||
            simplifiedMessage.find("GetMax") != std::string::npos ||
            simplifiedMessage.find("GetLevel") != std::string::npos ||
            simplifiedMessage.find("GetHealth") != std::string::npos ||
            simplifiedMessage.find("ContextCollection") != std::string::npos) {
            interval = std::chrono::milliseconds(50); // Very aggressive for spam patterns
        }
        
        return !IsRateLimited(key, interval);
    }
    catch (...) {
        return true; // On error, allow logging
    }
}

/**
 * @brief Format log level as string with consistent padding
 * @param level Log level to format
 * @return Formatted level string
 */
std::string Logger::LevelToString(Level level) noexcept {
    switch (level) {
        case DEBUG:    return "[DEBUG]";
        case INFO:     return "[INFO ]";
        case WARNING:  return "[WARN ]";
        case ERR:      return "[ERROR]";
        case CRITICAL: return "[CRIT ]";
        default:       return "[?????]";
    }
}

/**
 * @brief Core logging implementation with thread safety and error handling
 * @param level Log level
 * @param message Message to log
 * @param context Optional context information
 */
void Logger::LogImpl(Level level, const std::string& message, const std::string& context) noexcept {
    try {
        // Quick atomic check for log level filtering with proper memory ordering
        if (level < s_minLogLevel.load(std::memory_order_acquire)) {
            return;
        }
        
        // Build log message
        std::stringstream ss;
        ss << GetTimestamp() << " " << LevelToString(level) << " ";
        
        if (!context.empty()) {
            ss << "[" << context << "] ";
        }
        
        ss << message;
        std::string logLine = ss.str();
        
        // Thread-safe console and file output
        {
            std::lock_guard<std::mutex> lock(s_fileMutex);
            
            // Console output - check if console is available
            HWND consoleWindow = GetConsoleWindow();
            if (consoleWindow != NULL) {
                if (level >= ERR) {
                    std::cerr << logLine << std::endl;
                    std::cerr.flush();
                } else {
                    std::cout << logLine << std::endl;
                    std::cout.flush();
                }
            } else {
                // Fallback to OutputDebugString when console not available
                OutputDebugStringA((logLine + "\n").c_str());
            }
            
            // File output
            if (s_logFile && s_logFile->is_open()) {
                *s_logFile << logLine << std::endl;
                s_logFile->flush(); // Ensure immediate write for debugging
            }
        }
    }
    catch (...) {
        // Logging should never throw - if we can't log, fail silently
        // Could potentially write to Windows Event Log here as last resort
    }
}

/**
 * @brief Main logging function - thread-safe and exception-safe
 * @param level Log level
 * @param message Message to log
 */
void Logger::Log(Level level, const std::string& message) noexcept {
    try {
        // Quick level check first
        if (level < s_minLogLevel.load(std::memory_order_acquire)) {
            return;
        }
        
        // Apply rate limiting for DEBUG and INFO messages to prevent spam
        if (level <= INFO && !ShouldLogMessage(message)) {
            return; // Rate limited - skip this message
        }
        
        LogImpl(level, message, "");
    }
    catch (...) {
        // If rate limiting fails, fall back to logging without rate limiting
        LogImpl(level, message, "");
    }
}

/**
 * @brief Log pointer information for debugging
 * @param name Name/description of the pointer
 * @param ptr Pointer to log
 */
void Logger::LogPointer(const std::string& name, const void* ptr) noexcept {
    try {
        // Create a simplified key for rate limiting pointer logs
        std::string key = "ptr_" + name;
        if (!IsRateLimited(key, std::chrono::milliseconds(500))) {
            return; // Rate limited
        }
        
        std::stringstream ss;
        ss << name << ": 0x" << std::hex << reinterpret_cast<uintptr_t>(ptr);
        LogImpl(DEBUG, ss.str(), "PTR");
    }
    catch (...) {
        // Ignore formatting errors
    }
}

/**
 * @brief Log memory access for debugging foreign class operations
 * @param className Name of the class performing the access
 * @param method Name of the method
 * @param ptr Base pointer
 * @param offset Offset being accessed
 */
void Logger::LogMemoryAccess(const std::string& className, const std::string& method,
                           const void* ptr, uintptr_t offset) noexcept {
    try {
        // Create a simplified key for rate limiting memory access logs
        std::string key = "mem_" + className + "::" + method + "_" + std::to_string(offset);
        if (!IsRateLimited(key, std::chrono::milliseconds(500))) {
            return; // Rate limited
        }
        
        std::stringstream ss;
        ss << className << "::" << method << " accessing 0x" << std::hex 
           << reinterpret_cast<uintptr_t>(ptr) << " + 0x" << std::hex << offset
           << " = 0x" << std::hex << (reinterpret_cast<uintptr_t>(ptr) + offset);
        LogImpl(DEBUG, ss.str(), "MEM");
    }
    catch (...) {
        // Ignore formatting errors
    }
}

/**
 * @brief Log exception information
 * @param className Name of the class where exception occurred
 * @param method Name of the method where exception occurred
 * @param details Optional details about the exception
 */
void Logger::LogException(const std::string& className, const std::string& method,
                        const std::string& details) noexcept {
    try {
        std::stringstream ss;
        ss << "Exception in " << className << "::" << method;
        if (!details.empty()) {
            ss << " - " << details;
        }
        LogImpl(ERR, ss.str(), "EXC");
    }
    catch (...) {
        // Ignore formatting errors
    }
}

} // namespace Debug
} // namespace kx