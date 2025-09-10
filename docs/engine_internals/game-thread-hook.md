# Game Thread Hooking

To safely access gameplay data for our ESP, we must execute code on the game's main logic thread. This is achieved by hooking a high-level, per-frame update function. This document details the "what" and the "why" of our approach.

## 1. The Target: What Are We Hooking?

Our target is a virtual function within a core engine class instance. We refer to this instance as the `AlertContext` based on nearby performance marker strings like `"ViewAdvanceAlert"`.

**Why this target?**
- The object instance is a high-level manager that contains the main game loop function, `Arena::Services::Handle::Handle` (or a similar virtual wrapper like `DvContext`).
- Hooking a virtual function (`vtable[0]`, which is typically the `Update` or `Tick` method) is a standard, clean, and stable way to inject our code into a class's update cycle.
- This ensures our code runs exactly once per frame on the correct **game logic thread**, at a point where all game systems are initialized and in a valid state.

## 2. The Strategy: How Do We Find It?

We locate the target vtable function using a robust pattern scan that is resilient to game updates.

**Pattern (`ALERT_CONTEXT_LOCATOR_PATTERN`):**
```
48 8D 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 41 0F 28 CA 48 8B 08 48 8B 51 58
```

**The logic behind this strategy:**
1.  **Find a Unique Anchor:** The game's main update loop calls the update methods for dozens of subsystems in a fixed, predictable order. Our pattern finds a unique sequence of machine code instructions associated with the `"ViewAdvanceAlert"` performance marker. This sequence is far more unique than a generic function prologue and serves as a reliable anchor in memory.
2.  **Navigate with a Fixed Offset:** From this unique anchor, the `CALL` to the `AlertContext`'s getter function is located at a fixed negative offset (`-0x19` bytes). Because the update order is so stable, this offset is also very stable.
3.  **Resolve Pointers:** From the getter `CALL`, we programmatically resolve the relative pointers to get the address of the `AlertContext` instance, then its `vtable`, and finally the address of `vtable[0]`, which is our hook target.

This multi-step, context-aware approach is vastly superior to relying on fragile, direct patterns or magic number offsets from string literals.

## 3. The Implementation (`DetourGameThread`)

Our detour function, `DetourGameThread`, is injected at `vtable[0]`. Its primary responsibility is to execute the **"Capture"** step of our data access pattern. Each frame, it:
1.  Calls `GetContextCollection()`, which now succeeds because it's on the correct thread.
2.  Stores the returned pointer in a shared static variable (`AddressManager::s_pContextCollection`).
3.  Calls the original game function to ensure the game continues to operate normally.