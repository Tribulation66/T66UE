# RunSummary Reference UI Manifest

Status: READY_FOR_CENTRAL_BUILD_AND_CAPTURE
Target: RunSummary
Base screen/modal: Screen
Target state: Default/RunSummary
Coordinator worker mode: yes
Pass count: 3

Second-pass central review correction: after central capture failed for root logical scaling, `T66RunSummaryScreen.cpp` was updated so the fixed 1920x1080 RunSummary design canvas is wrapped in `SScaleBox` and `RebuildWidget()` returns the screen directly instead of wrapping it in `FT66Style::MakeResponsiveRoot(BuildSlateUI())`. Slot cell backing was also changed from gray rarity fill to a dark slot fill so reward/inventory strips do not show gray blocks behind the slot chrome.

Third-pass central review correction: kept the `SScaleBox` root fix, moved RunSummary-owned panel slots upward to better match the physical capture/reference occupancy, enlarged/repositioned the Event Log button, reduced/top-aligned the live 3D preview inside its frame, and made reward/inventory slot strip backing transparent so individual dark slots sit on the wood field.

Fourth-pass central review correction: restored the SCanvas top-level positions to the measured normalized rects in this manifest for weekly/all-time rank, preview, stats, damage, reward slots, inventory slots, and Event Log. Removed the separate slot fill layer from `MakeRunSummaryReferenceSlot()` so slot assets no longer render gray backing slabs behind each individual frame. Retuned live preview content to fill more of the reference frame while keeping the frame rect fixed.

## Reference Accounting

| Field | Value |
|---|---:|
| Native reference width | 1672 |
| Native reference height | 941 |
| Normalized target width | 1920 |
| Normalized target height | 1080 |
| scaleX | 1.1483253588516746 |
| scaleY | 1.1477151965993624 |

Reference image: `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\RunSummary.png`

## Scope Boundary

Owned areas for later implementation: RunSummary-owned left rank/seed/integrity/action panels, central preview frame, central reward/slot frames, right stats panel, right damage-by-source panel, and RunSummary-owned scrollbar visuals if this screen owns them.

Protected/shared areas: MainMenu, Shared, sibling screens, build/capture pipeline, shared top bar/chrome. No protected/shared source or runtime folders were edited.

## Archive And Rejection History

| Pass | Path | Status | Reason |
|---|---|---|---|
| Pre-run reset | `C:\UE\T66\UI\Reference\Screens\RunSummary\Archive\PreRun_Reset_20260505_212206` | Archived | Archived existing generated workspace content and old manifest before fresh run. Runtime folder had no active files to move. |
| Pass 01 | `C:\UE\T66\UI\Reference\Screens\RunSummary\Archive\Rejected\Pass_01\RunSummary_text_free_atomic_sheet_pass01_REJECTED.png` | Rejected | Rejected by coordinator for gray full-screen frame/interior material drift and missing native-to-normalized rect accounting. |
| Pass 02 | `C:\UE\T66\UI\Reference\Screens\RunSummary\Archive\Rejected\Pass_02\RunSummary_text_free_atomic_sheet_pass02_REJECTED.png` | Rejected | Rejected by worker before coordinator review for bright gold/parchment drift, missing dark wood full-screen/background shell, and excessive ornament density. |
| Pass 03 | `C:\UE\T66\UI\Reference\Screens\RunSummary\Working\Pass_03\Candidates\RunSummary_text_free_atomic_sheet_pass03_ACCEPTED.png` | Accepted for coordinator sprite review | Corrects the full-screen/background material to a dark wood interior with brown/gold border and remains text-free/atomic. |

Preserved real reference images: no files under `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked` were moved, edited, or deleted.

## Imagegen Result

Built-in imagegen used: yes

