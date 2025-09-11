# Data Access Patterns

This project prioritizes a clean, stable, and high-level approach to accessing game data. Our strategy is to interface with the game's structured data managers directly, rather than relying on low-level entity lists. This document outlines our recommended method and explains why it is superior to legacy alternatives.

## The Recommended Method: Direct Context Access via `ContextCollection`

Our primary and recommended strategy is to use the `ContextCollection`, a master structure that serves as the game's high-level directory for all major gameplay systems.

-   **Entry Point:** A dedicated getter function, `GetContextCollection()`, provides a direct pointer to this master object.
-   **What It Provides:** The `ContextCollection` gives us direct access to clean, pre-filtered, and organized lists of gameplay-relevant entities, including:
    -   `ChCliContext` (at offset `0x98`): Manages the authoritative `CharacterList` and `PlayerList`.
    -   `GdCliContext` (at offset `0x138`): Manages the authoritative `GadgetList`.
-   **Why This is the Best Approach:**
    -   **Architecturally Correct:** This is the game's intended "front door" for accessing gameplay state. By using it, we align our tool with the engine's design, which promotes stability.
    -   **Direct & Structured:** We get pointers directly to the final, detailed data structures (`ChCliCharacter`, `GdCliGadget`). This eliminates the need for complex and brittle pointer-chasing from a generic base type.
    -   **High Performance:** We iterate smaller, pre-filtered lists. To find all gadgets, we only need to loop through the `GadgetList`, not the entire world's entity list.
    -   **Rich Information:** The pointers in these lists lead to structures containing specific, valuable data (e.g., a resource node's `IsGatherable` state) that is not easily accessible from lower-level objects.
-   **Requirement:** This method requires a game thread hook to call `GetContextCollection()` successfully due to its reliance on Thread Local Storage (TLS). This is a one-time setup that is already implemented in this project.

## The Fallback Method: `AgentArray` (Low-Level Enumeration)

An alternative method involves iterating the master `AgentArray`, which is found within the `AgentViewContext`.

-   **Entry Point:** A pointer to the `AgentViewContext`, which contains the `AgentArray`.
-   **What It Is:** A low-level, flat list of every single entity the game is tracking in the world. This is the unfiltered "ground truth" of what exists.
-   **Why This is a Fallback, Not the Primary Choice:**
    -   **Unstructured Data:** The `AgentArray` is a mix of everything: players, NPCs, gadgets, invisible triggers, visual effects, and more. It requires significant manual filtering in our code to separate useful entities from noise.
    -   **Low-Level Pointers:** The pointers in this array are to generic `CAvAgent` wrappers. To get useful gameplay data, one must perform complex and unstable pointer-chasing (`CAvAgent -> CAgentBase -> CCharacter/GdCliGadget`), a process that is highly susceptible to breaking after game updates.
    -   **Inconsistent Data:** Data derived from this low-level source, such as agent coordinates, can sometimes be inconsistent with higher-level game systems.
-   **Valid Use Case:** The `AgentArray` should be considered a powerful **debugging and fallback tool**. If it is ever discovered that a specific entity is visible in-game but missing from the high-level `CharacterList` or `GadgetList`, the `AgentArray` would be the place to find it. For all primary ESP features, however, it is less reliable and less efficient than using the `ContextCollection`.

---

### Finding the `GetContextCollection` Function

There are at least two known function patterns that can be used to retrieve the `ContextCollection` pointer. We have chosen the one that targets a more stable, dedicated function.

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

This method is used in other tools and was considered for this project.

-   **Pattern (`BGFX_CONTEXT_FUNC_PATTERN`):**
    `BA 10 00 00 00 48 8B 04 C8 81 3C 02 62 67 66 78`

-   **Target Function:** This pattern resolves to a larger, more complex function that has dual responsibilities. It contains conditional logic to handle both BGFX rendering tasks and the same TLS lookup for the `ContextCollection`.

-   **Analysis:** This is a less stable target. While it works, it's brittle. A game update that changes anything about the BGFX rendering logic could break this pattern, even if the context-retrieval part of the function remains identical.

### Final Recommendation

The dedicated getter function targeted by **Signature 1 (`CONTEXT_COLLECTION_FUNC_PATTERN`) is the superior and officially recommended approach for this project.** It provides the most stable and reliable method for acquiring the `ContextCollection` pointer, which is the cornerstone of our high-level data access strategy.