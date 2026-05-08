# PowerUpDrugs MANIFEST V4

Target: PowerUpDrugs
Base screen/modal: PowerUp
Target state: DRUGS / single-use power-up tab
Reference image: C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\PowerUpDrugs.png

## Preflight

- C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md: exists, read.
- C:\UE\T66\Docs\UI\UI_GENERATION.md: exists, read.
- C:\UE\T66\Scripts\CaptureT66UIScreen.ps1: exists.
- C:\UE\T66\Binaries\Win64\T66.exe: exists.
- C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\PowerUpDrugs.png: exists.
- C:\UE\T66\Source\T66\UI\Screens\T66PowerUpScreen.cpp: exists.

## Reference Geometry Map 1920x1080

- Shared top bar/header: x=14, y=10, w=1892, h=116, role=out-of-scope shared top bar, asset family=shared dark wood navigation.
- Screen title: x=805, y=145, w=290, h=54, role=live text, asset family=none.
- Tab row: x=645, y=211, w=624, h=56, spacing=20, role=horizontal 3-slice buttons, asset family=dark wood/gold tab plates.
- Info strip: x=124, y=280, w=1664, h=58, role=9-slice parchment strip, asset family=quiet parchment panel.
- Main content viewport: x=75, y=356, w=1740, h=704, role=scrollable grid, asset family=dark wood background.
- Left category panels: x=76, y=356, w=331, h=339, spacing=20 vertical, role=9-slice dark wood category panels.
- Item cards: x=445, y=356, w=331, h=339, spacing=28 horizontal and 20 vertical, role=9-slice parchment cards.
- Item art wells: x=535, y=436, w=148, h=114, role=fixed/dark image well.
- Buy buttons: x=472, y=636, w=276, h=43, role=horizontal 3-slice dark wood buttons.
- Scrollbar: x=1819, y=340, w=33, h=703, role=vertical 3-slice/fixed rail and thumb.

## Pass 01

Built-in imagegen used: yes.

Generated candidates:
- C:\UE\T66\UI\Reference\Screens\PowerUp\Drugs\Working\Pass_01\Candidates\PowerUpDrugs_textfree_sheet_pass01.png
- C:\UE\T66\UI\Reference\Screens\PowerUp\Drugs\Archive\Rejected\Pass_01\PowerUpDrugs_rejected_sheet_01.png

Sprite sheet quality gate:
- Accepted sheet: C:\UE\T66\UI\Reference\Screens\PowerUp\Drugs\Working\Pass_01\Candidates\PowerUpDrugs_textfree_sheet_pass01.png
- Accepted because: text-free; card, left category panel, tab, buy button, parchment strip, ticket, and scrollbar families correspond to the reference; color temperature, paper tone, wood tone, gold border thickness, bevel strength, and geometry are close enough to begin runtime assembly; no runtime data or labels are baked into the accepted components.
- Rejected sheet: C:\UE\T66\UI\Reference\Screens\PowerUp\Drugs\Archive\Rejected\Pass_01\PowerUpDrugs_rejected_sheet_01.png
- Rejection reason: wrong UI family for this target; it generated broad parchment list rows and oversized frames instead of the PowerUp Drugs card/grid layout, with color/proportion drift from the reference.

Accepted runtime assets:
- C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Drugs\Panels\powerup_panels_upgrade_card_normal.png
- C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Drugs\Panels\powerup_panels_row_shell_quiet.png
- C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Drugs\Panels\powerup_panels_item_art_well.png
- C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Drugs\Panels\powerup_panels_info_strip.png
- C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Drugs\Buttons\powerupdrugs_buttons_buy_normal.png
- C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Drugs\Buttons\powerupdrugs_buttons_buy_selected.png
- C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Drugs\Buttons\powerupdrugs_buttons_buy_disabled.png
- C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Drugs\Buttons\powerupdrugs_buttons_pill_normal.png
- C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Drugs\Buttons\powerupdrugs_buttons_pill_hover.png
- C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Drugs\Buttons\powerupdrugs_buttons_pill_pressed.png
- C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Drugs\Buttons\powerupdrugs_buttons_pill_selected.png
- C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Drugs\Buttons\powerupdrugs_buttons_pill_disabled.png
- C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Drugs\Controls\powerupdrugs_controls_scrollbar_vertical.png
- C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Drugs\Icons\powerupdrugs_icons_coupon_ticket.png
- C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\Panels\powerup_panels_fullscreen_fullscreen_panel_wide.png
- C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\Panels\powerup_panels_fullscreen_row_shell_quiet.png
- C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\Buttons\powerup_buttons_pill_normal.png
- C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\Buttons\powerup_buttons_pill_hover.png
- C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\Buttons\powerup_buttons_pill_pressed.png
- C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\Buttons\powerup_buttons_pill_selected.png
- C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\Buttons\powerup_buttons_pill_disabled.png
- C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\Controls\powerup_controls_controls_sheet.png
- C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\Icons\powerup_iconsgenerated_icon_07_coupon_ticket_white_v1.png
- C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Common\ScreenArt\powerup_screen_art_mainmenu_main_menu_scene_plate_v1.png

Source files changed by this pass:
- None. Existing state-specific source routing in C:\UE\T66\Source\T66\UI\Screens\T66PowerUpScreen.cpp was used without modification.

Build command/status:
- Build skipped. This pass changed PNG/runtime asset files only.

Capture attempts:
- Attempt 1: powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Exe C:\UE\T66\Binaries\Win64\T66.exe -Screen PowerUp -ResX 1920 -ResY 1080 -Output C:\UE\T66\UI\Reference\Screens\PowerUp\Drugs\Proof\PowerUpDrugs_pass01_attempt1_delay3p5_working_1920x1080.png -DelaySeconds 3.5 -ExtraArgs "-T66PowerUpTab=SingleUse" ; failed, screenshot file does not exist.
- Attempt 2: powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Exe C:\UE\T66\Binaries\Win64\T66.exe -Screen PowerUp -ResX 1920 -ResY 1080 -Output C:\UE\T66\UI\Reference\Screens\PowerUp\Drugs\Proof\PowerUpDrugs_pass01_attempt2_delay6_working_1920x1080.png -DelaySeconds 6 -ExtraArgs "-T66PowerUpTab=SingleUse" ; failed, screenshot file does not exist.
- Attempt 3: powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Exe C:\UE\T66\Binaries\Win64\T66.exe -Screen PowerUp -ResX 1920 -ResY 1080 -Output C:\UE\T66\UI\Reference\Screens\PowerUp\Drugs\Proof\PowerUpDrugs_pass01_attempt3_delay10_working_1920x1080.png -DelaySeconds 10 -ExtraArgs "-T66PowerUpTab=SingleUse" ; failed, screenshot file does not exist.

Remaining differences:
- Unknown after runtime asset promotion because all required working visual capture attempts failed before a screenshot was created.

Approved live-data/top-bar differences:
- Shared top bar/header/nav/currency/avatar/back/settings differences are out of scope for this target.
- Live text, item names, values, cost text, and runtime-owned data remain live and are not baked into art.

Next action if not pass:
- Resolve TRUE_BLOCKED_CAPTURE_FAILURE, then rerun the attempt 3 command above or the prompt-provided working visual capture command and compare the resulting screenshot against the reference before any WORKING_VISUAL_PASS claim.
