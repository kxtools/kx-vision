# Data Access Patterns

There are multiple ways to access game data. This document compares the legacy `AgentArray` method with the superior `ContextCollection` method.

## Method 1: AgentArray (Legacy)

- **Entry Point:** A static pointer to the `AgentViewContext`, which contains an array of `CAvAgent` pointers.
- **Pros:** Relatively simple to find. Provides access to all agents (players, NPCs, gadgets).
- **Cons:**
    - The data is less structured. It requires pointer chasing (`CAvAgent -> CAgentBase -> CCharacter`) to get detailed information.
    - The coordinate system for agent positions (`Z-up`) is different from the MumbleLink and physics systems (`Y-up`), requiring conversion.
    - It is a lower-level system and may not represent the "true" list of gameplay-relevant characters.

## Method 2: ContextCollection (Current Method)

- **Entry Point:** A function, `GetContextCollection()`, that returns a pointer to a master object containing pointers to all major gameplay systems.
- **Pros:**
    - **Architecturally Correct:** This is the game's intended "front door" for accessing gameplay state, evidenced by over 6,000 internal cross-references to the getter function.
    - **Direct and Structured:** Provides direct pointers to high-level managers like `ChCliContext` (Character Context), eliminating complex pointer chains.
    - **Stable:** The entry point function is fundamental to the game's operation and is exceptionally unlikely to be removed or changed.
    - **Consistent Data:** The position data found via this path (`CoChar->vec3PositionVisual`) is often more consistent with other systems and may not require coordinate system conversions.
- **Cons:**
    - Requires a game thread hook to call `GetContextCollection()` successfully due to its reliance on Thread Local Storage (TLS).

## Conclusion

The **`ContextCollection` method is definitively superior.** While it requires the initial setup of a game thread hook, the resulting stability, clarity, and directness of data access make it the ideal foundation for this project. The `AgentArray` is retained as a fallback or for accessing non-character entities.

---

### Finding the `GetContextCollection` Function

There are at least two known function patterns that can be used to retrieve the `ContextCollection` pointer. While both can lead to the same result, they target different functions, and one is significantly more stable and reliable.

#### Signature 1: The Dedicated Getter (Recommended)

This is the method currently used by `kx-vision`.

-   **Pattern (`CONTEXT_COLLECTION_FUNC_PATTERN`):**
    `8B ? ? ? ? ? 65 ? ? ? ? ? ? ? ? BA ? ? ? ? 48 ? ? ? 48 ? ? ? C3`

-   **Target Function:** This pattern resolves to a small, dedicated getter function whose sole purpose is to access the Thread Local Storage (TLS) and return the `ContextCollection` pointer.

    ```c
    // Decompiled representation
    undefined8 GetContextCollection(void)
    {
      return *(undefined8 *)(*(longlong *)(..._tls_index...) + 8);
    }
    ```

-   **Analysis:** This is the ideal target. Single-responsibility functions are far less likely to be changed or refactored by game developers. Its simplicity and directness make the pattern highly resilient to game updates.

#### Signature 2: The Multi-Purpose Function (Legacy/Brittle)

This method is used in other tools (like `kx-zenith`) and was considered for this project.

-   **Pattern (`BGFX_CONTEXT_FUNC_PATTERN`):**
    `BA 10 00 00 00 48 8B 04 C8 81 3C 02 62 67 66 78`

-   **Target Function:** This pattern resolves to a larger, more complex function that has dual responsibilities. It contains conditional logic to handle both BGFX rendering tasks and the same TLS lookup for the `ContextCollection`.

    ```c
    // Decompiled representation
    int FUN_00b59ff0(int param_1)
    {
      // --- Rendering Path ---
      if (DAT_027bec38 != 0) {
        // ... BGFX rendering logic ...
      }
      
      // --- Context Path ---
      // This is the part the pattern targets, but it's inside a larger function.
      *(..._tls_index...) = 0x8799989d; 
      return 0;
    }
    ```

-   **Analysis:** This is a less stable target. While it works, it's brittle. A game update that changes anything about the BGFX rendering logic could break this pattern, even if the context-retrieval part of the function remains identical.

### Final Recommendation

The dedicated getter function targeted by **Signature 1 (`CONTEXT_COLLECTION_FUNC_PATTERN`) is the superior and officially recommended approach for this project.** It provides the most stable and reliable method for acquiring the `ContextCollection` pointer, which is the cornerstone of our data access strategy.