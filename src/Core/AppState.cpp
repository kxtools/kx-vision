#include "AppState.h"
#include <atomic>

namespace kx {

	// --- Status Information ---
	HookStatus g_presentHookStatus = HookStatus::Unknown;

	// --- UI State ---
	bool g_isVisionWindowOpen = true;
	bool g_showVisionWindow = true;
	bool g_espEnabled = true;       // ESP is enabled by default
	bool g_espRenderBox = true;     // ESP box is enabled by default
	bool g_espRenderDistance = true; // ESP distance is enabled by default
	bool g_espRenderDot = true;      // ESP mini dot is enabled by default

	// --- Shutdown Synchronization ---
	std::atomic<bool> g_isShuttingDown = false;

} // namespace kx