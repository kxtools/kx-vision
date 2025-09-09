#define NOMINMAX

#include "ImGuiManager.h"
#include "../../libs/ImGui/imgui.h"
#include "../../libs/ImGui/imgui_impl_win32.h"
#include "../../libs/ImGui/imgui_impl_dx11.h"
#include "../Core/AppState.h"
#include "GuiStyle.h"
#include "../Core/Config.h"
#include "ESPRenderer.h"
#include "../Hooking/D3DRenderHook.h"

#include <string>
#include <windows.h>

// Define static members
kx::Camera ImGuiManager::m_camera;
kx::MumbleLinkManager ImGuiManager::m_mumbleLinkManager;

bool ImGuiManager::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;

    GUIStyle::LoadAppFont();
    GUIStyle::ApplyCustomStyle();

    if (!ImGui_ImplWin32_Init(hwnd)) return false;
    if (!ImGui_ImplDX11_Init(device, context)) return false;
    
    // Initialize ESP renderer with our camera
    kx::ESPRenderer::Initialize(m_camera);

    return true;
}

void ImGuiManager::NewFrame() {
    // Prepare ImGui for a new frame of rendering
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void ImGuiManager::Render(ID3D11DeviceContext* context, ID3D11RenderTargetView* mainRenderTargetView) {
    // Finish the frame and render ImGui elements
    ImGui::EndFrame();
    ImGui::Render();
    
    // Set render target and draw ImGui data
    context->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiManager::RenderESPWindow() {
    if (!kx::g_isVisionWindowOpen) return;

    // Construct window title with version
    std::string windowTitle = "KX Vision v";
    windowTitle += kx::APP_VERSION.data();

    // Set window properties
    ImGui::SetNextWindowSize(ImVec2(250, 180), ImGuiCond_FirstUseEver);
    ImGui::Begin(windowTitle.c_str(), &kx::g_isVisionWindowOpen);

    // Display keyboard shortcuts and hints
    RenderHints();

    // ESP main toggle
    ImGui::Checkbox("Enable ESP", &kx::g_settings.espEnabled);

    // ESP feature toggles with side-by-side layout
    ImGui::Checkbox("Render Box", &kx::g_settings.espRenderBox);
    ImGui::SameLine();
    ImGui::Checkbox("Render Distance", &kx::g_settings.espRenderDistance);
    ImGui::SameLine();
    ImGui::Checkbox("Render Dot", &kx::g_settings.espRenderDot);

    // Connection status
    ImGui::Text("MumbleLink Status: %s", 
        m_mumbleLinkManager.IsInitialized() ? "Connected" : "Disconnected");

    // Visual separator
    ImGui::Separator();

    // Additional information section
    RenderInfoSection();

    ImGui::End();
}

void ImGuiManager::RenderInfoSection() {
    // Display info section as collapsible header
    if (ImGui::CollapsingHeader("Info")) {
        // Credits
        ImGui::Text("KX Vision by Krixx");
        ImGui::Text("Visit kxtools.xyz for more tools!");
        ImGui::Separator();

        // External links section with consistent layout
        const char* links[][3] = {
            {"GitHub:", "Repository", "https://github.com/kxtools/kx-vision"},
            {"Website:", "kxtools.xyz", "https://kxtools.xyz"},
            {"Discord:", "Join Server", "https://discord.gg/z92rnB4kHm"}
        };

        // Render all links with consistent formatting
        for (const auto& link : links) {
            ImGui::Text("%s", link[0]);
            ImGui::SameLine();
            if (ImGui::Button(link[1])) {
                ShellExecuteA(NULL, "open", link[2], NULL, NULL, SW_SHOWNORMAL);
            }
        }
        
        ImGui::Spacing();
    }
}

void ImGuiManager::RenderUI() {
    ImGuiIO& io = ImGui::GetIO();

    // Update MumbleLink and Camera
    m_mumbleLinkManager.Update();
    m_camera.Update(m_mumbleLinkManager.GetData(), kx::Hooking::D3DRenderHook::GetWindowHandle());

    // Render the ESP overlay
    kx::ESPRenderer::Render(io.DisplaySize.x, io.DisplaySize.y, m_mumbleLinkManager.GetData());
    
    // Render the UI window if it's shown
    if (kx::g_settings.showVisionWindow) {
        RenderESPWindow();
    }
}

void ImGuiManager::RenderHints() {
    // Display keyboard shortcuts with consistent styling
    const char* hints[] = {
        "Press INSERT to show/hide window.",
        "Press DELETE to unload DLL."
    };
    
    for (const auto& hint : hints) {
        ImGui::TextDisabled("Hint: %s", hint);
    }
    
    ImGui::Separator();
}

void ImGuiManager::Shutdown() {
    // Clean up ImGui resources in reverse order of initialization
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}
