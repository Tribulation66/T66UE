Codex draft: consolidated Run Will Not Count popup

Task contract
- Operator: Codex
- Validator: Claude
- Scope: build a consolidated "Run will not count" warning with per-popup suppression, support individual/team/offline/backend-style reasons, add Gameplay Settings reset-all for suppressed popups, keep the infrastructure extensible for future popups.
- Stop condition: scoped changes are implemented and verified from source build plus staged executable evidence.

Implemented changes
- Added keyed popup suppression infrastructure to `UT66PlayerSettingsSubsystem` and `UT66PlayerSettingsSaveGame`.
  - New `SuppressedPopupIds` set, schema version 26, migration from existing per-popup booleans.
  - Added `IsPopupSuppressed`, `SetPopupSuppressed`, `ResetAllPopupSuppressions`, and `GetSuppressedPopupCount`.
  - Existing Chad Coupon and party restriction show/hide wrappers now map through stable popup IDs.
- Replaced the old hero-selection party-only leaderboard warning with a generic `RunWillNotCount` popup.
  - Title: `RUN WILL NOT COUNT`.
  - Body uses a concrete reason.
  - Controls: `DO NOT SHOW AGAIN` checkbox and `OKAY` button.
  - Reasons currently include: local account suspended/restricted, party member suspended, backend unavailable, Steam authentication unavailable, offline run, and generic run-ineligible state.
- Generalized party/client messaging.
  - Added `BroadcastRunWillNotCountWarning(const FString& ReasonText)`.
  - Added `ClientShowRunWillNotCountWarning(const FString& ReasonText)`.
  - Kept previous party-leaderboard warning APIs as compatibility wrappers.
- Added Gameplay Settings reset control.
  - `SettingsGameplay.Rows.ResetPopupSuppressions` row.
  - Label `Do Not Show Popups`.
  - Button `RESET ALL`.
  - Handler calls `UT66PlayerSettingsSubsystem::ResetAllPopupSuppressions()`.
- Hardened `Scripts/RunStagedBuildReadinessGate.ps1`.
  - Guarded malformed CIM/DMTF creation date parsing.
  - Narrowed build-process preflight matching so a PowerShell monitor whose command text mentions `BuildCookRun` is not treated as a live build process.

Files changed for this task
- `Source/T66/Core/T66PlayerSettingsSaveGame.h`
- `Source/T66/Core/T66PlayerSettingsSubsystem.h`
- `Source/T66/Core/T66PlayerSettingsSubsystem.cpp`
- `Source/T66/Core/T66SessionSubsystem.h`
- `Source/T66/Core/T66SessionSubsystem.cpp`
- `Source/T66/Gameplay/T66PlayerController.h`
- `Source/T66/Gameplay/T66PlayerController.cpp`
- `Source/T66/UI/Screens/T66HeroSelectionScreen.h`
- `Source/T66/UI/Screens/T66HeroSelectionScreen.cpp`
- `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Build.cpp`
- `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Party.cpp`
- `Source/T66/UI/Screens/T66SettingsScreen.h`
- `Source/T66/UI/Screens/Settings/T66SettingsScreen_Gameplay.cpp`
- `Scripts/RunStagedBuildReadinessGate.ps1`

Verification performed
- `git diff --check -- <touched files>`
  - Exit 0. Only LF-to-CRLF warnings.
- Focused source build:
  - `C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex`
  - PASS. Only unrelated existing Niagara deprecation warning in `T66Hero1AxeAOEVFXLabActor.cpp`.
- Full staged readiness:
  - `powershell -ExecutionPolicy Bypass -File .\Scripts\RunStagedBuildReadinessGate.ps1`
  - PASS.
  - Summary: `C:\UE\T66\Saved\StagedBuildReadiness\20260608_031637\summary.json`
  - Staged exe: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
  - Stage PASS, shortcut checks PASS, smoke suite PASS.
- Focused staged popup proof:
  - `Scripts\CaptureT66UIScreen.ps1 -Screen HeroSelection -ClickTag HeroSelection.BottomRow.DifficultyPanel.EnterButton -ExecCmds="t66.AccountStatus.Force 1"`
  - Screenshot: `C:\UE\T66\Saved\RunWillNotCountPopupProof\20260608_ForceSuspended_ExecCmds\screen.png`
  - Dump: `C:\UE\T66\Saved\RunWillNotCountPopupProof\20260608_ForceSuspended_ExecCmds\dump.json`
  - Log: `C:\UE\T66\Saved\RunWillNotCountPopupProof\20260608_ForceSuspended_ExecCmds\run.log`
  - Evidence: command line set `t66.AccountStatus.Force = "1"`, Slate clicked Enter, dump contains `HeroSelection.RunWillNotCountWarning.Overlay`, body text `Your account is suspended. This run will not count for the leaderboard.`, checkbox `DO NOT SHOW AGAIN`, and button `OKAY`.
- Settings reset row proof:
  - Staged smoke dump: `C:\UE\T66\Saved\StagedBuildReadiness\20260608_031637\smoke_suite\01_FrontendTagClick\04_TopBarSettingsNavigation\dump.json`
  - Evidence: contains `SettingsGameplay.Rows.ResetPopupSuppressions`, label `Do Not Show Popups`, and button text `RESET ALL`.

Caveats
- I did not run a live two-client party session. The host/client messaging code is compiled and staged, and existing party-specific APIs now delegate to the generic RunWillNotCount API.
- The popup and suppression affect warning UX only. Leaderboard eligibility and backend acceptance remain governed by the existing run eligibility/backend paths.
- The worktree was already very dirty with many unrelated modifications and deletions before this task; I did not revert them.
