#include "SettingsTab.h"
#include <string>
#include <algorithm>
#include <format>
#include <map>

#include "DebugLogger.h"
#include "SettingsManager.h"

#include "ImGui/imgui.h"
#include "SDK/ContextStructs.h"
#include "SDK/GadgetStructs.h"
#include "../../../Utils/SafeIterators.h"
#include "../../../Rendering/Presentation/Formatting.h"
#include "../../../Game/AddressManager.h"

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
                        Debug::Logger::SetMinLogLevel(static_cast<Debug::Logger::Level>(settings.logLevel));
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
                            Debug::Logger::SetMinLogLevel(static_cast<Debug::Logger::Level>(settings.logLevel));
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
                    
#ifdef _DEBUG
                    // Debug builds: Allow DEBUG level
                    int currentLogLevel = static_cast<int>(Debug::Logger::GetMinLogLevel());
                    const char* logLevels[] = { "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL" };
                    
                    if (ImGui::Combo("##LogLevel", &currentLogLevel, logLevels, IM_ARRAYSIZE(logLevels))) {
                        Debug::Logger::SetMinLogLevel(static_cast<Debug::Logger::Level>(currentLogLevel));
                        auto& settings = AppState::Get().GetSettings();
                        settings.logLevel = currentLogLevel;
                    }
                    
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("DEBUG: Show all logs (very verbose)\n" 
                                        "INFO: Show info and above\n" 
                                        "WARNING: Show warnings and above\n" 
                                        "ERROR: Show only errors and critical (recommended)\n" 
                                        "CRITICAL: Show only critical errors");
                    }
#else
                    // Release builds: Start from INFO (skip DEBUG)
                    int currentLogLevel = static_cast<int>(Debug::Logger::GetMinLogLevel());
                    const char* logLevels[] = { "INFO", "WARNING", "ERROR", "CRITICAL" };
                    
                    // Adjust index: Logger uses 0=DEBUG, 1=INFO, but Release array starts at INFO
                    int comboIndex = std::max(0, currentLogLevel - 1);
                    
                    if (ImGui::Combo("##LogLevel", &comboIndex, logLevels, IM_ARRAYSIZE(logLevels))) {
                        // Map back: combo index 0=INFO (logger level 1), 1=WARNING (logger level 2), etc.
                        int actualLogLevel = comboIndex + 1;
                        Debug::Logger::SetMinLogLevel(static_cast<Debug::Logger::Level>(actualLogLevel));
                        auto& settings = AppState::Get().GetSettings();
                        settings.logLevel = actualLogLevel;
                    }
                    
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("INFO: Show info and above\n" 
                                        "WARNING: Show warnings and above\n" 
                                        "ERROR: Show only errors and critical (recommended)\n" 
                                        "CRITICAL: Show only critical errors");
                    }
