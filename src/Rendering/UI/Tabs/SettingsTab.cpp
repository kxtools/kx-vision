#define NOMINMAX

#include "SettingsTab.h"
#include <string>
#include <algorithm>
#include <format>
#include <map>
#include <Windows.h>

#include "DebugLogger.h"
#include "SettingsManager.h"

#include "ImGui/imgui.h"
#include "SDK/ContextStructs.h"
#include "SDK/GadgetStructs.h"
#include "../../../Memory/SafeGameArray.h"
#include "../../../Rendering/Presentation/Formatting.h"
#include "../../../Memory/AddressManager.h"

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
        
        void RenderItemStatistics(const ReClass::ItCliContext& itemCtx) {
            ImGui::Separator();
            static bool showItemStats = false;
            ImGui::Checkbox("Show Item Statistics", &showItemStats);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Show Item Statistics section, displaying counts of items by location type.");
            }

            static std::map<Game::ItemLocation, int> counts;
            static int totalItems = 0;
            static int peakItems = 0;
            static int groundLoot = 0;
            static int peakGroundLoot = 0;
            static uint64_t lastUpdate = 0;

            if (showItemStats && itemCtx.data()) {
                uint64_t now = GetTickCount64();
                if (now - lastUpdate > 500) {
                    lastUpdate = now;
                    
                    counts.clear();
                    totalItems = 0;
                    groundLoot = 0;

                    auto list = itemCtx.GetItems();
                    for (const auto& item : list) {
                        Game::ItemLocation loc = item.GetLocationType();
                        counts[loc]++;
                        totalItems++;
                        if (loc == Game::ItemLocation::Agent) {
                            groundLoot++;
                        }
                    }
                    
                    if (totalItems > peakItems) peakItems = totalItems;
                    if (groundLoot > peakGroundLoot) peakGroundLoot = groundLoot;
                }

                ImGui::Text("Total Items Loaded: %d", totalItems);
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1, 1, 0, 1), "(Peak: %d)", peakItems);
                
                ImGui::Text("Ground Loot (Agent): %d", groundLoot);
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1, 1, 0, 1), "(Peak: %d)", peakGroundLoot);
                
                if (ImGui::Button("Reset Peak")) {
                    peakItems = totalItems;
                    peakGroundLoot = groundLoot;
                }

                ImGui::Separator();

                if (ImGui::BeginTable("ItemStatsTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    ImGui::TableSetupColumn("ID");
                    ImGui::TableSetupColumn("Location Type");
                    ImGui::TableSetupColumn("Count");
                    ImGui::TableHeadersRow();

                    for (const auto& [loc, count] : counts) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", static_cast<int>(loc));
                        
                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted(Formatting::GetItemLocationName(loc));
                        
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", count);
                    }
                    ImGui::EndTable();
                }
            } else if (showItemStats && !itemCtx.data()) {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "ItCliContext is null");
            }
        }

        void RenderGadgetStatistics(const ReClass::GdCliContext& gadgetCtx) {
            ImGui::Separator();
            static bool showGadgetStats = false;
            ImGui::Checkbox("Show Gadget Statistics", &showGadgetStats);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Analyze Gadget types to optimize filters and pool sizes.");

            static std::map<Game::GadgetType, int> typeCounts;
            static int totalGadgets = 0;
            static int gatherableCount = 0;
            static int peakGadgets = 0;
            static int peakGatherable = 0;
            static uint64_t lastUpdate = 0;

            if (showGadgetStats && gadgetCtx.data()) {
                uint64_t now = GetTickCount64();
                if (now - lastUpdate > 500) {
                    lastUpdate = now;
                    
                    typeCounts.clear();
                    totalGadgets = 0;
                    gatherableCount = 0;

                    for (const auto& gadget : gadgetCtx.GetGadgets()) {
                        typeCounts[gadget.GetGadgetType()]++;
                        if (gadget.IsGatherable()) gatherableCount++;
                        totalGadgets++;
                    }
                    
                    if (totalGadgets > peakGadgets) peakGadgets = totalGadgets;
                    if (gatherableCount > peakGatherable) peakGatherable = gatherableCount;
                }

                ImGui::Text("Total Valid Gadgets: %d", totalGadgets);
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1, 1, 0, 1), "(Peak: %d)", peakGadgets);
                
                ImGui::Text("Gatherable Resources: %d", gatherableCount);
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1, 1, 0, 1), "(Peak: %d)", peakGatherable);
                
                if (ImGui::Button("Reset Peak")) {
                    peakGadgets = totalGadgets;
                    peakGatherable = gatherableCount;
                }
                
                if (ImGui::BeginTable("GadgetStatsTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    ImGui::TableSetupColumn("ID");
                    ImGui::TableSetupColumn("Gadget Type");
                    ImGui::TableSetupColumn("Count");
                    ImGui::TableHeadersRow();

                    for (const auto& [type, count] : typeCounts) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", static_cast<int>(type));
                        
                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted(Formatting::GetGadgetTypeName(type));
                        
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", count);
                    }
                    ImGui::EndTable();
                }
            }
        }

        void RenderCharacterStatistics(const ReClass::ChCliContext& charContext) {
            ImGui::Separator();
            static bool showCharStats = false;
            ImGui::Checkbox("Show Character Statistics", &showCharStats);

            static int totalChars = 0;
            static int validAgents = 0;
            static std::map<Game::AgentType, int> agentTypes;
            static int peakCharacters = 0;
            static int peakValidAgents = 0;
            static uint64_t lastUpdate = 0;

            if (showCharStats && charContext.data()) {
                uint64_t now = GetTickCount64();
                if (now - lastUpdate > 500) {
                    lastUpdate = now;
                    
                    totalChars = 0;
                    validAgents = 0;
                    agentTypes.clear();
                
                    for (const auto& character : charContext.GetCharacters()) {
                        totalChars++;
                        
                        auto agent = character.GetAgent();
                        if (agent.isValid()) {
                            validAgents++;
                            agentTypes[agent.GetType()]++;
                        }
                    }
                    
                    if (totalChars > peakCharacters) peakCharacters = totalChars;
                    if (validAgents > peakValidAgents) peakValidAgents = validAgents;
                }

                ImGui::Text("Total Characters: %d", totalChars);
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1, 1, 0, 1), "(Peak: %d)", peakCharacters);
                
                ImGui::Text("Valid Agents Linked: %d", validAgents);
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1, 1, 0, 1), "(Peak: %d)", peakValidAgents);
                
                if (ImGui::Button("Reset Peak")) {
                    peakCharacters = totalChars;
                    peakValidAgents = validAgents;
                }
                
                if (ImGui::BeginTable("CharStatsTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    ImGui::TableSetupColumn("ID");
                    ImGui::TableSetupColumn("Agent Type");
                    ImGui::TableSetupColumn("Count");
                    ImGui::TableHeadersRow();

                    for (const auto& [type, count] : agentTypes) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", static_cast<int>(type));
                        
                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted(Formatting::GetAgentTypeName(type));
                        
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", count);
                    }
                    ImGui::EndTable();
                }
            }
        }

        void RenderPlayerStatistics(const ReClass::ChCliContext& charContext) {
            ImGui::Separator();
            static bool showPlayerStats = false;
            ImGui::Checkbox("Show Player Statistics", &showPlayerStats);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Analyze Player distribution by profession and race to optimize pool sizes.");
            }

            static int totalPlayers = 0;
            static int validCharacters = 0;
            static std::map<Game::Profession, int> professions;
            static std::map<Game::Race, int> races;
            static int peakPlayers = 0;
            static int peakValidCharacters = 0;
            static uint64_t lastUpdate = 0;

            if (showPlayerStats && charContext.data()) {
                uint64_t now = GetTickCount64();
                if (now - lastUpdate > 500) {
                    lastUpdate = now;
                    
                    totalPlayers = 0;
                    validCharacters = 0;
                    professions.clear();
                    races.clear();
                
                    for (const auto& player : charContext.GetPlayers()) {
                        totalPlayers++;
                        
                        auto character = player.GetCharacter();
                        if (character.isValid()) {
                            validCharacters++;
                            
                            auto coreStats = character.GetCoreStats();
                            if (coreStats.isValid()) {
                                professions[coreStats.GetProfession()]++;
                                races[coreStats.GetRace()]++;
                            }
                        }
                    }
                    
                    if (totalPlayers > peakPlayers) peakPlayers = totalPlayers;
                    if (validCharacters > peakValidCharacters) peakValidCharacters = validCharacters;
                }

                ImGui::Text("Total Players: %d", totalPlayers);
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1, 1, 0, 1), "(Peak: %d)", peakPlayers);
                
                ImGui::Text("Valid Characters Linked: %d", validCharacters);
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1, 1, 0, 1), "(Peak: %d)", peakValidCharacters);
                
                if (ImGui::Button("Reset Peak")) {
                    peakPlayers = totalPlayers;
                    peakValidCharacters = validCharacters;
                }
                
                if (ImGui::BeginTable("PlayerProfessionTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    ImGui::TableSetupColumn("ID");
                    ImGui::TableSetupColumn("Profession");
                    ImGui::TableSetupColumn("Count");
                    ImGui::TableHeadersRow();

                    for (const auto& [prof, count] : professions) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", static_cast<int>(prof));
                        
                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted(Formatting::GetProfessionName(prof));
                        
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", count);
                    }
                    ImGui::EndTable();
                }

                ImGui::Separator();

                if (ImGui::BeginTable("PlayerRaceTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    ImGui::TableSetupColumn("ID");
                    ImGui::TableSetupColumn("Race");
                    ImGui::TableSetupColumn("Count");
                    ImGui::TableHeadersRow();

                    for (const auto& [race, count] : races) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", static_cast<int>(race));
                        
                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted(Formatting::GetRaceName(race));
                        
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", count);
                    }
                    ImGui::EndTable();
                }
            }
        }

        void RenderAttackTargetStatistics(const ReClass::GdCliContext& gadgetCtx) {
            ImGui::Separator();
            static bool showAttackTargetStats = false;
            ImGui::Checkbox("Show Attack Target Statistics", &showAttackTargetStats);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Analyze Attack Target distribution by combat state and agent type.");
            }

            static int totalTargets = 0;
            static int validAgents = 0;
            static std::map<Game::AttackTargetCombatState, int> combatStates;
            static std::map<Game::AgentType, int> agentTypes;
            static int peakAttackTargets = 0;
            static int peakValidAgents = 0;
            static uint64_t lastUpdate = 0;

            if (showAttackTargetStats && gadgetCtx.data()) {
                uint64_t now = GetTickCount64();
                if (now - lastUpdate > 500) {
                    lastUpdate = now;
                    
                    totalTargets = 0;
                    validAgents = 0;
                    combatStates.clear();
                    agentTypes.clear();
                
                    for (const auto& attackTarget : gadgetCtx.GetAttackTargets()) {
                        totalTargets++;
                        
                        auto agent = attackTarget.GetAgKeyFramed();
                        if (agent.isValid()) {
                            validAgents++;
                            agentTypes[agent.GetType()]++;
                        }
                        
                        combatStates[attackTarget.GetCombatState()]++;
                    }
                    
                    if (totalTargets > peakAttackTargets) peakAttackTargets = totalTargets;
                    if (validAgents > peakValidAgents) peakValidAgents = validAgents;
                }

                ImGui::Text("Total Attack Targets: %d", totalTargets);
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1, 1, 0, 1), "(Peak: %d)", peakAttackTargets);
                
                ImGui::Text("Valid Agents Linked: %d", validAgents);
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1, 1, 0, 1), "(Peak: %d)", peakValidAgents);
                
                if (ImGui::Button("Reset Peak")) {
                    peakAttackTargets = totalTargets;
                    peakValidAgents = validAgents;
                }
                
                if (ImGui::BeginTable("AttackTargetCombatStateTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    ImGui::TableSetupColumn("ID");
                    ImGui::TableSetupColumn("Combat State");
                    ImGui::TableSetupColumn("Count");
                    ImGui::TableHeadersRow();

                    for (const auto& [state, count] : combatStates) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", static_cast<int>(state));
                        
                        ImGui::TableNextColumn();
                        const char* stateName = (state == Game::AttackTargetCombatState::Idle) ? "Idle" : 
                                                 (state == Game::AttackTargetCombatState::InCombat) ? "In Combat" : 
                                                 "Unknown";
                        ImGui::TextUnformatted(stateName);
                        
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", count);
                    }
                    ImGui::EndTable();
                }

                ImGui::Separator();

                if (ImGui::BeginTable("AttackTargetAgentTypeTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    ImGui::TableSetupColumn("ID");
                    ImGui::TableSetupColumn("Agent Type");
                    ImGui::TableSetupColumn("Count");
                    ImGui::TableHeadersRow();

                    for (const auto& [type, count] : agentTypes) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", static_cast<int>(type));
                        
                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted(Formatting::GetAgentTypeName(type));
                        
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", count);
                    }
                    ImGui::EndTable();
                }
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

                        }

                        RenderCharacterStatistics(charContext);
                        RenderPlayerStatistics(charContext);
                        RenderGadgetStatistics(gadgetCtx);
                        RenderAttackTargetStatistics(gadgetCtx);
                        RenderItemStatistics(itemCtx);
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
