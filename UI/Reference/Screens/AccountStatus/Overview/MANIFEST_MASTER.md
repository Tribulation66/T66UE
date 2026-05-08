# AccountStatus Overview Manifest

Status: READY_FOR_CENTRAL_BUILD_AND_CAPTURE

Coordinator worker mode: yes

Target: AccountStatus Overview

Base screen/modal: Screen

Target state: Overview

Reference image: `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\AccountStatus.png`

Workspace folder: `C:\UE\T66\UI\Reference\Screens\AccountStatus\Overview`

Runtime asset folder: `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Overview`

Protected scope: AccountStatus History, AccountStatus Common, MainMenu, Shared, sibling screens, build/capture, and `T66AccountStatusScreen.cpp/.h`.

## Phase A Boundary

Owned in Phase A:

- Overview workspace generated sprite sheets and manifest.
- Overview runtime asset folder reset/archive only.
- Reference-derived atomic component sheet candidate.

Not touched in Phase A:

- `C:\UE\T66\Source\T66\UI\Screens\T66AccountStatusScreen.cpp`
- `C:\UE\T66\Source\T66\UI\Screens\T66AccountStatusScreen.h`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Common`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History`
- MainMenu, Shared, sibling screen assets.
- Build or capture output.

## Archive And Reset

Pre-run reset archive:

- `C:\UE\T66\UI\Reference\Screens\AccountStatus\Overview\Archive\PreRun_Reset_20260505_212206`

Archived/reset paths:

- `C:\UE\T66\UI\Reference\Screens\AccountStatus\Overview\Archive\PreRun_Reset_20260505_212206\Working`
- `C:\UE\T66\UI\Reference\Screens\AccountStatus\Overview\Archive\PreRun_Reset_20260505_212206\MANIFEST_V4.md`
- `C:\UE\T66\UI\Reference\Screens\AccountStatus\Overview\Archive\PreRun_Reset_20260505_212206\ARCHIVED_PATHS.txt`

Overview runtime folder:

- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Overview`
- No previous runtime files existed at reset time; folder was recreated empty for the target owner.

Real reference images preserved:

- `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\AccountStatus.png`

## Generated Candidate Paths

Accepted:

- `C:\UE\T66\UI\Reference\Screens\AccountStatus\Overview\Working\Pass_03\Candidates\accountstatus_overview_atomic_sheet_pass03_ACCEPTED.png`

Rejected:

- `C:\UE\T66\UI\Reference\Screens\AccountStatus\Overview\Archive\Rejected\Pass_01\accountstatus_overview_atomic_sheet_pass01.png`
- `C:\UE\T66\UI\Reference\Screens\AccountStatus\Overview\Archive\Rejected\Pass_02\accountstatus_overview_atomic_sheet_pass02.png`

Rejected notes:

- Pass 01: color/material family was usable, but atomic gate failed because arrow symbols were baked into scrollbar arrow buttons and a composite table strip included column dividers attached to a header-like dark bar. A large panel also included a non-reference cutout.
- Pass 02: improved isolated panels/rows/buttons, but atomic gate failed because the top and bottom scrollbar button shells still included baked arrow glyphs.

Accepted runtime asset paths:

- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Overview\Panels\accountstatus_overview_content_shell.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Overview\Panels\accountstatus_overview_paper_panel.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Overview\Panels\accountstatus_overview_small_card.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Overview\Panels\accountstatus_overview_row_shell.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Overview\Panels\accountstatus_overview_divider_horizontal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Overview\Panels\accountstatus_overview_divider_vertical.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Overview\Controls\accountstatus_overview_dropdown_field_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Overview\Controls\accountstatus_overview_scrollbar_vertical.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Overview\Controls\accountstatus_overview_scrollbar_thumb.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Overview\Controls\accountstatus_overview_scrollbar_cap.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Overview\Buttons\accountstatus_overview_pill_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Overview\Buttons\accountstatus_overview_pill_hover.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Overview\Buttons\accountstatus_overview_pill_pressed.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Overview\Buttons\accountstatus_overview_pill_selected.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Overview\Buttons\accountstatus_overview_tab_unselected.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Overview\Buttons\accountstatus_overview_tab_selected.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Overview\Buttons\accountstatus_overview_square_button_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Overview\Slots\accountstatus_overview_square_slot_frame_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Overview\Progress\accountstatus_overview_progress_track.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Overview\Progress\accountstatus_overview_progress_fill.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Overview\Ornaments\accountstatus_overview_corner_tl.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Overview\Ornaments\accountstatus_overview_corner_tr.png`