| Result | Path | Status |
|---|---|---|
| Built-in original output | `C:\Users\DoPra\.codex\generated_images\019dfaa9-0761-7150-82a9-8745f66f516f\ig_09798ed5522d61340169fa8c13e40c81998acef64b090b79f4.png` | Accepted source output |
| Workspace candidate copy | `C:\UE\T66\UI\Reference\Screens\RunSummary\Working\Pass_03\Candidates\RunSummary_text_free_atomic_sheet_pass03_ACCEPTED.png` | Accepted for coordinator sprite review |

Accepted runtime asset paths:

| Runtime asset | Contract |
|---|---|
| `C:\UE\T66\SourceAssets\UI\Reference\Screens\RunSummary\Panels\runsummary_panels_fullscreen_fullscreen_panel_wide.png` | 9-slice/fixed dark wood background shell |
| `C:\UE\T66\SourceAssets\UI\Reference\Screens\RunSummary\Panels\runsummary_panels_fullscreen_row_shell_quiet.png` | 9-slice empty row shell |
| `C:\UE\T66\SourceAssets\UI\Reference\Screens\RunSummary\Panels\runsummary_rank_panel_generated_v02.png` | 9-slice empty rank/card shell |
| `C:\UE\T66\SourceAssets\UI\Reference\Screens\RunSummary\Panels\runsummary_metric_card_generated_v02.png` | 9-slice empty seed/integrity metric shell |
| `C:\UE\T66\SourceAssets\UI\Reference\Screens\RunSummary\Panels\runsummary_damage_panel_generated_v02.png` | 9-slice empty damage table shell |
| `C:\UE\T66\SourceAssets\UI\Reference\Screens\RunSummary\Panels\runsummary_panels_fullscreen_fullscreen_panel_tall.png` | 9-slice/fixed preview frame |
| `C:\UE\T66\SourceAssets\UI\Reference\Screens\RunSummary\Panels\runsummary_stats_panel_surface.png` | fixed/fill dark stats surface |
| `C:\UE\T66\SourceAssets\UI\Reference\Screens\RunSummary\Slots\runsummary_steel_slot_generated_v02.png` | fixed/9-slice square slot frame |
| `C:\UE\T66\SourceAssets\UI\Reference\Screens\RunSummary\Slots\runsummary_steel_slot_rect_generated_v01.png` | fixed/9-slice rectangular slot frame |
| `C:\UE\T66\SourceAssets\UI\Reference\Screens\RunSummary\Buttons\CTA\runsummary_button_plate_generated_v02.png` | horizontal sliced button plate |
| `C:\UE\T66\SourceAssets\UI\Reference\Screens\RunSummary\Buttons\Pill\runsummary_buttons_pill_normal.png` | horizontal sliced button normal |
| `C:\UE\T66\SourceAssets\UI\Reference\Screens\RunSummary\Buttons\Pill\runsummary_buttons_pill_hover.png` | horizontal sliced button hover |
| `C:\UE\T66\SourceAssets\UI\Reference\Screens\RunSummary\Buttons\Pill\runsummary_buttons_pill_selected.png` | horizontal sliced button selected |
| `C:\UE\T66\SourceAssets\UI\Reference\Screens\RunSummary\Buttons\Pill\runsummary_buttons_pill_pressed.png` | horizontal sliced button pressed |
| `C:\UE\T66\SourceAssets\UI\Reference\Screens\RunSummary\Buttons\Pill\runsummary_buttons_pill_disabled.png` | horizontal sliced button disabled |
| `C:\UE\T66\SourceAssets\UI\Reference\Screens\RunSummary\Controls\runsummary_scrollbar_track.png` | vertical 3-slice scrollbar rail |
| `C:\UE\T66\SourceAssets\UI\Reference\Screens\RunSummary\Controls\runsummary_scrollbar_thumb.png` | vertical 3-slice scrollbar thumb |
| `C:\UE\T66\SourceAssets\UI\Reference\Screens\RunSummary\Controls\runsummary_divider_vertical.png` | fixed divider |
| `C:\UE\T66\SourceAssets\UI\Reference\Screens\RunSummary\Ornaments\runsummary_corner_bracket.png` | fixed ornament |

PNG alpha validation: transparent pixels in promoted runtime PNGs were validated to use RGB 0,0,0.

