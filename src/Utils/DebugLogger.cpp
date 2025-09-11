#include "DebugLogger.h"

namespace kx {
namespace Debug {

// Initialize the default minimum log level to ERROR (only show errors and critical by default)
Logger::Level Logger::s_minLogLevel = Logger::ERR;

// Initialize the rate limiting map
std::unordered_map<std::string, std::chrono::steady_clock::time_point> Logger::s_lastLogTime;

} // namespace Debug
} // namespace kx