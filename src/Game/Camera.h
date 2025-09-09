#pragma once
#include <cstdint>
#include <windows.h>
#include <chrono> // For std::chrono
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "MumbleLink.h" // Include the new struct

namespace kx {

    class Camera {
    public:
        Camera();
        ~Camera();
        void Update();

        const glm::mat4& GetViewMatrix() const { return m_viewMatrix; }
        const glm::mat4& GetProjectionMatrix() const { return m_projectionMatrix; }
        const glm::vec3& GetCameraPosition() const { return m_camPos; }
        const glm::vec3& GetPlayerPosition() const { return m_playerPosition; }
        bool IsMumbleLinkInitialized() const { return m_mumbleLinkInitialized; }
        const MumbleLinkData* GetMumbleLinkData() const { return m_mumbleLink; } // Added getter for MumbleLinkData
    private:
        bool InitializeMumbleLink();

        glm::mat4 m_viewMatrix;
        glm::mat4 m_projectionMatrix;
        glm::vec3 m_camPos;
        glm::vec3 m_playerPosition;

        HANDLE m_mumbleLinkFile = nullptr;
        MumbleLinkData* m_mumbleLink = nullptr;
        bool m_mumbleLinkInitialized = false;
        std::chrono::steady_clock::time_point m_lastMumbleRetryTime;
        const std::chrono::milliseconds MumbleRetryInterval = std::chrono::seconds(5);
        uint32_t m_lastTick = 0; // To detect MumbleLink updates
        const wchar_t* GW2_GAME_NAME = L"Guild Wars 2"; // Expected game name in MumbleLink
    };

} // namespace kx
