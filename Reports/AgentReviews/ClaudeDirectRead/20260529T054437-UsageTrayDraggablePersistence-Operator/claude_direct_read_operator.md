Operator work complete. Summary:

**Implemented** draggable persistence for the T66 AI Usage Tray widget within the approved write scope.

- **New:** `Models/WidgetSettings.cs` (position model), `Services/WidgetSettingsService.cs` (load/save `settings.json` + pure `IsPositionVisible` offscreen validation + `ScreenAreaDip`).
- **Modified:** `MainWindow.xaml.cs` — loads settings on startup, saves `Left`/`Top` after `DragMove()`, applies the saved position on the 500 ms loop instead of recomputing the Codex corner, guards the placement tick against in-progress drags, and `Reposition for Codex` now clears the sticky position and re-snaps to the Codex-relative default. `UsageParserTests.cs` — 8 new tests.

**Behavior:** drag saves; saved position survives restart; the placement loop no longer snaps back; offscreen/invalid (<50% on any work-area, or NaN/Inf) falls back to the Codex-relative default; Codex-foreground show/hide is unchanged.

**Settings file:** `%LOCALAPPDATA%\T66UsageTray\settings.json` — `{ "hasUserPosition", "left", "top" }`, camelCase/indented (matches existing service style).

**Verification:** `dotnet build -c Release` → 0 warnings/0 errors; `dotnet test -c Release` → 19/19 passed.

**Key caveats for Codex:** no live GUI smoke test was run (build + unit tests only) — recommend manually verifying drag/restart/reposition; DPI multi-monitor conversion relies on the window's `TransformFromDevice`; the 50% visibility threshold is a tunable judgment call; publish to `…\T66UsageTray\app` and any git commit are left to the Validator. Completion packet written to the report folder.

I did **not** commit, push, or deploy — those remain for Codex per the approval.
