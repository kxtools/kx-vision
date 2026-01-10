#pragma once

#include <atomic>
#include <mutex>
#include <memory>
#include "Settings.h"
#include "AdaptiveFarPlaneCalculator.h"

namespace kx {

    // Forward declarations
    struct PooledFrameRenderData;

    // --- Status Information ---
    enum class HookStatus {
        Unknown,
        OK,
        Failed
    };

    /**
     * @brief Central application state manager using the singleton pattern.
     * 
     * This class encapsulates all global application state including settings,
     * hook status, and shutdown synchronization. It provides thread-safe access
     * to the application state and reduces global namespace pollution.
     */
    class AppState {
    public:
        // Get the singleton instance
        static AppState& Get();

        // Deleted copy constructor and assignment operator for singleton
        AppState(const AppState&) = delete;
        AppState& operator=(const AppState&) = delete;

        // --- Settings Access ---
        Settings& GetSettings() { return m_settings; }
        const Settings& GetSettings() const { return m_settings; }

        // --- Hook Status Management ---
        HookStatus GetPresentHookStatus() const { return m_presentHookStatus; }
        void SetPresentHookStatus(HookStatus status) { m_presentHookStatus = status; }

        // --- Vision Window State ---
        bool IsVisionWindowOpen() const { return m_isVisionWindowOpen; }
        void SetVisionWindowOpen(bool open) { m_isVisionWindowOpen = open; }
        bool* GetVisionWindowOpenRef() { return &m_isVisionWindowOpen; }

        // --- Shutdown Synchronization ---
        bool IsShuttingDown() const { return m_isShuttingDown.load(std::memory_order_acquire); }
        void SetShuttingDown(bool shutting_down) { m_isShuttingDown.store(shutting_down, std::memory_order_release); }
        
        // --- Donation Prompt State ---
        bool IsDonationPromptShown() const { return m_donationPromptShown; }
        void SetDonationPromptShown(bool shown) { m_donationPromptShown = shown; }

        // --- Debug Logging Helper ---
        bool IsDebugLoggingEnabled() const { return m_settings.enableDebugLogging; }

        // --- Adaptive Far Plane (for "No Limit" mode) ---
        float GetAdaptiveFarPlane() const { return m_adaptiveFarPlaneCalculator.GetCurrentFarPlane(); }
        void UpdateAdaptiveFarPlane(const PooledFrameRenderData& frameData) {
            m_adaptiveFarPlaneCalculator.UpdateAndGetFarPlane(frameData);
        }

        // Private constructor for singleton
        AppState();
        ~AppState() = default;

    private:
        // Application state members
        Settings m_settings;
        HookStatus m_presentHookStatus = HookStatus::Unknown;
        #ifdef _DEBUG
        bool m_isVisionWindowOpen = true;   // Debug: Show GUI by default
        #else
        bool m_isVisionWindowOpen = false;  // Release: Hide GUI by default (press INSERT to toggle)
        #endif
        std::atomic<bool> m_isShuttingDown = false;
        bool m_donationPromptShown = false; // Session-only flag (not persisted)

        // Adaptive far plane calculator
        AdaptiveFarPlaneCalculator m_adaptiveFarPlaneCalculator;

        // Mutex for thread-safe access (if needed for future extensions)
        mutable std::mutex m_mutex;
    };

} // namespace kx
