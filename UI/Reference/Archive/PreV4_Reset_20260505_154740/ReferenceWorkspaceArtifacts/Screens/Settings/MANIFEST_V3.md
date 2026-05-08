# Settings V3 Manifest

## Pass 01

Status: NEEDS_ANOTHER_PASS_CONTINUING

Target: Settings

Reference: `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\Settings.png`

Geometry map at 1920x1080:

- Shared top/header chrome: x=0 y=0 w=1920 h=122. Frozen and out of scope.
- Settings owned tab row: x=25 y=139 w=1818 h=86. Eight dark wood tabs, each roughly 218x78 with 12 px gaps. Selected Retro FX tab has gold trim and a small purple gem at top center.
- Settings owned parchment body shell: x=25 y=224 w=1818 h=812. Aged parchment panel with thin gold/black frame, quiet corners, right scroll rail.
- Retro FX heading block: x=105 y=270 w=1320 h=168. Live heading/body text over parchment, no baked text.
- Master enable row shell: x=103 y=462 w=1619 h=121. Quiet parchment row, live label/description, ON/OFF dark wood buttons at right.
- Pending/apply area: x=132 y=620 w=1594 h=81. Live pending text left, APPLY dark wood button right.
- PS1 section header: x=102 y=705 w=345 h=45. Live section title plus small divider ornament.
- Slider rows: x=103 y=750 w=1619 h=116 and x=103 y=881 w=1619 h=113. Quiet parchment rows with gold slider track/thumb and live values.
- Scrollbar: x=1778 y=275 w=49 h=717. Gold up/down arrow caps, dark vertical track, gold thumb.

Built-in imagegen used: yes

Reference-derived sheet generated: yes

Generated candidate paths:

- `C:\UE\T66\UI\Reference\Screens\Settings\Working\Pass_01\Candidates\settings_reference_textfree_sheet_pass01.png`
- `C:\UE\T66\UI\Reference\Screens\Settings\Working\Pass_01\Candidates\Buttons\settings_buttons_pill_normal.png`
- `C:\UE\T66\UI\Reference\Screens\Settings\Working\Pass_01\Candidates\Buttons\settings_buttons_pill_hover.png`
- `C:\UE\T66\UI\Reference\Screens\Settings\Working\Pass_01\Candidates\Buttons\settings_buttons_pill_pressed.png`
- `C:\UE\T66\UI\Reference\Screens\Settings\Working\Pass_01\Candidates\Buttons\settings_buttons_tab_selected_gem.png`
- `C:\UE\T66\UI\Reference\Screens\Settings\Working\Pass_01\Candidates\Buttons\settings_buttons_pill_selected.png`
- `C:\UE\T66\UI\Reference\Screens\Settings\Working\Pass_01\Candidates\Buttons\settings_buttons_pill_disabled.png`
- `C:\UE\T66\UI\Reference\Screens\Settings\Working\Pass_01\Candidates\Panels\settings_panels_reference_scroll_paper_frame.png`
- `C:\UE\T66\UI\Reference\Screens\Settings\Working\Pass_01\Candidates\Panels\settings_panels_fullscreen_row_shell_quiet.png`
- `C:\UE\T66\UI\Reference\Screens\Settings\Working\Pass_01\Candidates\Controls\settings_controls_reference_dropdown_field_normal.png`
- `C:\UE\T66\UI\Reference\Screens\Settings\Working\Pass_01\Candidates\Controls\settings_controls_controls_sheet.png`
- `C:\UE\T66\UI\Reference\Screens\Settings\Working\Pass_01\Candidates\Controls\settings_controls_reference_progress_meter_sheet.png`
- `C:\UE\T66\UI\Reference\Screens\Settings\Working\Pass_01\Candidates\Controls\settings_controls_scrollbar_track_generated.png`
- `C:\UE\T66\UI\Reference\Screens\Settings\Working\Pass_01\Candidates\Controls\settings_controls_scrollbar_thumb_generated.png`
- `C:\UE\T66\UI\Reference\Screens\Settings\Working\Pass_01\Candidates\Controls\settings_controls_scrollbar_arrow_up.png`
- `C:\UE\T66\UI\Reference\Screens\Settings\Working\Pass_01\Candidates\Controls\settings_controls_scrollbar_arrow_down.png`
- `C:\UE\T66\UI\Reference\Screens\Settings\Working\Pass_01\Candidates\Dividers\settings_dividers_section_heading_rule.png`

Accepted runtime asset paths:

- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Buttons\settings_buttons_pill_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Buttons\settings_buttons_pill_hover.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Buttons\settings_buttons_pill_pressed.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Buttons\settings_buttons_tab_selected_gem.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Buttons\settings_buttons_pill_selected.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Buttons\settings_buttons_pill_disabled.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Panels\settings_panels_reference_scroll_paper_frame.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Panels\settings_panels_fullscreen_row_shell_quiet.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Controls\settings_controls_reference_dropdown_field_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Controls\settings_controls_controls_sheet.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Controls\settings_controls_reference_progress_meter_sheet.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Controls\settings_controls_scrollbar_track_generated.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Controls\settings_controls_scrollbar_thumb_generated.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Controls\settings_controls_scrollbar_arrow_up.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Controls\settings_controls_scrollbar_arrow_down.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Dividers\settings_dividers_section_heading_rule.png`

Archived/reset asset paths:

- PreV3 archive supplied by prompt: `C:\UE\T66\UI\Reference\Archive\PreV3_Reset_20260505_103841`
- Rejected candidates: none in Pass 01.

Source files changed:

- None by this pass. Existing Settings source modifications were already present in the working tree before Pass 01 asset promotion.

Build command/status:

- Command: `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -NoHotReloadFromIDE`
- Status: failed on broad unrelated compile errors outside Settings. Representative blockers include missing `AT66PlayerController::OpenCasinoShopTab`, missing `AT66PlayerController::StartWheelSpinHUD`, missing `AT66GameMode::SpawnLabFountainOfLife`, undefined `AT66CasinoInteractable`, undefined `AT66TricksterNPC`, and missing GameMode world-interactable declarations.

Working visual screenshot proof:

- Not produced. Capture attempts:
- `C:\UE\T66\UI\Reference\Screens\Settings\Proof\Settings_pass00_working_1920x1080.png`: timed out through `CaptureT66UIScreen.ps1`.
- `C:\UE\T66\UI\Reference\Screens\Settings\Proof\Settings_manual_probe_1920x1080.png`: timed out; no isolated log was produced.
- `C:\UE\T66\UI\Reference\Screens\Settings\Proof\Settings_pass01_working_1920x1080.png`: timed out; no isolated log was produced.

Remaining visual differences:

- Unknown until the normal build and working visual capture succeed.
- Likely next visual risks: generated sheet may need a second restrained pass if selected tab proportions, scrollbar thumb, or row-shell scale differs from reference after runtime proof.

Approved live-data/top-bar-shared differences:

- Shared top/header chrome is frozen by prompt.
- Settings tab labels and values stay live.
- Settings body labels, descriptions, values, ON/OFF/APPLY text, and slider values stay live.

Exact next action if not passing:

1. Resolve or wait for resolution of the unrelated broad gameplay compile failures outside Settings.
2. Rerun the normal Unreal build command above.
3. Rerun: `powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Exe C:\UE\T66\Binaries\Win64\T66.exe -Screen Settings -ResX 1920 -ResY 1080 -Output C:\UE\T66\UI\Reference\Screens\Settings\Proof\Settings_pass02_working_1920x1080.png`
4. Compare against the exact reference and run another imagegen/layout pass if any owned-area differences remain.
