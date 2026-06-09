Codex draft: Pause menu Quit button

Task contract
- Operator: Codex
- Validator: Claude
- Scope: add a separate `Quit` button to the pause menu while keeping existing `Save and Quit` intact.
- Stop condition: code is updated, focused proof is collected, and caveats are reported.

Implemented changes
- Added `UT66PauseMenuScreen::OnQuitClicked()` and `HandleQuitClicked()`.
- Added a new pause-menu button tagged `PauseMenu.QuitButton` with text `QUIT`.
- Kept `Save and Quit` unchanged; it still calls the existing save-and-return-to-frontend path.
- Routed the new `Quit` button through `UT66ShutdownSubsystem::RequestQuitGame(ET66ShutdownReason::UserQuit, 0)`, with `UKismetSystemLibrary::QuitGame` fallback if the shutdown subsystem is unavailable.
- Unpaused before requesting shutdown so quit works from a paused state.
- Tightened the pause-menu layout from six 95px buttons to seven 84px buttons, grew the panel slightly, and kept all controls inside the modal.
- Updated the pause-menu structural inventory and checklist to include `PauseMenu.QuitButton` and the new normalized layout.

Files changed for this task
- `Source/T66/UI/Screens/T66PauseMenuScreen.h`
- `Source/T66/UI/Screens/T66PauseMenuScreen.cpp`
- `UI/Geometry/pause_menu_structural_inventory.md`
- `UI/Checklists/pause_menu_checklist.md`

Verification performed
- `git diff --check -- Source/T66/UI/Screens/T66PauseMenuScreen.h Source/T66/UI/Screens/T66PauseMenuScreen.cpp UI/Geometry/pause_menu_structural_inventory.md UI/Checklists/pause_menu_checklist.md`
  - Exit 0. Only LF-to-CRLF warnings.
- Focused editor build:
  - `C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex`
  - PASS.
- Full staged readiness:
  - `powershell -ExecutionPolicy Bypass -File .\Scripts\RunStagedBuildReadinessGate.ps1`
  - PASS.
  - Summary: `C:\UE\T66\Saved\StagedBuildReadiness\20260608_034107\summary.json`
  - Stage PASS, shortcut checks PASS, smoke suite PASS.
- Focused staged pause-menu capture:
  - Screenshot: `C:\UE\T66\Saved\PauseMenuQuitButtonProof\20260608_034107\screen.png`
  - Dump: `C:\UE\T66\Saved\PauseMenuQuitButtonProof\20260608_034107\dump.json`
  - Log: `C:\UE\T66\Saved\PauseMenuQuitButtonProof\20260608_034107\run.log`
  - Evidence: dump contains `PauseMenu.QuitButton`, text `QUIT`, neighboring `SAVE AND QUIT`, `RESTART`, and `LEADERBOARD`; screenshot shows seven-button stack fits inside the modal.
- Focused staged click-to-exit proof:
  - Command clicked `PauseMenu.QuitButton` with `CaptureT66UIScreen.ps1 -WaitForExit`.
  - Log: `C:\UE\T66\Saved\PauseMenuQuitButtonProof\20260608_034107_ClickQuit\run.log`
  - Evidence: Slate click resolved `PauseMenu.QuitButton`; log shows `[Shutdown] Begin Reason=UserQuit`, `[Shutdown] Complete Reason=UserQuit`, `RequestExit ExitCode=0 Tag=T66ShutdownSystem`, and process exited cleanly with `ExitCode=0`.

Decisions and caveats
- `Quit` means quit to desktop through the shutdown owner. `Save and Quit` remains the save-and-return-to-frontend action.
- I did not route the pause-menu `Quit` button through the existing `QuitConfirmation` modal because the frontend UI manager is single-modal in related code paths, and the user's request was for a direct new pause-menu button distinct from `Save and Quit`.
- The worktree was already very dirty with unrelated changes before this task; I did not revert unrelated work.
