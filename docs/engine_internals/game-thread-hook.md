# Game Thread Hook

To safely access gameplay data for our ESP, we must execute code on the game's main logic thread. This is achieved by hooking a high-level, per-frame update function.

## 1. Target Identification

The target is a virtual function within a core engine class instance, which we refer to as the `AlertContext` (as it manages the update cycle for many systems, including UI alerts). This object contains the main game loop function, `Arena::Services::Handle::Handle` (or a similar virtual wrapper like `DvContext`).

## 2. Pattern and Location Strategy

The function is located using a unique signature of machine code, which is more stable than relying on direct memory addresses or fragile offsets from string literals.

**Pattern (`ALERT_CONTEXT_LOCATOR_PATTERN`):**
```
48 8D 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 41 0F 28 CA 48 8B 08 48 8B 51 58
```

**Strategy:**
1.  This pattern identifies a unique code block related to the `ViewAdvanceAlert` performance marker inside the main game loop. This block serves as a stable "anchor".
2.  The `CALL` instruction that retrieves the `AlertContext` instance is located at a fixed negative offset (`-0x19` bytes) from the start of this anchor.
3.  We resolve this relative `CALL` to find the getter function.
4.  The getter function contains a relative `MOV`, which we resolve to find a static pointer to the `AlertContext` instance.
5.  The first 8 bytes of this instance is a pointer to its vtable.
6.  We hook the first function (`vtable[0]`) in this vtable.

## 3. Implementation (`DetourGameThread`)

Our detour function, `DetourGameThread`, is injected at `vtable[0]`. This ensures our code runs once per frame on the correct game logic thread.

Its primary responsibility is to execute the "Capture" step of our data access pattern: it calls the `GetContextCollection()` function and stores the returned pointer in a shared static variable (`AddressManager::s_pContextCollection`) for the render thread to use.

This approach provides a stable, reliable, and architecturally sound method for interacting with the game's core logic.