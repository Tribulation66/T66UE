# SaveSlots Reference UI Manifest Master

Status: READY_FOR_CENTRAL_BUILD_AND_CAPTURE  
Target: SaveSlots  
Base screen/modal: Screen  
Target state: Default/SaveSlots  
Coordinator worker mode: yes  
Phase: B4 - button containment and card density correction

## Scope Boundary

Owned area: SaveSlots screen-owned generated component sheet, SaveSlots screen runtime asset folder, and SaveSlots screen source only.

Protected/shared areas:
- MainMenu, Shared, sibling screens, build, and capture were not touched.
- Source edits were confined to:
  - `C:\UE\T66\Source\T66\UI\Screens\T66SaveSlotsScreen.cpp`
- Header remains unchanged:
  - `C:\UE\T66\Source\T66\UI\Screens\T66SaveSlotsScreen.h`
- SaveSlot card owns its Preview/Load buttons at runtime. Generated parent card art does not bake Preview/Load buttons, slots, rows, text, icons, or mini-layout.

## Second-Pass Visual Review Response

Central build/capture before this pass: BUILD PASS, VISUAL PASS FAIL.  
Capture reviewed: `C:\UE\T66\UI\Reference\Screens\SaveSlots\Proof\SaveSlots_central_1920x1080.png`

Changes made for B2:
- Kept the 1920x1080 reference-occupancy canvas approach.
- Reworked each save-card interior onto a card-local `SConstraintCanvas`.
- Bound internal card title, title divider line, title ornament, slot grid, right-side vertical divider, right info block, horizontal separator, Preview button, and Load button to normalized reference-relative rects.
- Increased slot cell size to the normalized 86x86 target and darkened placeholder slot interiors so the gold/chrome frame reads more strongly.
- Added flanking title ornaments and retained the header divider under `LOAD GAME`.
- Kept Preview/Load buttons inside each card at their normalized reference y/height.
- Did not build or capture.

## Third-Pass Containment Correction

Second central capture result: VISUAL PASS FAIL with card-internal regression.

Correction made:
- Removed the inline card `SConstraintCanvas` slot chain that allowed default constraint anchors to interpret offsets like margins.
- Rebuilt each card as a fixed local 754x365 map using `AddSaveFlowCanvasSlot`, the same absolute x/y/w/h helper used by the top-level reference canvas.
- Kept all card children inside the card parent: title, title dividers, title ornament, 8-slot grid, right metadata block, vertical divider, horizontal separator, Preview button, and Load button.
- Kept the top-level screen/card normalized rects unchanged.
- Did not build or capture.

## Fourth-Pass Containment/Density Correction

Third central capture result: VISUAL PASS FAIL.

Correction made:
- Kept all top-level card rects unchanged.
- Reduced per-card Preview/Load button height from 61 to 54 and moved both buttons from y=289 to y=279 so each button rect is fully inside the 754x365 card parent.
- Increased slot frame size to 90 and tightened slot spacing so the left block reads denser and more chrome-forward while staying inside the card.
- Moved the title and slot grid upward slightly and strengthened title divider opacity.
- Kept all card child rects inside the local card map. Bounds check: button bottom is 333, slot grid bottom is 264, text block bottom is 260, title/ornament bottom is 58; all are within card height 365.
- Did not build or capture.

## Reference Scale Correction

Exact reference image: `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\SaveSlots.png`

Native reference size: 1672x941  
Normalized implementation target: 1920x1080  
scaleX: 1920 / 1672 = 1.1483253589  
scaleY: 1080 / 941 = 1.1477151966

The implementation binds to the normalized 1920x1080 rects. Native rects below are the corresponding source-space rects from the 1672x941 reference.

## Generated / Approved Sheet

| Pass | Built-in imagegen | Reference-derived | Candidate path | Result |
|---|---:|---:|---|---|
| 01 | yes | yes | `C:\UE\T66\UI\Reference\Screens\SaveSlots\Working\Pass_01\Candidates\SaveSlots_textfree_atomic_sheet_pass01_ACCEPTED.png` | coordinator-approved with conditions |

Original built-in output retained:
- `C:\Users\DoPra\.codex\generated_images\019dfaa9-11f7-7462-be10-25b925f5d8f0\ig_0fea4cea86a2c6f80169fa89b017c4819a8924c8e7949354ba.png`

