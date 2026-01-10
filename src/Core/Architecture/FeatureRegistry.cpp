#include "FeatureRegistry.h"
#include "FeatureManager.h"
#include "../../Features/Visuals/VisualsFeature.h"

namespace kx {

void RegisterFeatures(FeatureManager& manager) {
    // Register the core visuals/ESP feature
    manager.RegisterFeature(std::make_unique<VisualsFeature>());

    // Register additional features here
}

} // namespace kx
