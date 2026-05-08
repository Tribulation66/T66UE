# HeroSelection Reference UI Manifest

Status: READY_FOR_CENTRAL_BUILD_AND_CAPTURE
Target: HeroSelection
Base screen/modal: Screen
Target state: Default/HeroSelection
Coordinator worker mode: yes
Pass count: 1

Second-pass central review result:
- Central build: PASS
- Central visual: FAIL before this correction
- Primary failure: 1920x1080 reference canvas rendered too large in Slate logical units under global UI scale.
- Second-pass correction: the fixed 1920x1080 reference canvas is now wrapped in a logical viewport-sized `SScaleBox` using `ScaleToFit`, so normalized reference rects are scaled to the actual logical viewport before global UI scale is applied.
- Secondary correction: the dark wood panel runtime slice was re-promoted with an opaque interior; the previous matte cleanup had made near-black wood pixels transparent and allowed the 3D scene to bleed through the left panel body.

Third-pass central review result:
- Central visual after second pass: FAIL
- Remaining failures: footer shells used broad parchment fills, center preview read as full-page scene background, right stats/ability bands were undersized, and left skin rows were compressed.
- Third-pass corrections: kept the root `ScaleToFit` fix, moved left and right footer containers to dark wood content shells, fixed the left skin list card to the normalized reference height, clipped the center preview inside a black bounded frame, and expanded the right stats/ULT bands to the normalized reference vertical rhythm.

Fourth-pass central review result:
- Central visual after third pass: FAIL
- Critical blocker: center preview became black/empty because the previous bounded-frame wrapper covered or suppressed the live preview rendering.
- Fourth-pass correction: kept root `ScaleToFit` and top-level panel occupancy, restored direct live preview widget drawing in the center slot, and reduced the left skin parchment band height toward the compact two-row reference.

## Phase A Scope

Owned area: HeroSelection screen-owned component art and reference documentation under `C:\UE\T66\UI\Reference\Screens\HeroSelection` and `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection`.

Protected/shared area: MainMenu, Shared, sibling screens, shared top bar/chrome systems, build/capture scripts, and all listed target source files. No source/layout/build/capture work was performed during this phase.

Reference image: `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\HeroSelection.png`

## Reset Archive

Archive root: `C:\UE\T66\UI\Reference\Screens\HeroSelection\Archive\PreRun_Reset_20260505_212158`

| Archived path | Reason |
| --- | --- |
| `C:\UE\T66\UI\Reference\Screens\HeroSelection\Archive\PreRun_Reset_20260505_212158\Workspace_Working` | Previous generated sprite sheet workspace archived before fresh imagegen pass. |
| `C:\UE\T66\UI\Reference\Screens\HeroSelection\Archive\PreRun_Reset_20260505_212158\MANIFEST_V4.md` | Previous manifest archived before writing the Phase A master manifest. |
| `C:\UE\T66\UI\Reference\Screens\HeroSelection\Archive\PreRun_Reset_20260505_212158\Runtime_HeroSelection` | Previous target runtime asset folder archived before coordinator sprite review pass. |

Real proof/reference screenshots under `Proof` were preserved in place.

## Imagegen Candidates

Built-in imagegen used: yes
Reference-derived generation: yes, from the opened HeroSelection reference screenshot
OPENAI_API_KEY / SDK / `scripts/image_gen.py`: not used

| Result | Path | Notes |
| --- | --- | --- |
| Accepted | `C:\UE\T66\UI\Reference\Screens\HeroSelection\Working\Pass_01\Candidates\HeroSelection_textfree_atomic_sheet_pass01_ACCEPTED.png` | Text-free atomic component sheet. Parent shells are empty; slots, buttons, scrollbar parts, dividers, ornaments, and dropdown chevron are separated. |
| Rejected | `C:\UE\T66\UI\Reference\Screens\HeroSelection\Archive\Rejected\Pass_01\ig_0f8be39c39e3e6170169fa89aac058819b942c81543dd5e46d.png` | Rejected for baked dropdown arrow in shell and slightly over-polished large parchment density. |
| Rejected | `C:\UE\T66\UI\Reference\Screens\HeroSelection\Archive\Rejected\Pass_01\ig_0fea4cea86a2c6f80169fa89b017c4819a8924c8e7949354ba.png` | Rejected for chroma-green background, composite-like large panel families, and baked dropdown arrow. |
| Rejection notes | `C:\UE\T66\UI\Reference\Screens\HeroSelection\Archive\Rejected\Pass_01\REJECTION_NOTES.txt` | Short rejection notes for failed candidates. |