Runtime slicing note: all promoted PNGs were sliced from the approved Pass 03 sheet with connected sheet-background pixels converted to alpha 0 and RGB 0,0,0. No History or Common runtime assets were touched.

## Atomic Component Table

| Component | Atomic role | Parent/child status | Transparent/backgroundless expectation | Resize contract |
| --- | --- | --- | --- | --- |
| Large parchment panel shell | Empty parent panel | Parent only; no baked children | Backgroundless component expected after slicing | 9-slice |
| Medium parchment panel shell | Empty parent panel/card | Parent only; no baked children | Backgroundless component expected after slicing | 9-slice |
| Small parchment card shell | Empty parent card | Parent only; no baked children | Backgroundless component expected after slicing | 9-slice |
| Empty parchment row strip | Single row background | Child component only; no text/grid | Backgroundless component expected after slicing | Horizontal 3-slice |
| Horizontal divider | Decorative separator | Child component only | Backgroundless component expected after slicing | Fixed or horizontal 3-slice |
| Vertical divider | Decorative separator | Child component only | Backgroundless component expected after slicing | Fixed or vertical 3-slice |
| Dark wood dropdown shell | Empty selector shell | Child component only; no label/icon | Backgroundless component expected after slicing | Horizontal 3-slice |
| Selected tab plate | Empty selected tab state | Child component only; no label | Backgroundless component expected after slicing | Horizontal 3-slice |
| Unselected tab plate | Empty unselected tab state | Child component only; no label | Backgroundless component expected after slicing | Horizontal 3-slice |
| Long dark wood button normal | Empty button state | Child component only; no label/icon | Backgroundless component expected after slicing | Horizontal 3-slice |
| Long dark wood button highlighted | Empty button state | Child component only; no label/icon | Backgroundless component expected after slicing | Horizontal 3-slice |
| Small square dark wood button shell | Empty square button state | Child component only; no symbol | Backgroundless component expected after slicing | Fixed |
| Avatar frame | Empty frame with blank dark interior | Parent frame for live/avatar content; no portrait/emblem | Backgroundless component expected after slicing | Fixed or 9-slice |
| Progress meter track | Empty meter track | Child component only | Backgroundless component expected after slicing | Horizontal 3-slice |
| Blue progress fill strip | Meter fill | Child component only | Backgroundless component expected after slicing | Horizontal 3-slice |
| Vertical scrollbar rail | Scrollbar rail | Parent rail only | Backgroundless component expected after slicing | Vertical 3-slice |
| Vertical scrollbar thumb | Scrollbar thumb | Child component only | Backgroundless component expected after slicing | Vertical 3-slice |
| Empty scrollbar cap/button shell | Empty square cap/button state | Child component only; no arrow | Backgroundless component expected after slicing | Fixed |
| Corner ornament pieces | Decorative trim | Child ornaments only | Backgroundless component expected after slicing | Fixed |

Atomic gate result: PASS.

No accepted parent component includes baked controls, text, portraits, icons, tables, rows, tabs, buttons, or mini-layouts. No accepted button/dropdown/tab includes live label text or baked glyphs.

## Sprite Sheet Quality Acceptance Table

Accepted sheet: `C:\UE\T66\UI\Reference\Screens\AccountStatus\Overview\Working\Pass_03\Candidates\accountstatus_overview_atomic_sheet_pass03_ACCEPTED.png`