#endif
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
                    std::string ctxCollectionAddrStr = std::format("0x{:X}", reinterpret_cast<uintptr_t>(pContextCollection));
                    ImGui::Text("ContextCollection:");
                    ImGui::PushItemWidth(-1.0f);
                    ImGui::InputText("##ContextCollectionAddr", ctxCollectionAddrStr.data(), ctxCollectionAddrStr.size() + 1, ImGuiInputTextFlags_ReadOnly);
                    ImGui::PopItemWidth();

                    if (pContextCollection) {
                        ReClass::ContextCollection ctxCollection(pContextCollection);

                        // Character Context
                        ReClass::ChCliContext charContext = ctxCollection.GetChCliContext();
                        std::string charCtxAddrStr = std::format("0x{:X}", reinterpret_cast<uintptr_t>(charContext.data()));
                        ImGui::Text("ChCliContext:");
                        ImGui::PushItemWidth(-1.0f);
                        ImGui::InputText("##CharContextAddr", charCtxAddrStr.data(), charCtxAddrStr.size() + 1, ImGuiInputTextFlags_ReadOnly);
                        ImGui::PopItemWidth();

                        if (charContext) {
                            // Character List
                            ReClass::ChCliCharacter** characterList = charContext.GetCharacterList();
                            uint32_t characterCapacity = charContext.GetCharacterListCapacity();
                            uint32_t characterCount = charContext.GetCharacterListCount();
                            std::string charListAddrStr = std::format("0x{:X}", reinterpret_cast<uintptr_t>(characterList));
                            ImGui::Text("CharacterList (Count: %u / Capacity: %u):", characterCount, characterCapacity);
                            ImGui::PushItemWidth(-1.0f);
                            ImGui::InputText("##CharListAddr", charListAddrStr.data(), charListAddrStr.size() + 1, ImGuiInputTextFlags_ReadOnly);
                            ImGui::PopItemWidth();

                            // Player List
                            ReClass::ChCliPlayer** playerList = charContext.GetPlayerList();
                            uint32_t playerCapacity = charContext.GetPlayerListCapacity();
                            uint32_t playerCount = charContext.GetPlayerListCount();
                            std::string playerListAddrStr = std::format("0x{:X}", reinterpret_cast<uintptr_t>(playerList));
                            ImGui::Text("PlayerList (Count: %u / Capacity: %u):", playerCount, playerCapacity);
                            ImGui::PushItemWidth(-1.0f);
                            ImGui::InputText("##PlayerListAddr", playerListAddrStr.data(), playerListAddrStr.size() + 1, ImGuiInputTextFlags_ReadOnly);
                            ImGui::PopItemWidth();
                        }

                        ImGui::Separator();

                        // Gadget Context
                        ReClass::GdCliContext gadgetCtx = ctxCollection.GetGdCliContext();
                        std::string gadgetCtxAddrStr = std::format("0x{:X}", reinterpret_cast<uintptr_t>(gadgetCtx.data()));
                        ImGui::Text("GdCliContext:");
                        ImGui::PushItemWidth(-1.0f);
                        ImGui::InputText("##GadgetContextAddr", gadgetCtxAddrStr.data(), gadgetCtxAddrStr.size() + 1, ImGuiInputTextFlags_ReadOnly);
                        ImGui::PopItemWidth();

                        if (gadgetCtx) {
                            // Gadget List
                            ReClass::GdCliGadget** gadgetList = gadgetCtx.GetGadgetList();
                            uint32_t gadgetCapacity = gadgetCtx.GetGadgetListCapacity();
                            uint32_t gadgetCount = gadgetCtx.GetGadgetListCount();
                            std::string gadgetListAddrStr = std::format("0x{:X}", reinterpret_cast<uintptr_t>(gadgetList));
                            ImGui::Text("GadgetList (Count: %u / Capacity: %u):", gadgetCount, gadgetCapacity);
                            ImGui::PushItemWidth(-1.0f);
                            ImGui::InputText("##GadgetListAddr", gadgetListAddrStr.data(), gadgetListAddrStr.size() + 1, ImGuiInputTextFlags_ReadOnly);
                            ImGui::PopItemWidth();

                            // Attack Target List
                            ReClass::AgentInl** attackTargetList = gadgetCtx.GetAttackTargetList();
                            uint32_t attackTargetCapacity = gadgetCtx.GetAttackTargetListCapacity();
                            uint32_t attackTargetCount = gadgetCtx.GetAttackTargetListCount();
                            std::string attackTargetListAddrStr = std::format("0x{:X}", reinterpret_cast<uintptr_t>(attackTargetList));
                            ImGui::Text("AttackTargetList (Count: %u / Capacity: %u):", attackTargetCount, attackTargetCapacity);
                            ImGui::PushItemWidth(-1.0f);
                            ImGui::InputText("##AttackTargetListAddr", attackTargetListAddrStr.data(), attackTargetListAddrStr.size() + 1, ImGuiInputTextFlags_ReadOnly);
                            ImGui::PopItemWidth();
                        }

                        ImGui::Separator();

                        // Item Context
                        ReClass::ItCliContext itemCtx = ctxCollection.GetItCliContext();
                        std::string itemCtxAddrStr = std::format("0x{:X}", reinterpret_cast<uintptr_t>(itemCtx.data()));
                        ImGui::Text("ItCliContext:");
                        ImGui::PushItemWidth(-1.0f);
                        ImGui::InputText("##ItemContextAddr", itemCtxAddrStr.data(), itemCtxAddrStr.size() + 1, ImGuiInputTextFlags_ReadOnly);
                        ImGui::PopItemWidth();

                        if (itemCtx) {
                            // Item List
                            ReClass::ItCliItem** itemList = itemCtx.GetItemList();
                            uint32_t itemCapacity = itemCtx.GetCapacity();
                            uint32_t itemCount = itemCtx.GetCount();
                            std::string itemListAddrStr = std::format("0x{:X}", reinterpret_cast<uintptr_t>(itemList));
                            ImGui::Text("ItemList (Count: %u / Capacity: %u):", itemCount, itemCapacity);
                            ImGui::PushItemWidth(-1.0f);
                            ImGui::InputText("##ItemListAddr", itemListAddrStr.data(), itemListAddrStr.size() + 1, ImGuiInputTextFlags_ReadOnly);
                            ImGui::PopItemWidth();

                            ImGui::Checkbox("Show Item Statistics", &settings.showItemStatistics);
                            if (ImGui::IsItemHovered()) {
                                ImGui::SetTooltip("Show Item Statistics section, displaying counts of items by location type.");
                            }
                        }

                        ImGui::Separator();

                        // Item Statistics
                        if (settings.showItemStatistics) {
                            if (itemCtx.data()) {
                                std::map<Game::ItemLocation, int> counts;
                                int totalItems = 0;

                                SafeAccess::ItemList list(itemCtx);
                                for (const auto& item : list) {
                                    counts[item.GetLocationType()]++;
                                    totalItems++;
                                }

                                ImGui::Text("Total Items Loaded: %d", totalItems);
                                ImGui::Separator();

                                if (ImGui::BeginTable("ItemStatsTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                                    ImGui::TableSetupColumn("Location Type");
                                    ImGui::TableSetupColumn("Count");
                                    ImGui::TableHeadersRow();

                                    for (const auto& [loc, count] : counts) {
                                        ImGui::TableNextRow();
                                        ImGui::TableNextColumn();
                                        ImGui::TextUnformatted(Formatting::GetItemLocationName(loc));
                                        
                                        ImGui::TableNextColumn();
                                        ImGui::Text("%d", count);
                                    }
                                    ImGui::EndTable();
                                }
                            } else {
                                ImGui::TextColored(ImVec4(1, 0, 0, 1), "ItCliContext is null");
                            }
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
