#include "AppState.h"
#include "../Rendering/Data/ESPData.h"
#include "../Rendering/Data/RenderableData.h"
#include "../Rendering/Utils/ESPConstants.h"
#include "../Utils/DebugLogger.h"
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

        // 1. Collect distances from gadgets/objects only
        // Rationale: Players and NPCs are limited to ~200m by game mechanics,
        // but objects (waypoints, vistas, resource nodes) can be 1000m+ away.
        // Using only object distances gives us the true scene depth for intelligent scaling.
        std::vector<float> distances;
        distances.reserve(frameData.gadgets.size()); // Pre-allocate memory
        
        for (const auto* g : frameData.gadgets) {
            if (g) distances.push_back(g->gameplayDistance);
        }

        // 2. Handle edge cases - too few objects for reliable percentile statistics
        if (distances.size() < AdaptiveScaling::MIN_ENTITIES_FOR_PERCENTILE) {
            // Use average distance of available objects with reasonable bounds
            if (distances.empty()) {
                m_adaptiveFarPlane = AdaptiveScaling::FAR_PLANE_DEFAULT; // No objects - use conservative mid-range default
                return;
            }
            
            // Calculate average distance from the few objects we have
            float sum = 0.0f;
            for (float d : distances) {
                sum += d;
            }
            float avgDistance = sum / distances.size();
            
            // Clamp and smooth the result
            float targetFarPlane = (std::clamp)(avgDistance, AdaptiveScaling::FAR_PLANE_MIN, AdaptiveScaling::FAR_PLANE_MAX);
            float oldFarPlane = m_adaptiveFarPlane;
            m_adaptiveFarPlane = m_adaptiveFarPlane + (targetFarPlane - m_adaptiveFarPlane) * 0.5f; // LERP
            
            LOG_DEBUG("[AdaptiveFarPlane] Few objects (%zu), using average: %.1fm (was %.1fm)", 
                      distances.size(), m_adaptiveFarPlane, oldFarPlane);
            return;
        }

        // 3. Find the 95th percentile. std::nth_element is faster than a full sort.
        size_t percentile_index = static_cast<size_t>(distances.size() * 0.95);
        std::nth_element(distances.begin(), distances.begin() + percentile_index, distances.end());
        float newFarPlane = distances[percentile_index];
        
        // Clamp the value to a reasonable range to prevent extreme outliers
        newFarPlane = (std::clamp)(newFarPlane, AdaptiveScaling::FAR_PLANE_MIN, AdaptiveScaling::FAR_PLANE_MAX);

        // 4. Smoothly interpolate to the new value to prevent visual "snapping"
        // This makes the transition invisible to the user if the range changes
        float oldFarPlane = m_adaptiveFarPlane;
        m_adaptiveFarPlane = m_adaptiveFarPlane + (newFarPlane - m_adaptiveFarPlane) * 0.5f; // LERP
        
        // Log the adaptive far plane calculation for debugging (only visible when debug logging enabled)
        LOG_DEBUG("[AdaptiveFarPlane] Entities: %zu | 95th percentile: %.1fm | Smoothed: %.1fm (was %.1fm)", 
                  distances.size(), newFarPlane, m_adaptiveFarPlane, oldFarPlane);
    }

} // namespace kx