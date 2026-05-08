# HeroSelection MANIFEST_V4

## Pass 01

Status: TRUE_BLOCKED_CAPTURE_FAILURE, later cleared by retry.

Preflight:
- C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md: exists and read.
- C:\UE\T66\Docs\UI\UI_GENERATION.md: exists and read.
- C:\UE\T66\Scripts\CaptureT66UIScreen.ps1: exists.
- C:\UE\T66\Binaries\Win64\T66.exe: exists.
- C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\HeroSelection.png: exists and viewed.
- All target source files listed in the V4 prompt exist.

Reference geometry map at 1920x1080:
- Left shell: x=18, y=26, w=575, h=833, role=9-slice dark wood/parchment panel, asset family=left fashion panel.
- Left top buttons/header: BACK x=40 y=53 w=137 h=54; FASHION title x=253 y=61; currency x=456 y=54 w=119 h=54, role=live text over simple dark buttons.
- Left parchment list: x=39, y=128, w=539, h=242, row height about 114, role=9-slice parchment rows with live labels/actions.
- Left bottom buff slots inside panel: y=756, four square plus slots plus CLEAR button, role=fixed square slots / horizontal sliced clear button.
- Center preview area: x=613, y=27, w=780, h=831, role=live 3D preview background and hero model with top carousel.
- Top hero carousel: x=622, y=27, w=764, h=98, 7 visible portrait slots plus arrows, role=fixed image slots and live portraits.
- Right shell: x=1405, y=26, w=496, h=833, role=9-slice dark wood/parchment panel, asset family=right info panel.
- Right header/settings: HERO NAME x=1441 y=61; settings button x=1718 y=54 w=155 h=54, role=live text over button.
- Right preview parchment: x=1429, y=128, w=436, h=304, role=9-slice parchment panel.
- Right medal row: x=1429, y=443, w=436, h=66, role=9-slice parchment row with live labels/icons.
- Right stats panel: x=1429, y=523, w=436, h=220, role=9-slice parchment panel with live stat text.
- Right ability row: x=1429, y=756, w=436, h=77, role=9-slice parchment row with live ability icon/text.
- Bottom left party bar: x=18, y=877, w=720, h=176, role=wide dark shell with four fixed party slots and ready button.
- Bottom center companion bar: x=743, y=878, w=500, h=175, role=wide dark shell with two name buttons and companion button.
- Bottom right run bar: x=1248, y=878, w=654, h=175, role=wide dark shell with dropdown, enter button, challenge icon.

## Pass 16

Status: WORKING_VISUAL_PASS

Generated candidate paths:
- C:\UE\T66\UI\Reference\Screens\HeroSelection\Working\Pass_01\Candidates\HeroSelection_textfree_sheet_pass01.png
- Original built-in imagegen output: C:\Users\DoPra\.codex\generated_images\019df989-1f07-72c0-8b41-33917d7dfaaa\ig_0db65db1fce2d72d0169fa3feafa3c819ba9464099973196da.png

Sprite sheet quality gate:
- PASS: accepted C:\UE\T66\UI\Reference\Screens\HeroSelection\Working\Pass_01\Candidates\HeroSelection_textfree_sheet_pass01.png.
- Reasons: generated from the exact HeroSelection reference screenshot; text-free; removes labels, numbers, names, hero portraits, and runtime values; preserves the main dark wood, parchment, gold trim, bottom bar, carousel, square slot, and central stone preview families with restrained decoration.
- Rejected sheets: none.

