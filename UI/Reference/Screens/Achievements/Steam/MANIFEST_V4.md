# AchievementsSteam MANIFEST V4

Target: AchievementsSteam
Base screen/modal: Achievements
Target state: Steam achievements tab
Reference image: `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\Achievements.png`

## Preflight

- `C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md`: exists, read.
- `C:\UE\T66\Docs\UI\UI_GENERATION.md`: exists, read.
- `C:\UE\T66\Scripts\CaptureT66UIScreen.ps1`: exists.
- `C:\UE\T66\Binaries\Win64\T66.exe`: exists.
- Target reference image exists.
- Target source file exists: `C:\UE\T66\Source\T66\UI\Screens\T66AchievementsScreen.cpp`.

## Geometry Map 1920x1080 Reference

- Shared top bar: x 0, y 0, w 1920, h 132, shared frozen/out of scope.
- Owned dark wood achievements body: x 0, y 132, w 1920, h 948, fills remaining viewport.
- Title row: x 586, y 143, w 748, h 85, centered live title with small gold ornaments.
- Steam/Secret tabs: x 647, y 238, w 628, h 62, two 304x44 button plates with 20px gap.
- Total progress panel: x 77, y 312, w 1771, h 130, 9-slice parchment panel.
- Total progress text cells: x 118, y 341, w 1269, h 37 and x 1338, y 341, w 75, h 37, live data.
- Progress track: x 481, y 390, w 962, h 28, horizontal 3-slice track/fill, live percent fill.
- Section header: x 108, y 458, w 352, h 38, live text.
- Achievement rows: x 80, y 502, w 1696, h 86 each, 9-slice row shell with fixed slot/favorite plates.
- Row left slot: x 121, y 521, w 65, h 65, fixed square slot frame.
- Row main text area: x 207, y 520, w 908, h 52, live text.
- Row progress column: x 1149, y 520, w 244, h 67, live text centered.
- Row reward column: x 1396, y 520, w 244, h 67, live text centered.
- Row favorite plate: x 1658, y 519, w 68, h 68, fixed square button plate with live star glyph.
- Scrollbar: x 1819, y 479, w 47, h 556, vertical track/thumb.

## Pass 01

- Built-in imagegen used: yes.
- Generated candidate: `C:\UE\T66\UI\Reference\Screens\Achievements\Steam\Archive\Rejected\Pass_01\AchievementsSteam_text_free_sheet_pass01_REJECTED.png`.
- Rejected: text-free but too ornate in places, included unrelated large frames, had a gray matte/background, and was not a clean exact component-family match for the Achievements Steam reference.

## Pass 02

- Built-in imagegen used: yes.
- Reference-derived sheet generated: `C:\UE\T66\UI\Reference\Screens\Achievements\Steam\Working\Pass_02\Candidates\AchievementsSteam_text_free_sheet_pass02_source.png`.
- Sprite sheet quality gate: PASS.
- Gate reason: text-free, component-only, no labels/numbers/player data/screenshots/portraits, restrained gold trim, warm parchment tone, dark slot plates, simple brown tabs, and scrollbar/progress families correspond to the reference without extra gems/curls/filigree.
- Extraction script: `C:\UE\T66\UI\Reference\Screens\Achievements\Steam\Working\Pass_02\extract_accepted_sheet.py`.

Accepted runtime assets:

- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements\Steam\Panels\achievements_panels_reference_progress_panel_v2.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements\Steam\Panels\achievements_panels_reference_row_shell_v2.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements\Steam\Slots\achievements_slots_reference_square_slot_frame_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements\Steam\Buttons\SquareIcon\normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements\Steam\Buttons\SquareIcon\hover.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements\Steam\Buttons\SquareIcon\pressed.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements\Steam\Buttons\SquareIcon\disabled.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements\Steam\Buttons\SquareIcon\selected.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements\Steam\Buttons\Pill\normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements\Steam\Buttons\Pill\hover.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements\Steam\Buttons\Pill\pressed.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements\Steam\Buttons\Pill\disabled.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements\Steam\Buttons\Pill\selected.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements\Steam\Controls\achievements_controls_progress_track_v2.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements\Steam\Controls\achievements_controls_progress_fill_v2.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements\Steam\Controls\achievements_controls_scrollbar_track_v2.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements\Steam\Controls\achievements_controls_scrollbar_thumb_v2.png`

Source files changed in this pass:

- None by this pass. `C:\UE\T66\Source\T66\UI\Screens\T66AchievementsScreen.cpp` was already dirty at preflight and already contained Steam/Secret state-folder routing.

Build command/status:

- Build attempts: 0.
- Reason: this pass changed only loose runtime PNG assets and workspace manifest/rejection files.

Capture attempts:

- Attempt 1: delay 3.5 seconds, output `C:\UE\T66\UI\Reference\Screens\Achievements\Steam\Proof\AchievementsSteam_pass01_working_1920x1080.png`, failed: screenshot was not created before timeout.
- Attempt 2: delay 6 seconds, output `C:\UE\T66\UI\Reference\Screens\Achievements\Steam\Proof\AchievementsSteam_pass01_working_1920x1080.png`, failed: screenshot was not created before timeout.
- Attempt 3: delay 10 seconds, output `C:\UE\T66\UI\Reference\Screens\Achievements\Steam\Proof\AchievementsSteam_pass02_working_1920x1080.png`, failed: screenshot was not created before timeout.
- Verification: neither output file exists after the failed attempts.

Remaining differences:

- TRUE_BLOCKED_CAPTURE_FAILURE: no current implementation screenshot exists, so final visual comparison cannot be completed in this pass.

Approved live-data/top-bar-shared differences:

- Shared top bar/header/nav/currency/avatar/back/settings component is frozen and out of scope.
- Live text, achievement names/descriptions, progress counts, reward values, and favorite/claim state remain live runtime data.

Next action:

- Re-run the working visual capture after the capture path is available: `powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Exe C:\UE\T66\Binaries\Win64\T66.exe -Screen Achievements -ResX 1920 -ResY 1080 -DelaySeconds 10 -TimeoutSeconds 120 -Output C:\UE\T66\UI\Reference\Screens\Achievements\Steam\Proof\AchievementsSteam_pass02_working_1920x1080.png -ExtraArgs "-T66AchievementsTab=Steam"`.
