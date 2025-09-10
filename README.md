# KX-Vision for Guild Wars 2

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Version](https://img.shields.io/badge/version-0.1-green.svg)

![KX-Vision GUI](images/gui.jpg)

**Educational Purpose Only:** This project is developed solely for educational purposes, allowing developers to learn about game rendering concepts, memory manipulation, and MumbleLink API integration in Guild Wars 2.

## Overview

KX-Vision is an open-source ESP (Extra Sensory Perception) overlay for Guild Wars 2. It uses the officially supported MumbleLink API for positional data and is designed as a learning platform for real-time overlay rendering with ImGui and DirectX 11, 3D-to-2D projection, and clean C++ architecture for game tools.

## Features

*   **MumbleLink Integration:** Utilizes GW2's MumbleLink API for player and game state data.
*   **Visual ESP:** Agent box rendering, distance measurements, position indicators, and color coding.
*   **Minimalist UI:** Simple, configurable interface.
*   **Patch-Resistant:** Designed to avoid direct memory manipulation where possible.

## Known Issues

*   Current ESP agent rendering primarily targets "gadgets" (e.g., resource nodes, specific interactables). The pointer chain used for agent position retrieval currently works most reliably for these types of entities. Rendering other agent types (characters, NPCs) may be inconsistent or inaccurate.

## Building

### Prerequisites

*   **Visual Studio:** 2019 or newer
*   **Windows SDK:** 10.0.19041.0 or newer
*   **C++17 Support**

### Build Instructions

1.  Clone the repository:
    ```bash
    git clone https://github.com/Krixx1337/kx-vision.git
    cd kx-vision
    ```
2.  Open `KX-Vision.sln` in Visual Studio.
3.  Set the configuration to `Release | x64`.
4.  Build the solution (F7 or Build → Build Solution).
5.  Find the output DLL in the `x64/Release` directory.

## Usage for Educational Purposes

1.  Launch Guild Wars 2.
2.  Inject the DLL using a tool like Process Hacker or Xenos.
3.  Use `INSERT` key to toggle the overlay window.
4.  Configure ESP options through the UI.
5.  Press `DELETE` to safely unload the DLL.

## Disclaimer

This software is created and released for **EDUCATIONAL PURPOSE ONLY**. The use of KX-Vision may be against the Guild Wars 2 Terms of Service. The developers and contributors are not responsible for any consequences that may arise from using this software in violation of the game's terms.

**Please use responsibly:**
*   Only use for learning about game development concepts.
*   Do not use in competitive environments.
*   Be aware of ArenaNet's policies regarding third-party tools.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Credits

*   Initial concept and development by Krixx
*   Uses [Dear ImGui](https://github.com/ocornut/imgui)
*   Uses [MinHook](https://github.com/TsudaKageyu/minhook)
*   Uses [SafetyHook](https://github.com/cursey/safetyhook)
*   Uses [GLM](https://g-truc.github.io/glm/)
*   **Hacklib:** A valuable learning resource for the initial development of this project. [https://bitbucket.org/rafzi/hacklib_gw2/src/master/](https://bitbucket.org/rafzi/hacklib_gw2/src/master/)

## Links

*   🌐 **Website:** [kxtools.xyz](https://kxtools.xyz)
*   💬 **Discord:** [Join Server](https://discord.gg/z92rnB4kHm)
*   📚 **GitHub:** [Repository](https://github.com/Krixx1337/kx-vision)