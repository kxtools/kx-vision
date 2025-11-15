#include "LayoutCalculator.h"
#include "LayoutElementKeys.h"
#include "../Utils/TextElementFactory.h"
#include "../Renderers/TextRenderer.h"
#include "../Utils/ESPPlayerDetailsBuilder.h"
#include "../Utils/ESPFormatting.h"
#include "../Utils/ESPStyling.h"
#include "../Utils/LayoutConstants.h"
#include "../Data/ESPEntityTypes.h"

namespace kx {

namespace {
    // Helper function to calculate energy percentage for players
    float CalculateEnergyPercent(const RenderablePlayer* player, EnergyDisplayType displayType) {
        if (displayType == EnergyDisplayType::Dodge) {
            if (player->maxEnergy > 0) {
                return player->currentEnergy / player->maxEnergy;
            }
        } else { // Special
            if (player->maxSpecialEnergy > 0) {
                return player->currentSpecialEnergy / player->maxSpecialEnergy;
            }
        }
        return -1.0f;
    }
}

LayoutResult LayoutCalculator::CalculateLayout(const LayoutRequest& request) {
    LayoutResult result;
    
    // Initialize arrays
    result.elementPositions.fill(glm::vec2(0.0f));
    result.hasElement.fill(false);
    
    // Define the single, universal anchor point at the entity's projected origin
    glm::vec2 infoAnchor = { request.visualProps.screenPos.x, request.visualProps.screenPos.y };

    // Gather all elements that need layout into a single stack
    std::vector<std::pair<LayoutElementKey, ImVec2>> layoutStack;
    GatherLayoutElements(request, layoutStack);

    // Calculate positions for all elements, stacking downward from the anchor
    CalculateVerticalStack(infoAnchor, layoutStack, result.elementPositions, result.hasElement, false);

    // Set health bar anchor for dependent elements
    if (result.HasElement(LayoutElementKey::HealthBar)) {
        glm::vec2 healthBarPos = result.GetElementPosition(LayoutElementKey::HealthBar);
        result.healthBarAnchor = { healthBarPos.x - request.visualProps.finalHealthBarWidth / 2.0f, healthBarPos.y };
    }

    return result;
}

void LayoutCalculator::GatherLayoutElements(
    const LayoutRequest& request,
    std::vector<std::pair<LayoutElementKey, ImVec2>>& outLayoutStack)
{
    const auto& entityContext = request.entityContext;
    const auto& props = request.visualProps;
    const auto& context = request.frameContext;

    // --- MERGED IDENTITY LINE (Name + Distance) ---
    std::string entityName = "";
    bool showName = false;
    bool showDistance = entityContext.renderDistance;

    // Determine the name and if it should be shown
    switch (entityContext.entityType) {
        case ESPEntityType::Player:
            if (entityContext.renderPlayerName) {
                entityName = entityContext.playerName;
                // Fallback for anonymous hostile players in WvW
                if (entityName.empty()) {
                    const auto* player = static_cast<const RenderablePlayer*>(entityContext.entity);
                    const char* profName = ESPFormatting::GetProfessionName(player->profession);
                    if (profName) entityName = profName;
                }
                showName = !entityName.empty();
            }
            break;
        case ESPEntityType::NPC:
        case ESPEntityType::Gadget:
            // For non-players, the "name" is part of the details panel.
            // For this layout, we only show Name for Players.
            break;
    }

    // Create the merged identity line element
    if (showName || showDistance) {
        TextElement identityElement = TextElementFactory::CreateIdentityLine(request, showName, showDistance);
        ImVec2 size = TextRenderer::CalculateSize(identityElement);
        // Reuse PlayerName key for the merged identity line
        outLayoutStack.push_back({LayoutElementKey::PlayerName, size});
    }

    // --- GATHER REMAINING ELEMENTS ---
    GatherStatusBarElements(request, outLayoutStack);
    
    // Player Gear/Attribute Summary (gathered before details panel)
    if (entityContext.entityType == ESPEntityType::Player) {
        const auto* player = static_cast<const RenderablePlayer*>(entityContext.entity);
        if (player != nullptr) {
            switch (entityContext.playerGearDisplayMode) {
                case GearDisplayMode::Compact: {
                    auto summary = ESPPlayerDetailsBuilder::BuildCompactGearSummary(player);
                    TextElement element = TextElementFactory::CreateGearSummary(summary, {0,0}, 0, props.finalFontSize);
                    outLayoutStack.push_back({LayoutElementKey::GearSummary, TextRenderer::CalculateSize(element)});
                    break;
                }
                case GearDisplayMode::Attributes: {
                    auto stats = ESPPlayerDetailsBuilder::BuildDominantStats(player);
                    auto rarity = ESPPlayerDetailsBuilder::GetHighestRarity(player);
                    TextElement element = TextElementFactory::CreateDominantStats(stats, rarity, {0,0}, 0, props.finalFontSize);
                    outLayoutStack.push_back({LayoutElementKey::DominantStats, TextRenderer::CalculateSize(element)});
                    break;
                }
                default: break;
            }
        }
    }
    
    // Details Panel (gathered last to ensure it's always at the bottom of the stack)
    GatherDetailElements(request, outLayoutStack);
}

void LayoutCalculator::GatherStatusBarElements(
    const LayoutRequest& request,
    std::vector<std::pair<LayoutElementKey, ImVec2>>& outLayoutStack)
{
    const auto& entityContext = request.entityContext;
    const auto& props = request.visualProps;
    const auto& context = request.frameContext;

    // Health Bar
    bool isLivingEntity = (entityContext.entityType == ESPEntityType::Player || entityContext.entityType == ESPEntityType::NPC);
    bool isGadget = (entityContext.entityType == ESPEntityType::Gadget || entityContext.entityType == ESPEntityType::AttackTarget);
    float healthPercent = entityContext.entity->maxHealth > 0 ? (entityContext.entity->currentHealth / entityContext.entity->maxHealth) : -1.0f;
    if ((isLivingEntity || isGadget) && healthPercent >= 0.0f && entityContext.renderHealthBar) {
        ImVec2 size = {props.finalHealthBarWidth, props.finalHealthBarHeight};
        outLayoutStack.push_back({LayoutElementKey::HealthBar, size});
    }

    // Energy Bar (Players only)
    if (entityContext.entityType == ESPEntityType::Player) {
        const auto* player = static_cast<const RenderablePlayer*>(entityContext.entity);
        float energyPercent = CalculateEnergyPercent(player, entityContext.playerEnergyDisplayType);
        if (energyPercent >= 0.0f && entityContext.renderEnergyBar) {
            ImVec2 size = {props.finalHealthBarWidth, props.finalHealthBarHeight};
            outLayoutStack.push_back({LayoutElementKey::EnergyBar, size});
        }
    }
}


void LayoutCalculator::GatherDetailElements(
    const LayoutRequest& request,
    std::vector<std::pair<LayoutElementKey, ImVec2>>& outLayoutStack)
{
    const auto& entityContext = request.entityContext;
    const auto& props = request.visualProps;

    // Entity Details
    if (entityContext.renderDetails && !entityContext.details.empty()) {
        TextElement element = TextElementFactory::CreateDetailsText(entityContext.details, {0,0}, 0, props.finalFontSize);
        ImVec2 size = TextRenderer::CalculateSize(element);
        outLayoutStack.push_back({LayoutElementKey::Details, size});
    }
}

void LayoutCalculator::CalculateVerticalStack(
    glm::vec2 startAnchor,
    const std::vector<std::pair<LayoutElementKey, ImVec2>>& elements,
    std::array<glm::vec2, (size_t)LayoutElementKey::Count>& outPositions,
    std::array<bool, (size_t)LayoutElementKey::Count>& outHasElement,
    bool stackUpwards)
{
    float currentY = startAnchor.y;
    float direction = stackUpwards ? -1.0f : 1.0f;

    // Add initial margin from the anchor point.
    currentY += RenderingLayout::REGION_MARGIN_VERTICAL * direction;

    for (const auto& item : elements) {
        const LayoutElementKey& key = item.first;
        const ImVec2& size = item.second;
        float elementHeight = size.y;

        if (stackUpwards) {
            currentY -= elementHeight; // Position the top of the element at the cursor
        }

        // Center the element horizontally on the anchor's X.
        size_t index = (size_t)key;
        outPositions[index] = { startAnchor.x, currentY };
        outHasElement[index] = true;

        if (!stackUpwards) {
            currentY += elementHeight; // Move cursor down for the next element
        }
        
        // Add margin for the next element.
        currentY += RenderingLayout::ELEMENT_MARGIN_VERTICAL * direction;
    }
}

} // namespace kx
