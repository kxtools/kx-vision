#include "DebugLogger.h"
#include "../Core/Settings.h"
#include "Config.h"
#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/null_sink.h>

namespace kx {
namespace Debug {

// Initialize static members
std::shared_ptr<spdlog::logger> Logger::s_logger = nullptr;
std::atomic<Logger::Level> Logger::s_minLogLevel{static_cast<Level>(AppConfig::DEFAULT_LOG_LEVEL)};
std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> Logger::s_ringbuffer_sink = nullptr;

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
 * @brief Convert our log level to spdlog level
 * @param level Our internal log level
 * @return Corresponding spdlog level
 */
spdlog::level::level_enum Logger::ConvertLevel(Level level) noexcept {
    switch (level) {
        case DEBUG:    return spdlog::level::debug;
        case INFO:     return spdlog::level::info;
        case WARNING:  return spdlog::level::warn;
        case ERR:      return spdlog::level::err;
        case CRITICAL: return spdlog::level::critical;
        default:       return spdlog::level::info;
    }
}

/**
 * @brief Initialize spdlog logger with console and file sinks
 */
void Logger::Initialize() noexcept {
    try {
        // Create sinks
        std::vector<spdlog::sink_ptr> sinks;
        
        // Ringbuffer sink for GUI log viewer (1000 messages)
        try {
            s_ringbuffer_sink = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(1000);
            s_ringbuffer_sink->set_level(spdlog::level::debug);
            s_ringbuffer_sink->set_pattern("[%H:%M:%S.%e] [%l] %v");
            sinks.push_back(s_ringbuffer_sink);
        }
        catch (...) {
            // Ringbuffer sink failed, continue without it
            s_ringbuffer_sink = nullptr;
        }
        
        // Console sink with colors (Debug builds only)
        // Check if console is available before creating console sink
#ifdef _DEBUG
        HWND consoleWindow = GetConsoleWindow();
        if (consoleWindow != NULL) {
            try {
                auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
                console_sink->set_level(spdlog::level::debug);
                console_sink->set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");
                sinks.push_back(console_sink);
            }
            catch (...) {
                // Console sink failed, continue without it
            }
        }
#endif
        
        // Rotating file sink
        try {
            auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                GetLogFilePath(), 1024 * 1024 * 5, 3); // 5MB per file, 3 files
            file_sink->set_level(spdlog::level::debug);
            file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
            sinks.push_back(file_sink);
        }
        catch (...) {
            // File sink failed, try basic file sink as fallback
            try {
                auto basic_file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(GetLogFilePath());
                basic_file_sink->set_level(spdlog::level::debug);
                basic_file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
                sinks.push_back(basic_file_sink);
            }
            catch (...) {
                // Both file sinks failed, continue without file logging
            }
        }
        
        // If no sinks were created, create a null sink to prevent crashes
        if (sinks.empty()) {
            auto null_sink = std::make_shared<spdlog::sinks::null_sink_mt>();
            sinks.push_back(null_sink);
        }
        
        // Create logger
        s_logger = std::make_shared<spdlog::logger>("kx-vision", sinks.begin(), sinks.end());
        s_logger->set_level(ConvertLevel(static_cast<Level>(AppConfig::DEFAULT_LOG_LEVEL)));
        s_logger->flush_on(spdlog::level::err); // Flush on errors
        
        // Register as default logger
        spdlog::register_logger(s_logger);
        spdlog::set_default_logger(s_logger);
        
        // Set periodic flush for crash safety
        spdlog::flush_every(std::chrono::seconds(2));
        
        // Log startup message
        s_logger->info("=== KX-Vision Debug Logger Started ===");
    }
    catch (...) {
        // If initialization fails, create a null logger
        s_logger = nullptr;
    }
}

/**
 * @brief Reinitialize logger (call after console setup to enable console output)
 */
void Logger::Reinitialize() noexcept {
    try {
        if (!s_logger) return;
        
        // Check if console is now available and rebuild logger if needed
#ifdef _DEBUG
        HWND consoleWindow = GetConsoleWindow();
        if (consoleWindow != NULL) {
            // Check if we already have a console sink
            bool hasConsoleSink = false;
            for (auto& sink : s_logger->sinks()) {
                if (dynamic_cast<spdlog::sinks::stdout_color_sink_mt*>(sink.get()) != nullptr) {
                    hasConsoleSink = true;
                    break;
                }
            }
            
            // Rebuild logger with console sink if we don't have one
            if (!hasConsoleSink) {
                try {
                    // Create new sinks list
                    std::vector<spdlog::sink_ptr> sinks;
                    
                    // Add console sink
                    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
                    console_sink->set_level(spdlog::level::debug);
                    console_sink->set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");
                    sinks.push_back(console_sink);
                    
                    // Copy existing file sinks and ringbuffer sink
                    for (auto& sink : s_logger->sinks()) {
                        if (dynamic_cast<spdlog::sinks::stdout_color_sink_mt*>(sink.get()) == nullptr) {
                            sinks.push_back(sink);
                        }
                    }
                    
                    // Create new logger with updated sinks
                    auto new_logger = std::make_shared<spdlog::logger>("kx-vision", sinks.begin(), sinks.end());
                    new_logger->set_level(s_logger->level());
                    new_logger->flush_on(spdlog::level::err);
                    
                    // Replace the logger atomically
                    s_logger = new_logger;
                    spdlog::set_default_logger(s_logger);
                    
                    s_logger->info("Console output enabled");
                }
                catch (...) {
                    // Console sink failed, continue without it
                }
            }
        }
#endif
    }
    catch (...) {
        // Ignore reinitialization errors
    }
}

/**
 * @brief Cleanup spdlog logger
 */
void Logger::Cleanup() noexcept {
    try {
        if (s_logger) {
            s_logger->info("=== KX-Vision Debug Logger Shutting Down ===");
            s_logger->flush();
            s_logger.reset();
        }
        spdlog::shutdown();
    }
    catch (...) {
        // Ignore cleanup errors
    }
}

/**
 * @brief Set minimum log level
 */
void Logger::SetMinLogLevel(Level level) noexcept {
    try {
        s_minLogLevel.store(level, std::memory_order_release);
        if (s_logger) {
            s_logger->set_level(ConvertLevel(level));
        }
    }
    catch (...) {
        // Ignore errors
    }
}

/**
 * @brief Get minimum log level
 */
Logger::Level Logger::GetMinLogLevel() noexcept {
    return s_minLogLevel.load(std::memory_order_acquire);
}

/**
 * @brief Check if a level should be logged (used by macros)
 */
bool Logger::ShouldLog(Level level) noexcept {
    try {
        // First check the internal log level
        if (level < s_minLogLevel.load(std::memory_order_acquire)) {
            return false;
        }
        
        // Then check the AppState debug logging setting
        // Only check for DEBUG level logs - errors should always be allowed
        if (level == DEBUG) {
            return AppState::Get().IsDebugLoggingEnabled();
        }
        
        // For non-debug levels (INFO, WARNING, ERROR, CRITICAL), always allow if level passes
        return true;
    }
    catch (...) {
        // If we can't check AppState for any reason, fall back to level-only check
        return level >= s_minLogLevel.load(std::memory_order_acquire);
    }
}

/**
 * @brief Debug function to verify current log level (useful for troubleshooting)
 */
void Logger::PrintCurrentLogLevel() noexcept {
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
        
        bool guiDebugEnabled = AppState::Get().IsDebugLoggingEnabled();
        
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

/**
 * @brief Main logging function - thread-safe and exception-safe
 */
void Logger::Log(Level level, const std::string& message) noexcept {
    try {
        if (!s_logger) return;
        
        // Quick level check first
        if (level < s_minLogLevel.load(std::memory_order_acquire)) {
            return;
        }
        
        // Log using spdlog
        s_logger->log(ConvertLevel(level), message);
    }
    catch (...) {
        // Ignore logging errors
    }
}


/**
 * @brief Log pointer information for debugging
 */
void Logger::LogPointer(const std::string& name, const void* ptr) noexcept {
    try {
        if (!s_logger) return;
        
        s_logger->debug("[PTR] {}: 0x{:x}", name, reinterpret_cast<uintptr_t>(ptr));
    }
    catch (...) {
        // Ignore formatting errors
    }
}

/**
 * @brief Log memory access for debugging foreign class operations
 */
void Logger::LogMemoryAccess(const std::string& className, const std::string& method,
                           const void* ptr, uintptr_t offset) noexcept {
    try {
        if (!s_logger) return;
        
        uintptr_t baseAddr = reinterpret_cast<uintptr_t>(ptr);
        uintptr_t finalAddr = baseAddr + offset;
        s_logger->debug("[MEM] {}::{} accessing 0x{:x} + 0x{:x} = 0x{:x}", 
                       className, method, baseAddr, offset, finalAddr);
    }
    catch (...) {
        // Ignore formatting errors
    }
}

/**
 * @brief Log exception information
 */
void Logger::LogException(const std::string& className, const std::string& method,
                        const std::string& details) noexcept {
    try {
        if (!s_logger) return;
        
        std::stringstream ss;
        ss << "Exception in " << className << "::" << method;
        if (!details.empty()) {
            ss << " - " << details;
        }
        s_logger->error("[EXC] {}", ss.str());
    }
    catch (...) {
        // Ignore formatting errors
    }
}

/**
 * @brief Get recent logs from ringbuffer for GUI display
 */
std::vector<std::string> Logger::GetRecentLogs(size_t limit) noexcept {
    try {
        if (!s_ringbuffer_sink) {
            return std::vector<std::string>();
        }
        
        return s_ringbuffer_sink->last_formatted(limit);
    }
    catch (...) {
        return std::vector<std::string>();
    }
}


} // namespace Debug
} // namespace kx