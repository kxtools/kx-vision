# Dual-Mode Build System

KX-Vision supports two build modes: **DLL Mode** (manual injection) and **GW2AL Mode** (addon loader).

## Build Configuration

The build mode is controlled by a single macro in `src/Core/Config.h`:

```cpp
// Uncomment for GW2AL addon loader mode
#define GW2AL_BUILD

// Comment out for DLL injection mode
//#define GW2AL_BUILD
```

## Mode Comparison

| Feature | DLL Mode | GW2AL Mode |
|---------|----------|------------|
| **Entry Point** | `DllMain()` in `Main.cpp` | `gw2addon_load()` in `GW2AL_Integration.cpp` |
| **D3D Hook** | MinHook on Present | d3d9_wrapper events |
| **Initialization** | `InitializeHooks()` | `OnRendererInitialized()` |
| **Rendering** | `D3DRenderHook::RenderFrame()` | `OnPresent()` callback |
| **MinHook Init** | Early in `InitializeHooks()` | In `InitializeGameServices()` |
| **Game Thread Hook** | ✅ Supported | ✅ Supported |
| **Unload** | DELETE key | GW2AL unload |

## Architecture

### DLL Mode Flow
1. `DllMain()` → Creates worker thread
2. `Initialize()` → Initializes MinHook and D3D hook
3. `Update()` loop → State machine transitions
4. `RenderFrame()` → Per-frame rendering and updates

### GW2AL Mode Flow
1. `gw2addon_load()` → Registers event watchers
2. `D3D9_POST_DXGI_CreateSwapChain` → Initializes D3D from device
3. `OnRendererInitialized()` → Transitions to WaitingForGame
4. `OnPresent()` → Per-frame updates and `CheckAndInitializeServices()`
5. Once in-game → `InitializeGameServices()` → Initialize MinHook, AddressManager, ESPRenderer

## Key Differences

### Conditional Compilation
Many files use `#ifdef GW2AL_BUILD` to switch between modes:

- **Main.cpp**: Excluded in GW2AL mode (`#ifndef GW2AL_BUILD`)
- **GW2AL_Integration.cpp**: Included only in GW2AL mode (`#ifdef GW2AL_BUILD`)
- **D3DRenderHook**: Different initialization paths
- **AppLifecycleManager**: Different state machine entry points

### State Machine
Both modes use `AppLifecycleManager` but enter at different points:

**DLL Mode:**
```
PreInit → WaitingForImGui → WaitingForGame → InitializingServices → Running
```

**GW2AL Mode:**
```
WaitingForRenderer → WaitingForGame → InitializingServices → Running
```

### MinHook Initialization
- **DLL Mode**: Initialized early in `InitializeHooks()` (called from `Initialize()`)
- **GW2AL Mode**: Initialized late in `InitializeGameServices()` (called after player enters game)

This difference exists because GW2AL mode doesn't call `InitializeHooks()` - it relies on GW2AL's d3d9_wrapper for rendering hooks.

## Files by Mode

### DLL-Only Files
- `src/Core/Main.cpp` - DLL entry point

### GW2AL-Only Files
- `src/Core/GW2AL_Integration.cpp` - GW2AL entry point and callbacks
- `src/Hooking/GW2AL/d3d9_wrapper_structs.h` - GW2AL event structures

### Shared Files (with conditional compilation)
- `src/Core/AppLifecycleManager.cpp` - Different initialization paths
- `src/Hooking/D3DRenderHook.cpp` - Different hook mechanisms
- `src/Hooking/Hooks.cpp` - Conditional D3D hook initialization

### Mode-Agnostic Files
- `src/Rendering/ImGuiManager.cpp` - Same rendering code
- `src/Rendering/Core/ESPRenderer.cpp` - Same ESP logic
- `src/Game/Camera.cpp` - Same camera updates
- `src/Game/MumbleLinkManager.cpp` - Same MumbleLink handling

## Building

1. Edit `src/Core/Config.h` to enable/disable `GW2AL_BUILD`
2. Clean build: `msbuild KX-Vision.sln /t:Clean,Build /p:Configuration=Release /p:Platform=x64`
3. Output: `x64/Release/KX-Vision.dll`

**DLL Mode**: Inject using any DLL injector
**GW2AL Mode**: Place in `Guild Wars 2/addons/` and run with GW2AL

## Troubleshooting

### ESP Not Rendering (GW2AL Mode)
- **Symptom**: `MH_ERROR_NOT_INITIALIZED` when creating game thread hook
- **Cause**: MinHook not initialized before game thread hook
- **Fix**: Ensure `HookManager::Initialize()` is called in `InitializeGameServices()` for GW2AL mode

### Player In-Game Detection
Both modes wait for:
1. MumbleLink connected (`IsInitialized()`)
2. Player in map (`mapId != 0`)

Before initializing AddressManager and ESP services.
