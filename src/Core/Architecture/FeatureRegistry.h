#pragma once

namespace kx {

class FeatureManager;

/**
 * @brief Register all features with the FeatureManager.
 * 
 * This function acts as the central registration point for all features.
 * Add new feature registrations here.
 * 
 * @param manager The FeatureManager to register features with.
 */
void RegisterFeatures(FeatureManager& manager);

} // namespace kx