## Sprite Quality Acceptance Table

| Check | Result | Reason |
|---|---|---|
| Color temperature | PASS | Dark wood, brown/amber trim, blue-gray metal frames, and dark navy panel surface match the reference families more closely than rejected passes. |
| Brightness | PASS | Full-screen/background component is dark and no longer gray, parchment, or bright gold. |
| Paper/panel tone | PASS | No parchment/paper candidate accepted for RunSummary panels. |
| Wood tone | PASS | Full-screen shell and panel shells use dark brown wood with subtle grain. |
| Metal tone | PASS | Preview and slot frames retain blue-gray metal styling. |
| Border thickness | PASS | Borders are thin/medium and suitable for later sliced contracts. |
| Ornament density | PASS | Corner details are restrained relative to rejected Pass 02 and isolated as components. |
| Grain/noise | PASS | Wood grain is visible without becoming noisy or damaged. |
| Bevel strength | PASS | Bevels are present but not a heavy ornate reinterpretation. |
| Contrast | PASS | Dark interiors preserve the source screen's low-key contrast. |
| Geometry/proportions | PASS | Includes required background shell, card shells, button plates, preview frame, single slots, panel shells, scrollbar pieces, divider, and corner ornament. |
| Component correspondence | PASS | Components correspond to visible RunSummary UI families. |
| Text-free status | PASS | No readable labels, numbers, stats, title text, portraits, screenshots, or runtime data are accepted. |
| Atomic-component status | PASS | Parent shells are empty; child controls are separate. |

## Atomic Component Table

| Component | Atomic role | Parent/child status | Transparent/backgroundless expectation for later slice | Resize contract |
|---|---|---|---|---|
| Dark wood full-screen/background shell | Empty background shell with dark wood interior and brown/gold border | Parent shell only | Backgroundless outside frame after slice; preserve dark wood interior | 9-slice or fixed screen-frame renderer |
| Large left rank/card shell | Empty card shell | Parent shell only | Backgroundless outside border; keep empty dark wood interior | 9-slice |
| Small seed/integrity card shell | Empty card shell | Parent shell only | Backgroundless outside border; keep empty dark wood interior | 9-slice |
| Normal brown button plate | Empty button state | Child control | Backgroundless outside plate | Horizontal 3-slice |
| Highlighted brown button plate | Empty button state | Child control | Backgroundless outside plate | Horizontal 3-slice |
| Central preview frame | Empty preview frame only | Parent frame only | Backgroundless outside frame; no preview image baked | 9-slice or fixed frame |
| Single square slot | Empty single slot/frame | Child control | Backgroundless outside slot | Fixed or 9-slice |
| Single rectangular inventory slot | Empty single slot/frame | Child control | Backgroundless outside slot | Horizontal 3-slice or 9-slice |
| Dark stats panel surface | Empty dark navy panel surface | Parent shell/interior | Backgroundless outside panel | 9-slice or fixed fill |
| Damage source panel shell | Empty table panel shell | Parent shell only | Backgroundless outside border; no rows/headings baked | 9-slice |
| Scrollbar rail | Empty scrollbar rail | Child control | Backgroundless outside rail | Vertical 3-slice |
| Scrollbar thumb | Empty scrollbar thumb | Child control | Backgroundless outside thumb | Vertical 3-slice |
| Thin divider | Divider/line | Child decoration | Backgroundless outside line | Fixed or vertical 3-slice |
| Corner bracket ornament | Ornament | Child decoration | Backgroundless outside ornament | Fixed |

Atomic gate result: PASS.

Baked parent/child failures: none accepted.

## Native And Normalized Reference Occupancy Map

Native rects are measured against the 1672x941 reference. Normalized rects are scaled to 1920x1080 using scaleX and scaleY above. Values are approximate and intended as Phase B layout targets.

