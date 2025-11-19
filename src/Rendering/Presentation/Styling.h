#pragma once

#include "../../Game/GameEnums.h"
#include "../../libs/ImGui/imgui.h"
#include "Generated/EnumsAndStructs.h"
#include "../Data/EntityTypes.h"

namespace kx {
    struct RenderableEntity;
    struct RenderablePlayer;
    struct RenderableNpc;
}

namespace kx::Styling {

    ImU32 GetRarityColor(Game::ItemRarity rarity);
    ImU32 GetTacticalColor(data::ApiAttribute attribute);
    bool ShouldHideCombatUIForGadget(Game::GadgetType type);
    
    ImU32 GetEntityColor(const RenderableEntity& entity);

    float GetRankMultiplier(Game::CharacterRank rank);
    float GetGadgetHealthMultiplier(float maxHealth);
    float GetDamageNumberFontSizeMultiplier(float damageToDisplay);

} // namespace kx::Styling
