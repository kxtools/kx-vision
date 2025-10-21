#define NOMINMAX

#include "ImGuiStyle.h"

#include <algorithm> // Required for std::min/max
#include <ShlObj.h> // For SHGetFolderPath
#include <string>
#include <windows.h>

#include "../../libs/ImGui/imgui.h"
#pragma comment(lib, "Shell32.lib")

namespace kx {
namespace GUI {

namespace {
    constexpr float DEFAULT_BASE_FONT_SIZE = 16.0f;  // Industry standard for game overlays
    constexpr const char* CUSTOM_FONT_NAME = "bahnschrift.ttf";
}

    // Helper function to convert RGB to ImVec4 (alpha defaults to 1.0f)
    inline ImVec4 RgbToVec4(int r, int g, int b) {
        return ImVec4(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f, 1.0f);
    }

    // Gets the system Fonts directory path
    std::string GetSystemFontsPath() {
        char fontsPath[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_FONTS, NULL, 0, fontsPath))) {
            return std::string(fontsPath);
        }
        return ""; // Return empty string on failure
    }

    // Loads the primary application font (Bahnschrift).
    // Should be called after ImGui::CreateContext() and before renderer init.
    // Returns true if custom font was loaded successfully, false otherwise.
    bool LoadAppFont(float scale) {
        // Critical: Check if ImGui context is still valid before any ImGui operations
        if (!ImGui::GetCurrentContext()) {
            return false;
        }

        ImGuiIO& io = ImGui::GetIO();
        bool success = false;

        // Clear existing fonts
        io.Fonts->Clear();

        // Calculate scaled font size
        float baseFontSize = DEFAULT_BASE_FONT_SIZE;
        float scaledFontSize = baseFontSize * scale;

        // Add default font first as a fallback
        io.Fonts->AddFontDefault();

        std::string fontsDir = GetSystemFontsPath();
        if (!fontsDir.empty()) {
            std::string fontPath = fontsDir + "\\" + std::string(CUSTOM_FONT_NAME); // Use Bahnschrift

            ImFont* customFont = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), scaledFontSize);

            if (customFont) {
                // Set the loaded font as the default for ImGui to use.
                io.FontDefault = customFont;
                success = true;
            }
            else {
                // Log or notify if Bahnschrift isn't found
                MessageBoxA(NULL, ("Failed to load Bahnschrift font from: " + fontPath + ". Using default.").c_str(), "Font Warning", MB_OK | MB_ICONWARNING);
            }
        }
        else {
            MessageBoxA(NULL, "Could not determine System Fonts directory path! Using default font.", "Font Error", MB_OK | MB_ICONERROR);
        }
        return success;
    }

    void ApplyCustomStyle() {
        // Critical: Check if ImGui context is still valid before any ImGui operations
        if (!ImGui::GetCurrentContext()) {
            return;
        }

        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // Define Palette Colors
        const ImVec4 richBlack = RgbToVec4(17, 19, 37);   // #111325
        const ImVec4 oxfordBlue = RgbToVec4(26, 31, 52);   // #1a1f34
        const ImVec4 spaceCadet = RgbToVec4(37, 43, 69);   // #252b45
        const ImVec4 coolGray = RgbToVec4(128, 138, 184);// #808ab8
        const ImVec4 neonBlue = RgbToVec4(0, 98, 255);   // #0062ff
        const ImVec4 azure = RgbToVec4(51, 129, 255); // #3381ff
        const ImVec4 aliceBlue = RgbToVec4(229, 236, 244);// #e5ecf4

        // Calculate derived colors
        ImVec4 spaceCadetHover = ImVec4(
            std::min(spaceCadet.x * 1.3f, 1.0f),
            std::min(spaceCadet.y * 1.3f, 1.0f),
            std::min(spaceCadet.z * 1.3f, 1.0f),
            1.0f
        );
        ImVec4 spaceCadetActive = ImVec4(
            std::max(spaceCadet.x * 0.9f, 0.0f),
            std::max(spaceCadet.y * 0.9f, 0.0f),
            std::max(spaceCadet.z * 0.9f, 0.0f),
            1.0f
        );
        ImVec4 neonBlueActive = ImVec4(
            std::max(neonBlue.x * 0.9f, 0.0f),
            std::max(neonBlue.y * 0.9f, 0.0f),
            std::max(neonBlue.z * 0.9f, 0.0f),
            1.0f
        );


        // Layout & Rounding
        style.WindowPadding = ImVec2(8.0f, 8.0f);
        style.FramePadding = ImVec2(5.0f, 4.0f);
        style.ItemSpacing = ImVec2(6.0f, 4.0f);
        style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
        style.ScrollbarSize = 14.0f;
        style.GrabMinSize = 12.0f;
        style.WindowRounding = 4.0f;
        style.ChildRounding = 2.0f;
        style.FrameRounding = 3.0f;
        style.ScrollbarRounding = 9.0f;
        style.GrabRounding = 3.0f;
        style.TabRounding = 4.0f;

        // Apply Colors (With Natural Transparency)
        colors[ImGuiCol_Text] = aliceBlue;
        colors[ImGuiCol_TextDisabled] = coolGray;
        colors[ImGuiCol_WindowBg] = ImVec4(richBlack.x, richBlack.y, richBlack.z, 0.90f); // 90% opacity for natural look
        colors[ImGuiCol_ChildBg] = ImVec4(oxfordBlue.x, oxfordBlue.y, oxfordBlue.z, 0.85f); // Slightly transparent child windows
        colors[ImGuiCol_PopupBg] = ImVec4(richBlack.x, richBlack.y, richBlack.z, 0.95f); // Popups more opaque for readability
        colors[ImGuiCol_Border] = spaceCadet;
        colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        colors[ImGuiCol_FrameBg] = spaceCadet;
        colors[ImGuiCol_FrameBgHovered] = spaceCadetHover;
        colors[ImGuiCol_FrameBgActive] = spaceCadetActive;
        colors[ImGuiCol_TitleBg] = richBlack;
        colors[ImGuiCol_TitleBgActive] = oxfordBlue;
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(richBlack.x, richBlack.y, richBlack.z, 0.85f);
        colors[ImGuiCol_MenuBarBg] = oxfordBlue;
        colors[ImGuiCol_ScrollbarBg] = richBlack;
        colors[ImGuiCol_ScrollbarGrab] = coolGray;
        colors[ImGuiCol_ScrollbarGrabHovered] = aliceBlue;
        colors[ImGuiCol_ScrollbarGrabActive] = azure;
        colors[ImGuiCol_CheckMark] = neonBlue;
        colors[ImGuiCol_SliderGrab] = neonBlue;
        colors[ImGuiCol_SliderGrabActive] = azure;
        colors[ImGuiCol_Button] = neonBlue;
        colors[ImGuiCol_ButtonHovered] = azure;
        colors[ImGuiCol_ButtonActive] = neonBlueActive;
        colors[ImGuiCol_Header] = spaceCadet;
        colors[ImGuiCol_HeaderHovered] = spaceCadetHover;
        colors[ImGuiCol_HeaderActive] = spaceCadetHover;
        colors[ImGuiCol_Separator] = spaceCadet;
        colors[ImGuiCol_SeparatorHovered] = azure;
        colors[ImGuiCol_SeparatorActive] = neonBlue;
        colors[ImGuiCol_ResizeGrip] = ImVec4(coolGray.x, coolGray.y, coolGray.z, 0.5f);
        colors[ImGuiCol_ResizeGripHovered] = coolGray;
        colors[ImGuiCol_ResizeGripActive] = neonBlue;
        colors[ImGuiCol_Tab] = oxfordBlue;
        colors[ImGuiCol_TabHovered] = azure;
        colors[ImGuiCol_TabActive] = neonBlue;
        colors[ImGuiCol_TabUnfocused] = ImVec4(oxfordBlue.x, oxfordBlue.y, oxfordBlue.z, 0.8f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(neonBlue.x, neonBlue.y, neonBlue.z, 0.6f);
        colors[ImGuiCol_PlotLines] = coolGray;
        colors[ImGuiCol_PlotLinesHovered] = azure;
        colors[ImGuiCol_PlotHistogram] = neonBlue;
        colors[ImGuiCol_PlotHistogramHovered] = azure;
        colors[ImGuiCol_TableHeaderBg] = oxfordBlue;
        colors[ImGuiCol_TableBorderStrong] = spaceCadet;
        colors[ImGuiCol_TableBorderLight] = ImVec4(spaceCadet.x, spaceCadet.y, spaceCadet.z, 0.6f);
        colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        colors[ImGuiCol_TableRowBgAlt] = ImVec4(aliceBlue.x, aliceBlue.y, aliceBlue.z, 0.07f);
        colors[ImGuiCol_TextSelectedBg] = ImVec4(azure.x, azure.y, azure.z, 0.40f);
        colors[ImGuiCol_DragDropTarget] = ImVec4(neonBlue.x, neonBlue.y, neonBlue.z, 0.95f);
        colors[ImGuiCol_NavHighlight] = azure;
        colors[ImGuiCol_NavWindowingHighlight] = aliceBlue;
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(coolGray.x, coolGray.y, coolGray.z, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(richBlack.x, richBlack.y, richBlack.z, 0.75f);
    }

} // namespace GUI
} // namespace kx