## Atomic Component Table

| Component name | Atomic role | Parent/child status | Backgroundless expectation | Resize contract | Acceptance |
| --- | --- | --- | --- | --- | --- |
| HS_ParchmentPanel_Large | Empty panel shell | Parent shell only; no baked children | Slice/export later with alpha outside bounds | 9-slice | PASS |
| HS_ParchmentPanel_Small | Empty panel/card shell | Parent shell only; no baked children | Slice/export later with alpha outside bounds | 9-slice | PASS |
| HS_ParchmentRow | Empty row strip | Child row background only | Slice/export later with alpha outside bounds | Horizontal 3-slice | PASS |
| HS_DarkWoodPanel | Empty dark wood panel shell | Parent shell only; no baked children | Slice/export later with alpha outside bounds | 9-slice | PASS |
| HS_Slot_Normal | Single empty slot frame | Child slot only | Slice/export later with alpha outside bounds | Fixed or 9-slice depending crop | PASS |
| HS_Slot_Selected | Single empty selected slot frame | Child slot only | Slice/export later with alpha outside bounds | Fixed or 9-slice depending crop | PASS |
| HS_SquareButton_Normal | Single empty square button state | Child button only | Slice/export later with alpha outside bounds | Fixed or 9-slice | PASS |
| HS_SquareButton_Selected | Single empty square selected button state | Child button only | Slice/export later with alpha outside bounds | Fixed or 9-slice | PASS |
| HS_Button_Normal | Single empty slim button state | Child button only | Slice/export later with alpha outside bounds | Horizontal 3-slice | PASS |
| HS_Button_Hover | Single empty slim button state | Child button only | Slice/export later with alpha outside bounds | Horizontal 3-slice | PASS |
| HS_Button_Pressed | Single empty slim button state | Child button only | Slice/export later with alpha outside bounds | Horizontal 3-slice | PASS |
| HS_Button_Disabled | Single empty slim button state | Child button only | Slice/export later with alpha outside bounds | Horizontal 3-slice | PASS |
| HS_DropdownShell | Empty dropdown shell | Child control shell only; chevron separate | Slice/export later with alpha outside bounds | Horizontal 3-slice | PASS |
| HS_DropdownChevron | Separate empty down-chevron ornament | Child control ornament only | Slice/export later with alpha outside bounds | Fixed | PASS |
| HS_ScrollbarRail | Vertical rail only | Child control only; thumb separate | Slice/export later with alpha outside bounds | Vertical 3-slice | PASS |
| HS_ScrollbarThumb | Vertical thumb only | Child control only; rail separate | Slice/export later with alpha outside bounds | Vertical 3-slice | PASS |
| HS_Divider | Thin horizontal divider | Child divider only | Slice/export later with alpha outside bounds | Horizontal 3-slice or fixed | PASS |
| HS_CornerOrnaments | Isolated corner ornaments | Child ornament only | Slice/export later with alpha outside bounds | Fixed | PASS |
| HS_BorderCaps | Isolated border caps | Child ornament only | Slice/export later with alpha outside bounds | Fixed | PASS |

Atomic component gate: PASS

No baked parent/child failures remain in the accepted sheet. The dropdown arrow is separate from the dropdown shell.

## Sprite Sheet Quality Acceptance Table

