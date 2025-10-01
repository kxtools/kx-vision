# Dual-Mode Build System

KX-Vision can be built in two different modes depending on how you want to use it.

## Why Two Modes?

**DLL Injection Mode** - The traditional way. You manually inject the DLL into the game process using tools like Process Hacker or Xenos Injector.

**GW2AL Addon Mode** - Uses the [Guild Wars 2 Addon Loader](https://github.com/gw2-addon-loader/loader-core) framework. This is safer and more convenient because the addon loader handles everything for you. Just drop the file in a folder and launch the game.

## How to Switch Modes

Open `src/Core/Config.h` and look for this line:

```cpp
//#define GW2AL_BUILD
```

- **For DLL Injection Mode (default)**: Keep it commented with `//` like shown above
- **For GW2AL Addon Mode**: Remove the `//` so it looks like: `#define GW2AL_BUILD`

Then rebuild the project in Visual Studio.

## Installation

### DLL Injection Mode
1. Build the project
2. Inject `KX-Vision.dll` using your preferred injector
3. Press INSERT in-game to show/hide the overlay

### GW2AL Addon Mode
1. Uncomment `#define GW2AL_BUILD` in Config.h
2. Build the project
3. Rename the output to `gw2addon_kxvision.dll`
4. Place it in `Guild Wars 2/addons/kxvision/` folder
5. Launch the game - the addon loads automatically
6. Press INSERT to show/hide the overlay

## That's It!

Both modes give you the same features. GW2AL mode is recommended for most users because it's easier to use and works alongside other addons like ArcDPS.
