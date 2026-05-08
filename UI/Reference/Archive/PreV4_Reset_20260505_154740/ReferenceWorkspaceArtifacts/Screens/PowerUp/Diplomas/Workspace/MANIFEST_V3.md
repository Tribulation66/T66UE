# PowerUpDiplomas V3 Manifest

## Pass 01

Status: NEEDS_ANOTHER_PASS_CONTINUING

Target: PowerUpDiplomas

Reference: `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\PowerUp.png`

Geometry map at 1920x1080:

- Frozen shared top bar: x=16, y=12, w=1888, h=112, out of scope.
- Owned dark wood screen panel: x=35, y=132, w=1849, h=925, 9-slice/fullscreen panel.
- Title: x=792, y=152, w=336, h=72, live text.
- Diplomas tab: x=640, y=225, w=303, h=58, horizontal 3-slice button, selected.
- Drugs tab: x=964, y=225, w=300, h=58, horizontal 3-slice button, inactive; sibling state preserved.
- Hint strip: x=118, y=291, w=1667, h=67, 9-slice info strip with live text.
- Diploma card row: first card x=55, y=391, w=389, h=595; gap about 32px; horizontal scroll content.
- Card art well: within each card x=130, y=520, w=242, h=215, fixed/sliced simple inset.
- Graduate button: within each card x=81, y=874, w=337, h=80, horizontal 3-slice button with live label/cost.
- Horizontal scrollbar: x=143, y=1010, w=1641, h=34, track/thumb with fixed arrow caps.

Generated candidate paths:

- `C:\Users\DoPra\.codex\generated_images\019df885-d184-7611-8bde-8da75c4a308b\ig_055c5283c8ff78d40169f9fdda853c819a975783774201ed5d.png`
- `C:\UE\T66\UI\Reference\Screens\PowerUp\Diplomas\Workspace\Working\Pass_01\Candidates\PowerUpDiplomas_text_free_sheet_pass01.png`

Accepted runtime asset paths:

- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Diplomas\Buttons\powerup_buttons_pill_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Diplomas\Buttons\powerup_buttons_pill_hover.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Diplomas\Buttons\powerup_buttons_pill_pressed.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Diplomas\Buttons\powerup_buttons_pill_selected.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Diplomas\Buttons\powerup_buttons_pill_disabled.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Diplomas\Panels\powerup_panels_fullscreen_fullscreen_panel_wide.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Diplomas\Panels\powerup_panels_upgrade_card_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Diplomas\Panels\powerup_panels_item_art_well.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Diplomas\Panels\powerup_panels_info_strip.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\Buttons\powerup_buttons_pill_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\Buttons\powerup_buttons_pill_hover.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\Buttons\powerup_buttons_pill_pressed.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\Buttons\powerup_buttons_pill_selected.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\Buttons\powerup_buttons_pill_disabled.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\Controls\powerup_controls_controls_sheet.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\Controls\powerup_controls_reference_dropdown_field_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\Icons\powerup_iconsgenerated_icon_07_coupon_ticket_white_v1.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\Panels\powerup_panels_fullscreen_fullscreen_panel_wide.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\Panels\powerup_panels_fullscreen_row_shell_quiet.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\ScreenArt\powerup_screen_art_mainmenu_main_menu_scene_plate_v1.png`

Source files changed:

- `C:\UE\T66\Source\T66\UI\Screens\T66PowerUpScreen.cpp`
- `C:\UE\T66\Source\T66\Gameplay\T66GameMode.h` compile-only fix: removed a dangling empty `UPROPERTY()` that blocked UHT after unrelated TricksterNPC deletion.

Build command/status:

- Command: `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -NoHotReloadFromIDE`
- First run: failed at `Source\T66\Gameplay\T66GameMode.h(300)` because a dangling empty `UPROPERTY()` had no member declaration.
- Second run: `T66PowerUpScreen.cpp` compiled, but the full build failed on broad unrelated gameplay API/refactor errors: missing `OpenCasinoShopTab`, `StartWheelSpinHUD`, `SpawnLabFountainOfLife`, `SpawnCasinoInteractableIfNeeded`, `SpawnSupportShopAtStartIfNeeded`, missing interactable class declarations, and `T66GameMode_WorldInteractables.cpp` exceeded 100 compile errors.

Screenshot proof path:

- Attempted current capture before edits: `C:\UE\T66\UI\Reference\Screens\PowerUp\Diplomas\Proof\PowerUpDiplomas_pass00_working_1920x1080.png`
- Result: not created; capture script timed out. Other active screen automation processes were present, and the PowerUp capture process was no longer running afterward.
- No post-change working proof was possible because the normal C++ build is blocked by broad unrelated gameplay compile errors, so `Binaries\Win64\T66.exe` does not contain this routing pass.

Remaining differences:

- Visual proof cannot yet validate the owned area.
- Runtime assets are first-pass simple chrome and likely still need exact art/spacing refinement against screenshot proof.
- Drugs sibling state remains routed away from Diplomas; its own runtime folder is still not populated by this pass.

Approved live-data/top-bar differences:

- Shared top bar/header/nav/currency/avatar/back/settings differences are out of scope for this target.
- Live labels, costs, stat names, coupon balance, and owned/unlocked values remain runtime Slate text/data.

Next action if not pass:

- Restore the unrelated gameplay build enough for the normal Development build to complete, without using UAT/staging.
- Rerun the exact build command above.
- Rerun: `powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Exe C:\UE\T66\Binaries\Win64\T66.exe -Screen PowerUp -ResX 1920 -ResY 1080 -Output C:\UE\T66\UI\Reference\Screens\PowerUp\Diplomas\Proof\PowerUpDiplomas_pass01_working_1920x1080.png -ExtraArgs "-T66PowerUpTab=Permanent"`
- Compare against the reference and continue another V3 pass.