| Check | Result | Reason |
| --- | --- | --- |
| Color temperature | PASS | Warm amber parchment, dark wood, and gold trim match the reference family. |
| Brightness | PASS | Components are not materially brighter or darker than the reference chrome. |
| Paper tone | PASS | Parchment panels retain the reference's warm tan tone and restrained texture. |
| Wood tone | PASS | Dark wood controls remain near-black/brown with low-key grain. |
| Metal/gold tone | PASS | Gold trim is close to the reference hue without extra gems or saturated bevels. |
| Border thickness | PASS | Borders remain thin and readable, consistent with the source style. |
| Ornament density | PASS | Ornamentation is restrained and isolated; no extra composite mini-layouts. |
| Grain/noise | PASS | Pixel grain is present but not blurrier or noisier than the reference family. |
| Bevel strength | PASS | Bevels stay modest and sliceable. |
| Contrast | PASS | Contrast is close to the reference UI chrome and not over-polished. |
| Geometry/proportions | PASS | Sheet contains the needed panel, slot, button, dropdown, scrollbar, divider, and ornament families. |
| Component correspondence | PASS | Components correspond to visible HeroSelection UI families. |
| Text-free status | PASS | No letters, numbers, labels, names, stats, portraits, or live data are present. |
| Atomic-component status | PASS | Parent shells are empty and child controls are separated. |

Sprite sheet quality gate: PASS

## Reference Hierarchy And Occupancy Map

Native reference size: 1672x941
Normalized implementation size: 1920x1080
scaleX: 1920/1672 = 1.1483253589
scaleY: 1080/941 = 1.1477151966

Implementation targets are the normalized 1920x1080 rects. Native rects are kept for traceability to the actual reference file dimensions.

