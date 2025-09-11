#include "AppState.h"

#include <atomic>

namespace kx {

	// --- Global Settings Instance ---
	Settings g_settings;

	// --- Status Information ---
	HookStatus g_presentHookStatus = HookStatus::Unknown;

	// --- App State ---
	bool g_isVisionWindowOpen = true;

	// --- Shutdown Synchronization ---
	std::atomic<bool> g_isShuttingDown = false;

	// --- Debug Logging Helper ---
	bool IsDebugLoggingEnabled() {
		return g_settings.enableDebugLogging;
	}

} // namespace kx