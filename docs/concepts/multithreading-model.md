# Multithreading Model

A critical architectural aspect of the Guild Wars 2 engine is its use of multiple threads to handle different tasks concurrently. Understanding this is essential for stable tool development.

## Key Threads

### 1. Game Logic Thread (Main Thread)
This thread is responsible for updating the state of the game world. Its tasks include:
- Processing player input and movement.
- Executing AI routines for NPCs and monsters.
- Managing character state (health, energy, buffs).
- Running the main "tick" or "update" loop for all gameplay systems.

The primary data hub for gameplay, referred to as the **`ContextCollection`**, is created, managed, and stored in the Thread Local Storage (TLS) of this thread.

### 2. Render Thread
This thread's sole purpose is to communicate with the graphics API (DirectX 11) and send drawing commands to the GPU.
- It receives the state of the game world from the logic thread.
- It is responsible for all rendering, including the game world, UI, and any hooked overlays like ImGui.

Our project's primary hook point, `D3DRenderHook::DetourPresent`, injects our code directly into this thread.

## The Challenge for Tooling

The separation of these threads presents a challenge:
- **Data Access:** Gameplay data (like the character array) is owned by the logic thread.
- **Rendering Code:** Our ESP rendering code runs on the render thread.

Directly calling a function like `GetContextCollection()` from our render thread hook will fail, as the necessary pointers are not initialized in that thread's TLS. Attempting to do so will correctly return `nullptr`.

The solution is the **"Capture and Store"** pattern, which requires establishing a presence on both threads. This is detailed in the [Game Thread Hook](./../engine_internals/game-thread-hook.md) documentation.