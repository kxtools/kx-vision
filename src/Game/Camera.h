#pragma once

#include "glm.hpp"
#include "MumbleLink.h" // For the struct in the Update method parameter
#include "gtc/matrix_transform.hpp"

namespace kx {

    class Camera {
    public:
        Camera();
        ~Camera();
        void Update(const MumbleLinkData* mumbleData, HWND hWnd);

        const glm::mat4& GetViewMatrix() const { return m_viewMatrix; }
        const glm::mat4& GetProjectionMatrix() const { return m_projectionMatrix; }
        const glm::vec3& GetCameraPosition() const { return m_camPos; }
        const glm::vec3& GetPlayerPosition() const { return m_playerPosition; }

        glm::vec3 GetRight() const;
        glm::vec3 GetUp() const;
        glm::vec3 GetForward() const;

    private:
        glm::mat4 m_viewMatrix;
        glm::mat4 m_projectionMatrix;
        glm::vec3 m_camPos;
        glm::vec3 m_playerPosition;
    };

} // namespace kx