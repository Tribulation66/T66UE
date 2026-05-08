# SaveSlots MANIFEST V4

## Inputs

- Target: SaveSlots default screen state
- Reference image: `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\SaveSlots.png`
- Runtime asset folder: `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots`
- Source file: `C:\UE\T66\Source\T66\UI\Screens\T66SaveSlotsScreen.cpp`
- Capture command: `powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Exe C:\UE\T66\Binaries\Win64\T66.exe -Screen SaveSlots -ResX 1920 -ResY 1080 -Output <proof path>`

## Imagegen

- Built-in imagegen used: yes
- Pass 01 candidate: `C:\UE\T66\UI\Reference\Screens\SaveSlots\Working\Pass_01\Candidates\SaveSlots_textfree_sheet_pass01.png`
- Pass 01 rejected: `C:\UE\T66\UI\Reference\Screens\SaveSlots\Archive\Rejected\Pass_01\SaveSlots_textfree_sheet_pass01.png`
- Pass 01 rejection reason: text-free, but too ornate and polished versus the quiet reference; button bevels and outer wood ornament density exceeded the SaveSlots reference.
- Pass 02 accepted sheet: `C:\UE\T66\UI\Reference\Screens\SaveSlots\Working\Pass_02\Candidates\SaveSlots_textfree_sheet_pass02_ACCEPTED.png`
- Pass 02 accepted reason: text-free; no labels, numbers, runtime data, portraits, or screenshots; preserved the required wood shell, parchment save card, dropdown, square slot, and plain brown button families with restrained ornament.

## Accepted Runtime Assets

- `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\ScreenArt\saveslots_screen_art_mainmenu_main_menu_scene_plate_v1.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Panels\saveslots_panels_fullscreen_fullscreen_panel_wide.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Panels\saveslots_panels_fullscreen_row_shell_quiet.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Panels\saveslots_panels_save_card_paper_frame.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Controls\saveslots_controls_reference_dropdown_field_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Slots\saveslots_slots_reference_square_slot_frame_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Buttons\saveslots_buttons_pill_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Buttons\saveslots_buttons_pill_hover.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Buttons\saveslots_buttons_pill_pressed.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Buttons\saveslots_buttons_pill_disabled.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Buttons\saveslots_buttons_pill_selected.png`

## Source Changes

- `C:\UE\T66\Source\T66\UI\Screens\T66SaveSlotsScreen.cpp`
- Placeholder slots now occupy page card positions even when no save matches the active filter, so the empty state still shows four save-slot cards.
- Surface/card geometry was adjusted to better match the reference card scale and placement.
- Preview slot size was reduced to avoid oversized empty slot frames.

## Build And Proof

- Build command used: `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -NoHotReloadFromIDE`
- Build attempts: 7 source-build attempts after source changes; all succeeded.
- Capture note: the capture script reported timeout because Unreal wrote screenshots under `C:\UE\T66\Saved\Cooked\Windows\T66\UI\Reference\Screens\SaveSlots\Proof\...`; each produced screenshot was copied back to the requested proof folder.
- Current proof: `C:\UE\T66\UI\Reference\Screens\SaveSlots\Proof\SaveSlots_pass09_working_1920x1080.png`

## Remaining Differences

- Runtime proof shows the four empty save cards with generated target-owned chrome.
- Approved live-data differences: empty save placeholders remain live Slate text, including slot labels, stage placeholders, empty slot text, and disabled action labels.
- Shared/top-level differences not changed in this target pass: title decoration and outer shared background treatment.
