#pragma once

#include "IFeature.h"
#include "../Settings/SettingsFields.h"

namespace kx {

template <typename SettingsType>
class FeatureWithSettings : public IFeature {
public:
    explicit FeatureWithSettings(const char* sectionKey) : m_settingsSectionKey(sectionKey) {}

    void LoadSettings(const nlohmann::json& root) final {
        m_settings = SettingsType{};
        if (root.is_object()) {
            const auto section = root.find(m_settingsSectionKey);
            if (section != root.end()) {
                try { section->get_to(m_settings); } catch (const nlohmann::json::exception&) {}
            }
        }
        NormalizeSettings();
    }

    void SaveSettings(nlohmann::json& root) final {
        NormalizeSettings();
        root[m_settingsSectionKey] = m_settings;
    }

    void ResetSettings() final {
        m_settings = SettingsType{};
        NormalizeSettings();
    }

    // Deserialization must remain side-effect free. FeatureManager invokes this only
    // after Initialize(), when hooks/services and ServiceContext are valid.
    void ApplyLoadedSettings() final { ApplySettingsAfterInitialize(); }

protected:
    virtual void NormalizeSettings() {}
    virtual void ApplySettingsAfterInitialize() {}

    SettingsType m_settings{};

private:
    const char* m_settingsSectionKey;
};

} // namespace kx
