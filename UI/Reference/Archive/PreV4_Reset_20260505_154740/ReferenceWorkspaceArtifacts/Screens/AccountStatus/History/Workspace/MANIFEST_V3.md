# AccountStatusHistory V3 Manifest

## Pass 01

Status: NEEDS_ANOTHER_PASS_CONTINUING
Target: AccountStatusHistory
Built-in imagegen used: yes

Phase 0 exact reference:
- Created `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\AccountStatusHistory.png` from the existing `AccountStatus.png` style/layout anchor.

Geometry map from reference:
- Full screen: 1920 x 1080.
- Shared top bar: x=0 y=0 w=1920 h=134, frozen/out of scope.
- Title band: x=60 y=135 w=1800 h=145, shared screen title and tab row.
- Overview tab inactive: x=647 y=216 w=300 h=54.
- History tab active: x=977 y=216 w=300 h=54.
- Content shell: x=52 y=278 w=1755 h=760.
- Left column: x=86 y=306 w=718 h=716.
- Profile panel: x=86 y=306 w=688 h=185.
- Status panel: x=86 y=498 w=688 h=202.
- Progress panel: x=86 y=713 w=688 h=307.
- Right column: x=842 y=306 w=963 h=716.
- Filter fields: x=868 y=306 w=430 h=54 and x=1354 y=306 w=410 h=54.
- Upper history table: x=850 y=378 w=918 h=292.
- Lower history table: x=850 y=686 w=918 h=292.
- Vertical scrollbar: x=1833 y=278 w=38 h=744.

Reference-derived sheet generated:
- `C:\UE\T66\UI\Reference\Screens\AccountStatus\History\Workspace\Working\Pass_01\Candidates\AccountStatusHistory_textfree_sheet_pass01.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\AccountStatusHistory_textfree_sheet_pass01.png`

Accepted runtime assets:
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Buttons\accountstatus_buttons_help_square.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Buttons\accountstatus_buttons_pill_disabled.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Buttons\accountstatus_buttons_pill_hover.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Buttons\accountstatus_buttons_pill_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Buttons\accountstatus_buttons_pill_pressed.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Buttons\accountstatus_buttons_pill_selected.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Controls\accountstatus_controls_controls_sheet.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Controls\accountstatus_controls_progress_fill.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Controls\accountstatus_controls_progress_track.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Controls\accountstatus_controls_reference_dropdown_field_hover.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Controls\accountstatus_controls_reference_dropdown_field_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Controls\accountstatus_controls_reference_dropdown_field_pressed.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Icons\accountstatus_iconsgenerated_icon_16_dropdown_chevron_v2.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Panels\accountstatus_panels_fullscreen_fullscreen_panel_wide.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Panels\accountstatus_panels_fullscreen_row_shell_quiet.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Panels\accountstatus_panels_reference_scroll_paper_frame.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Slots\accountstatus_slots_reference_square_slot_frame_normal.png`

Source files changed:
- `C:\UE\T66\Source\T66\UI\Screens\T66AccountStatusScreen.cpp`
  - Fixed active content selection so `EAccountTab::History` renders `HistoryContent`.
  - Generalized AccountStatus reference asset lookup from Overview-only to active-state first, then same-screen Common, then legacy fallback.
  - Set the active reference state folder during `BuildSlateUI()`.

Build command/status:
- Command: `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -NoHotReloadFromIDE`
- Status: failed on broad unrelated gameplay compile errors before link/output.
- Representative unrelated blockers:
  - `T66GamblerNPC.cpp`: `AT66PlayerController::OpenCasinoShopTab` missing from declaration.
  - `T66WheelSpinInteractable.cpp`: `AT66PlayerController::StartWheelSpinHUD` missing from declaration.
  - `T66LabOverlayWidget.cpp` and `T66GameMode_Lab.cpp`: `AT66GameMode::SpawnLabFountainOfLife` missing from declaration.
  - `T66GameMode_WorldInteractables.cpp`: many casino/trickster/world interactable declaration/scope errors.
- `T66AccountStatusScreen.cpp` was reached by UBT and did not emit compile errors in the captured build output.

Working visual screenshot proof:
- Not produced after source changes because the normal Unreal build did not complete.
- A pre-build capture attempt at `C:\UE\T66\UI\Reference\Screens\AccountStatus\History\Proof\AccountStatusHistory_pass01_working_1920x1080.png` timed out before creating the screenshot.

Remaining visual differences:
- Requires another pass after a successful normal build and working capture.
- Expected owned-area checks: History tab active state, right-column history tables, filter field geometry, panel trims, scrollbar, and row spacing.

Approved live-data/top-bar-shared differences:
- Shared top bar/header differences are out of scope for this state prompt.
- Live player/account/history values may differ from reference placeholders.

Next action if not passing:
- Restore the unrelated gameplay build break or wait for the branch owner to restore it.
- Rerun the build command above.
- Then run: `powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Exe C:\UE\T66\Binaries\Win64\T66.exe -Screen AccountStatus -ResX 1920 -ResY 1080 -Output C:\UE\T66\UI\Reference\Screens\AccountStatus\History\Proof\AccountStatusHistory_pass02_working_1920x1080.png -ExtraArgs "-T66AccountTab=History"`
- Compare `AccountStatusHistory.png` to the pass 02 capture and continue visual iteration.
