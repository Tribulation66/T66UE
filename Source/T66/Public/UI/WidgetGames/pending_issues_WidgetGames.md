# Pending Issues - WidgetGames

## Module Extraction for WidgetGames

- Severity tag: [Minor]
- What's wrong: WidgetGames infrastructure currently lives inside the T66 main module per the v3 review decision to avoid cycle risk during initial implementation. Now that the foundation is stabilizing across casino and frontend game surfaces, extraction to a dedicated `T66WidgetGames` module is feasible but not yet designed.
- Why it's out of scope now: Pablo deferred extraction until after the full backend integration completes. Session 3A is additive registry groundwork and must not introduce module churn.
- What fixing it would entail: Create a `T66WidgetGames` module, move `Source/T66/Public/UI/WidgetGames` and `Source/T66/UI/WidgetGames` into it, update `T66.Build.cs` dependency chains, resolve cycle risks around launch ownership in T66 main, UI manager references, casino integration, update include paths across consumers, and validate a clean build.
