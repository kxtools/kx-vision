# The Multi-threading Model of Guild Wars 2

A critical architectural aspect of the Guild Wars 2 engine is its use of multiple threads to handle different tasks concurrently. A stable tool **must** respect this design to avoid crashes and race conditions.

## Key Threads

### 1. Game Logic Thread (or "Main Thread")
This thread is the heart of the game simulation. It is responsible for updating the state of the game world on a per-frame basis.

**Responsibilities:**
- Processing player input and character movement.
- Executing AI routines for NPCs and monsters.
- Managing character state (health, energy, buffs, position).
- Running the main "tick" or "update" loop for all gameplay systems.

The primary data hub for gameplay, which we call the **`ContextCollection`**, is created, managed, and exists within the memory space of this thread. The game uses **Thread Local Storage (TLS)** to give this thread fast access to its essential pointers.

### 2. Render Thread
This thread's sole purpose is to communicate with the graphics API (DirectX 11) and send drawing commands to the GPU.

**Responsibilities:**
- Receiving the state of the game world from the logic thread.
- Rendering all visuals, including the 3D world, characters, and the game's UI.

Our project's primary hook point, `D3DRenderHook::DetourPresent`, injects our code directly into this thread. This is where all ImGui rendering, including our ESP, occurs.

## The Challenge for Tooling: A Tale of Two Threads

The separation of these threads creates the central architectural challenge for this project:

- **Data is on the Logic Thread:** All the data we want to display (character positions, etc.) is owned and actively modified by the game logic thread.
- **Drawing is on the Render Thread:** Our code to draw the ESP runs on the render thread.

A naive attempt to access game data directly from the render thread will fail for two reasons:
1.  **Invalid Pointers:** Functions like `GetContextCollection()` rely on TLS, which is specific to each thread. Calling it from the render thread will correctly return `nullptr` because the `ContextCollection` pointer doesn't exist in the render thread's TLS.
2.  **Race Conditions:** Even if we could get the pointers, reading data that is being simultaneously written to by another thread is a race condition that will inevitably lead to crashes from reading partially-updated or deallocated memory.

The solution is the **"Capture and Store"** pattern, which requires establishing a presence on both threads. This is detailed in our [Game Thread Hook](./../engine_internals/game-thread-hook.md) and [Data Access Patterns](./../engine_internals/data-access-patterns.md) documentation.