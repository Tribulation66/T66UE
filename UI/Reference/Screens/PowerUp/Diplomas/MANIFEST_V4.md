# PowerUpDiplomas V4 Manifest

## Pass 01

- Target: PowerUpDiplomas
- Base screen/modal: PowerUp
- Target state: DIPLOMAS / permanent power-up tab
- Reference image: `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\PowerUp.png`
- Geometry map: 1920x1080 reference has approved shared top bar at y 10..125; owned PowerUp body starts below it. Body shell x 35 y 134 w 1851 h 928; title centered x 794 y 150 w 332 h 55; tab row centered x 640 y 227 w 621 h 58; info strip x 119 y 291 w 1669 h 70; diploma card x 55 y 393 w 391 h 592 repeated with 33 px spacing; inner art well x 130 y 523 w 244 h 217; button x 82 y 881 w 336 h 79; horizontal scrollbar x 120 y 1014 w 1660 h 36.
- Built-in imagegen used: yes
- Reference-derived sheet generated: yes, `C:\Users\DoPra\.codex\generated_images\019df98a-5e5d-7770-bcf2-f614817c80e4\ig_0340c911dfacc9e90169fa4046a40c819b9603448ef28e6333.png`
- Workspace candidate sheet: `C:\UE\T66\UI\Reference\Screens\PowerUp\Diplomas\Working\Pass_01\Candidates\PowerUpDiplomas_textfree_sheet_pass01.png`
- Rejected sheets: none
- Sprite sheet quality gate: PASS
- Sprite sheet pass reasons: clean text-free sheet; no labels, numbers, title, player data, portraits, or runtime values; parchment, dark wood, bronze/gold frame, card, pill button, coupon, and scrollbar families visually correspond to the reference; restrained ornament density and simple dark slots/buttons preserved. It is acceptable as the source sheet for slicing, with final runtime proof still blocked by capture failure.

## Accepted Runtime Assets

- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Diplomas\Buttons\powerup_buttons_pill_disabled.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Diplomas\Buttons\powerup_buttons_pill_hover.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Diplomas\Buttons\powerup_buttons_pill_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Diplomas\Buttons\powerup_buttons_pill_pressed.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Diplomas\Buttons\powerup_buttons_pill_selected.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Diplomas\Panels\powerup_diplomas_art_placeholder.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Diplomas\Panels\powerup_panels_info_strip.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Diplomas\Panels\powerup_panels_item_art_well.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Diplomas\Panels\powerup_panels_upgrade_card_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\Buttons\powerup_buttons_pill_disabled.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\Buttons\powerup_buttons_pill_hover.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\Buttons\powerup_buttons_pill_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\Buttons\powerup_buttons_pill_pressed.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\Buttons\powerup_buttons_pill_selected.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\Controls\powerup_controls_controls_sheet.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\Controls\powerup_controls_reference_dropdown_field_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\Controls\powerup_controls_scrollbar_horizontal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\Icons\powerup_iconsgenerated_icon_07_coupon_ticket_white_v1.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\Panels\powerup_panels_fullscreen_fullscreen_panel_wide.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\Panels\powerup_panels_fullscreen_row_shell_quiet.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\ScreenArt\powerup_screen_art_mainmenu_main_menu_scene_plate_v1.png`

## Source Files Changed

- `C:\UE\T66\Source\T66\UI\Screens\T66PowerUpScreen.cpp`
- Change: permanent diploma image path now resolves to the Diplomas-owned blank placeholder panel asset instead of non-V4 `SourceAssets/UI/PowerUp/Diplomas/Generated/...` paths.

## Build

- Command: `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -NoHotReloadFromIDE`
- Attempt 1: succeeded.

## Capture Attempts

- Attempt 1: delay 3.5 seconds, command `powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Exe C:\UE\T66\Binaries\Win64\T66.exe -Screen PowerUp -ResX 1920 -ResY 1080 -Output C:\UE\T66\UI\Reference\Screens\PowerUp\Diplomas\Proof\PowerUpDiplomas_pass01_working_1920x1080.png -ExtraArgs "-T66PowerUpTab=Permanent"`, failed with timeout, output file missing.
- Attempt 2: delay 6 seconds, command `powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Exe C:\UE\T66\Binaries\Win64\T66.exe -Screen PowerUp -ResX 1920 -ResY 1080 -Output C:\UE\T66\UI\Reference\Screens\PowerUp\Diplomas\Proof\PowerUpDiplomas_pass01_working_1920x1080.png -ExtraArgs "-T66PowerUpTab=Permanent -T66AutoScreenshotDelay=6"`, failed with timeout, output file missing. This included a duplicate delay flag after the script default.
- Attempt 3: delay 10 seconds, command `powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Exe C:\UE\T66\Binaries\Win64\T66.exe -Screen PowerUp -ResX 1920 -ResY 1080 -DelaySeconds 10 -TimeoutSeconds 120 -Output C:\UE\T66\UI\Reference\Screens\PowerUp\Diplomas\Proof\PowerUpDiplomas_pass01_working_1920x1080.png -ExtraArgs "-T66PowerUpTab=Permanent"`, failed with timeout, output file missing.
- Proof file exists: no
- Working visual screenshot proof: blocked by capture failure.

## Remaining Differences

- Unknown because the working visual screenshot could not be captured after three required attempts.

## Approved Live Data / Shared Top Bar Differences

- Shared top bar/header/nav/currency/avatar/back/settings differences are out of scope for this target prompt.
- Runtime labels, stat values, costs, balances, item names, and descriptions remain live data.

## Next Action

- Resolve the capture automation failure, then rerun the working visual capture command and compare against the reference. Do not claim `WORKING_VISUAL_PASS` until the screenshot exists and owned-area differences are empty or approved-only.