| Check | Result | Reason |
| --- | --- | --- |
| Color temperature | PASS | Warm gold/parchment and dark wood match the reference family. |
| Brightness | PASS | Components remain in the same readable high-contrast range as the reference. |
| Paper tone | PASS | Parchment panels use the same tan/yellow paper family. |
| Wood tone | PASS | Buttons/tabs/dropdowns use restrained dark brown wood tones. |
| Metal/gold tone | PASS | Gold trim is aged yellow-gold, close to reference trim. |
| Border thickness | PASS | Panel, button, tab, and scrollbar borders are thin enough for the reference style. |
| Ornament density | PASS | Corner ornaments are present but isolated; parent panels are not packed with extra controls. |
| Grain/noise | PASS | Texture is lightly worn and not materially grainier than the reference. |
| Bevel strength | PASS | Bevels are visible but not overmodeled. |
| Contrast | PASS | Dark controls and parchment panels preserve the reference contrast. |
| Geometry/proportions | PASS | Sheet includes the required panel, row, tab, dropdown, button, meter, frame, scrollbar, divider, and ornament families. |
| Component correspondence | PASS | Components correspond to AccountStatus Overview UI families. |
| Text-free status | PASS | No labels, numbers, player names, XP values, score data, title text, or table text are present. |
| Atomic-component status | PASS | Components are isolated; accepted parents do not bake child controls or mini-layouts. |

Sprite sheet quality gate: PASS.

## 1920x1080 Reference Hierarchy And Occupancy Map

Shared/protected top chrome is mapped for context only and remains out of scope for Phase A edits.

Native reference size: 1672x941.

ScaleX: 1920/1672 = 1.148325.

ScaleY: 1080/941 = 1.147715.

Implementation target: normalized 1920x1080 rects.

## Corrected Native And Normalized Occupancy Table

