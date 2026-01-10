#pragma once
#include <glm.hpp>
#include "../../../Rendering/Shared/LayoutConstants.h"

namespace kx {

struct LayoutCursor {
    glm::vec2 anchor;   // The starting X,Y (center of entity for X, top/bottom for Y)
    float currentY;     // Current Y offset from anchor
    float direction;    // 1.0f for growing Down, -1.0f for growing Up
    float spacing;      // Gap between elements

    LayoutCursor(const glm::vec2& startPos, float growDirection, float margin = kx::RenderingLayout::REGION_MARGIN_VERTICAL) 
        : anchor(startPos), direction(growDirection), spacing(kx::RenderingLayout::ELEMENT_MARGIN_VERTICAL) 
    {
        // Apply initial margin immediately
        currentY = margin * direction;
    }

    // Returns the coordinate where the next element should be centered
    glm::vec2 GetPosition() const {
        return { anchor.x, anchor.y + currentY };
    }

    // Advances the cursor past the element we just drew
    void Advance(float height) {
        // Move further in the growth direction by height + spacing
        currentY += (height + spacing) * direction;
    }
    
    // Special helper to get the Top-Left corner for a health bar based on current center cursor
    glm::vec2 GetTopLeftForBar(float width, float height) const {
        glm::vec2 pos = GetPosition();
        // If growing UP, the current cursor is the bottom of the element. 
        // If growing DOWN, it's the top.
        // However, our standard bar renderers usually take Top-Left.
        // Let's standardize: The cursor represents the Y-coordinate of the EDGE of the element closest to the entity.
        
        float yPos;
        if (direction > 0) { // Growing Down
             yPos = anchor.y + currentY; 
        } else { // Growing Up
             yPos = anchor.y + currentY - height; 
        }
        
        return { pos.x - width * 0.5f, yPos };
    }
};

}
