# Phase 1B Direct Entry Report

## Goal

Create a repo-native way to reach frontend screens and gameplay modes without mouse click automation.

## Implementation

- Added `Source/T66/Core/T66DirectEntry.h/.cpp`.
- Added command-line entry `-T66Entry=<entry>`.
- Added runtime console commands:
  - `T66.Entry`
  - `T66.Screen`
  - `T66.Run`
- Retained `-T66AutomationTestRoom` as a compatibility alias for `-T66Entry=Run:TestRoom`.
- Added pending direct-gameplay state to `UT66GameInstance` so frontend startup can immediately transition through the existing `TransitionToGameplayLevel()` path.
- Updated frontend startup to consume pending direct gameplay entries before showing a startup screen.
- Gameplay startup clears stale pending direct gameplay state so launching directly into `GameplayLevel` cannot later loop back into gameplay after returning to frontend.
- Documented usage in `ToonStyle/Docs/DirectEntryAutomation.md`.

## Supported Examples

```powershell
T66.exe -T66Entry=Screen:Settings
T66.exe -T66Entry=Screen:HeroSelection
T66.exe -T66Entry=Run:TestRoom -T66Hero=Hero_2
T66.exe -T66Entry=Run:Lab -T66Hero=Hero_2
```

Runtime console:

```text
T66.Screen Settings
T66.Run TestRoom Hero=Hero_2
```

## Steam Deck / Controller Testing Direction

Direct entry solves the first problem: deterministic access without mouse clicks. The second problem is per-screen controller focus validation. That should be a separate focus/navigation harness that starts from direct-entry targets and verifies gamepad movement/accept/back behavior instead of pointer interaction.

## Verification

- Source build passed:
  - `C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat T66 Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex -NoHotReloadFromIDE`
- Staged standalone refresh passed:
  - `.\Scripts\StageStandaloneBuild.ps1 -ClientConfig Development`
- Shortcut verification passed:
  - `C:\UE\T66\T66 Standalone.lnk` targets `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
  - Taskbar `T66 Standalone.lnk` targets the same executable.
- Direct frontend smoke passed:
  - `T66.exe -T66Entry=Screen:Settings -T66AutoScreenshot=C:\UE\T66\Saved\Codex\DirectEntry\settings_smoke.png`
  - Exit code `0`; screenshot exists and shows the Settings screen.
  - Log: `C:\UE\T66\Saved\Codex\DirectEntry\settings_smoke.log`
- Direct TestRoom smoke passed:
  - `T66.exe -T66Entry=Run:TestRoom -T66Hero=Hero_2 -T66GameplayAutoScreenshot=C:\UE\T66\Saved\Codex\DirectEntry\testroom_smoke.png`
  - Exit code `0`; screenshot exists and shows the TestRoom.
  - Log confirms `Direct entry configured gameplay run Category=3 Hero=Hero_2` and frontend startup transitioned to `GameplayLevel`.
  - Log: `C:\UE\T66\Saved\Codex\DirectEntry\testroom_smoke.log`

## Notes

- `T66PlayerController_Frontend.cpp` was locked by a Windows user-mapped section during editing. The staged process was closed, but the mapped section persisted, so the file was replaced via a temp-file swap. No backup artifact remains.
- The worktree already contained many unrelated source/content changes. This pass did not revert or stage unrelated files.
