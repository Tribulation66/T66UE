Codex Approval: APPROVE

# Codex Operator Approval: Usage Tray Draggable Persistence

## Approval

Codex approves Claude Operator work for this task.

## Goal

Make the T66 AI Usage Tray widget remember its manually dragged screen position and stop snapping back to the default Codex-window corner during the normal visibility refresh loop.

## Operator

- Operator: Claude
- Validator: Codex
- Tool profile: FullOperator

## Approved Write Scope

- `C:\Users\DoPra\Tools\AIUsageTray\src\T66.AIUsageTray\MainWindow.xaml`
- `C:\Users\DoPra\Tools\AIUsageTray\src\T66.AIUsageTray\MainWindow.xaml.cs`
- `C:\Users\DoPra\Tools\AIUsageTray\src\T66.AIUsageTray\AppPaths.cs`
- `C:\Users\DoPra\Tools\AIUsageTray\src\T66.AIUsageTray\Services\*`
- `C:\Users\DoPra\Tools\AIUsageTray\src\T66.AIUsageTray\Models\*`
- `C:\Users\DoPra\Tools\AIUsageTray\src\T66.AIUsageTray.Tests\*`
- `C:\Users\DoPra\Tools\AIUsageTray\README.md` only if the behavior or validation commands need a small update
- `C:\UE\T66\Reports\AgentReviews\20260529_UsageTrayDraggablePersistence\*`

Codex may publish the validated app to:

- `C:\Users\DoPra\AppData\Local\T66UsageTray\app`

## Out Of Scope

- Unreal gameplay, assets, Niagara, Blender, staged builds, and repository runtime systems.
- Taskbar embedding or Windows shell-host changes.
- Redesigning the widget layout, colors, or usage collection behavior.
- Changing `.t66\operator-state.json` or model routing.

## Required Behavior

- Dragging the widget saves its position.
- The saved position is loaded on startup when it is still on a visible screen work area.
- The 500 ms Codex foreground/placement loop must not overwrite a saved user position.
- The widget must still hide when Codex is not the foreground app and reappear when Codex returns.
- The tray menu item `Reposition for Codex` should intentionally reset/recompute the default Codex-relative position. It may also save that position as the new sticky position.
- If a saved position is invalid or offscreen, fall back to the existing Codex-relative default placement.
- Runtime persistence should use `%LOCALAPPDATA%\T66UsageTray`, preferably the existing `settings.json` path.

## Validation Expected From Operator

- Build/test commands or the exact reason a test could not be added.
- File-level summary of changes.
- Notes on persistence format and offscreen fallback.
- Any caveat Codex should verify before publishing.