Rejected candidate paths:
- none.

## Runtime Assets Promoted

All promoted assets are target-owned transparent PNGs under `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots`. The generated green sheet background was used only as a candidate-sheet/chroma background; promoted runtime slices have no opaque green pixels.

| Asset | Path | Contract |
|---|---|---|
| Outer/fullscreen wood shell | `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Panels\saveslots_panels_fullscreen_fullscreen_panel_wide.png` | 9-slice / tiled interior |
| Screen art compatibility shell | `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\ScreenArt\saveslots_screen_art_mainmenu_main_menu_scene_plate_v1.png` | fixed/background plate |
| Save-card paper shell | `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Panels\saveslots_panels_save_card_paper_frame.png` | 9-slice |
| Quiet row/paper strip | `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Panels\saveslots_panels_fullscreen_row_shell_quiet.png` | horizontal 3-slice |
| Dropdown field | `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Controls\saveslots_controls_reference_dropdown_field_normal.png` | horizontal 3-slice |
| Slot frame | `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Slots\saveslots_slots_reference_square_slot_frame_normal.png` | fixed/uniform |
| Button normal | `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Buttons\saveslots_buttons_pill_normal.png` | horizontal 3-slice |
| Button hover | `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Buttons\saveslots_buttons_pill_hover.png` | horizontal 3-slice |
| Button pressed | `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Buttons\saveslots_buttons_pill_pressed.png` | horizontal 3-slice |
| Button disabled | `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Buttons\saveslots_buttons_pill_disabled.png` | horizontal 3-slice |
| Button selected | `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Buttons\saveslots_buttons_pill_selected.png` | horizontal 3-slice |
| Scrollbar rail | `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Controls\saveslots_controls_scrollbar_rail.png` | vertical 3-slice |
| Scrollbar thumb | `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Controls\saveslots_controls_scrollbar_thumb.png` | vertical 3-slice |
| Divider long | `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Controls\saveslots_controls_divider_long.png` | fixed / horizontal stretch |
| Ornament diamond | `C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots\Controls\saveslots_controls_ornament_diamond.png` | fixed |

## Atomic Component Table

| Component name | Atomic role | Parent/child status | Runtime background expectation | Resize contract | Status |
|---|---|---|---|---|---|
| `saveslots_fullscreen_wood_frame_shell` | Empty screen frame/panel shell | parent only; no child controls baked | transparent outside, RGB 0,0,0 where alpha 0 | 9-slice / tiled interior | promoted |
| `saveslots_save_card_paper_shell` | Empty save-card panel shell | parent only; no child controls baked | transparent outside, RGB 0,0,0 where alpha 0 | 9-slice | promoted |
| `saveslots_quiet_row_strip` | Empty paper row/strip | child component only | transparent outside, RGB 0,0,0 where alpha 0 | horizontal 3-slice | promoted |
| `saveslots_square_slot_frame_normal` | Single empty square slot/frame | child component only | transparent outside, RGB 0,0,0 where alpha 0 | fixed/uniform | promoted |
| `saveslots_button_normal` | Single empty button state | child component only | transparent outside, RGB 0,0,0 where alpha 0 | horizontal 3-slice | promoted |
| `saveslots_button_hover` | Single empty button state | child component only | transparent outside, RGB 0,0,0 where alpha 0 | horizontal 3-slice | promoted |
| `saveslots_button_pressed` | Single empty button state | child component only | transparent outside, RGB 0,0,0 where alpha 0 | horizontal 3-slice | promoted |
| `saveslots_button_disabled` | Single empty button state | child component only | transparent outside, RGB 0,0,0 where alpha 0 | horizontal 3-slice | promoted |
| `saveslots_button_selected` | Single empty button state | child component only | transparent outside, RGB 0,0,0 where alpha 0 | horizontal 3-slice | promoted |
| `saveslots_dropdown_field_shell` | Empty dropdown field | child component only | transparent outside, RGB 0,0,0 where alpha 0 | horizontal 3-slice | promoted |
| `saveslots_scrollbar_rail` | Scrollbar rail | child component only | transparent outside, RGB 0,0,0 where alpha 0 | vertical 3-slice | promoted |
| `saveslots_scrollbar_thumb` | Scrollbar thumb | child component only | transparent outside, RGB 0,0,0 where alpha 0 | vertical 3-slice | promoted |
| `saveslots_divider_ornaments` | Small divider/ornament pieces | child component only | transparent outside, RGB 0,0,0 where alpha 0 | fixed | promoted |