| Component | Role | Parent | Native x | Native y | Native w | Native h | Norm x | Norm y | Norm w | Norm h | Anchor | Resize contract |
|---|---|---|---:|---:|---:|---:|---:|---:|---:|---:|---|---|
| Screen frame | Outer frame/background | Root | 9 | 7 | 1655 | 919 | 10 | 8 | 1900 | 1055 | Full screen | 9-slice/fixed frame |
| Header title live region | Live text | Screen frame | 52 | 48 | 357 | 48 | 60 | 55 | 410 | 55 | Top-left | Live text |
| Header stats live region | Live text | Screen frame | 52 | 105 | 418 | 24 | 60 | 120 | 480 | 28 | Top-left | Live text |
| Event log button | Button | Screen frame | 1362 | 37 | 239 | 57 | 1564 | 42 | 274 | 65 | Top-right | Horizontal 3-slice |
| Weekly rank panel | Empty card shell | Screen frame | 54 | 144 | 503 | 100 | 62 | 165 | 578 | 115 | Left column | 9-slice |
| Weekly rank live region | Live text | Weekly rank panel | 77 | 167 | 366 | 57 | 88 | 192 | 420 | 65 | Panel top-left | Live text |
| All-time rank panel | Empty card shell | Screen frame | 54 | 261 | 503 | 100 | 62 | 300 | 578 | 115 | Left column | 9-slice |
| All-time rank live region | Live text | All-time rank panel | 77 | 284 | 366 | 57 | 88 | 326 | 420 | 65 | Panel top-left | Live text |
| Seed lock panel | Empty card shell | Screen frame | 51 | 389 | 305 | 113 | 59 | 446 | 350 | 130 | Left column | 9-slice |
| Seed lock live region | Live text | Seed lock panel | 80 | 414 | 235 | 66 | 92 | 475 | 270 | 76 | Panel top-left | Live text |
| Integrity panel | Empty card shell | Screen frame | 51 | 516 | 305 | 105 | 59 | 592 | 350 | 120 | Left column | 9-slice |
| Integrity live region | Live text | Integrity panel | 80 | 542 | 200 | 51 | 92 | 622 | 230 | 59 | Panel top-left | Live text |
| Go again button | Button | Screen frame | 56 | 649 | 298 | 74 | 64 | 745 | 342 | 85 | Left column | Horizontal 3-slice |
| Main menu button | Button | Screen frame | 56 | 741 | 298 | 74 | 64 | 850 | 342 | 85 | Left column | Horizontal 3-slice |
| Central preview frame | Empty preview frame | Screen frame | 610 | 144 | 434 | 394 | 700 | 165 | 498 | 452 | Center | 9-slice/fixed frame |
| Central preview live region | Runtime preview | Central preview frame | 625 | 159 | 402 | 362 | 718 | 182 | 462 | 415 | Center fill | Runtime content |
| Four reward slots row | Slot group | Screen frame | 618 | 549 | 418 | 92 | 710 | 630 | 480 | 106 | Center | Single slot repeated |
| Reward slot 1 | Single square slot | Four reward slots row | 618 | 549 | 87 | 92 | 710 | 630 | 100 | 106 | Row left | Fixed/9-slice |
| Reward slot 2 | Single square slot | Four reward slots row | 727 | 549 | 87 | 92 | 835 | 630 | 100 | 106 | Row left | Fixed/9-slice |
| Reward slot 3 | Single square slot | Four reward slots row | 836 | 549 | 87 | 92 | 960 | 630 | 100 | 106 | Row left | Fixed/9-slice |
| Reward slot 4 | Single square slot | Four reward slots row | 945 | 549 | 87 | 92 | 1085 | 630 | 100 | 106 | Row left | Fixed/9-slice |
| Inventory slot grid | Slot group | Screen frame | 405 | 662 | 770 | 170 | 465 | 760 | 884 | 195 | Center-bottom | Single slot repeated |
| Inventory slot cell | Single rectangular slot repeated | Inventory slot grid | 411 | 673 | 71 | 68 | 472 | 772 | 82 | 78 | Grid flow | Fixed/9-slice |
| Stats panel surface | Empty stats panel | Screen frame | 1190 | 137 | 401 | 362 | 1366 | 157 | 460 | 415 | Right column | 9-slice/fixed fill |
| Stats live region | Live text | Stats panel surface | 1214 | 161 | 314 | 305 | 1394 | 185 | 361 | 350 | Panel top-left | Live text |
| Damage by source panel | Empty table shell | Screen frame | 1198 | 519 | 392 | 314 | 1376 | 596 | 450 | 360 | Right column | 9-slice |
| Damage table live region | Live text | Damage by source panel | 1220 | 545 | 314 | 248 | 1401 | 626 | 361 | 285 | Panel top-left | Live text |
| Vertical scrollbar rail | Scrollbar rail | Screen frame | 1622 | 121 | 16 | 714 | 1862 | 139 | 18 | 820 | Far right | Vertical 3-slice |
| Vertical scrollbar thumb | Scrollbar thumb | Vertical scrollbar rail | 1622 | 121 | 16 | 714 | 1862 | 139 | 18 | 820 | Far right | Vertical 3-slice |