Accepted runtime asset paths:
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Buttons\heroselection_buttons_imagegen20260505_slim_pill_disabled.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Buttons\heroselection_buttons_imagegen20260505_slim_pill_hover.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Buttons\heroselection_buttons_imagegen20260505_slim_pill_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Buttons\heroselection_buttons_imagegen20260505_slim_pill_pressed.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Buttons\heroselection_buttons_imagegen20260505_slim_pill_selected.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Buttons\heroselection_buttons_imagegen20260505_slim_square_hover.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Buttons\heroselection_buttons_imagegen20260505_slim_square_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Buttons\heroselection_buttons_imagegen20260505_slim_square_pressed.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Buttons\heroselection_buttons_imagegen20260505_slim_square_selected.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Controls\heroselection_controls_imagegen20260505_slim_dropdown_field.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Controls\heroselection_controls_imagegen20260505_slim_scrollbar_rail.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Controls\heroselection_controls_imagegen20260505_slim_scrollbar_thumb.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Icons\heroselection_iconsgenerated_icon_07_coupon_ticket_white_v1.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Panels\heroselection_panels_imagegen20260505_slim_parchment_panel.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Panels\heroselection_panels_imagegen20260505_slim_parchment_row.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Panels\heroselection_panels_imagegen20260505_slim_tall_panel.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Panels\heroselection_panels_imagegen20260505_slim_wide_panel.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Slots\heroselection_slots_portrait_slot_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Slots\heroselection_slots_portrait_slot_selected.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Slots\heroselection_slots_reference_square_slot_frame_normal.png`

Archived/reset asset paths:
- PreV4 reset archive noted by prompt: C:\UE\T66\UI\Reference\Archive\PreV4_Reset_20260505_154740
- No Pass 01 rejected candidates.

Source files changed:
- C:\UE\T66\Source\T66\UI\Screens\HeroSelection\T66HeroSelectionScreen_Build.cpp
- C:\UE\T66\Source\T66\UI\Screens\HeroSelection\T66HeroSelectionPreviewController.cpp
- C:\UE\T66\Source\T66\UI\Screens\HeroSelection\T66HeroSelectionPreviewController.h

Build command/status:
- & "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -NoHotReloadFromIDE
- Attempts: multiple normal Unreal builds during passes 04-16. Final build succeeded.
- One narrow compile-only issue in T66HeroSelectionPreviewController.cpp was fixed by constructing the weak pointer from a non-const controller pointer inside a const method.

Capture attempts:
- Pass 01 attempts at 3.5, 6, and 10 seconds failed before the retry cleared the capture issue.
- Pass 02 captured C:\UE\T66\UI\Reference\Screens\HeroSelection\Proof\HeroSelection_pass02_working_1920x1080.png.
- Pass 03 captured after asset promotion.
- Passes 04-16 captured after layout/preview fixes.
- Final proof: C:\UE\T66\UI\Reference\Screens\HeroSelection\Proof\HeroSelection_pass16_working_1920x1080.png.

Remaining differences:
- None in owned UI chrome/layout that block the working visual pass.

Approved live-data/top-bar-shared differences:
- Hero name, stat values, medal/rank placeholders, selected hero model, selected hero skin/body, portrait carousel content, achievement coin balance, party identity, and ability icon are live runtime data and can differ from the static reference.
- Shared top-bar/header behavior remains out of scope for this HeroSelection prompt.

Exact next action:
- Coordinator packaged proof after all individual target agents finish.

## Pass 17-21 Layout Follow-up

Status: LAYOUT_ITERATION_CAPTURED

Source files changed:
- C:\UE\T66\Source\T66\UI\Screens\HeroSelection\T66HeroSelectionScreen_Build.cpp

Build command/status:
- & "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -NoHotReloadFromIDE
- Pass 17 build attempt 1 reported a transient unrelated T66TowerMapTerrain.cpp compile error at line 550; retry succeeded without source changes.
- Pass 19 build attempt 1 failed because CrashReportClient.exe temporarily locked C:\UE\T66\Binaries\Win64\T66.exe; after the required one-minute wait, retry succeeded.
- Final layout follow-up build for pass 21 succeeded.

Capture attempts:
- Pass 17 captured C:\UE\T66\UI\Reference\Screens\HeroSelection\Proof\HeroSelection_pass17_working_1920x1080.png.
- Pass 18 attempts at 3.5 and 6 seconds timed out; 10-second retry captured C:\UE\T66\UI\Reference\Screens\HeroSelection\Proof\HeroSelection_pass18_working_1920x1080.png.
- Pass 19 captured C:\UE\T66\UI\Reference\Screens\HeroSelection\Proof\HeroSelection_pass19_working_1920x1080.png.
- Pass 20 captured C:\UE\T66\UI\Reference\Screens\HeroSelection\Proof\HeroSelection_pass20_working_1920x1080.png.
- Pass 21 captured C:\UE\T66\UI\Reference\Screens\HeroSelection\Proof\HeroSelection_pass21_working_1920x1080.png.

Layout changes made:
- Enlarged footer panel height and moved the bottom row to the reference baseline.
- Horizontally scaled the left and right footer panels so their x/width coverage matches the reference bottom bars without changing the upper side-panel column widths.
- Kept the center companion footer at the reference width and increased its reserved chrome height.
- Centered side-footer controls inside the enlarged footer panels so party/run buttons keep closer natural proportions.

Remaining differences:
- Current pass 21 outer bottom panel sizes and x/y anchors are much closer to the reference.
- Side-footer fill art is now visibly parchment-toned instead of the darker reference footer family after the full-height inner framing change.
- Live hero, portrait, stat, rank, currency, party, and ability data remain approved runtime differences.

Exact next action:
- If continuing beyond this layout pass, restore the dark footer art family while preserving pass 21 panel geometry, then capture the next proof.
