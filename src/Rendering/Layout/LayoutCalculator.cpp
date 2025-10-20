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
    
    // Gather all elements that need layout
    std::vector<std::pair<std::string, ImVec2>> aboveBoxElements;
    std::vector<std::pair<std::string, ImVec2>> belowBoxElements;
    GatherLayoutElements(request, aboveBoxElements, belowBoxElements);

    // Calculate positions for elements below the box
    glm::vec2 belowBoxAnchor = { request.visualProps.center.x, request.visualProps.boxMax.y };
    CalculateVerticalStack(belowBoxAnchor, belowBoxElements, result.elementPositions, false);

    // Calculate positions for elements above the box
    glm::vec2 aboveBoxAnchor = { request.visualProps.center.x, request.visualProps.boxMin.y };
    CalculateVerticalStack(aboveBoxAnchor, aboveBoxElements, result.elementPositions, true);

    // Set health bar anchor for dependent elements
    auto healthBarIt = result.elementPositions.find(LayoutKeys::HEALTH_BAR);
    if (healthBarIt != result.elementPositions.end()) {
        result.healthBarAnchor = { healthBarIt->second.x - request.visualProps.finalHealthBarWidth / 2.0f, healthBarIt->second.y };
    }

    return result;
}

void LayoutCalculator::GatherLayoutElements(
    const LayoutRequest& request,
    std::vector<std::pair<std::string, ImVec2>>& outAboveElements,
    std::vector<std::pair<std::string, ImVec2>>& outBelowElements)
{
    const auto& entityContext = request.entityContext;
    const auto& props = request.visualProps;
    const auto& context = request.frameContext;

    // --- GATHER ABOVE BOX ELEMENTS ---
    if (entityContext.renderDistance) {
        TextElement element = TextElementFactory::CreateDistanceText(entityContext.gameplayDistance, {0,0}, 0, props.finalFontSize);
        ImVec2 size = TextRenderer::CalculateSize(element);
        outAboveElements.push_back({LayoutKeys::DISTANCE, size});
    }

    // --- GATHER BELOW BOX ELEMENTS using helper functions ---
    GatherStatusBarElements(request, outBelowElements);
    GatherPlayerIdentityElements(request, outBelowElements);
    GatherDetailElements(request, outBelowElements);
}

void LayoutCalculator::GatherStatusBarElements(
    const LayoutRequest& request,
    std::vector<std::pair<std::string, ImVec2>>& outBelowElements)
{
    const auto& entityContext = request.entityContext;
    const auto& props = request.visualProps;
    const auto& context = request.frameContext;

    // Health Bar
    bool isLivingEntity = (entityContext.entityType == ESPEntityType::Player || entityContext.entityType == ESPEntityType::NPC);
    bool isGadget = (entityContext.entityType == ESPEntityType::Gadget);
    float healthPercent = entityContext.entity->maxHealth > 0 ? (entityContext.entity->currentHealth / entityContext.entity->maxHealth) : -1.0f;
    if ((isLivingEntity || isGadget) && healthPercent >= 0.0f && entityContext.renderHealthBar) {
        ImVec2 size = {props.finalHealthBarWidth, props.finalHealthBarHeight};
        outBelowElements.push_back({LayoutKeys::HEALTH_BAR, size});
    }

    // Energy Bar (Players only)
    if (entityContext.entityType == ESPEntityType::Player) {
        const auto* player = static_cast<const RenderablePlayer*>(entityContext.entity);
        float energyPercent = CalculateEnergyPercent(player, entityContext.playerEnergyDisplayType);
        if (energyPercent >= 0.0f && entityContext.renderEnergyBar) {
            ImVec2 size = {props.finalHealthBarWidth, props.finalHealthBarHeight}; // Assuming same size as health bar
            outBelowElements.push_back({LayoutKeys::ENERGY_BAR, size});
        }
    }
}

