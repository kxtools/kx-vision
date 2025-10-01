# KX-Vision for Guild Wars 2

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Version](https://img.shields.io/badge/version-0.5-green.svg)

![KX-Vision GUI](images/gui_v2.jpg)

## 🎥 Preview

**KX Vision in Action:** [Watch Demo Video](https://streamable.com/zzq3vc)

[➡️ Download the Latest Release](https://github.com/kxtools/kx-vision/releases/latest)

**Educational Purpose Only:** This project is developed solely for educational purposes, demonstrating advanced addon development techniques including real-time overlay rendering, DirectX 11 integration, and MumbleLink API usage in Guild Wars 2.

## Overview

KX-Vision is an open-source Guild Wars 2 addon that demonstrates advanced UI overlay techniques using the officially supported MumbleLink API. It features a real-time information display for player positions, a multi-level gear inspector, and showcases clean C++ architecture for game addon development with ImGui and DirectX 11.

**Dual Mode Support:** KX-Vision can be built in two modes:
- **DLL Injection Mode:** Traditional standalone DLL that can be injected into the game process.
- **GW2AL Addon Mode:** Integrates with the [Guild Wars 2 Addon Loader](https://github.com/gw2-addon-loader/loader-core) framework for safer, community-supported addon loading.

## Features

*   **MumbleLink Integration:** Utilizes GW2's officially supported MumbleLink API for player and game state data.
*   **Real-Time Information Display:** Shows character positions, distances, and game world data with comprehensive memory safety.
*   **Advanced Gear Inspector:** Examine player equipment with three detailed modes (Off, Compact, Detailed). The compact view provides a color-coded summary of stat sets and rarity for quick build analysis.
*   **Stable Architecture:** A two-stage, thread-safe rendering pipeline that ensures crash-free operation and smooth integration with the game.
*   **Developer Tools:** Comprehensive debug logging system for addon development and troubleshooting.
*   **Customizable Overlay:** Flexible visual options including boxes, distance indicators, position markers, and status bars.
*   **Modern UI Design:** Clean, organized ImGui interface with collapsible sections and intuitive controls.
*   **Update Resilient:** Uses pattern scanning to locate game functions, maintaining compatibility across game updates.

## Building

### Prerequisites

*   **Visual Studio:** 2019 or newer
*   **Windows SDK:** 10.0.19041.0 or newer
*   **C++17 Support**

### Build Instructions

1.  Clone the repository:
    ```bash
    git clone https://github.com/kxtools/kx-vision.git
    cd kx-vision
    ```
2.  Open `KX-Vision.sln` in Visual Studio.
3.  Choose your build configuration:
    - **For DLL Injection Mode:** `Release | x64`
    - **For GW2AL Addon Mode:** `Release-GW2AL | x64`
4.  Build the solution (F7 or Build → Build Solution).
5.  Find the output DLL in the `x64/Release` or `x64/Release-GW2AL` directory.

## Installation & Usage

### Method 1: GW2AL Addon Mode (Recommended)

**Prerequisites:**
- Install the [Guild Wars 2 Addon Loader](https://github.com/gw2-addon-loader/loader-core) and d3d11 wrapper.

**Installation:**
1. Download and extract the archive `kx-vision_gw2al.zip` found in the [latest release](https://github.com/kxtools/kx-vision/releases/latest).
2. Place `gw2addon_kxvision.dll` in your `addons` folder inside a new folder named `kxvision` (with the default game install path, this would be `C:\Program Files\Guild Wars 2\addons\kxvision`).
3. Run the game! If everything was set up properly, the overlay will load automatically.
4. Press `INSERT` to toggle the overlay window visibility.

### Method 2: DLL Injection Mode (Educational Purposes Only)

1.  Launch Guild Wars 2.
2.  Inject the DLL using a tool like Process Hacker or Xenos.
3.  Use `INSERT` key to toggle the overlay window.
4.  Configure ESP options and debug settings through the UI.
5.  Press `DELETE` to safely unload the DLL.

## Contributing

Contributions are welcome! This project serves as a learning platform for game addon development, and we encourage developers to learn by contributing.

If you'd like to help improve the codebase or documentation, please check the [GitHub Issues](https://github.com/kxtools/kx-vision/issues) tab to find tasks to work on. Look for issues tagged with `good first issue` if you're new to addon development.

When submitting a pull request, please provide a clear description of your changes and how they improve the addon's educational value or functionality.

## Disclaimer

This software is created and released for **EDUCATIONAL AND DEVELOPMENT PURPOSES ONLY**. KX-Vision is designed as a learning tool to help developers understand addon development, real-time rendering, and game integration techniques.

**Please use responsibly:**
*   This project is intended for learning about game addon development concepts.
*   Always review and respect ArenaNet's policies regarding third-party tools.
*   Use in private instances or for development purposes only.
*   The developers are not responsible for any consequences arising from use of this software.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Credits

*   Initial concept and development by Krixx
*   Uses [Dear ImGui](https://github.com/ocornut/imgui) for the user interface
*   Uses [GLM](https://github.com/g-truc/glm) for mathematics
*   Uses [MinHook](https://github.com/TsudaKageyu/minhook) for function hooking
*   Uses [nlohmann/json](https://github.com/nlohmann/json) for JSON parsing
*   Uses [Catch2](https://github.com/catchorg/Catch2) for unit testing
*   **Hacklib:** A valuable learning resource for understanding GW2's internal structures. [https://bitbucket.org/rafzi/hacklib_gw2/src/master/](https://bitbucket.org/rafzi/hacklib_gw2/src/master/)
*   **Community Contributors:** Thanks to the GW2 addon development community for sharing knowledge and best practices

## Links

*   🌐 **Website:** [kxtools.xyz](https://kxtools.xyz)
*   💬 **Discord:** [Join Server](https://discord.gg/z92rnB4kHm)