Atomic component gate: PASS.

## Sprite Sheet Quality Acceptance Table

Accepted sheet: `C:\UE\T66\UI\Reference\Screens\SaveSlots\Working\Pass_01\Candidates\SaveSlots_textfree_atomic_sheet_pass01_ACCEPTED.png`

| Check | Result | Notes |
|---|---|---|
| Color temperature | PASS | warm parchment, dark wood, and muted gold match reference family |
| Brightness | PASS | comparable contrast; not washed out or excessively dark |
| Paper tone | PASS | SaveSlot card shell uses warm tan parchment with subtle grain |
| Wood tone | PASS | outer frame shell uses dark brown wood with reference-like bands |
| Metal/gold tone | PASS | gold trim stays muted and restrained |
| Border thickness | PASS | frame, card, slot, and button borders correspond to reference proportions |
| Ornament density | PASS | corner/divider details are restrained; no extra dense filigree inside parent controls |
| Grain/noise | PASS | visible texture without heavy damage or blur |
| Bevel strength | PASS | button and slot bevels read like reference chrome |
| Contrast | PASS | dark slots and brown buttons preserve reference separation |
| Geometry/proportions | PASS | component families correspond to full-screen panel, card shell, slot, button, dropdown, scrollbar, and divider shapes |
| Component correspondence | PASS | every generated family maps to a visible reference UI family |
| Text-free status | PASS | no labels, title, numbers, save metadata, player data, or watermark |
| Atomic-component status | PASS | parent shells are empty; child controls are separate components |

## Corrected Reference Occupancy Map

