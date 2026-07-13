#pragma once

#include "../../../libs/nlohmann/json.hpp"
#include "SettingsConstants.h"

// Guardrail: each persisted field belongs in exactly one field list. These passes
// generate storage and both JSON directions, so keys/defaults cannot drift apart.
#define KX_SETTING_DECLARE(type, name, defaultValue) type name = defaultValue;
#define KX_SETTING_KEY(name) #name
#define KX_SETTING_SAVE(type, name, defaultValue) j[KX_SETTING_KEY(name)] = value.name;
#define KX_SETTING_LOAD(type, name, defaultValue)                                      \
    do {                                                                               \
        const auto it = j.find(KX_SETTING_KEY(name));                                  \
        if (it != j.end()) {                                                           \
            try { it->get_to(value.name); } catch (const nlohmann::json::exception&) {} \
        }                                                                              \
    } while (false);

#define KX_DEFINE_SETTINGS_TYPE(typeName, fields)                                      \
    struct typeName { fields(KX_SETTING_DECLARE) };                                    \
    KX_DEFINE_SETTINGS_JSON(typeName, fields)

#define KX_DEFINE_SETTINGS_JSON(typeName, fields)                                      \
    inline void to_json(nlohmann::json& j, const typeName& value) {                    \
        j = nlohmann::json::object();                                                  \
        fields(KX_SETTING_SAVE)                                                        \
    }                                                                                  \
    inline void from_json(const nlohmann::json& j, typeName& value) {                  \
        value = typeName{};                                                            \
        if (!j.is_object()) return;                                                    \
        fields(KX_SETTING_LOAD)                                                        \
    }
