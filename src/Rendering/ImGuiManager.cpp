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
#include "../Game/ReClassStructs.h"
#include "../Hooking/D3DRenderHook.h"
#include "../Utils/DebugLogger.h"

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

    // Connection status
    ImGui::Text("MumbleLink Status: %s", 
        m_mumbleLinkManager.IsInitialized() ? "Connected" : "Disconnected");
    ImGui::Separator(); // Add a separator for visual clarity

    if (ImGui::BeginTabBar("##ESPCategories")) {
        RenderPlayersTab();
        RenderNPCsTab();
        RenderObjectsTab();
        RenderAppearanceTab();
        RenderSettingsTab();
        RenderInfoTab();
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void ImGuiManager::RenderPlayersTab() {
    if (ImGui::BeginTabItem("Players")) {
        auto& settings = kx::AppState::Get().GetSettings();
        
        ImGui::Checkbox("Enable Player ESP", &settings.playerESP.enabled);
        
        if (settings.playerESP.enabled) {
            ImGui::Separator();
            ImGui::Text("Player Filter Options");
            ImGui::Checkbox("Show Local Player", &settings.playerESP.showLocalPlayer);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Show your own character in the ESP overlay.");
            }
        }
        ImGui::EndTabItem();
    }
}

void ImGuiManager::RenderNPCsTab() {
    if (ImGui::BeginTabItem("NPCs")) {
        auto& settings = kx::AppState::Get().GetSettings();
        
        ImGui::Checkbox("Enable NPC ESP", &settings.npcESP.enabled);
        
        if (settings.npcESP.enabled) {
            ImGui::Separator();
            ImGui::Text("Attitude Filter");
            
            ImGui::Checkbox("Show Friendly", &settings.npcESP.showFriendly);
            ImGui::SameLine();
            ImGui::Checkbox("Show Hostile", &settings.npcESP.showHostile);
            ImGui::SameLine();
            ImGui::Checkbox("Show Neutral", &settings.npcESP.showNeutral);
            ImGui::Checkbox("Show Indifferent", &settings.npcESP.showIndifferent);
            
            ImGui::Separator();
            ImGui::Text("Health Filter");
            ImGui::Checkbox("Show Dead NPCs", &settings.npcESP.showDeadNpcs);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Show NPCs with 0 HP (defeated enemies, corpses).\nUseful for loot opportunities and understanding combat situations.");
            }
        }
        ImGui::EndTabItem();
    }
}

