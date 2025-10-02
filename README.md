# KX-Vision for Guild Wars 2

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Version](https://img.shields.io/badge/version-0.5-green.svg)

![KX-Vision GUI](images/gui_v2.jpg)

## 🎥 Preview

**KX-Vision in Action:** [Watch Demo Video](https://streamable.com/zzq3vc)

---

## 🚀 Get Started

### [➡️ Download the Latest Release](https://github.com/kxtools/kx-vision/releases/latest)

### ⚙️ Installation

> **Quick Tip:** After installing, press the **`INSERT`** key in-game to show or hide the main window.

For detailed, step-by-step instructions, please read our complete:
### [**Installation Guide (INSTALL.md)**](INSTALL.md)

---

## Overview

KX-Vision is an open-source Guild Wars 2 addon that demonstrates advanced UI overlay techniques using the officially supported MumbleLink API. It features a real-time information display for player positions, a multi-level gear inspector, and showcases clean C++ architecture for game addon development with ImGui and DirectX 11.

**Educational Purpose Only:** This project is developed solely for educational purposes, demonstrating advanced addon development techniques including real-time overlay rendering, DirectX 11 integration, and MumbleLink API usage in Guild Wars 2.

**Dual Mode Support:** KX-Vision can be built in two modes:
- **GW2AL Addon Mode:** Integrates with the [Guild Wars 2 Addon Loader](https://github.com/gw2-addon-loader/loader-core) framework for safer, community-supported addon loading.
- **DLL Injection Mode:** A traditional standalone DLL for development and testing.

## Features

*   **MumbleLink Integration:** Utilizes GW2's officially supported MumbleLink API for player and game state data.
*   **Real-Time Information Display:** Shows character positions, distances, and game world data with comprehensive memory safety.
*   **Advanced Gear Inspector:** Examine player equipment with three detailed modes (Off, Compact, Detailed). The compact view provides a color-coded summary of stat sets and rarity for quick build analysis.
*   **Stable Architecture:** A two-stage, thread-safe rendering pipeline that ensures crash-free operation.
*   **Customizable Overlay:** Flexible visual options including boxes, distance indicators, position markers, and status bars.
*   **Modern UI Design:** Clean, organized ImGui interface with collapsible sections and intuitive controls.
*   **Update Resilient:** Uses pattern scanning to locate game functions, maintaining compatibility across game updates.
*   **Developer Tools:** Comprehensive debug logging system for addon development and troubleshooting.

## 🛠️ For Developers

### Building From Source

**Prerequisites:**
*   **Visual Studio:** 2019 or newer
*   **Windows SDK:** 10.0.19041.0 or newer
*   **C++17 Support**

**Build Instructions:**
1.  Clone the repository:
    ```bash
    git clone https://github.com/kxtools/kx-vision.git
    cd kx-vision
    ```
2.  Open `KX-Vision.sln` in Visual Studio.
3.  Choose your build configuration:
    - **For GW2AL Addon Mode:** `Release-GW2AL | x64`
    - **For DLL Injection Mode:** `Release | x64`
4.  Build the solution (F7 or Build → Build Solution).
5.  Find the output DLL in the `x64/Release` or `x64/Release-GW2AL` directory.

### Contributing

Contributions are welcome! This project serves as a learning platform, and we encourage developers to learn by contributing.

Please check the [GitHub Issues](https://github.com/kxtools/kx-vision/issues) tab to find tasks to work on. Look for issues tagged with `good first issue` if you're new to addon development. When submitting a pull request, please provide a clear description of your changes.

## Disclaimer

This software is created and released for **EDUCATIONAL AND DEVELOPMENT PURPOSES ONLY**. It is a learning tool to help developers understand addon development, real-time rendering, and game integration.

**Please use responsibly:**
*   Always review and respect ArenaNet's policies regarding third-party tools.
*   Use in private instances or for development purposes only.
*   The developers are not responsible for any consequences arising from the use of this software.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Credits

*   Initial concept and development by Krixx
*   Uses [Dear ImGui](https://github.com/ocornut/imgui), [GLM](https://github.com/g-truc/glm), [MinHook](https://github.com/TsudaKageyu/minhook), [nlohmann/json](https://github.com/nlohmann/json), and [Catch2](https://github.com/catchorg/Catch2)
*   **Hacklib:** A valuable learning resource. [https://bitbucket.org/rafzi/hacklib_gw2/src/master/](https://bitbucket.org/rafzi/hacklib_gw2/src/master/)
*   **Community Contributors:** Thanks to the GW2 addon development community for sharing knowledge.

## Links

*   🌐 **Website:** [kxtools.xyz](https://kxtools.xyz)
*   💬 **Discord:** [Join Server](https://discord.gg/z92rnB4kHm)