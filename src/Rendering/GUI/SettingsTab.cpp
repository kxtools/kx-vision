#include "SettingsTab.h"
#include <string>
#include "../../../libs/ImGui/imgui.h"
#include "../../Core/AppState.h"
#include "../../Core/Config.h"
#include "../../Utils/DebugLogger.h"
#include "../../Game/AddressManager.h"
#include "../../Game/ReClassStructs.h"

namespace kx {
    namespace GUI {
        void RenderSettingsTab() {
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
                        
                        static int currentLogLevel = static_cast<int>(kx::AppConfig::DEFAULT_LOG_LEVEL);
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
    }
}
