#include "LayoutCalculator.h"
#include "../Utils/TextElementFactory.h"
#include "../Renderers/TextRenderer.h"
#include "../Utils/ESPPlayerDetailsBuilder.h"
#include "../Utils/ESPStyling.h"
#include "../Utils/LayoutConstants.h"
#include "../Data/ESPEntityTypes.h"

namespace kx {

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
    auto healthBarIt = result.elementPositions.find("healthBar");
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
        outAboveElements.push_back({"distance", size});
    }

    // --- GATHER BELOW BOX ELEMENTS (Combat Priority Layout) ---
    // 1. Status Bars (Health & Energy)
    bool isLivingEntity = (entityContext.entityType == ESPEntityType::Player || entityContext.entityType == ESPEntityType::NPC);
    bool isGadget = (entityContext.entityType == ESPEntityType::Gadget);
    if ((isLivingEntity || isGadget) && entityContext.healthPercent >= 0.0f && entityContext.renderHealthBar) {
        ImVec2 size = {props.finalHealthBarWidth, props.finalHealthBarHeight};
        outBelowElements.push_back({"healthBar", size});
    }
    if (entityContext.entityType == ESPEntityType::Player && entityContext.energyPercent >= 0.0f && entityContext.renderEnergyBar) {
        ImVec2 size = {props.finalHealthBarWidth, props.finalHealthBarHeight}; // Assuming same size as health bar
        outBelowElements.push_back({"energyBar", size});
    }

    // 2. Identity Info (Name & Gear)
    if (entityContext.renderPlayerName && !entityContext.playerName.empty()) {
        TextElement element = TextElementFactory::CreatePlayerName(entityContext.playerName, {0,0}, 0, 0, props.finalFontSize);
        ImVec2 size = TextRenderer::CalculateSize(element);
        outBelowElements.push_back({"playerName", size});
    }
    if (entityContext.entityType == ESPEntityType::Player && entityContext.player != nullptr) {
        switch (context.settings.playerESP.gearDisplayMode) {
            case GearDisplayMode::Compact: {
                auto summary = ESPPlayerDetailsBuilder::BuildCompactGearSummary(entityContext.player);
                TextElement element = TextElementFactory::CreateGearSummary(summary, {0,0}, 0, props.finalFontSize);
                outBelowElements.push_back({"gearSummary", TextRenderer::CalculateSize(element)});
                break;
            }
            case GearDisplayMode::Attributes: {
                auto stats = ESPPlayerDetailsBuilder::BuildDominantStats(entityContext.player);
                auto rarity = ESPPlayerDetailsBuilder::GetHighestRarity(entityContext.player);
                TextElement element = TextElementFactory::CreateDominantStats(stats, rarity, {0,0}, 0, props.finalFontSize);
                outBelowElements.push_back({"dominantStats", TextRenderer::CalculateSize(element)});
                break;
            }
            default: break;
        }
    }

    // 3. Verbose Details (Placed last)
    if (entityContext.renderDetails && !entityContext.details.empty()) {
        TextElement element = TextElementFactory::CreateDetailsText(entityContext.details, {0,0}, 0, props.finalFontSize);
        ImVec2 size = TextRenderer::CalculateSize(element);
        outBelowElements.push_back({"details", size});
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
    currentY += kx::RenderingLayout::REGION_MARGIN_VERTICAL * direction;

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
        currentY += kx::RenderingLayout::ELEMENT_MARGIN_VERTICAL * direction;
    }
}

} // namespace kx
