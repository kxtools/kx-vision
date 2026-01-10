#include "FeatureManager.h"
#include "IFeature.h"
#include "../../Game/Data/FrameData.h"
#include "../../Utils/DebugLogger.h"

namespace kx {

// Define constructor and destructor here where IFeature is complete
FeatureManager::FeatureManager() = default;
FeatureManager::~FeatureManager() = default;

void FeatureManager::RegisterFeature(std::unique_ptr<IFeature> feature) {
    if (feature) {
        LOG_INFO("Registering feature: %s", feature->GetName());
        m_features.push_back(std::move(feature));
    }
}

bool FeatureManager::InitializeAll() {
    LOG_INFO("Initializing %zu feature(s)...", m_features.size());
    
    for (auto& feature : m_features) {
        if (!feature->Initialize()) {
            LOG_ERROR("Failed to initialize feature: %s", feature->GetName());
            return false;
        }
        LOG_INFO("Initialized feature: %s", feature->GetName());
    }
    
    return true;
}

void FeatureManager::UpdateAll(float deltaTime, const FrameGameData& frameData) {
    for (auto& feature : m_features) {
        feature->Update(deltaTime, frameData);
    }
}

void FeatureManager::RenderAllDrawLists(ImDrawList* drawList) {
    if (!drawList) return;
    
    for (auto& feature : m_features) {
        feature->RenderDrawList(drawList);
    }
}

void FeatureManager::RenderAllMenus() {
    for (auto& feature : m_features) {
        feature->OnMenuRender();
    }
}

size_t FeatureManager::GetFeatureCount() const {
    return m_features.size();
}

bool FeatureManager::BroadcastInput(UINT message, WPARAM wParam, LPARAM lParam) {
    for (auto& feature : m_features) {
        if (feature->OnInput(message, wParam, lParam)) {
            return true; // Feature consumed the input
        }
    }
    return false; // No feature consumed the input
}

void FeatureManager::RunGameThreadUpdates() {
    for (auto& feature : m_features) {
        feature->OnGameThreadUpdate();
    }
}

} // namespace kx