| Name | Role | Parent | Native rect x,y,w,h | Normalized 1920x1080 rect x,y,w,h | Anchor | Padding/spacing | Resize contract | Intended implementation rect | Scope |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Screen root | 1920x1080 viewport | None | 0,0,1672,941 | 0,0,1920,1080 | Full screen | N/A | N/A | 0,0,1920,1080 | Context |
| Shared top navigation bar | Shared header/nav/currency/power chrome | Screen root | 0,0,1672,112 | 0,0,1920,128 | Top stretch | Buttons spaced horizontally | Mixed fixed/3-slice | Protected existing shared chrome | Protected |
| Account title region | Screen title and ornament line | Screen root | 557,110,557,71 | 640,126,640,82 | Top center | Title centered above tabs | Live text + fixed ornaments | Coordinator/source phase only | Owned layout |
| Overview tab | Selected tab | Account title region | 563,188,260,49 | 647,216,298,56 | Top center | 24 px gap to History tab | Horizontal 3-slice | 647,216,298,56 | Owned layout |
| History tab | Unselected sibling tab | Account title region | 846,188,260,49 | 972,216,298,56 | Top center | 24 px gap from Overview tab | Horizontal 3-slice | Protected sibling state visual only | Protected/sibling |
| Main content frame | Decorative content boundary and scrollbar area | Screen root | 15,118,1611,810 | 17,136,1850,930 | Center stretch | 32 px inner margins | 9-slice/fixed ornaments | 17,136,1850,930 | Owned layout |
| Left column panel | Parchment panel group | Main content frame | 46,240,651,653 | 53,276,748,750 | Left stretch | 24 px internal section gap | 9-slice | 53,276,748,750 | Owned layout |
| Player summary card | Parchment card | Left column panel | 73,266,615,131 | 84,305,706,150 | Top left | 24 px avatar/text padding | 9-slice | 84,305,706,150 | Owned layout |
| Avatar frame | Empty portrait frame | Player summary card | 94,286,135,124 | 108,328,155,142 | Left | 20 px from card left | Fixed | 108,328,155,142 | Owned layout |
| Player text/live XP region | Live text and meter region | Player summary card | 251,275,411,105 | 288,316,472,120 | Fill right | Text stack 12-18 px vertical | Live text + meter track | 288,316,472,120 | Live/runtime |
| XP meter track | Progress meter track | Player text/live XP region | 252,368,330,21 | 289,422,379,24 | Bottom left | XP text to right | Horizontal 3-slice | 289,422,379,24 | Owned layout |
| Account status card | Parchment card | Left column panel | 73,417,615,173 | 84,479,706,198 | Middle left | 23 px padding | 9-slice | 84,479,706,198 | Owned layout |
| Status help button | Small square button | Account status card | 606,443,51,51 | 696,509,58,58 | Right | 24 px from card right/top | Fixed | 696,509,58,58 | Owned layout |
| Account status live text region | Live text/data | Account status card | 95,435,529,133 | 109,499,608,153 | Fill | 10-14 px text spacing | Live text | 109,499,608,153 | Live/runtime |
| Account progress card | Parchment card | Left column panel | 73,601,615,261 | 84,690,706,300 | Bottom left | Rows 45 px apart | 9-slice | 84,690,706,300 | Owned layout |
| Progress rows group | Five repeated progress rows | Account progress card | 95,655,557,179 | 109,752,640,205 | Fill | 45 px row pitch | Row + meter track/fill | 109,752,640,205 | Owned layout |
| Right column panel | Parchment panel group | Main content frame | 711,240,866,653 | 816,276,994,750 | Right stretch | 30 px inner margins | 9-slice | 816,276,994,750 | Owned layout |
| Filter dropdown left | Dropdown selector | Right column panel | 756,266,371,48 | 868,305,426,55 | Top left | 58 px gap to right dropdown | Horizontal 3-slice | 868,305,426,55 | Owned layout |
| Filter dropdown right | Dropdown selector | Right column panel | 1176,266,355,48 | 1350,305,408,55 | Top right | Top aligned to left dropdown | Horizontal 3-slice | 1350,305,408,55 | Owned layout |
| Highest score table panel | Table/card area | Right column panel | 738,328,809,263 | 847,376,929,302 | Top | 27 px below filters | 9-slice panel + rows/dividers | 847,376,929,302 | Owned layout |
| Highest score title/live header | Live table title/header labels | Highest score table panel | 758,344,740,70 | 870,395,850,80 | Top | Column spacing from reference | Live text/dividers | 870,395,850,80 | Live/runtime |
| Highest score rows | Five table rows | Highest score table panel | 758,429,740,139 | 870,492,850,160 | Fill | 36-38 px row pitch | Row strip + dividers | 870,492,850,160 | Owned layout |
| Best speed run table panel | Table/card area | Right column panel | 738,600,809,263 | 847,689,929,302 | Bottom | 16 px gap from upper table | 9-slice panel + rows/dividers | 847,689,929,302 | Owned layout |
| Best speed run title/live header | Live table title/header labels | Best speed run table panel | 758,613,740,71 | 870,704,850,82 | Top | Column spacing from reference | Live text/dividers | 870,704,850,82 | Live/runtime |
| Best speed run rows | Five table rows | Best speed run table panel | 758,700,740,139 | 870,803,850,160 | Fill | 36-38 px row pitch | Row strip + dividers | 870,803,850,160 | Owned layout |
| Content scrollbar | Vertical scrollbar | Main content frame | 1593,241,31,651 | 1829,277,36,747 | Right | Arrow caps top/bottom | Rail/thumb/cap components | 1829,277,36,747 | Owned layout |

