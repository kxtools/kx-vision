#pragma once

#include "glm.hpp"
#include "../Mumble/MumbleLink.h"
#include "gtc/matrix_transform.hpp"

namespace kx {

    // Forward declaration
    class MumbleLinkManager;

    class Camera {
    public:
        Camera();
        ~Camera();
        void Update(const MumbleLinkManager& mumbleManager, HWND hWnd);

        const glm::mat4& GetViewMatrix() const { return m_viewMatrix; }
        const glm::mat4& GetProjectionMatrix() const { return m_projectionMatrix; }
        const glm::vec3& GetCameraPosition() const { return m_camPos; }
        const glm::vec3& GetPlayerPosition() const { return m_playerPosition; }

    private:
        glm::mat4 m_viewMatrix;
        glm::mat4 m_projectionMatrix;
        glm::vec3 m_camPos;
        glm::vec3 m_playerPosition;
    };

} // namespace kx