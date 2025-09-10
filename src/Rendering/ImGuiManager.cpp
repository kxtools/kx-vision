#define NOMINMAX

#include "ImGuiManager.h"

#include <string>
#include <windows.h>

#include "ESPRenderer.h"
#include "GuiStyle.h"
#include "../../libs/ImGui/imgui.h"
#include "../../libs/ImGui/imgui_impl_dx11.h"
#include "../../libs/ImGui/imgui_impl_win32.h"
#include "../Core/AppState.h"
#include "../Core/Config.h"
#include "../Game/AddressManager.h"
#include "../Game/GameStructs.h"
#include "../Hooking/D3DRenderHook.h"

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
    ImGui::Checkbox("Render Agents", &kx::g_settings.espRenderAgents);
    ImGui::Checkbox("Render Characters", &kx::g_settings.espRenderCharacters);

    // ESP feature toggles with side-by-side layout
    ImGui::Checkbox("Render Box", &kx::g_settings.espRenderBox);
    ImGui::SameLine();
    ImGui::Checkbox("Render Distance", &kx::g_settings.espRenderDistance);
    ImGui::SameLine();
    ImGui::Checkbox("Render Dot", &kx::g_settings.espRenderDot);
    ImGui::SameLine();
    ImGui::Checkbox("ESP Details", &kx::g_settings.espRenderDetails);

    ImGui::Checkbox("Use Distance Limit", &kx::g_settings.espUseDistanceLimit);
    if (kx::g_settings.espUseDistanceLimit) {
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::SliderFloat("Render Distance Limit", &kx::g_settings.espRenderDistanceLimit, 10.0f, 2000.0f, "%.0fm");
        ImGui::PopItemWidth();
    }

    // Connection status
    ImGui::Text("MumbleLink Status: %s", 
        m_mumbleLinkManager.IsInitialized() ? "Connected" : "Disconnected");

    // Visual separator
    ImGui::Separator();

    // Additional information section
    RenderInfoSection();

    // Debug information section (only in debug builds)
    RenderDebugSection();

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

void ImGuiManager::RenderDebugSection() {
#ifdef _DEBUG
    if (ImGui::CollapsingHeader("Debug Info")) {
        uintptr_t agentArrayAddr = kx::AddressManager::GetAgentArray();
        uintptr_t worldViewCtxAddr = kx::AddressManager::GetWorldViewContextPtr();

        char addrStr1[32];
        char addrStr2[32];
        snprintf(addrStr1, sizeof(addrStr1), "0x%p", (void*)agentArrayAddr);
        snprintf(addrStr2, sizeof(addrStr2), "0x%p", (void*)worldViewCtxAddr);

        ImGui::Text("AgentArray:");
        ImGui::PushItemWidth(-1.0f);
        ImGui::InputText("##AgentArrayAddr", addrStr1, sizeof(addrStr1), ImGuiInputTextFlags_ReadOnly);
        ImGui::PopItemWidth();

        ImGui::Text("WorldViewContext:");
        ImGui::PushItemWidth(-1.0f);
        ImGui::InputText("##WorldViewContextAddr", addrStr2, sizeof(addrStr2), ImGuiInputTextFlags_ReadOnly);
        ImGui::PopItemWidth();

        uint32_t agentCount = 0;
        uint32_t agentCapacity = 0;
        kx::Agent firstAgent(nullptr);
        int firstAgentIndex = -1; // Variable to store the index

        if (agentArrayAddr) {
            kx::AgentArray agentArray(reinterpret_cast<void*>(agentArrayAddr));
            agentCount = agentArray.Count();
            agentCapacity = agentArray.Capacity();

            // Find the first valid agent to display its info
            for (uint32_t i = 0; i < agentCount; ++i) {
                kx::Agent agent = agentArray.GetAgent(i);
                if (agent) {
                    firstAgent = agent;
                    firstAgentIndex = i; // Store the index
                    break;
                }
            }
        }
        ImGui::Text("Agents: %u / %u", agentCount, agentCapacity);

        ImGui::Separator();
        ImGui::Text("First Valid Agent Details:");
        if (firstAgent) {
            ImGui::Text("Index: %d (0x%X)", firstAgentIndex, firstAgentIndex);
            ImGui::Text("ID: %u (0x%X)", firstAgent.GetId(), firstAgent.GetId());

            // Display memory address of the first agent
            char agentAddrStr[32];
            snprintf(agentAddrStr, sizeof(agentAddrStr), "0x%p", (void*)firstAgent.GetAddress());
            ImGui::Text("Address:");
            ImGui::PushItemWidth(-1.0f);
            ImGui::InputText("##AgentAddress", agentAddrStr, sizeof(agentAddrStr), ImGuiInputTextFlags_ReadOnly);
            ImGui::PopItemWidth();

            ImGui::Text("Type: %d", firstAgent.GetType());
            ImGui::Text("Gadget Type: %d", firstAgent.GetGadgetType());
        } else {
            ImGui::Text("No valid agents found in array.");
        }
    }
#endif
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
