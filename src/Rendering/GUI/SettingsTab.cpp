#include "SettingsTab.h"
#include <string>
#include <algorithm>
#include "../../../libs/ImGui/imgui.h"
#include "../../Core/AppState.h"
#include "../../Core/SettingsManager.h"
#include "../../Utils/DebugLogger.h"
#include "../../Game/AddressManager.h"
#include "../../Game/ReClassStructs.h"
#include "../../Core/Config.h"

namespace kx {
    namespace GUI {
        
        // Helper function to parse log level from formatted log string
        static std::string ExtractLogLevel(const std::string& logLine) {
            // Format: [HH:MM:SS.mmm] [level] message
            size_t firstBracket = logLine.find(']');
            if (firstBracket != std::string::npos && firstBracket + 2 < logLine.length()) {
                size_t secondBracket = logLine.find(']', firstBracket + 1);
                if (secondBracket != std::string::npos) {
                    return logLine.substr(firstBracket + 2, secondBracket - firstBracket - 2);
                }
            }
            return "info"; // Default fallback
        }
        
        // Helper function to get color for log level
        static ImVec4 GetLogLevelColor(const std::string& level) {
            if (level == "debug") return ImVec4(0.7f, 0.7f, 0.7f, 1.0f);      // Gray
            if (level == "info") return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);       // White
            if (level == "warn") return ImVec4(1.0f, 0.8f, 0.0f, 1.0f);       // Yellow
            if (level == "err") return ImVec4(1.0f, 0.3f, 0.3f, 1.0f);        // Red
            if (level == "critical") return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);   // Bright red
            return ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // Default white
        }
        
        void RenderLogViewer() {
            // Cache logs for performance - only fetch when count changes
            static std::vector<std::string> cachedLogs;
            static size_t lastLogCount = 0;
            
            // Check if we need to refresh logs
            auto currentLogs = Debug::Logger::GetRecentLogs();
            if (currentLogs.size() != lastLogCount) {
                cachedLogs = std::move(currentLogs);
                lastLogCount = cachedLogs.size();
            }
            
            // Log viewer controls
            ImGui::Text("Showing %zu logs", cachedLogs.size());
            ImGui::SameLine();
            if (ImGui::Button("Copy All")) {
                std::string allLogs;
                for (const auto& log : cachedLogs) {
                    allLogs += log + "\n";
                }
                ImGui::SetClipboardText(allLogs.c_str());
            }
            
            // Log display area
            ImGui::Separator();
            if (ImGui::BeginChild("LogViewer", ImVec2(0, 200), true, ImGuiWindowFlags_HorizontalScrollbar)) {
                for (const auto& log : cachedLogs) {
                    std::string level = ExtractLogLevel(log);
                    ImVec4 color = GetLogLevelColor(level);
                    
                    ImGui::PushStyleColor(ImGuiCol_Text, color);
                    ImGui::TextUnformatted(log.c_str());
                    ImGui::PopStyleColor();
                }
                
                ImGui::EndChild();
            }
        }
        
        void RenderSettingsTab() {
            if (ImGui::BeginTabItem("Settings")) {
                auto& settings = AppState::Get().GetSettings();

                // NEW SECTION
                if (ImGui::CollapsingHeader("Settings Management")) {
                    if (ImGui::Button("Save Settings")) {
                        SettingsManager::Save(settings);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Reload Settings")) {
                        SettingsManager::Load(settings);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Reset to Defaults")) {
                        ImGui::OpenPopup("Confirm Reset");
                    }

                    // Logic for the confirmation popup modal
                    if (ImGui::BeginPopupModal("Confirm Reset", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                        ImGui::Text("Are you sure? This will reset all settings to their default values.");
                        ImGui::Separator();
                        if (ImGui::Button("OK", ImVec2(120, 0))) { 
                            settings = Settings(); // Reset to default
                            ImGui::CloseCurrentPopup(); 
                        }
                        ImGui::SetItemDefaultFocus();
                        ImGui::SameLine();
                        if (ImGui::Button("Cancel", ImVec2(120, 0))) { 
                            ImGui::CloseCurrentPopup(); 
                        }
                        ImGui::EndPopup();
                    }

                    ImGui::Separator(); // Visually separate the buttons from the option
                    ImGui::Checkbox("Automatically save settings on exit", &settings.autoSaveOnExit);
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("If enabled, any changes you make will be saved automatically when the game closes.\nIf disabled, you must use the 'Save Settings' button to persist changes.");
                    }
                }

                ImGui::Separator();
                ImGui::Text("System Configuration");
                ImGui::Separator();
                
                // Performance Settings
                if (ImGui::CollapsingHeader("Performance", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::SliderFloat("ESP Update Rate", &settings.espUpdateRate, 30.0f, 360.0f, "%.0f FPS");
                    ImGui::SameLine();
                    ImGui::TextDisabled("(?)");
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Lower values improve performance but make ESP less responsive.\nRecommended: 60-120 FPS for good balance, up to 360 FPS for high refresh displays.");
                    }
                }
                
                // Debug Settings
                if (ImGui::CollapsingHeader("Debug Options")) {
                    // Access debug logging through AppState singleton
                    ImGui::Checkbox("Enable Debug Logging", &settings.enableDebugLogging);
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Enable detailed logging to console and kx_debug.log file.\nHelps diagnose crashes and memory access issues.");
                    }

#ifdef _DEBUG
                    ImGui::Checkbox("Show Debug Addresses", &settings.showDebugAddresses);
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Show entity memory addresses on the ESP overlay.");
                    }
#endif

                    // Log level selection (always visible)
                    ImGui::Separator();
                    ImGui::Text("Log Level:");
                    
                    static int currentLogLevel = static_cast<int>(AppConfig::DEFAULT_LOG_LEVEL);
                    const char* logLevels[] = { "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL" };
                    
                    if (ImGui::Combo("##LogLevel", &currentLogLevel, logLevels, IM_ARRAYSIZE(logLevels))) {
                        // Update log level when changed
                        Debug::Logger::SetMinLogLevel(static_cast<Debug::Logger::Level>(currentLogLevel));
                    }
                    
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("DEBUG: Show all logs (very verbose)\n" 
                                        "INFO: Show info and above\n" 
                                        "WARNING: Show warnings and above\n" 
                                        "ERROR: Show only errors and critical (recommended)\n" 
                                        "CRITICAL: Show only critical errors");
                    }
                }

                // Log Viewer (only show when debug logging is enabled)
                if (settings.enableDebugLogging) {
                    if (ImGui::CollapsingHeader("Log Viewer")) {
                        RenderLogViewer();
                    }
                }