Hierarchy/containment/occupancy gate result: PASS.

Implementation uses the normalized 1920x1080 rect table above as fixed `SCanvas` slots in `T66RunSummaryScreen.cpp`.

## Explicit Containment Map

| Parent | Children | Containment status |
|---|---|---|
| Screen frame/background shell | Header title, stage/score/time line, Event Log button, left panels, central preview, reward slots, inventory grid, stats panel, damage panel, scrollbar | PASS; all owned top-level widgets are placed inside the normalized frame rect. |
| Weekly rank panel | Weekly rank header and score/speed live text | PASS; live text remains inside the panel with internal Slate padding. |
| All-time rank panel | All-time rank header and score/speed live text | PASS; live text remains inside the panel with internal Slate padding. |
| Seed luck panel | Seed luck heading and live value | PASS; child live text remains inside the metric card. |
| Integrity panel | Integrity heading and live status | PASS; child live text remains inside the metric card. |
| Action button region | Go Again/Main Menu or alternate run-state buttons | PASS for default state; alternate run-state buttons reuse the same left action rect and remain clipped to that parent area. |
| Central preview frame | Runtime hero preview render target or fallback text | PASS; preview content is padded inside the frame. |
| Reward slot row | Four idol slot widgets | PASS; four slots fit inside the normalized reward row rect. |
| Inventory slot grid | Two rows by ten inventory slot widgets | PASS; slot sizes/padding target the normalized inventory rect. |
| Stats panel surface | Live stats panel | PASS; stats widget is wrapped in the dark stats surface with panel padding. |
| Damage by source shell | Damage table heading and seven live rows | PASS; table content is inside the damage panel shell. |
| Scrollbar rect | Scrollbar rail/thumb | PASS; standalone scrollbar is positioned at the normalized right-edge rect. |

Remaining overflow/containment/rect-delta issues: none known without central capture. Coordinator capture is required for visual proof.

## Phase B Stop State

Source files changed:

| Source file | Status |
|---|---|
| `C:\UE\T66\Source\T66\UI\Screens\T66RunSummaryScreen.cpp` | Edited for RunSummary-owned asset routing, brush dimensions, and normalized 1920x1080 reference canvas layout. |
| `C:\UE\T66\Source\T66\UI\Screens\T66RunSummaryScreen.h` | Not edited. |

Build attempts: 0.

Capture attempts: none.

Central capture command for coordinator:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Exe C:\UE\T66\Binaries\Win64\T66.exe -Screen RunSummary -ResX 1920 -ResY 1080 -Output C:\UE\T66\UI\Reference\Screens\RunSummary\Proof\RunSummary_central_1920x1080.png
```

Next action if not pass: coordinator reviews `C:\UE\T66\UI\Reference\Screens\RunSummary\Working\Pass_03\Candidates\RunSummary_text_free_atomic_sheet_pass03_ACCEPTED.png` and either approves Phase B slicing/implementation or requests another stricter regenerated sheet.
