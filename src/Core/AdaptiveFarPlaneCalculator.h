#pragma once

#include <vector>
#include <chrono>

namespace kx {

    // Forward declarations
    struct PooledFrameRenderData;

    /**
     * @brief Handles adaptive far plane calculation for "No Limit" mode
     * 
     * This class encapsulates the logic for calculating an adaptive far plane
     * based on gadget distances. It uses statistical analysis to determine
     * the optimal far plane distance for rendering.
     */
    class AdaptiveFarPlaneCalculator {
    public:
        AdaptiveFarPlaneCalculator();
        
        // Main calculation method
        float UpdateAndGetFarPlane(const PooledFrameRenderData& frameData);
        
        // Get current far plane value
        float GetCurrentFarPlane() const { return m_currentFarPlane; }
        
        // Reset to default value
        void Reset();

    private:
        // Helper methods
        bool ShouldRecalculate() const;
        std::vector<float> CollectGadgetDistances(const PooledFrameRenderData& frameData);
        float CalculateTargetFarPlane(const std::vector<float>& distances);
        float CalculateFarPlaneFromFewSamples(const std::vector<float>& distances);
        float CalculateFarPlaneFromPercentile(std::vector<float>& distances); // Non-const for nth_element
        void LogFarPlaneUpdate(size_t entityCount, float targetFarPlane, float oldFarPlane);

        // State
        float m_currentFarPlane;
        std::chrono::steady_clock::time_point m_lastRecalc;
    };

} // namespace kx
