#include "VisualsRegistration.h"

#include "VisualsFeature.h"
#include "../../Core/Architecture/FeatureManager.h"

#include <memory>

namespace kx {

void RegisterVisualsFeatures(FeatureManager& manager) {
    manager.RegisterFeature(std::make_unique<VisualsFeature>());
}

} // namespace kx
