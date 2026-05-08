# Challenges V3 Manifest

Target: Challenges
State: default screen state
Reference: `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\Challenges.png`

## Preflight

- `C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md`: exists, read.
- `C:\UE\T66\Docs\UI\UI_GENERATION.md`: exists, read.
- `C:\UE\T66\Scripts\CaptureT66UIScreen.ps1`: exists.
- `C:\UE\T66\Binaries\Win64\T66.exe`: exists.
- Target source exists: `C:\UE\T66\Source\T66\UI\Screens\T66ChallengesScreen.cpp`.

## Reference Geometry Map

- Shared top bar: x=0, y=0, w=1920, h=126, shared/out-of-scope.
- Owned outer content frame: x=0, y=126, w=1920, h=954, dark wood/gold frame, 9-slice/fill background.
- Title: x=664, y=142, w=594, h=82, live text.
- Source tabs: official x=374 y=243 w=334 h=60; community x=749 y=243 w=429 h=60; create x=1210 y=244 w=382 h=59; horizontal sliced button plates with live labels.
- Status bar: x=42, y=311, w=1838, h=50, horizontal sliced dark strip with live status text.
- Left challenge list panel: x=40, y=383, w=956, h=650, parchment panel with live rows.
- Row plates: x=69, y=405, w=910, h=105 and x=69, y=520, w=910, h=105, parchment row plates with live labels/values.
- Row state sockets: x=91, y=428, w=67, h=67 and x=91, y=543, w=67, h=67, fixed dark socket.
- Right detail panel: x=1030, y=382, w=846, h=650, dark framed panel.
- Detail description paper: x=1052, y=497, w=755, h=170, parchment 9-slice with live description.
- Rules paper: x=1052, y=724, w=755, h=276, parchment 9-slice with live rules.
- Scrollbar: x=1815, y=459, w=37, h=511, fixed/vertical sliced parts.

## Pass 01

Generated candidate paths:

- `C:\UE\T66\UI\Reference\Screens\Challenges\Working\Pass_01\Candidates\challenges_reference_derived_text_free_sheet_pass01.png`
- `C:\UE\T66\UI\Reference\Screens\Challenges\Working\Pass_01\Candidates\*.png`

Accepted runtime asset paths:

- Temporarily promoted Pass 01 slices to `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges`, then rejected after proof.

Build:

- Skipped, asset-only pass.

Proof:

- `C:\UE\T66\UI\Reference\Screens\Challenges\Proof\Challenges_pass01_working_1920x1080.png`

Remaining differences:

- Generated sheet was composite rather than atomic; list parchment and detail parchment were double-laid over runtime layout.
- List sockets appeared on the wrong edge in generated list panel art.
- Owned body layout did not match reference geometry.

Archive:

- Rejection note: `C:\UE\T66\UI\Reference\Screens\Challenges\Archive\Rejected\Pass_01\rejection_note.txt`

## Pass 02

Generated candidate paths:

- `C:\UE\T66\UI\Reference\Screens\Challenges\Working\Pass_02\Candidates\challenges_reference_derived_text_free_sheet_pass02.png`
- `C:\UE\T66\UI\Reference\Screens\Challenges\Working\Pass_02\Candidates\*.png`

Accepted runtime asset paths:

- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Panels\challenges_panels_fullscreen_fullscreen_panel_wide.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Panels\challenges_panels_fullscreen_row_shell_quiet.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Panels\challenges_panels_fullscreen_list_panel_v2.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Panels\challenges_panels_challenge_detail_frame_v2.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Panels\challenges_panels_challenge_row_plate_v2.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Panels\challenges_panels_challenge_detail_paper_v2.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Panels\challenges_panels_challenge_rules_paper_v2.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Panels\challenges_panels_challenge_status_bar_v2.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Buttons\challenges_buttons_pill_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Buttons\challenges_buttons_pill_selected.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Buttons\challenges_buttons_pill_hover.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Buttons\challenges_buttons_pill_pressed.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Buttons\challenges_buttons_pill_disabled.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Controls\challenges_controls_controls_sheet.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Controls\challenges_controls_header_divider_v2.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Controls\challenges_controls_state_socket_v1.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Controls\challenges_controls_tag_pill_v2.png`

Source files changed:

- `C:\UE\T66\Source\T66\UI\Screens\T66ChallengesScreen.cpp`

Build:

- Command: `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -NoHotReloadFromIDE`
- Status: failed on unrelated gameplay compile errors after `T66ChallengesScreen.cpp` compiled. Blocking errors include missing `AT66PlayerController::OpenCasinoShopTab`, missing `AT66PlayerController::StartWheelSpinHUD`, missing `AT66GameMode::SpawnLabFountainOfLife`, undefined `AT66CasinoInteractable` usage, and undefined `TricksterNPC`/wheel spawn symbols in gameplay files.
- Action taken: did not broaden into gameplay compile repair because the errors are not a narrow Challenges/UI blocker.

Proof:

- `C:\UE\T66\UI\Reference\Screens\Challenges\Proof\Challenges_pass02_working_1920x1080.png`

Remaining differences:

- Pass 02 proof used the current executable before the C++ layout changes could be built; it still shows old layout behavior.
- Owned body chrome is closer in asset families, but left panel/list row spacing and right panel geometry still need a built executable proof after the external compile blocker clears.
- Top bar/currency/avatar differences are shared UI and approved out-of-scope for this target.
- Challenge labels, rewards, author/source, description, rules, and status values are live data/text and approved to differ from the static reference placeholders.

Next action:

- Resolve the unrelated gameplay compile blocker or get a clean build slot/source state, rerun the normal Unreal build, then capture `Challenges_pass03_working_1920x1080.png` with the required working visual capture command and continue visual iteration.
