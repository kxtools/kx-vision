#include "AdaptiveFarPlaneCalculator.h"
#include "../Game/Data/FrameData.h"
#include "../Game/Data/RenderableData.h"
#include "../Utils/DebugLogger.h"
#include <vector>
#include <algorithm>

#include "Shared/ScalingConstants.h"

namespace kx {

    AdaptiveFarPlaneCalculator::AdaptiveFarPlaneCalculator() 
        : m_currentFarPlane(AdaptiveScaling::FAR_PLANE_INITIAL) // Default value on startup
        , m_lastRecalc(std::chrono::steady_clock::now()) {
    }

    float AdaptiveFarPlaneCalculator::UpdateAndGetFarPlane(const FrameGameData& frameData) {
        if (!ShouldRecalculate()) {
            return m_currentFarPlane;
        }
        
        m_lastRecalc = std::chrono::steady_clock::now();
        
        auto distances = CollectGadgetDistances(frameData);
        float targetFarPlane = CalculateTargetFarPlane(distances);
        float oldFarPlane = m_currentFarPlane;
        
        // Apply temporal smoothing to prevent jarring visual changes when scene depth fluctuates
        m_currentFarPlane = m_currentFarPlane + (targetFarPlane - m_currentFarPlane) * AdaptiveScaling::SMOOTHING_FACTOR;
        
        LogFarPlaneUpdate(distances.size(), targetFarPlane, oldFarPlane);
        return m_currentFarPlane;
    }

    void AdaptiveFarPlaneCalculator::Reset() {
        m_currentFarPlane = AdaptiveScaling::FAR_PLANE_INITIAL;
        m_lastRecalc = std::chrono::steady_clock::now();
    }

    bool AdaptiveFarPlaneCalculator::ShouldRecalculate() const {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::seconds>(now - m_lastRecalc).count() >= AdaptiveScaling::RECALC_INTERVAL_SECONDS;
    }

    std::vector<float> AdaptiveFarPlaneCalculator::CollectGadgetDistances(const FrameGameData& frameData) {
        // Collect distances from gadgets/objects only
        // Rationale: Players and NPCs are limited to ~200m by game mechanics,
        // but objects (waypoints, vistas, resource nodes) can be 1000m+ away.
        // Using only object distances gives us the true scene depth for intelligent scaling.
        std::vector<float> distances;
        distances.reserve(frameData.gadgets.size());
        
        for (const auto* g : frameData.gadgets) {
            if (g) distances.push_back(g->gameplayDistance);
        }
        
        return distances;
    }

    float AdaptiveFarPlaneCalculator::CalculateTargetFarPlane(const std::vector<float>& distances) {
        if (distances.empty()) {
            return AdaptiveScaling::FAR_PLANE_DEFAULT;
        }
        
        if (distances.size() < AdaptiveScaling::MIN_ENTITIES_FOR_PERCENTILE) {
            return CalculateFarPlaneFromFewSamples(distances);
        }
        
        // Create a mutable copy for percentile calculation
        std::vector<float> mutableDistances = distances;
        return CalculateFarPlaneFromPercentile(mutableDistances);
    }

    float AdaptiveFarPlaneCalculator::CalculateFarPlaneFromFewSamples(const std::vector<float>& distances) {
        // Calculate average distance from the few objects we have
        float sum = 0.0f;
        for (float d : distances) {
            sum += d;
        }
        float avgDistance = sum / distances.size();
        
        // Clamp the result to reasonable bounds
        if (avgDistance < AdaptiveScaling::FAR_PLANE_MIN) return AdaptiveScaling::FAR_PLANE_MIN;
        if (avgDistance > AdaptiveScaling::FAR_PLANE_MAX) return AdaptiveScaling::FAR_PLANE_MAX;
        return avgDistance;
    }

    float AdaptiveFarPlaneCalculator::CalculateFarPlaneFromPercentile(std::vector<float>& distances) {
        // Find the 95th percentile. std::nth_element is faster than a full sort.
        size_t percentile_index = static_cast<size_t>(distances.size() * AdaptiveScaling::PERCENTILE_THRESHOLD);
        std::nth_element(distances.begin(), distances.begin() + percentile_index, distances.end());
        float percentileFarPlane = distances[percentile_index];
        
        // Clamp the value to a reasonable range to prevent extreme outliers
        if (percentileFarPlane < AdaptiveScaling::FAR_PLANE_MIN) return AdaptiveScaling::FAR_PLANE_MIN;
        if (percentileFarPlane > AdaptiveScaling::FAR_PLANE_MAX) return AdaptiveScaling::FAR_PLANE_MAX;
        return percentileFarPlane;
    }

    void AdaptiveFarPlaneCalculator::LogFarPlaneUpdate(size_t entityCount, float targetFarPlane, float oldFarPlane) {
        if (entityCount < AdaptiveScaling::MIN_ENTITIES_FOR_PERCENTILE) {
            LOG_DEBUG("[AdaptiveFarPlane] Few objects (%zu), using average: %.1fm (was %.1fm)", 
                      entityCount, m_currentFarPlane, oldFarPlane);
        } else {
            LOG_DEBUG("[AdaptiveFarPlane] Entities: %zu | %.0fth percentile: %.1fm | Smoothed: %.1fm (was %.1fm)", 
                      entityCount, AdaptiveScaling::PERCENTILE_THRESHOLD * 100.0f, targetFarPlane, m_currentFarPlane, oldFarPlane);
        }
    }

} // namespace kx