| Name | Role | Parent | Reference rect x,y,w,h | Anchor | Padding/spacing | Resize contract | Intended implementation rect | Scope |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Screen root | 1920x1080 viewport | None | 0,0,1920,1080 | Full screen | N/A | N/A | 0,0,1920,1080 | Context |
| Shared top navigation bar | Shared header/nav/currency/power chrome | Screen root | 0,0,1920,128 | Top stretch | Buttons spaced horizontally | Mixed fixed/3-slice | Protected existing shared chrome | Protected |
| Account title region | Screen title and ornament line | Screen root | 640,126,640,82 | Top center | Title centered above tabs | Live text + fixed ornaments | Coordinator/source phase only | Owned layout later |
| Overview tab | Selected tab | Account title region | 647,216,298,56 | Top center | 24 px gap to History tab | Horizontal 3-slice | 647,216,298,56 | Owned layout later |
| History tab | Unselected sibling tab | Account title region | 972,216,298,56 | Top center | 24 px gap from Overview tab | Horizontal 3-slice | Protected sibling state visual only | Protected/sibling |
| Main content frame | Decorative content boundary and scrollbar area | Screen root | 17,136,1850,930 | Center stretch | 32 px inner margins | 9-slice/fixed ornaments | 17,136,1850,930 | Owned layout later |
| Left column panel | Parchment panel group | Main content frame | 53,276,748,750 | Left stretch | 24 px internal section gap | 9-slice | 53,276,748,750 | Owned layout later |
| Player summary card | Parchment card | Left column panel | 84,305,706,150 | Top left | 24 px avatar/text padding | 9-slice | 84,305,706,150 | Owned layout later |
| Avatar frame | Empty portrait frame | Player summary card | 108,328,155,142 | Left | 20 px from card left | Fixed | 108,328,155,142 | Owned layout later |
| Player text/live XP region | Live text and meter region | Player summary card | 288,316,472,120 | Fill right | Text stack 12-18 px vertical | Live text + meter track | 288,316,472,120 | Live/runtime |
| XP meter track | Progress meter track | Player text/live XP region | 289,422,379,24 | Bottom left | XP text to right | Horizontal 3-slice | 289,422,379,24 | Owned layout later |
| Account status card | Parchment card | Left column panel | 84,479,706,198 | Middle left | 23 px padding | 9-slice | 84,479,706,198 | Owned layout later |
| Status help button | Small square button | Account status card | 696,509,58,58 | Right | 24 px from card right/top | Fixed | 696,509,58,58 | Owned layout later |
| Account status live text region | Live text/data | Account status card | 109,499,608,153 | Fill | 10-14 px text spacing | Live text | 109,499,608,153 | Live/runtime |
| Account progress card | Parchment card | Left column panel | 84,690,706,300 | Bottom left | Rows 45 px apart | 9-slice | 84,690,706,300 | Owned layout later |
| Progress rows group | Five repeated progress rows | Account progress card | 109,752,640,205 | Fill | 45 px row pitch | Row + meter track/fill | 109,752,640,205 | Owned layout later |
| Right column panel | Parchment panel group | Main content frame | 816,276,994,750 | Right stretch | 30 px inner margins | 9-slice | 816,276,994,750 | Owned layout later |
| Filter dropdown left | Dropdown selector | Right column panel | 868,305,426,55 | Top left | 58 px gap to right dropdown | Horizontal 3-slice | 868,305,426,55 | Owned layout later |
| Filter dropdown right | Dropdown selector | Right column panel | 1350,305,408,55 | Top right | Top aligned to left dropdown | Horizontal 3-slice | 1350,305,408,55 | Owned layout later |
| Highest score table panel | Table/card area | Right column panel | 847,376,929,302 | Top | 27 px below filters | 9-slice panel + rows/dividers | 847,376,929,302 | Owned layout later |
| Highest score title/live header | Live table title/header labels | Highest score table panel | 870,395,850,80 | Top | Column spacing from reference | Live text/dividers | 870,395,850,80 | Live/runtime |
| Highest score rows | Five table rows | Highest score table panel | 870,492,850,160 | Fill | 36-38 px row pitch | Row strip + dividers | 870,492,850,160 | Owned layout later |
| Best speed run table panel | Table/card area | Right column panel | 847,689,929,302 | Bottom | 16 px gap from upper table | 9-slice panel + rows/dividers | 847,689,929,302 | Owned layout later |
| Best speed run title/live header | Live table title/header labels | Best speed run table panel | 870,704,850,82 | Top | Column spacing from reference | Live text/dividers | 870,704,850,82 | Live/runtime |
| Best speed run rows | Five table rows | Best speed run table panel | 870,803,850,160 | Fill | 36-38 px row pitch | Row strip + dividers | 870,803,850,160 | Owned layout later |
| Content scrollbar | Vertical scrollbar | Main content frame | 1829,277,36,747 | Right | Arrow caps top/bottom | Rail/thumb/cap components | 1829,277,36,747 | Owned layout later |

