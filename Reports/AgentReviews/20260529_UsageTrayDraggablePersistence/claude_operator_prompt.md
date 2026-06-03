# Claude Operator Prompt: Usage Tray Draggable Persistence

You are the Operator for the current task. Codex is the Validator/Finisher. Codex approval is in:

`C:\UE\T66\Reports\AgentReviews\20260529_UsageTrayDraggablePersistence\codex_operator_approval.md`

## Task

Implement the approved widget behavior in the T66 AI Usage Tray app:

- The user can drag the floating widget.
- The widget saves its dragged screen position.
- The normal 500 ms Codex foreground/placement refresh must not snap the widget back to its default bottom-right placement after a manual drag.
- The saved position should survive app restart.
- If the saved position is offscreen or invalid, fall back to the current Codex-relative placement.
- Preserve the current behavior where the widget only shows when Codex is foreground, unless the existing tray command explicitly forces show.

## Source Roots

- App source: `C:\Users\DoPra\Tools\AIUsageTray`
- Runtime app data: `C:\Users\DoPra\AppData\Local\T66UsageTray`
- Report folder: `C:\UE\T66\Reports\AgentReviews\20260529_UsageTrayDraggablePersistence`

## Relevant Existing Seams

- `C:\Users\DoPra\Tools\AIUsageTray\src\T66.AIUsageTray\MainWindow.xaml.cs`
  - `Window_MouseLeftButtonDown` currently calls `DragMove()`.
  - `UpdateCodexScopedVisibility()` currently calls `PositionWithinCodexWindow(state)` on every placement tick.
  - `PositionWithinCodexWindow()` writes `Left` and `Top`.
- `C:\Users\DoPra\Tools\AIUsageTray\src\T66.AIUsageTray\AppPaths.cs`
  - already defines `SettingsPath = %LOCALAPPDATA%\T66UsageTray\settings.json`.

## Implementation Guidance

Prefer a small settings model/service over ad hoc scattered JSON logic if that fits the app style. Keep the edit narrowly scoped.

The likely shape is:

- Store `Left`, `Top`, and a flag indicating a user/saved widget position in `%LOCALAPPDATA%\T66UsageTray\settings.json`.
- Load settings during startup before placement is applied.
- After `DragMove()` returns, save the final `Left` and `Top`.
- When the Codex placement loop runs, use a valid saved position instead of recomputing default placement.
- The explicit tray menu command `Reposition for Codex` should reset the saved manual position or overwrite it with the recomputed Codex-relative position, so the user can recover if needed.
- Validate saved positions against visible monitor working areas using WPF/System.Windows.Forms screen data or an equivalent local API.

## Constraints

- Do not change usage collection, operator state semantics, auth/token handling, or widget visual design unless required by the persistence change.
- Do not touch Unreal/game files.
- Do not deploy outside the approved write scope.
- Do not use Anthropic API billing. Use the local Claude Code CLI environment only.

## Output Contract

Write a concise completion packet in:

`C:\UE\T66\Reports\AgentReviews\20260529_UsageTrayDraggablePersistence\claude_completion_packet.md`

Include:

- Files changed.
- Exact behavior implemented.
- Settings file shape.
- Verification commands run and results.
- Any caveats for Codex validation/publish.