#ifdef _DEBUG
                // Debug Info (Development only)
                if (ImGui::CollapsingHeader("Debug Info")) {
                    // Context Collection
                    void* pContextCollection = AddressManager::GetContextCollectionPtr();
                    char ctxCollectionAddrStr[32];
                    snprintf(ctxCollectionAddrStr, sizeof(ctxCollectionAddrStr), "0x%p", pContextCollection);
                    ImGui::Text("ContextCollection:");
                    ImGui::PushItemWidth(-1.0f);
                    ImGui::InputText("##ContextCollectionAddr", ctxCollectionAddrStr, sizeof(ctxCollectionAddrStr), ImGuiInputTextFlags_ReadOnly);
                    ImGui::PopItemWidth();

                    if (pContextCollection) {
                        ReClass::ContextCollection ctxCollection(pContextCollection);

                        // Character Context
                        ReClass::ChCliContext charContext = ctxCollection.GetChCliContext();
                        char charCtxAddrStr[32];
                        snprintf(charCtxAddrStr, sizeof(charCtxAddrStr), "0x%p", charContext.data());
                        ImGui::Text("ChCliContext:");
                        ImGui::PushItemWidth(-1.0f);
                        ImGui::InputText("##CharContextAddr", charCtxAddrStr, sizeof(charCtxAddrStr), ImGuiInputTextFlags_ReadOnly);
                        ImGui::PopItemWidth();

                        if (charContext) {
                            // Character List
                            ReClass::ChCliCharacter** characterList = charContext.GetCharacterList();
                            uint32_t characterCapacity = charContext.GetCharacterListCapacity();
                            char charListAddrStr[32];
                            snprintf(charListAddrStr, sizeof(charListAddrStr), "0x%p", (void*)characterList);
                            ImGui::Text("CharacterList (Capacity: %u):", characterCapacity);
                            ImGui::PushItemWidth(-1.0f);
                            ImGui::InputText("##CharListAddr", charListAddrStr, sizeof(charListAddrStr), ImGuiInputTextFlags_ReadOnly);
                            ImGui::PopItemWidth();

                            // Player List
                            ReClass::ChCliPlayer** playerList = charContext.GetPlayerList();
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
                        ReClass::GdCliContext gadgetCtx = ctxCollection.GetGdCliContext();
                        char gadgetCtxAddrStr[32];
                        snprintf(gadgetCtxAddrStr, sizeof(gadgetCtxAddrStr), "0x%p", gadgetCtx.data());
                        ImGui::Text("GdCliContext:");
                        ImGui::PushItemWidth(-1.0f);
                        ImGui::InputText("##GadgetContextAddr", gadgetCtxAddrStr, sizeof(gadgetCtxAddrStr), ImGuiInputTextFlags_ReadOnly);
                        ImGui::PopItemWidth();

                        if (gadgetCtx) {
                            // Gadget List
                            ReClass::GdCliGadget** gadgetList = gadgetCtx.GetGadgetList();
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