| Component | Role | Parent | Native rect x,y,w,h | Normalized 1920 rect x,y,w,h | Anchor | Padding/spacing | Resize contract | Intended implementation rect | Containment notes |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| ScreenRoot | Full screen | none | 0,0,1672,941 | 0,0,1920,1080 | Fill | none | fixed root | 0,0,1920,1080 | Contains all screen-owned and shared chrome. |
| SharedTopBar_Protected | Shared top/menu strip | ScreenRoot | 0,0,1672,112 | 0,0,1920,129 | Top fill | protected | protected | protected | Includes Back, Fashion title, hero carousel, Hero Name, Settings; shared chrome remains protected. |
| LeftFashionPanel | Skin/fashion panel | ScreenRoot | 16,24,512,730 | 18,28,588,838 | Left top | outer 18-24 px | 9-slice shell | 18,28,588,838 | Owns parchment skin list, bottom skin slots, and left trim within HeroSelection. |
| LeftSkinListCard | Parchment list card | LeftFashionPanel | 35,112,472,211 | 40,129,542,242 | Top fill within left panel | 20 px internal | 9-slice | implementation child of LeftFashionPanel | Contains live skin names, checkbox/slot icons, Preview/Equipped/cost buttons as live child widgets. |
| SkinRow_Default | Empty row region | LeftSkinListCard | 52,134,444,80 | 60,154,510,92 | Top | row gap 16 px | horizontal/fill | implementation child of LeftSkinListCard | Live text and checkbox remain runtime. |
| SkinRow_Beachgoer | Empty row region | LeftSkinListCard | 52,227,444,80 | 60,261,510,92 | Top | row gap 16 px | horizontal/fill | implementation child of LeftSkinListCard | Live text, preview button, cost, and icon remain runtime children. |
| SkinSlotsFooter | Slot/button row | LeftFashionPanel | 35,648,477,87 | 40,744,548,100 | Bottom | 16 px gaps | fixed/9-slice children | implementation child of LeftFashionPanel | Contains four empty skin slots and Clear button. |
| CenterPreviewViewport | Character preview scene | ScreenRoot | 533,113,692,636 | 612,130,795,730 | Center fill | protected scene art | not generated | 612,28,795,838 canvas slot with internal top strip | 3D/scene preview content, not sprite-sheet chrome. |
| RightStatsPanel | Hero info panel | ScreenRoot | 1228,24,429,730 | 1410,28,493,838 | Right top | outer 18-24 px | 9-slice shell | 1410,28,493,838 | Owns parchment description, medal/rank row, stats card, ult row, right trim. |
| HeroDescriptionCard | Parchment description shell | RightStatsPanel | 1252,112,402,267 | 1438,129,462,306 | Top | 18 px internal | 9-slice | implementation child of RightStatsPanel | Empty art only; live description text absent in reference state. |
| MedalRankRow | Parchment row shell | RightStatsPanel | 1252,389,402,61 | 1438,446,462,70 | Mid | two cells, 12 px gutters | horizontal/9-slice | implementation child of RightStatsPanel | Live medal/rank labels and icons remain runtime children. |
| StatsCard | Parchment stats card | RightStatsPanel | 1252,460,402,193 | 1438,528,462,222 | Mid | two columns, center divider | 9-slice | implementation child of RightStatsPanel | Live stat labels/values remain runtime; divider can use atomic divider asset. |
| UltRow | Parchment row shell | RightStatsPanel | 1252,666,402,75 | 1438,764,462,86 | Bottom | centered live content | horizontal/9-slice | implementation child of RightStatsPanel | Live ULT label/icon remain runtime children. |
| BottomPartyPanel | Party/player slots footer | ScreenRoot | 16,762,627,162 | 18,875,720,186 | Bottom left | slot gaps 20-24 px | 9-slice shell + fixed slots | 18,875,720,186 | Owns player card, invite slots, Ready button. |
| PartyPlayerSlot | Player slot frame | BottomPartyPanel | 44,790,132,82 | 51,907,152,94 | Left | label below | fixed/9-slice | implementation child of BottomPartyPanel | Live avatar/image/text/check remain runtime children. |
| PartyInviteSlot1 | Empty invite slot | BottomPartyPanel | 197,791,132,80 | 226,908,152,92 | Center | label below | fixed/9-slice | implementation child of BottomPartyPanel | Plus icon/text stay runtime unless separate static icon is approved. |
| PartyInviteSlot2 | Empty invite slot | BottomPartyPanel | 352,791,132,80 | 404,908,152,92 | Center | label below | fixed/9-slice | implementation child of BottomPartyPanel | Plus icon/text stay runtime unless separate static icon is approved. |
| PartyInviteSlot3 | Empty invite slot | BottomPartyPanel | 507,791,132,80 | 582,908,152,92 | Center | label below | fixed/9-slice | implementation child of BottomPartyPanel | Plus icon/text stay runtime unless separate static icon is approved. |
| ReadyButton | Empty button plate | BottomPartyPanel | 524,790,105,85 | 602,907,121,98 | Right | 16 px from panel edge | 9-slice/fixed | implementation child of BottomPartyPanel | Live Ready label remains runtime. |
| BottomCompanionPanel | Companion chooser footer | ScreenRoot | 646,762,441,162 | 742,875,506,186 | Bottom center | 18 px internal | 9-slice shell + buttons | 742,875,506,186 | Owns Chad/Stacy buttons and Choose Companion plate. |
| CompanionButton_Chad | Empty slim button | BottomCompanionPanel | 676,791,179,54 | 776,908,206,62 | Top left | 16 px gap | horizontal 3-slice | implementation child of BottomCompanionPanel | Live name/icon remain runtime children. |
| CompanionButton_Stacy | Empty slim button | BottomCompanionPanel | 869,791,179,54 | 998,908,206,62 | Top right | 16 px gap | horizontal 3-slice | implementation child of BottomCompanionPanel | Live name/icon remain runtime children. |
| ChooseCompanionButton | Empty wide button | BottomCompanionPanel | 678,847,375,51 | 779,972,431,59 | Bottom | 12 px top gap | horizontal 3-slice | implementation child of BottomCompanionPanel | Live label remains runtime. |
| BottomEnterPanel | Difficulty/enter footer | ScreenRoot | 1091,762,566,162 | 1253,875,650,186 | Bottom right | 18 px internal | 9-slice shell + controls | 1253,875,650,186 | Owns difficulty dropdown, Enter button, icon button. |
| DifficultyDropdown | Empty dropdown shell | BottomEnterPanel | 1118,796,148,84 | 1284,914,170,96 | Left | 24 px from panel edge | horizontal 3-slice | implementation child of BottomEnterPanel | Live difficulty label and separate chevron remain runtime children. |
| EnterButton | Empty wide button | BottomEnterPanel | 1286,796,268,84 | 1477,914,308,96 | Center | 20 px gap | horizontal 3-slice | implementation child of BottomEnterPanel | Live Enter label remains runtime. |
| WeaponIconButton | Empty square button | BottomEnterPanel | 1568,796,75,84 | 1801,914,86,96 | Right | 16 px from edge | fixed/9-slice | implementation child of BottomEnterPanel | Weapon/crossed-sword icon is runtime/static child, not baked into button plate. |

