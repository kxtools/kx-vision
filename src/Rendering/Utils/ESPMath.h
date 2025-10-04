#pragma once

#include "Camera.h" // For Camera class
#include "glm.hpp" 

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

} // namespace ESPUtils

} // namespace kx