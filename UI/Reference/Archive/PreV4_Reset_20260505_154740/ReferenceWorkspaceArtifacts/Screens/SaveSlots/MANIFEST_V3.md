# SaveSlots MANIFEST V3

## Pass 01

Status: NEEDS_ANOTHER_PASS_CONTINUING

Target: SaveSlots

Reference image: `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\SaveSlots.png`

Geometry map from reference, 1920x1080:

- Full owned wood frame/panel: x=100, y=11, w=1724, h=1038, role=9-slice/fullscreen shell, asset family=dark carved wood panel.
- Back button: x=165, y=61, w=150, h=62, role=horizontal 3-slice, asset family=plain brown button plate.
- Title cluster: x=655, y=61, w=610, h=96, role=live text plus fixed ornaments/dividers, asset family=gold text and simple divider ornament.
- Dropdown field: x=166, y=161, w=243, h=62, role=horizontal 3-slice/dropdown shell, asset family=plain brown dropdown.
- Filter hint/page live text row: x=435, y=181, w=1324, h=34, role=live text.
- Save card 1: x=169, y=255, w=766, h=367, role=9-slice/card shell, asset family=parchment save card.
- Save card 2: x=978, y=255, w=766, h=367, role=9-slice/card shell, asset family=parchment save card.
- Save card 3: x=169, y=634, w=766, h=357, role=9-slice/card shell, asset family=parchment save card.
- Save card 4: x=978, y=634, w=766, h=357, role=9-slice/card shell, asset family=parchment save card.
- Party/hero slots inside each card: 4 columns x 2 rows, each about 88x88 with 10-13 px gaps, role=fixed square image, asset family=simple dark slot with brass trim.
- Card action buttons: each about 333x62, role=horizontal 3-slice, asset family=plain brown button plate.
- Bottom pager buttons: Prev x=177, y=1004, w=179, h=62; Next x=367, y=1004, w=179, h=62; role=horizontal 3-slice disabled/normal, asset family=plain brown button plate.

Generated candidate paths:

- `C:\UE\T66\UI\Reference\Screens\SaveSlots\Working\Pass_01\Candidates\saveslots_reference_derived_textfree_sheet_pass01.png`

Accepted runtime asset paths:

- `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\ScreenArt\saveslots_screen_art_mainmenu_main_menu_scene_plate_v1.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Panels\saveslots_panels_fullscreen_fullscreen_panel_wide.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Panels\saveslots_panels_fullscreen_row_shell_quiet.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Panels\saveslots_panels_save_card_paper_frame.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Panels\saveslots_panels_title_divider_long.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Panels\saveslots_panels_card_divider.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Panels\saveslots_panels_card_ornament_small.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Slots\saveslots_slots_reference_square_slot_frame_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Controls\saveslots_controls_reference_dropdown_field_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Buttons\saveslots_buttons_pill_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Buttons\saveslots_buttons_pill_hover.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Buttons\saveslots_buttons_pill_pressed.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Buttons\saveslots_buttons_pill_disabled.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Buttons\saveslots_buttons_pill_selected.png`

Source files changed:

- None by this pass. `C:\UE\T66\Source\T66\UI\Screens\T66SaveSlotsScreen.cpp` was inspected and already had a modified worktree state before this pass.

Build command/status:

- Asset-only pass initially skipped build per V3 policy.
- Normal development build was attempted after repeated SaveSlots capture timeouts:
  `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -NoHotReloadFromIDE`
- Build status: failed on broad unrelated gameplay compile errors in casino/shop/fountain/wheel-spawn code, including `OpenCasinoShopTab`, `StartWheelSpinHUD`, `SpawnLabFountainOfLife`, `AT66GamblerNPC`, `AT66CasinoInteractable`, and `AT66WheelSpinInteractable`. This was not a narrow SaveSlots compile-only fix.

Screenshot proof path:

- Intended proof path: `C:\UE\T66\UI\Reference\Screens\SaveSlots\Proof\SaveSlots_pass01_working_1920x1080.png`
- Capture command timed out before creating the proof image, including a retry with longer delay/timeout.

Remaining differences:

- Unverified visually in runtime because SaveSlots working capture did not complete.
- Expected next visual risks: generated full wood panel corners may be more ornate than the reference, generated card corners may be denser than the reference, and source layout may still need card/dropdown/button geometry tuning after capture becomes available.

Approved live-data/top-bar-shared differences:

- Runtime labels, save slot numbers, stage/date/time placeholders, empty-slot text, page text, and dropdown text remain live data/text.
- Shared top-bar/header differences are out of scope for this target.

Archived/reset asset paths:

- PreV3 reset archive supplied by prompt: `C:\UE\T66\UI\Reference\Archive\PreV3_Reset_20260505_103841`
- No V2 archived assets were copied back.

Exact next action if not passing:

- Resolve or wait for the unrelated repo-wide gameplay compile blocker if a synced executable is required.
- Retry the exact SaveSlots working capture command:
  `powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Exe C:\UE\T66\Binaries\Win64\T66.exe -Screen SaveSlots -ResX 1920 -ResY 1080 -Output C:\UE\T66\UI\Reference\Screens\SaveSlots\Proof\SaveSlots_pass02_working_1920x1080.png`
- Compare the runtime capture against the reference and do another restrained imagegen/layout pass for any unapproved differences.
