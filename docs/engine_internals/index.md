# Engine Internals

This section documents the reverse-engineered details of specific Guild Wars 2 engine systems and the patterns used to locate them. Understanding these implementation details is key to maintaining and extending the project.

### Articles

- **[Game Thread Hook](./game-thread-hook.md):**
  Details the methodology for establishing a stable hook on the main game logic thread, which is the cornerstone of our data access strategy.

- **[Data Access Patterns](./data-access-patterns.md):**
  Compares different methods for accessing core gameplay data and justifies why the `ContextCollection` approach was chosen for its superior stability and architectural soundness.