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