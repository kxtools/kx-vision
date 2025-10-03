#include "AppState.h"
#include "../Rendering/Data/ESPData.h"
#include "../Rendering/Data/RenderableData.h"
#include <vector>
#include <algorithm>

namespace kx {

    AppState::AppState() {
        // Constructor initializes default values (already done in member initializer list)
        m_lastFarPlaneRecalc = std::chrono::steady_clock::now();
    }

    AppState& AppState::Get() {
        static AppState instance;
        return instance;
    }

    void AppState::UpdateAdaptiveFarPlane(const PooledFrameRenderData& frameData) {
        // Only recalculate once per second
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - m_lastFarPlaneRecalc).count() < 1) {
            return;
        }
        m_lastFarPlaneRecalc = now;

        // 1. Collect all relevant gameplay distances
        std::vector<float> distances;
        distances.reserve(frameData.players.size() + frameData.npcs.size()); // Pre-allocate memory
        
        for (const auto* p : frameData.players) {
            if (p) distances.push_back(p->gameplayDistance);
        }
        for (const auto* n : frameData.npcs) {
            if (n) distances.push_back(n->gameplayDistance);
        }

        // 2. Handle edge cases
        // If we have too few entities, the statistic is meaningless. Fall back to a safe default.
        if (distances.size() < 10) {
            m_adaptiveFarPlane = 1500.0f; // Default far plane
            return;
        }

        // 3. Find the 95th percentile. std::nth_element is faster than a full sort.
        size_t percentile_index = static_cast<size_t>(distances.size() * 0.95);
        std::nth_element(distances.begin(), distances.begin() + percentile_index, distances.end());
        float newFarPlane = distances[percentile_index];
        
        // Clamp the value to a reasonable range to prevent extreme values if everything is up close
        newFarPlane = (std::max)(newFarPlane, 400.0f); // Ensure the plane is at least 400m

        // 4. Smoothly interpolate to the new value to prevent visual "snapping"
        // This makes the transition invisible to the user if the range changes
        m_adaptiveFarPlane = m_adaptiveFarPlane + (newFarPlane - m_adaptiveFarPlane) * 0.5f; // LERP
    }

} // namespace kx