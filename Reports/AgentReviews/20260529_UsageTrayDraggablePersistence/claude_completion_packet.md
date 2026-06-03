# Claude Completion Packet: Usage Tray Draggable Persistence

Operator: Claude. Validator: Codex. Status: implementation complete, build + tests green.

## Files Changed

New:
- `src/T66.AIUsageTray/Models/WidgetSettings.cs` — persisted widget position model.
- `src/T66.AIUsageTray/Services/WidgetSettingsService.cs` — load/save of `settings.json` plus the pure `IsPositionVisible` offscreen-validation helper and `ScreenAreaDip` struct.

Modified:
- `src/T66.AIUsageTray/MainWindow.xaml.cs` — settings load on startup, drag-to-save, sticky placement, drag guard, `Reposition for Codex` reset.
- `src/T66.AIUsageTray.Tests/UsageParserTests.cs` — 7 new tests + `SettingsPath` on the `TempDir` helper.

Not changed:
- `MainWindow.xaml`, `AppPaths.cs` (existing `SettingsPath` reused as-is), `README.md` — no behavior/command change needed, so left untouched.

## Exact Behavior Implemented

- **Drag saves position.** `Window_MouseLeftButtonDown` wraps `DragMove()` in a try/finally that sets `_isDragging`, then after the drag stores `new WidgetSettings(true, Left, Top)` and persists it.
- **No snap-back on the 500 ms loop.** `UpdateCodexScopedVisibility` now calls `ApplyPlacement(state)`. When `_settings.HasUserPosition` is true and the saved position is still on a visible work area, it sets `Left`/`Top` to the saved values instead of recomputing the Codex-relative corner. The placement tick also returns early while `_isDragging` is true so it never fights an in-progress drag.
- **Survives restart.** Position is read from `settings.json` in `OnLoaded` (`_settings = _widgetSettingsService.Load()`) before the first placement pass.
- **Offscreen / invalid fallback.** `TryGetValidSavedPosition` builds the DIP work-area list from `Forms.Screen.AllScreens` (converted device→DIP via `TransformFromDevice`, matching the existing `PositionWithinCodexWindow` convention) and calls `WidgetSettingsService.IsPositionVisible`. If the saved rect is not at least 50% on some work area (or coords are NaN/Infinity), it falls back to the existing `PositionWithinCodexWindow(state)` default.
- **Codex-foreground visibility preserved.** The hide/show/`Topmost` branch in `UpdateCodexScopedVisibility` is unchanged; only the positioning call was swapped. The widget still hides when Codex isn't foreground and reappears when it returns; the `forceShow` path (double-click / Show-hide) still works.
- **`Reposition for Codex` recovery.** Now calls `RepositionForCodex()`, which clears the sticky position (`WidgetSettings.Empty`), persists that, then forces a default Codex-relative placement — letting the user recover from a bad/offscreen drag.

## Settings File Shape

`%LOCALAPPDATA%\T66UsageTray\settings.json`, camelCase, indented (matches `OperatorStateService` conventions):

```json
{
  "hasUserPosition": true,
  "left": 320.5,
  "top": 180.25
}
```

`Left`/`Top` are WPF DIP coordinates (same space as `Window.Left`/`Top`). When no manual position has been set, `hasUserPosition` is `false` and the Codex-relative default placement is used.

## Verification Commands Run

```
dotnet build C:\Users\DoPra\Tools\AIUsageTray\T66.AIUsageTray.sln -c Release
  -> Build succeeded. 0 Warning(s) 0 Error(s)

dotnet test C:\Users\DoPra\Tools\AIUsageTray\T66.AIUsageTray.sln -c Release --no-build
  -> Passed! Failed: 0, Passed: 19, Skipped: 0, Total: 19
```

New tests added — 8 methods, all passing (11 pre-existing + 8 new = 19 total):
- `WidgetSettingsServiceReturnsEmptyWhenMissing`
- `WidgetSettingsServiceRoundTripsSavedPosition`
- `WidgetSettingsServiceReturnsEmptyOnCorruptJson`
- `IsPositionVisibleAcceptsPositionWithinWorkArea`
- `IsPositionVisibleRejectsFullyOffscreenPosition`
- `IsPositionVisibleRejectsMostlyOffscreenPosition`
- `IsPositionVisibleAcceptsSecondaryMonitor`
- `IsPositionVisibleRejectsInvalidCoordinates`

## Caveats for Codex Validation / Publish

- **No runtime UI test performed.** This is a WPF tray app; I did not launch the GUI. Verification is build + unit tests only. Recommend a manual smoke test: drag the widget, confirm it stays put across a 500 ms tick and across an app restart, then exercise `Reposition for Codex` to confirm it snaps back to the Codex corner.
- **DPI / multi-monitor:** work-area validation uses the window's `TransformFromDevice`, so it depends on the window having a live `PresentationSource` (true after `SourceInitialized`, which precedes `OnLoaded`). If the source is somehow null it falls back to the identity matrix (treats device px as DIP). Worth a glance on a mixed-DPI multi-monitor setup.
- **50% visibility threshold** is a judgment call for "offscreen or invalid"; a widget straddling two monitors with <50% on each would be treated as invalid and fall back to default. Adjust the threshold in `WidgetSettingsService.IsPositionVisible` if you want it more permissive.
- **Publish** to `%LOCALAPPDATA%\T66UsageTray\app` is left for Codex per the approval (not in my write scope to deploy).
- No git commit/push performed — left for the Validator/Finisher.