void LayoutCalculator::GatherPlayerIdentityElements(
    const LayoutRequest& request,
    std::vector<std::pair<std::string, ImVec2>>& outBelowElements)
{
    const auto& entityContext = request.entityContext;
    const auto& props = request.visualProps;
    const auto& context = request.frameContext;

    // Player Name (fallback to profession if name is empty)
    if (entityContext.renderPlayerName) {
        std::string displayName = entityContext.playerName;
        
        // Fallback to profession for hostile players without names (WvW)
        if (displayName.empty() && entityContext.entityType == ESPEntityType::Player) {
            const auto* player = static_cast<const RenderablePlayer*>(entityContext.entity);
            if (player != nullptr) {
                const char* profName = ESPFormatting::GetProfessionName(player->profession);
                if (profName != nullptr) {
                    displayName = profName;
                }
            }
        }
        
        if (!displayName.empty()) {
            TextElement element = TextElementFactory::CreatePlayerName(displayName, {0,0}, 0, 0, props.finalFontSize);
            ImVec2 size = TextRenderer::CalculateSize(element);
            outBelowElements.push_back({LayoutKeys::PLAYER_NAME, size});
        }
    }

    // Player Gear (Players only)
    if (entityContext.entityType == ESPEntityType::Player) {
        const auto* player = static_cast<const RenderablePlayer*>(entityContext.entity);
        if (player != nullptr) {
            switch (entityContext.playerGearDisplayMode) {
                case GearDisplayMode::Compact: {
                    auto summary = ESPPlayerDetailsBuilder::BuildCompactGearSummary(player);
                    TextElement element = TextElementFactory::CreateGearSummary(summary, {0,0}, 0, props.finalFontSize);
                    outBelowElements.push_back({LayoutKeys::GEAR_SUMMARY, TextRenderer::CalculateSize(element)});
                    break;
                }
                case GearDisplayMode::Attributes: {
                    auto stats = ESPPlayerDetailsBuilder::BuildDominantStats(player);
                    auto rarity = ESPPlayerDetailsBuilder::GetHighestRarity(player);
                    TextElement element = TextElementFactory::CreateDominantStats(stats, rarity, {0,0}, 0, props.finalFontSize);
                    outBelowElements.push_back({LayoutKeys::DOMINANT_STATS, TextRenderer::CalculateSize(element)});
                    break;
                }
                default: break;
            }
        }
    }
}

void LayoutCalculator::GatherDetailElements(
    const LayoutRequest& request,
    std::vector<std::pair<std::string, ImVec2>>& outBelowElements)
{
    const auto& entityContext = request.entityContext;
    const auto& props = request.visualProps;

    // Entity Details
    if (entityContext.renderDetails && !entityContext.details.empty()) {
        TextElement element = TextElementFactory::CreateDetailsText(entityContext.details, {0,0}, 0, props.finalFontSize);
        ImVec2 size = TextRenderer::CalculateSize(element);
        outBelowElements.push_back({LayoutKeys::DETAILS, size});
    }
}

void LayoutCalculator::CalculateVerticalStack(
    glm::vec2 startAnchor,
    const std::vector<std::pair<std::string, ImVec2>>& elements,
    std::map<std::string, glm::vec2>& outPositions,
    bool stackUpwards)
{
    float currentY = startAnchor.y;
    float direction = stackUpwards ? -1.0f : 1.0f;

    // Start with a larger margin from the box itself.
    currentY += RenderingLayout::REGION_MARGIN_VERTICAL * direction;

    for (const auto& item : elements) {
        const std::string& name = item.first;
        const ImVec2& size = item.second;
        float elementHeight = size.y;

        if (stackUpwards) {
            currentY -= elementHeight; // Position the top of the element at the cursor
        }

        // Center the element horizontally on the anchor's X.
        outPositions[name] = { startAnchor.x, currentY };

        if (!stackUpwards) {
            currentY += elementHeight; // Move cursor down for the next element
        }
        
        // Add margin for the next element.
        currentY += RenderingLayout::ELEMENT_MARGIN_VERTICAL * direction;
    }
}

} // namespace kx