void ImGuiManager::RenderObjectsTab() {
    if (ImGui::BeginTabItem("Objects")) {
        auto& settings = kx::AppState::Get().GetSettings();
        
        ImGui::Checkbox("Enable Object ESP", &settings.objectESP.enabled);
        
        if (settings.objectESP.enabled) {
            ImGui::Separator();
            ImGui::Text("Object Type Filter");
            
            // Resource and Collection Objects
            ImGui::Checkbox("Resource Nodes", &settings.objectESP.showResourceNodes);
            ImGui::SameLine();
            ImGui::Checkbox("Crafting Stations", &settings.objectESP.showCraftingStations);
            
            // Navigation and Points of Interest
            ImGui::Checkbox("Waypoints", &settings.objectESP.showWaypoints);
            ImGui::SameLine();
            ImGui::Checkbox("Vistas", &settings.objectESP.showVistas);
            ImGui::SameLine();
            ImGui::Checkbox("Portals", &settings.objectESP.showPortals);
            
            // Interactive Objects
            ImGui::Checkbox("Interactables", &settings.objectESP.showInteractables);
            ImGui::SameLine();
            ImGui::Checkbox("Doors", &settings.objectESP.showDoors);
            
            // Combat and Player-related
            ImGui::Checkbox("Attack Targets", &settings.objectESP.showAttackTargets);
            ImGui::SameLine();
            ImGui::Checkbox("Player Created", &settings.objectESP.showPlayerCreated);
            
            ImGui::Separator();
            ImGui::Text("Quick Selection");
            if (ImGui::Button("Select Important")) {
                // Set individual checkboxes based on important gadget types
                settings.objectESP.showResourceNodes = true;
                settings.objectESP.showWaypoints = true;
                settings.objectESP.showVistas = true;
                settings.objectESP.showAttackTargets = true;
                settings.objectESP.showInteractables = true;
                settings.objectESP.showCraftingStations = false;
                settings.objectESP.showPlayerCreated = false;
                settings.objectESP.showDoors = false;
                settings.objectESP.showPortals = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Select All")) {
                // Enable all object types
                settings.objectESP.showResourceNodes = true;
                settings.objectESP.showWaypoints = true;
                settings.objectESP.showVistas = true;
                settings.objectESP.showCraftingStations = true;
                settings.objectESP.showAttackTargets = true;
                settings.objectESP.showPlayerCreated = true;
                settings.objectESP.showInteractables = true;
                settings.objectESP.showDoors = true;
                settings.objectESP.showPortals = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("Clear All")) {
                // Disable all object types
                settings.objectESP.showResourceNodes = false;
                settings.objectESP.showWaypoints = false;
                settings.objectESP.showVistas = false;
                settings.objectESP.showCraftingStations = false;
                settings.objectESP.showAttackTargets = false;
                settings.objectESP.showPlayerCreated = false;
                settings.objectESP.showInteractables = false;
                settings.objectESP.showDoors = false;
                settings.objectESP.showPortals = false;
            }
            
            ImGui::Separator();
            ImGui::Text("Special Filters");
            ImGui::Checkbox("Hide Depleted Resource Nodes", &settings.hideDepletedNodes);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Hide resource nodes that cannot be gathered (depleted).");
            }
        }
        ImGui::EndTabItem();
    }
}

