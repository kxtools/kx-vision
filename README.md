# KX-Vision for Guild Wars 2

![License](https://img.shields.io/badge/license-GPL--3.0-blue.svg)
![Version](https://img.shields.io/badge/version-1.3-green.svg)

![KX-Vision GUI](images/gui_v2.jpg)

## 🎥 Preview

**KX-Vision in Action:** [Watch Demo Video](https://streamable.com/zzq3vc)

---

## 🚀 Get Started

### [➡️ Download the Latest Release](https://github.com/kxtools/kx-vision/releases/latest)

### ⚙️ Installation

> **Quick Tip:** After installing, press the **`INSERT`** key in-game to show or hide the main window. Adjust ESP update rate (30-360 FPS) in settings for optimal performance.

For detailed, step-by-step instructions, please read our complete:
### [**Installation Guide (INSTALL.md)**](INSTALL.md)

---

## Overview

KX-Vision is a powerful Guild Wars 2 addon that enhances your gameplay with real-time information overlays. Track players, enemies, and objects around you with customizable ESP displays. Inspect player gear at a glance, monitor combat performance, and never miss a resource node or vista again. Built using GW2's official MumbleLink API.

## Features

*   **Player ESP:** Track nearby players with customizable filters (friendly, hostile, neutral), gear inspection (3 modes), profession/race/level display.
*   **NPC ESP:** See all enemies and NPCs with rank filtering (legendary, champion, elite, veteran), health tracking, attitude colors.
*   **Object ESP:** Find resource nodes, waypoints, vistas, crafting stations, and 15+ object types with smart filtering.
*   **Real-time Combat Info:** Burst DPS tracking, animated damage numbers, health/energy bars with smooth transitions.
*   **Adaptive Visuals:** Automatic distance-based scaling and fading for clean, uncluttered display.
*   **Flexible Customization:** Per-entity visual controls (boxes, dots, health bars, distance), collapsible detail panels.
*   **Performance Tuning:** Adjustable ESP update rate (30-360 FPS), zero performance impact even in WvW zergs.
*   **Settings Management:** Save/load presets, auto-save on exit, reset to defaults.
*   **MumbleLink Integration:** Utilizes GW2's officially supported MumbleLink API for player and game state data.
*   **Modern UI Design:** Clean, organized ImGui interface with collapsible sections and intuitive controls.
*   **Update Resilient:** Uses pattern scanning to locate game functions, maintaining compatibility across game updates.

## ⚡ Performance

*   **Zero FPS impact** - even in crowded WvW/meta events
*   **Configurable update rate** - tune 30-360 FPS in settings (default: 60)
*   **Optimized pipeline** - two-stage architecture separates expensive calculations from rendering

### Compatibility

- Works alongside popular GW2 addons (e.g., ArcDPS, GW2Radial) via GW2AL.
- Compatible with Raidcore Nexus via chainloading — see [INSTALL.md](INSTALL.md) for setup.

## 🛠️ For Developers

### Architecture Highlights

This isn't your typical game overlay. KX-Vision demonstrates production-grade patterns rarely seen in open-source game addons:

*   **Crash-proof memory safety** - multi-layer validation (SafeForeignClass, SafeIterators, VirtualQuery caching)
*   **Thread-safe capture system** - stable data extraction from game logic thread
*   **Two-stage rendering pipeline** - throttled calculations + per-frame screen projection
*   **Adaptive far plane** - intelligent scaling based on scene depth
*   **Combat tracking** - burst DPS calculator with smooth animations
*   **Object pooling** - zero heap allocations during rendering for consistent frame times
*   **In-game validation system** - built-in Catch2 tests verify pattern scanning and memory offsets
*   **Modern ImGui 1.92+** - unlike most GW2 addons limited to 1.80

📚 **[Read Technical Documentation](docs/index.md)** for architecture deep-dives

### Building From Source

**Prerequisites:** Visual Studio 2019+, Windows SDK 10.0.19041.0+, C++23

**Build Instructions:**
1. Clone: `git clone https://github.com/kxtools/kx-vision.git`
2. **(Optional)** For GW2AL mode: Uncomment `#define GW2AL_BUILD` in `src/Core/Config.h`
3. Open `KX-Vision.sln` and build (F7)

Output: `x64/Release/KX-Vision.dll`

**Dual Mode Support:**
- **GW2AL Mode** - Integrates with [GW2 Addon Loader](https://github.com/gw2-addon-loader/loader-core) (uncomment Config.h define)
- **DLL Mode** - Standalone injection (default)

### Contributing

Pull requests welcome! Check [Issues](https://github.com/kxtools/kx-vision/issues) for tasks, look for `good first issue` tags if you're new to addon development.

## Disclaimer

This software is created and released for **EDUCATIONAL AND DEVELOPMENT PURPOSES ONLY**. It is a learning tool to help developers understand addon development, real-time rendering, DirectX 11 integration, and MumbleLink API usage in Guild Wars 2.

**Please use responsibly:**
*   Always review and respect ArenaNet's policies regarding third-party tools.
*   Use in private instances or for development purposes only.
*   The developers are not responsible for any consequences arising from the use of this software.

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

## Credits

*   Created and developed by [Krixx](https://github.com/Krixx1337)
*   Uses [Dear ImGui](https://github.com/ocornut/imgui), [GLM](https://github.com/g-truc/glm), [MinHook](https://github.com/TsudaKageyu/minhook), [spdlog](https://github.com/gabime/spdlog), [nlohmann/json](https://github.com/nlohmann/json), and [Catch2](https://github.com/catchorg/Catch2)
*   **Hacklib:** A valuable learning resource. [https://bitbucket.org/rafzi/hacklib_gw2/src/master/](https://bitbucket.org/rafzi/hacklib_gw2/src/master/)
*   **Community Contributors:** Thanks to the GW2 addon development community for sharing knowledge.

## Links

*   🌐 **Website:** [kxtools.xyz](https://kxtools.xyz)
*   💬 **Discord:** [Join Server](https://discord.gg/z92rnB4kHm)