| Component | Role | Parent | Native rect x,y,w,h | Normalized 1920 rect x,y,w,h | Intended implementation rect |
|---|---|---|---|---|---|
| `screen_outer_frame` | full screen wooden frame | `viewport` | 87.1,8.7,1496.1,925.3 | 100,10,1718,1062 | 100,10,1718,1062 |
| `back_button` | nav button | `screen_outer_frame` | 147.2,52.3,124.5,54.0 | 169,60,143,62 | 169,60,143,62 |
| `title_live_text_region` | live title text | `screen_outer_frame` | 651.4,50.5,345.7,53.1 | 748,58,397,61 | 748,58,397,61 |
| `title_divider` | divider | `screen_outer_frame` | 566.9,117.6,512.0,9.6 | 651,135,588,11 | 651,135,588,11 |
| `mode_dropdown` | dropdown | `screen_outer_frame` | 145.4,137.7,210.7,55.8 | 167,158,242,64 | 167,158,242,64 |
| `mode_description_live_text` | live body text | `screen_outer_frame` | 376.2,155.1,492.9,24.4 | 432,178,566,28 | 432,178,566,28 |
| `page_count_live_text` | live page count | `screen_outer_frame` | 1429.9,156.0,94.9,23.5 | 1642,179,109,27 | 1642,179,109,27 |
| `content_frame` | inner list frame | `screen_outer_frame` | 141.9,208.2,1385.5,655.2 | 163,239,1591,752 | 163,239,1591,752 |
| `save_card_1` | empty save-card shell | `content_frame` | 155.9,221.3,656.6,318.0 | 179,254,754,365 | 179,254,754,365 |
| `save_card_2` | empty save-card shell | `content_frame` | 849.1,221.3,656.6,318.0 | 975,254,754,365 | 975,254,754,365 |
| `save_card_3` | empty save-card shell | `content_frame` | 155.9,550.7,656.6,318.0 | 179,632,754,365 | 179,632,754,365 |
| `save_card_4` | empty save-card shell | `content_frame` | 849.1,550.7,656.6,318.0 | 975,632,754,365 | 975,632,754,365 |
| `card_title_live_text` | live slot title | `each save_card` | 33.1,23.5,156.8,31.4 | 38,27,180,36 | relative inside each card |
| `card_title_divider_left` | thin divider | `each save_card` | 186.4,37.5,145.4,2.6 | 214,43,167,3 | relative inside each card |
| `card_title_ornament` | small ornament | `each save_card` | 341.4,28.8,47.0,20.9 | 392,33,54,24 | relative inside each card |
| `card_title_divider_right` | thin divider | `each save_card` | 389.3,37.5,126.3,2.6 | 447,43,145,3 | relative inside each card |
| `card_slot_grid` | slot-grid container | `each save_card` | 33.1,71.4,331.8,170.8 | 38,82,381,196 | relative inside each card |
| `card_slot_1_1` | single slot | `card_slot_grid` | 0.0,0.0,74.9,74.9 | 0,0,86,86 | relative inside grid |
| `card_slot_1_2` | single slot | `card_slot_grid` | 83.6,0.0,74.9,74.9 | 96,0,86,86 | relative inside grid |
| `card_slot_1_3` | single slot | `card_slot_grid` | 167.2,0.0,74.9,74.9 | 192,0,86,86 | relative inside grid |
| `card_slot_1_4` | single slot | `card_slot_grid` | 250.8,0.0,74.9,74.9 | 288,0,86,86 | relative inside grid |
| `card_slot_2_1` | single slot | `card_slot_grid` | 0.0,88.0,74.9,74.9 | 0,101,86,86 | relative inside grid |
| `card_slot_2_2` | single slot | `card_slot_grid` | 83.6,88.0,74.9,74.9 | 96,101,86,86 | relative inside grid |
| `card_slot_2_3` | single slot | `card_slot_grid` | 167.2,88.0,74.9,74.9 | 192,101,86,86 | relative inside grid |
| `card_slot_2_4` | single slot | `card_slot_grid` | 250.8,88.0,74.9,74.9 | 288,101,86,86 | relative inside grid |
| `card_vertical_divider` | right info divider | `each save_card` | 386.7,66.2,2.6,174.3 | 444,76,3,200 | relative inside each card |
| `card_detail_live_text_region` | live metadata/details | `each save_card` | 413.6,68.0,224.7,169.0 | 475,78,258,194 | relative inside each card |
| `card_detail_separator` | right info horizontal separator | `card_detail_live_text_region` | 413.6,124.6,224.7,12.2 | 475,143,258,14 | relative inside each card |
| `card_preview_button` | button inside card | `each save_card` | 34.0,251.8,293.5,53.1 | 39,289,337,61 | relative inside each card; stays inside card |
| `card_load_button` | button inside card | `each save_card` | 338.8,251.8,293.5,53.1 | 389,289,337,61 | relative inside each card; stays inside card |
| `pager_prev_button` | disabled pager button | `screen_outer_frame` | 152.4,871.3,155.9,52.3 | 175,1000,179,60 | 175,1000,179,60 |
| `pager_next_button` | disabled pager button | `screen_outer_frame` | 319.6,871.3,157.6,52.3 | 367,1000,181,60 | 367,1000,181,60 |

Hierarchy/containment/occupancy gate: PASS for coordinator build/capture handoff. The source uses a 1920x1080 design canvas and now also positions the save-card internal slot grid, title ornaments/dividers, right info dividers, and Preview/Load buttons against normalized rect targets.

## Build / Capture

Build attempts: 0. Coordinator worker mode forbids build.  
Capture attempts: 0. Coordinator worker mode forbids capture.  
Current implementation screenshot path: none; coordinator mode Phase B does not capture.  
Working visual screenshot proof: none; coordinator mode Phase B does not capture.

Central capture command:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Exe C:\UE\T66\Binaries\Win64\T66.exe -Screen SaveSlots -ResX 1920 -ResY 1080 -Output C:\UE\T66\UI\Reference\Screens\SaveSlots\Proof\SaveSlots_central_1920x1080.png
```

## Remaining Visual Differences

Not evaluated by this worker. Runtime source/layout was changed, but no build or capture was run because coordinator mode is active.

Approved live-data/top-bar-shared differences:
- none recorded.

## Next Action

Coordinator runs the central build/capture pass using the command above and compares `SaveSlots_central_1920x1080.png` against the normalized 1920x1080 reference occupancy table in this manifest.
