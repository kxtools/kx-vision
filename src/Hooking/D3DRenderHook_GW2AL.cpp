/**
 * @file D3DRenderHook_GW2AL.cpp
 * @brief GW2AL addon mode specific functionality
 * 
 * This file contains GW2AL-mode specific code:
 * - Simple WndProc that relies on GW2AL's input routing
 * - No Present hook (GW2AL provides the device directly)
 * 
 * This file is only compiled when GW2AL_BUILD is defined.
 */

#include "D3DRenderHook.h"

#ifdef GW2AL_BUILD // Only compile this file in GW2AL mode

#include "../Core/Config.h"
#include "../Core/AppState.h"
#include "../../libs/ImGui/imgui.h"
#include "../../libs/ImGui/imgui_impl_win32.h"

// Declare the external ImGui Win32 handler
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace kx::Hooking {

    LRESULT __stdcall D3DRenderHook::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        // Simple WndProc for GW2AL mode - just handle ImGui input
        // GW2AL loader manages input routing and game/addon separation
        if (m_isInit && kx::AppState::Get().IsVisionWindowOpen()) {
            if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) {
                ImGuiIO& io = ImGui::GetIO();
                // Block input to game if ImGui wants it
                if (io.WantCaptureMouse || io.WantCaptureKeyboard) {
                    return 1;
                }
            }
        }

        // Pass to original game window procedure
        return m_pOriginalWndProc ? CallWindowProc(m_pOriginalWndProc, hWnd, uMsg, wParam, lParam)
            : DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

} // namespace kx::Hooking

#endif // GW2AL_BUILD
