#include "FeatureComposition.h"

#include "../Core/Architecture/FeatureManager.h"
#include "../Features/Visuals/VisualsRegistration.h"

namespace kx {
namespace {

void RegisterConfiguredFeatures(FeatureManager& manager) {
    RegisterVisualsFeatures(manager);
}

bool SyncConfiguredFeatures(FeatureManager& manager) {
    const auto beforeCount = manager.GetFeatureCount();
    RegisterConfiguredFeatures(manager);

    return manager.GetFeatureCount() != beforeCount;
}

} // namespace

AppComposition CreateAppComposition() {
    return {
        {
            &RegisterConfiguredFeatures,
            &SyncConfiguredFeatures,
        },
        std::make_unique<IRuntimeExtension>(),
    };
}

} // namespace kx
