#pragma once

#include "Camera.h" // For Camera class
#include "glm.hpp" 
#include "ESPData.h"

namespace kx {

namespace ESPMath {

    /**
     * @brief Projects a 3D world coordinate to 2D screen coordinates.
     * @param worldPos The 3D world position to project.
     * @param camera The Camera object containing view and projection matrices.
     * @param screenWidth The width of the screen/viewport.
     * @param screenHeight The height of the screen/viewport.
     * @param outScreenPos Output parameter for the calculated 2D screen position.
     * @return True if the projected point is in front of the camera (Z <= 1.0), false otherwise.
     */
    bool WorldToScreen(const glm::vec3& worldPos, const Camera& camera, float screenWidth, float screenHeight, glm::vec2& outScreenPos);

    /**
     * @brief Calculate proper player ESP data based on world position
     * @param worldPos 3D world position of the entity
     * @param camera Camera for projection
     * @param screenWidth Screen width
     * @param screenHeight Screen height
     * @return PlayerESPData with calculated screen bounds and dimensions
     */
    PlayerESPData GetPlayerESPData(const glm::vec3& worldPos, const Camera& camera, float screenWidth, float screenHeight);

    /**
     * @brief Calculate proper NPC ESP data based on world position (square boxes for various creature types)
     * @param worldPos 3D world position of the entity
     * @param camera Camera for projection
     * @param screenWidth Screen width
     * @param screenHeight Screen height
     * @return PlayerESPData with calculated screen bounds and dimensions (square aspect ratio)
     */
    PlayerESPData GetNpcESPData(const glm::vec3& worldPos, const Camera& camera, float screenWidth, float screenHeight);

    /**
     * @brief Calculate proper gadget/object ESP data based on world position  
     * @param worldPos 3D world position of the entity
     * @param camera Camera for projection
     * @param screenWidth Screen width
     * @param screenHeight Screen height
     * @return GadgetESPData with calculated screen bounds and dimensions
     */
    GadgetESPData GetGadgetESPData(const glm::vec3& worldPos, const Camera& camera, float screenWidth, float screenHeight);

    /**
     * @brief Calculate screen distance between two 2D points
     * @param p1 First point
     * @param p2 Second point
     * @return Distance between points
     */
    float CalculateScreenDistance(const glm::vec2& p1, const glm::vec2& p2);

} // namespace ESPUtils

} // namespace kx