Hierarchy/containment/occupancy gate: PASS for Phase B source binding.

Remaining overflow/containment/rect-delta issues: none documented pre-capture after second-pass scale fix. Central capture must audit the normalized rects.

Scale fit contract:
- `ReferenceCanvas`: fixed 1920x1080 logical design canvas containing normalized rect slots.
- `Root`: logical viewport-sized wrapper from `FT66Style::GetViewportLogicalSize()` or automation `T66AutomationResX/Y` divided by `FT66Style::GetGlobalUIScale()`.
- Fit behavior: `SScaleBox` with `EStretch::ScaleToFit` and `EStretchDirection::Both`.
- Expected central capture result: after Slate/global UI scaling, top-level owned regions should land at left x~18 w~588, center x~612 w~795, right x~1410 w~493, bottom y~875 h~186 in the physical 1920x1080 capture.

## Runtime Asset Promotion

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
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Controls\heroselection_controls_imagegen20260505_slim_divider.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Controls\heroselection_controls_imagegen20260505_slim_dropdown_chevron.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Controls\heroselection_controls_imagegen20260505_slim_dropdown_field.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Controls\heroselection_controls_imagegen20260505_slim_scrollbar_rail.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Controls\heroselection_controls_imagegen20260505_slim_scrollbar_thumb.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Ornaments\heroselection_border_cap_a.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Ornaments\heroselection_corner_ornament_a.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Ornaments\heroselection_corner_ornament_b.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Panels\heroselection_panels_imagegen20260505_slim_parchment_card.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Panels\heroselection_panels_imagegen20260505_slim_parchment_panel.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Panels\heroselection_panels_imagegen20260505_slim_parchment_row.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Panels\heroselection_panels_imagegen20260505_slim_tall_panel.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Panels\heroselection_panels_imagegen20260505_slim_wide_panel.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Slots\heroselection_slots_portrait_slot_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Slots\heroselection_slots_portrait_slot_selected.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Slots\heroselection_slots_reference_square_slot_frame_normal.png`

Promotion note: slices were created only from the approved sheet. Edge-connected sheet matte was cleared to alpha 0 with RGB 0,0,0 outside component silhouettes.

Second-pass asset correction note: rectangular filled components were re-promoted from the approved sheet with opaque interiors to prevent sheet-matte cleanup from removing dark wood/panel body pixels. Non-rect ornament/control slices remain separated in the target runtime folder.

## Source, Build, And Capture

Source files changed:
- `C:\UE\T66\Source\T66\UI\Screens\HeroSelection\T66HeroSelectionScreen_Build.cpp`
- `C:\UE\T66\Source\T66\UI\Screens\HeroSelection\T66HeroSelectionScreen_Private.h`
- `C:\UE\T66\Source\T66\UI\Screens\HeroSelection\T66HeroSelectionScreen_Stats.cpp`

Build attempts: 0, coordinator worker mode
Capture attempts: none, coordinator worker mode
Working visual screenshot proof: none, coordinator worker mode

Central capture command for later coordinator/build phase:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Exe C:\UE\T66\Binaries\Win64\T66.exe -Screen HeroSelection -ResX 1920 -ResY 1080 -Output C:\UE\T66\UI\Reference\Screens\HeroSelection\Proof\HeroSelection_central_1920x1080.png
```

## Remaining Visual Differences

Not evaluated against current implementation in this worker pass because coordinator mode forbids build/capture. Central capture should compare against the corrected normalized occupancy table.

Approved live-data/top-bar-shared differences:
- Shared top bar is protected for this target pass.
- Live labels, names, stats, values, icons, portraits, player data, and 3D preview content are not baked into generated art and remain runtime responsibilities.

Next action if not pass: central build and capture with the command above, then visual audit against the normalized 1920x1080 occupancy map.
