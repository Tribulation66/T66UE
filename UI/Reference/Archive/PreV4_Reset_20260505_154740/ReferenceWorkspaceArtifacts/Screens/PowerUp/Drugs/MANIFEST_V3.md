# PowerUpDrugs MANIFEST_V3

## Pass 01

Status: NEEDS_ANOTHER_PASS_CONTINUING

Generated candidate paths:
- C:\UE\T66\UI\Reference\Screens\PowerUp\Drugs\Working\Pass_01\Candidates\powerupdrugs_reference_derived_textfree_sheet.png
- C:\UE\T66\UI\Reference\Screens\PowerUp\Drugs\Working\Pass_01\Candidates\Panels\powerupdrugs_panels_fullscreen_panel_wide.png
- C:\UE\T66\UI\Reference\Screens\PowerUp\Drugs\Working\Pass_01\Candidates\Panels\powerupdrugs_panels_info_strip.png
- C:\UE\T66\UI\Reference\Screens\PowerUp\Drugs\Working\Pass_01\Candidates\Panels\powerupdrugs_panels_row_shell_quiet.png
- C:\UE\T66\UI\Reference\Screens\PowerUp\Drugs\Working\Pass_01\Candidates\Panels\powerupdrugs_panels_row_shell_quiet_tall.png
- C:\UE\T66\UI\Reference\Screens\PowerUp\Drugs\Working\Pass_01\Candidates\Panels\powerupdrugs_panels_upgrade_card_normal.png
- C:\UE\T66\UI\Reference\Screens\PowerUp\Drugs\Working\Pass_01\Candidates\Panels\powerupdrugs_panels_upgrade_card_wide.png
- C:\UE\T66\UI\Reference\Screens\PowerUp\Drugs\Working\Pass_01\Candidates\Panels\powerupdrugs_panels_item_art_well.png
- C:\UE\T66\UI\Reference\Screens\PowerUp\Drugs\Working\Pass_01\Candidates\Buttons\powerupdrugs_buttons_pill_normal.png
- C:\UE\T66\UI\Reference\Screens\PowerUp\Drugs\Working\Pass_01\Candidates\Buttons\powerupdrugs_buttons_pill_hover.png
- C:\UE\T66\UI\Reference\Screens\PowerUp\Drugs\Working\Pass_01\Candidates\Buttons\powerupdrugs_buttons_pill_pressed.png
- C:\UE\T66\UI\Reference\Screens\PowerUp\Drugs\Working\Pass_01\Candidates\Buttons\powerupdrugs_buttons_pill_selected.png
- C:\UE\T66\UI\Reference\Screens\PowerUp\Drugs\Working\Pass_01\Candidates\Buttons\powerupdrugs_buttons_pill_disabled.png
- C:\UE\T66\UI\Reference\Screens\PowerUp\Drugs\Working\Pass_01\Candidates\Buttons\powerupdrugs_buttons_buy_normal.png
- C:\UE\T66\UI\Reference\Screens\PowerUp\Drugs\Working\Pass_01\Candidates\Buttons\powerupdrugs_buttons_buy_selected.png
- C:\UE\T66\UI\Reference\Screens\PowerUp\Drugs\Working\Pass_01\Candidates\Buttons\powerupdrugs_buttons_buy_disabled.png
- C:\UE\T66\UI\Reference\Screens\PowerUp\Drugs\Working\Pass_01\Candidates\Controls\powerupdrugs_controls_scrollbar_vertical.png
- C:\UE\T66\UI\Reference\Screens\PowerUp\Drugs\Working\Pass_01\Candidates\Icons\powerupdrugs_icons_coupon_ticket.png

Accepted runtime paths:
- C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Drugs\Panels
- C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Drugs\Buttons
- C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Drugs\Controls
- C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Drugs\Icons

Source files changed:
- C:\UE\T66\Source\T66\UI\Screens\T66PowerUpScreen.cpp

Reference geometry map at 1920x1080:
- Shared top bar frozen: x=11 y=10 w=1898 h=119, out of scope for this prompt.
- Owned title/tab area: title x=803 y=147 w=290 h=50; tabs x=644 y=210 w=625 h=55; tab gap 18.
- Info strip: x=123 y=280 w=1664 h=58, horizontal 9-slice parchment.
- Drugs content left category column: x=77 y=357 w=329 h=696; two dark category cards, each about w=329 h=338, vertical gap 20.
- Drug card grid: x=445 y=356 w=1331 h=698; 4 columns by 2 rows visible; card w=331 h=339; column gap about 28; row gap about 18.
- Scrollbar: x=1803 y=341 w=42 h=717; gold rail and thumb at right edge of owned content.
- Resize roles: fullscreen shell and cards are 9-slice; buttons and tabs are horizontal sliced plates; item wells are fixed/9-slice frame; scrollbar is vertical 3-slice/box; coupon icon is fixed image.

Build command/status:
- Command: & "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -NoHotReloadFromIDE
- Status: failed on unrelated gameplay compile errors before visual proof. Representative blockers: `OpenCasinoShopTab` and `StartWheelSpinHUD` missing from `AT66PlayerController`; `SpawnLabFountainOfLife` missing from `AT66GameMode`; unresolved casino/shop/wheel/fountain types and variables across `T66GameMode_*` and `T66PlayerController_Overlays.cpp`.

Screenshot proof:
- Not produced for Pass 01 after source changes because the normal build failed.

Remaining differences:
- Unknown after Pass 01 because the updated code could not be built and captured.
- Pre-build pass00 capture did not produce a valid target proof; the script left a 5-byte file under `C:\UE\T66\Saved\Cooked\Windows\T66\UI\Reference\Screens\PowerUp\Drugs\Proof\PowerUpDrugs_pass00_working_1920x1080.png`.

Approved live-data/top-bar-shared differences:
- Shared top bar/header/nav/currency/avatar/back/settings components are out of scope.
- Live text, drug names, effect text, costs, owned counts, and runtime item art remain live.

Rejected candidates:
- None in Pass 01.

Exact next action if not passing:
- Restore the unrelated gameplay compile blockers, rerun the normal Unreal build, then capture with:
  `powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Exe C:\UE\T66\Binaries\Win64\T66.exe -Screen PowerUp -ResX 1920 -ResY 1080 -Output C:\UE\T66\UI\Reference\Screens\PowerUp\Drugs\Proof\PowerUpDrugs_pass01_working_1920x1080.png -ExtraArgs "-T66PowerUpTab=SingleUse"`
- Compare against `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\PowerUpDrugs.png` and continue with Pass 02 if any owned-area visible differences remain.