Hierarchy/containment/occupancy gate result: PASS for Phase A reference mapping.

Remaining overflow/containment/rect-delta issues: None recorded in Phase A because no implementation assembly was attempted.

## Build And Capture

Source files changed:

- `C:\UE\T66\Source\T66\UI\Screens\T66AccountStatusScreen.cpp`

Source change summary:

- Overview state now resolves its own promoted scrollbar thumb and progress meter assets.
- Overview layout is constrained to the normalized reference width balance, with left/right column weights based on 748/994 normalized rects.
- Overview profile, status, and progress cards are height-bound to their normalized reference occupancy.
- Second shared-source pass re-established the AccountStatus dark body scaffold below the protected top nav and stopped using the parchment content shell as the page background.
- Overview and History content now use the same 1760 px shared scaffold width so the left/right columns fill the central framed content area.
- ACCOUNT title/tabs were moved upward and scaled closer to the reference header band.
- History behavior remains state-gated; History layout uses the shared scaffold but no History/Common runtime assets were edited.
- Third shared-source pass changed the Overview/History content wrapper from centered child alignment to fill alignment, removing the large blank left/right margins.
- Third pass reduced Overview lower card/table heights and History table heights so lower panels fit between the tabs and bottom chrome.
- Third pass removed extra History side padding and added a simple dark wood/gold outer content frame approximation around the AccountStatus body.
- Third pass moved the AccountStatus body scaffold upward to better match the normalized title/tab/content y positions.
- Fourth shared-source pass removed fill/center scaffold behavior and replaced the body with an explicit fixed 1828x852 AccountStatus canvas.
- Fourth pass uses explicit rect slots for title, tabs, primary content, and scrollbar: title `603,24,622,58`, tabs `530,88,768,48`, content `48,150,1760,648`, scrollbar `1776,150,36,648` within the body canvas.
- Fourth pass removed the content-driving scroll viewport around primary panels; the scrollbar remains as a fixed visual/control slot so primary Overview and History panels can fit in one 1920x1080 capture.
- Fourth pass clamps Overview/History content boxes to `1760x648` and keeps panel/table heights reduced for no bottom clipping.

Build attempts:

- 0. Coordinator worker mode forbids build in this handoff.

Capture attempts:

- 0. Coordinator worker mode forbids local capture in this handoff.

Central capture commands:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Exe C:\UE\T66\Binaries\Win64\T66.exe -Screen AccountStatus -ResX 1920 -ResY 1080 -Output C:\UE\T66\UI\Reference\Screens\AccountStatus\Overview\Proof\AccountStatus_Overview_central_1920x1080.png -ExtraArgs "-T66AccountTab=Overview"
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Exe C:\UE\T66\Binaries\Win64\T66.exe -Screen AccountStatus -ResX 1920 -ResY 1080 -Output C:\UE\T66\UI\Reference\Screens\AccountStatus\History\Proof\AccountStatus_History_central_1920x1080.png -ExtraArgs "-T66AccountTab=History"
```

## Remaining Differences

Pending central capture. This worker did not build or capture by coordinator instruction.

Approved out-of-scope differences:

- Shared top bar/header/nav/currency/avatar/back/settings chrome.
- History/Common runtime assets.
- Live account/player/stat/score/date/rank/time text and values.

Next action:

- Coordinator should run central build/capture with the commands above and compare Overview plus History against the normalized 1920x1080 occupancy table.
