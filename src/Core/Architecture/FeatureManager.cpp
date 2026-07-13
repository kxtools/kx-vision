#include "FeatureManager.h"
#include "IFeature.h"
#include "ServiceContext.h"
#include "../../Game/Data/FrameData.h"
#include "../../Utils/DebugLogger.h"
#include "../../../libs/ImGui/imgui.h"

#include <cstring>
#include <algorithm>
#include <mutex>
#include <exception>

namespace kx {

namespace {

bool HasRenderedGroupBefore(const std::vector<std::unique_ptr<IFeature>>& features,
                            size_t currentIndex,
                            const char* groupName) {
    if (groupName == nullptr || groupName[0] == '\0') {
        return false;
    }

    for (size_t i = 0; i < currentIndex; ++i) {
        const auto& feature = features[i];
        const char* previousGroupName = feature ? feature->GetTabGroupName() : nullptr;
        if (previousGroupName && std::strcmp(previousGroupName, groupName) == 0) {
            return true;
        }
    }

    return false;
}

void RenderFeatureGroup(const std::vector<std::unique_ptr<IFeature>>& features,
                        const char* groupName) {
    if (!ImGui::BeginTabItem(groupName)) {
        return;
    }

    ImGui::PushID(groupName);
    if (ImGui::BeginTabBar("##FeatureGroupTabs")) {
        for (const auto& feature : features) {
            const char* featureGroupName = feature ? feature->GetTabGroupName() : nullptr;
            if (!featureGroupName || std::strcmp(featureGroupName, groupName) != 0) {
                continue;
            }

            if (ImGui::BeginTabItem(feature->GetGroupedTabLabel())) {
                feature->OnGroupedTabRender();
                ImGui::EndTabItem();
            }
        }

        ImGui::EndTabBar();
    }
    ImGui::PopID();

    ImGui::EndTabItem();
}

} // namespace

// Define constructor and destructor here where IFeature is complete
FeatureManager::FeatureManager() = default;
FeatureManager::~FeatureManager() = default;

void FeatureManager::RegisterFeature(std::unique_ptr<IFeature> feature) {
    std::unique_lock lock(m_featureMutex);
    if (feature) {
        const bool duplicate = std::any_of(m_features.begin(), m_features.end(), [&](const auto& current) {
            return current && std::strcmp(current->GetName(), feature->GetName()) == 0;
        });
        if (duplicate) {
            LOG_INFO("Skipping duplicate feature registration: %s", feature->GetName());
            return;
        }

        LOG_INFO("Registering feature: %s", feature->GetName());
        m_features.push_back(std::move(feature));
    }
}

bool FeatureManager::HasFeature(const char* featureName) const {
    std::shared_lock lock(m_featureMutex);
    if (featureName == nullptr || featureName[0] == '\0') {
        return false;
    }

    for (const auto& feature : m_features) {
        if (feature && std::strcmp(feature->GetName(), featureName) == 0) {
            return true;
        }
    }

    return false;
}

bool FeatureManager::RemoveFeature(const char* featureName) {
    if (featureName == nullptr || featureName[0] == '\0') {
        return false;
    }

    std::unique_ptr<IFeature> removed;
    bool wasInitialized = false;
    {
        std::unique_lock lock(m_featureMutex);
        for (size_t i = 0; i < m_features.size(); ++i) {
            auto& feature = m_features[i];
            if (!feature || std::strcmp(feature->GetName(), featureName) != 0) continue;
            wasInitialized = i < m_initializedFeatureCount;
            removed = std::move(feature);
            m_features.erase(m_features.begin() + i);
            if (wasInitialized) --m_initializedFeatureCount;
            break;
        }
    }

    if (!removed) return false;
    // Guardrail: worker joins belong outside feature-list lock. Otherwise shutdown can
    // deadlock when worker completion needs a read-side feature callback.
    if (wasInitialized) removed->Shutdown();
    LOG_INFO("Removed feature: %s", removed->GetName());
    return true;
}

bool FeatureManager::InitializeAll(const ServiceContext& ctx) {
    std::unique_lock lock(m_featureMutex);
    const size_t pendingCount = m_features.size() - m_initializedFeatureCount;
    LOG_INFO("Initializing %zu feature(s)...", pendingCount);

    for (size_t i = m_initializedFeatureCount; i < m_features.size(); ++i) {
        auto& feature = m_features[i];
        if (!feature->Initialize(ctx)) {
            LOG_ERROR("Failed to initialize feature: %s", feature->GetName());
            return false;
        }
        LOG_INFO("Initialized feature: %s", feature->GetName());
        try {
            feature->ApplyLoadedSettings();
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to apply settings for feature '%s': %s", feature->GetName(), e.what());
            feature->Shutdown();
            return false;
        }
        ++m_initializedFeatureCount;
    }
    
    return true;
}

void FeatureManager::ShutdownAll() {
    std::vector<std::unique_ptr<IFeature>> features;
    size_t initializedCount = 0;
    {
        std::unique_lock lock(m_featureMutex);
        features.swap(m_features);
        initializedCount = m_initializedFeatureCount;
        m_initializedFeatureCount = 0;
    }
    LOG_INFO("Shutting down %zu feature(s)...", features.size());

    for (size_t i = initializedCount; i > 0; --i) {
        auto& feature = features[i - 1];
        if (feature) {
            LOG_INFO("Shutting down feature: %s", feature->GetName());
            feature->Shutdown();
        }
    }
}

void FeatureManager::LoadPendingSettings(const nlohmann::json& root) {
    std::shared_lock lock(m_featureMutex);
    for (size_t i = m_initializedFeatureCount; i < m_features.size(); ++i) {
        if (!m_features[i]) continue;
        try {
            m_features[i]->LoadSettings(root);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to load settings for feature '%s': %s", m_features[i]->GetName(), e.what());
            m_features[i]->ResetSettings();
        }
    }
}

void FeatureManager::LoadAllSettings(const nlohmann::json& root) {
    std::shared_lock lock(m_featureMutex);
    for (size_t i = 0; i < m_features.size(); ++i) {
        if (!m_features[i]) continue;
        try {
            m_features[i]->LoadSettings(root);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to load settings for feature '%s': %s", m_features[i]->GetName(), e.what());
            m_features[i]->ResetSettings();
        }
        if (i < m_initializedFeatureCount) m_features[i]->ApplyLoadedSettings();
    }
}

void FeatureManager::ResetAllSettings() {
    std::shared_lock lock(m_featureMutex);
    for (size_t i = 0; i < m_features.size(); ++i) {
        if (!m_features[i]) continue;
        try {
            m_features[i]->ResetSettings();
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to reset settings for feature '%s': %s", m_features[i]->GetName(), e.what());
            continue;
        }
        if (i < m_initializedFeatureCount) m_features[i]->ApplyLoadedSettings();
    }
}

void FeatureManager::UpdateAll(float deltaTime, const FrameSnapshotHandle& snapshot, const ServiceContext& ctx) {
    std::shared_lock lock(m_featureMutex);
    for (auto& feature : m_features) {
        if (feature && feature->IsEnabled()) {
            feature->Update(deltaTime, snapshot, ctx);
        }
    }
}

void FeatureManager::RenderAllDrawLists(ImDrawList* drawList, const FrameSnapshotHandle& snapshot, const ServiceContext& ctx) {
    if (!drawList) return;
    std::shared_lock lock(m_featureMutex);
    
    for (auto& feature : m_features) {
        if (feature && feature->IsEnabled()) {
            feature->RenderDrawList(drawList, snapshot, ctx);
        }
    }
}

void FeatureManager::RenderAllTabs() {
    std::shared_lock lock(m_featureMutex);
    for (size_t i = 0; i < m_features.size(); ++i) {
        auto& feature = m_features[i];
        if (!feature) {
            continue;
        }

        const char* groupName = feature->GetTabGroupName();
        if (groupName == nullptr || groupName[0] == '\0') {
            feature->OnTabRender();
            continue;
        }

        if (!HasRenderedGroupBefore(m_features, i, groupName)) {
            RenderFeatureGroup(m_features, groupName);
        }
    }
}

void FeatureManager::RenderAllStandaloneWindows() {
    std::shared_lock lock(m_featureMutex);
    for (auto& feature : m_features) {
        feature->OnStandaloneRender();
    }
}

void FeatureManager::RenderAllMenus() {
    // Legacy compatibility path via IFeature defaults:
    // OnTabRender() forwards to OnMenuRender() when only legacy override exists.
    RenderAllTabs();
}

size_t FeatureManager::GetFeatureCount() const {
    std::shared_lock lock(m_featureMutex);
    return m_features.size();
}

bool FeatureManager::BroadcastInput(UINT message, WPARAM wParam, LPARAM lParam) {
    std::shared_lock lock(m_featureMutex);
    for (auto& feature : m_features) {
        if (feature && feature->IsEnabled() && feature->OnInput(message, wParam, lParam)) {
            return true; // Feature consumed the input
        }
    }
    return false; // No feature consumed the input
}

void FeatureManager::RunGameThreadUpdates() {
    std::shared_lock lock(m_featureMutex);
    for (auto& feature : m_features) {
        if (feature && feature->IsEnabled()) {
            feature->OnGameThreadUpdate();
        }
    }
}

} // namespace kx