void ImGuiManager::RenderAppearanceTab() {
    if (ImGui::BeginTabItem("Appearance")) {
        auto& settings = kx::AppState::Get().GetSettings();
        
        ImGui::Text("Visual Style Settings");
        ImGui::Separator();
        
        // Global Style Settings
        if (ImGui::CollapsingHeader("Global Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("Use Distance Limit", &settings.espUseDistanceLimit);
            if (settings.espUseDistanceLimit) {
                ImGui::SliderFloat("Render Distance Limit", &settings.espRenderDistanceLimit, 10.0f, 2000.0f, "%.0fm");
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Entities beyond this distance will not be rendered.\nEntities fade out in the last 11% of this distance.");
                }
            }
            
            ImGui::Separator();
            ImGui::Text("ESP Scaling Configuration");
            
            ImGui::SliderFloat("Min Scale", &settings.espMinScale, 0.1f, 1.0f, "%.2f");
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Minimum scale factor for very distant entities.\nLower values = smaller distant entities.");
            }
            
            ImGui::SliderFloat("Max Scale", &settings.espMaxScale, 1.0f, 5.0f, "%.2f"); 
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Maximum scale factor for very close entities.\nHigher values = larger close entities.");
            }
            
            ImGui::SliderFloat("Scale Factor", &settings.espScaleFactor, 10.0f, 100.0f, "%.0f");
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Base scaling divisor that controls overall entity size.\nHigher values = smaller entities overall.\nDefault: 35 (matches original fixed sizes at typical distances)");
            }
        }
        
        // Player Style Settings
        if (ImGui::CollapsingHeader("Player Style", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Visual Elements");
            ImGui::Checkbox("Show Box##Player", &settings.playerESP.renderBox);
            ImGui::SameLine();
            ImGui::Checkbox("Show Distance##Player", &settings.playerESP.renderDistance);
            ImGui::SameLine();
            ImGui::Checkbox("Show Dot##Player", &settings.playerESP.renderDot);
            
            ImGui::Checkbox("Show Health Bar##Player", &settings.playerESP.renderHealthBar);
            ImGui::SameLine();
            ImGui::Checkbox("Show Details##Player", &settings.playerESP.renderDetails);
            ImGui::Checkbox("Show Player Name##Player", &settings.playerESP.renderPlayerName);
        }
        
        // NPC Style Settings
        if (ImGui::CollapsingHeader("NPC Style", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Visual Elements");
            ImGui::Checkbox("Show Box##NPC", &settings.npcESP.renderBox);
            ImGui::SameLine();
            ImGui::Checkbox("Show Distance##NPC", &settings.npcESP.renderDistance);
            ImGui::SameLine();
            ImGui::Checkbox("Show Dot##NPC", &settings.npcESP.renderDot);
            
            ImGui::Checkbox("Show Health Bar##NPC", &settings.npcESP.renderHealthBar);
            ImGui::SameLine();
            ImGui::Checkbox("Show Details##NPC", &settings.npcESP.renderDetails);
        }
        
        // Object Style Settings
        if (ImGui::CollapsingHeader("Object Style", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Visual Elements");
            ImGui::Checkbox("Show Box##Object", &settings.objectESP.renderBox);
            ImGui::SameLine();
            ImGui::Checkbox("Show Distance##Object", &settings.objectESP.renderDistance);
            ImGui::SameLine();
            ImGui::Checkbox("Show Dot##Object", &settings.objectESP.renderDot);
            
            ImGui::Checkbox("Show Details##Object", &settings.objectESP.renderDetails);
        }
        ImGui::EndTabItem();
    }
}

void ImGuiManager::RenderSettingsTab() {
    if (ImGui::BeginTabItem("Settings")) {
        auto& settings = kx::AppState::Get().GetSettings();
        
        ImGui::Text("System Configuration");
        ImGui::Separator();
        
        // Performance Settings
        if (ImGui::CollapsingHeader("Performance", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::SliderFloat("ESP Update Rate", &settings.espUpdateRate, 15.0f, 120.0f, "%.0f FPS");
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Lower values improve performance but make ESP less responsive.\nRecommended: 30-60 FPS for good balance.");
            }
        }
        
        // Debug Settings
        if (ImGui::CollapsingHeader("Debug Options")) {
            // Access debug logging through AppState singleton
            ImGui::Checkbox("Enable Debug Logging", &settings.enableDebugLogging);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Enable detailed logging to console and kx_debug.log file.\nHelps diagnose crashes and memory access issues.");
            }

            // Log level selection
            if (settings.enableDebugLogging) {
                ImGui::Separator();
                ImGui::Text("Log Level:");
                
                static int currentLogLevel = 3; // Default to ERROR (index 3)
                const char* logLevels[] = { "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL" };
                
                if (ImGui::Combo("##LogLevel", &currentLogLevel, logLevels, IM_ARRAYSIZE(logLevels))) {
                    // Update log level when changed
                    kx::Debug::Logger::SetMinLogLevel(static_cast<kx::Debug::Logger::Level>(currentLogLevel));
                }
                
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("DEBUG: Show all logs (very verbose)\n"
                                    "INFO: Show info and above\n"
                                    "WARNING: Show warnings and above\n"
                                    "ERROR: Show only errors and critical (recommended)\n"
                                    "CRITICAL: Show only critical errors");
                }
            }
        }

#ifdef _DEBUG
        // Debug Info (Development only)
        if (ImGui::CollapsingHeader("Debug Info")) {
            // Context Collection
            void* pContextCollection = kx::AddressManager::GetContextCollectionPtr();
            char ctxCollectionAddrStr[32];
            snprintf(ctxCollectionAddrStr, sizeof(ctxCollectionAddrStr), "0x%p", pContextCollection);
            ImGui::Text("ContextCollection:");
            ImGui::PushItemWidth(-1.0f);
            ImGui::InputText("##ContextCollectionAddr", ctxCollectionAddrStr, sizeof(ctxCollectionAddrStr), ImGuiInputTextFlags_ReadOnly);
            ImGui::PopItemWidth();

            if (pContextCollection) {
                kx::ReClass::ContextCollection ctxCollection(pContextCollection);

                // Character Context
                kx::ReClass::ChCliContext charContext = ctxCollection.GetChCliContext();
                char charCtxAddrStr[32];
                snprintf(charCtxAddrStr, sizeof(charCtxAddrStr), "0x%p", charContext.data());
                ImGui::Text("ChCliContext:");
                ImGui::PushItemWidth(-1.0f);
                ImGui::InputText("##CharContextAddr", charCtxAddrStr, sizeof(charCtxAddrStr), ImGuiInputTextFlags_ReadOnly);
                ImGui::PopItemWidth();

                if (charContext) {
                    // Character List
                    kx::ReClass::ChCliCharacter** characterList = charContext.GetCharacterList();
                    uint32_t characterCapacity = charContext.GetCharacterListCapacity();
                    char charListAddrStr[32];
                    snprintf(charListAddrStr, sizeof(charListAddrStr), "0x%p", (void*)characterList);
                    ImGui::Text("CharacterList (Capacity: %u):", characterCapacity);
                    ImGui::PushItemWidth(-1.0f);
                    ImGui::InputText("##CharListAddr", charListAddrStr, sizeof(charListAddrStr), ImGuiInputTextFlags_ReadOnly);
                    ImGui::PopItemWidth();

                    // Player List
                    kx::ReClass::ChCliPlayer** playerList = charContext.GetPlayerList();
                    uint32_t playerListSize = charContext.GetPlayerListSize();
                    char playerListAddrStr[32];
                    snprintf(playerListAddrStr, sizeof(playerListAddrStr), "0x%p", (void*)playerList);
                    ImGui::Text("PlayerList (Size: %u):", playerListSize);
                    ImGui::PushItemWidth(-1.0f);
                    ImGui::InputText("##PlayerListAddr", playerListAddrStr, sizeof(playerListAddrStr), ImGuiInputTextFlags_ReadOnly);
                    ImGui::PopItemWidth();
                }

                ImGui::Separator();

                // Gadget Context
                kx::ReClass::GdCliContext gadgetCtx = ctxCollection.GetGdCliContext();
                char gadgetCtxAddrStr[32];
                snprintf(gadgetCtxAddrStr, sizeof(gadgetCtxAddrStr), "0x%p", gadgetCtx.data());
                ImGui::Text("GdCliContext:");
                ImGui::PushItemWidth(-1.0f);
                ImGui::InputText("##GadgetContextAddr", gadgetCtxAddrStr, sizeof(gadgetCtxAddrStr), ImGuiInputTextFlags_ReadOnly);
                ImGui::PopItemWidth();

                if (gadgetCtx) {
                    // Gadget List
                    kx::ReClass::GdCliGadget** gadgetList = gadgetCtx.GetGadgetList();
                    uint32_t gadgetCapacity = gadgetCtx.GetGadgetListCapacity();
                    uint32_t gadgetCount = gadgetCtx.GetGadgetListCount();
                    char gadgetListAddrStr[32];
                    snprintf(gadgetListAddrStr, sizeof(gadgetListAddrStr), "0x%p", (void*)gadgetList);
                    ImGui::Text("GadgetList (Count: %u / Capacity: %u):", gadgetCount, gadgetCapacity);
                    ImGui::PushItemWidth(-1.0f);
                    ImGui::InputText("##GadgetListAddr", gadgetListAddrStr, sizeof(gadgetListAddrStr), ImGuiInputTextFlags_ReadOnly);
                    ImGui::PopItemWidth();
                }
            } else {
                ImGui::Text("ContextCollection not available.");
            }
        }
#endif
        ImGui::EndTabItem();
    }
}

void ImGuiManager::RenderInfoTab() {
    if (ImGui::BeginTabItem("Info")) {
        ImGui::Text("About KX Vision");
        ImGui::Separator();
        
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
        ImGui::EndTabItem();
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
    if (kx::AppState::Get().GetSettings().showVisionWindow) {
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
