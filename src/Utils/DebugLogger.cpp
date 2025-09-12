#include "DebugLogger.h"
#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>

namespace kx {
namespace Debug {

// Initialize static atomic members with thread-safe default values
std::atomic<Logger::Level> Logger::s_minLogLevel{Logger::ERR};
std::atomic<size_t> Logger::s_rateLimitCacheSize{0};

// Initialize rate limiting infrastructure with proper thread safety
std::unordered_map<std::string, std::chrono::steady_clock::time_point> Logger::s_lastLogTime;
std::mutex Logger::s_rateLimitMutex;
std::mutex Logger::s_fileMutex;
std::chrono::steady_clock::time_point Logger::s_lastCleanup{std::chrono::steady_clock::now()};

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
        if (localtime_s(&timeinfo, &time_t) != 0) {
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
 */
void Logger::CleanupRateLimitCache() noexcept {
    try {
        std::lock_guard<std::mutex> lock(s_rateLimitMutex);
        
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
        
        s_rateLimitCacheSize.store(s_lastLogTime.size());
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
bool Logger::ShouldRateLimit(const std::string& key, std::chrono::milliseconds interval) noexcept {
    try {
        std::lock_guard<std::mutex> lock(s_rateLimitMutex);
        
        auto now = std::chrono::steady_clock::now();
        auto it = s_lastLogTime.find(key);
        
        if (it == s_lastLogTime.end() || (now - it->second) >= interval) {
            s_lastLogTime[key] = now;
            s_rateLimitCacheSize.store(s_lastLogTime.size());
            
            // Trigger cleanup if cache is getting too large
            if (s_rateLimitCacheSize.load() > MAX_RATE_LIMIT_ENTRIES ||
                (now - s_lastCleanup) > RATE_LIMIT_CLEANUP_INTERVAL) {
                CleanupRateLimitCache();
            }
            
            return false; // Not rate limited
        }
        
        return true; // Rate limited
    }
    catch (...) {
        return false; // On error, allow logging
    }
}

/**
 * @brief Check if a message should be logged based on rate limiting
 * @param message The message to potentially log
 * @return true if message should be logged, false if rate limited
 */
bool Logger::ShouldLogMessage(const std::string& message) noexcept {
    try {
        // Create a simple hash-like key from the message for rate limiting
        std::hash<std::string> hasher;
        auto messageHash = hasher(message);
        std::string key = "msg_" + std::to_string(messageHash);
        
        // Use default rate limit interval
        return !ShouldRateLimit(key, RATE_LIMIT_INTERVAL);
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
        
        // Thread-safe console output
        {
            std::lock_guard<std::mutex> lock(s_fileMutex);
            
            // Use appropriate output stream based on log level
            if (level >= ERR) {
                std::cerr << logLine << std::endl;
                std::cerr.flush();
            } else {
                std::cout << logLine << std::endl;
                std::cout.flush();
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
    LogImpl(level, message, "");
}

/**
 * @brief Log pointer information for debugging
 * @param name Name/description of the pointer
 * @param ptr Pointer to log
 */
void Logger::LogPointer(const std::string& name, const void* ptr) noexcept {
    try {
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