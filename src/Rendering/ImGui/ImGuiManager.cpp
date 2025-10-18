#define NOMINMAX

#include "ImGuiManager.h"

#include <string>
#include <windows.h>

#include "Core/ESPRenderer.h"
#include "ImGuiStyle.h"
#include "../../libs/ImGui/imgui.h"
#include "../../libs/ImGui/imgui_impl_dx11.h"
#include "../../libs/ImGui/imgui_impl_win32.h"
#include "../Core/AppState.h"
#include "../Core/Config.h"
#include "../Game/MumbleLinkManager.h"
#include "../Hooking/D3DRenderHook.h"
#include "../Utils/DebugLogger.h"

#include "GUI/AppearanceTab.h"
#include "GUI/InfoTab.h"
#include "GUI/NpcsTab.h"
#include "GUI/ObjectsTab.h"
#include "GUI/PlayersTab.h"
#include "GUI/SettingsTab.h"
#include "GUI/ValidationTab.h"

// Define static members
bool ImGuiManager::m_isInitialized = false;

// NEW MEMBERS for UI-side timeout logic
std::chrono::steady_clock::time_point ImGuiManager::m_connectingStartTime;
bool ImGuiManager::m_isWaitingForConnection = false;

bool ImGuiManager::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;

    kx::GUI::LoadAppFont();
    kx::GUI::ApplyCustomStyle();

    if (!ImGui_ImplWin32_Init(hwnd)) return false;
    if (!ImGui_ImplDX11_Init(device, context)) return false;

    m_isInitialized = true;
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

void ImGuiManager::RenderESPWindow(kx::MumbleLinkManager& mumbleLinkManager, const kx::MumbleLinkData* mumbleData) {
    // Use AppState singleton instead of global variable
    if (!kx::AppState::Get().IsVisionWindowOpen()) return;

    // Get settings reference once for the entire function
    auto& settings = kx::AppState::Get().GetSettings();

    std::string windowTitle = "KX Vision v";
    windowTitle += kx::APP_VERSION.data();

    // Set larger initial window size and center it on first use
    ImVec2 initialSize(600, 450);
    ImGui::SetNextWindowSize(initialSize, ImGuiCond_FirstUseEver);
    
    // Center the window on first spawn
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
    ImGui::SetNextWindowPos(ImVec2(center.x - initialSize.x * 0.5f, center.y - initialSize.y * 0.5f), ImGuiCond_FirstUseEver);
    
    // Pass a direct pointer to the singleton's vision window state
    ImGui::Begin(windowTitle.c_str(), kx::AppState::Get().GetVisionWindowOpenRef());

    RenderHints();

    auto status = mumbleLinkManager.GetStatus();
    uint32_t mapId = mumbleLinkManager.mapId();

    switch (status) {
        case kx::MumbleLinkManager::MumbleStatus::Connected:
            m_isWaitingForConnection = false; // We are connected, so reset the timer flag.
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "MumbleLink Status: Connected");
            if (mapId != 0) {
                ImGui::SameLine(); ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "| In-Map");
            } else {
                ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "| Waiting for map...");
            }
            break;

        case kx::MumbleLinkManager::MumbleStatus::Connecting:
            if (!m_isWaitingForConnection) {
                // This is the first frame we've been in the "Connecting" state. Start the timer.
                m_isWaitingForConnection = true;
                m_connectingStartTime = std::chrono::steady_clock::now();
            }

            // Check if the timer has expired.
            if (std::chrono::steady_clock::now() - m_connectingStartTime > std::chrono::seconds(15)) {
                // Timer expired. We are in the "Stale" state from the UI's perspective.
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "MumbleLink Status: Connection Failed");
                ImGui::Separator();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.8f, 1.0f));
                ImGui::TextWrapped("The tool is connected but not receiving live data. This commonly happens when using Gw2Launcher with a custom 'Mumble link name'.");
                ImGui::Spacing();
                ImGui::Text("SOLUTION:");
                ImGui::BulletText("In Gw2Launcher, open the settings for your account.");
                ImGui::BulletText("Find the 'Mumble link name' option.");
                ImGui::BulletText("Uncheck the box to disable it and use the default name.");
                ImGui::PopStyleColor();
            } else {
                // Timer has not expired. Show a generic connecting/disconnected message.
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "MumbleLink Status: Connecting...");
            }
            break;
            
        case kx::MumbleLinkManager::MumbleStatus::Disconnected:
        default:
            m_isWaitingForConnection = false; // Reset the timer flag.
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "MumbleLink Status: Disconnected");
            break;
    }
    
    ImGui::Separator();

    if (ImGui::BeginTabBar("##ESPCategories")) {
	    kx::GUI::RenderPlayersTab();
	    kx::GUI::RenderNPCsTab();
	    kx::GUI::RenderObjectsTab();
	    kx::GUI::RenderAppearanceTab();
	    kx::GUI::RenderSettingsTab();
	    kx::GUI::RenderInfoTab();

#ifdef _DEBUG
        kx::GUI::RenderValidationTab();
#endif

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void ImGuiManager::RenderUI(kx::Camera& camera, 
                            kx::MumbleLinkManager& mumbleLinkManager,
                            const kx::MumbleLinkData* mumbleLinkData,
                            HWND windowHandle,
                            float displayWidth,
                            float displayHeight) {
    // Render the ESP overlay
    kx::ESPRenderer::Render(displayWidth, displayHeight, mumbleLinkData);
    
    // Render the UI window if it's shown (check AppState's unified visibility flag)
    if (kx::AppState::Get().IsVisionWindowOpen()) {
        RenderESPWindow(mumbleLinkManager, mumbleLinkData);
    }
}

void ImGuiManager::RenderHints() {
    // Display keyboard shortcuts with consistent styling
#ifdef GW2AL_BUILD
    const char* hints[] = {
        "Press INSERT to show/hide window."
    };
#else
    const char* hints[] = {
        "Press INSERT to show/hide window.",
        "Press DELETE to unload DLL."
    };
#endif
    
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
    
    m_isInitialized = false;
}
