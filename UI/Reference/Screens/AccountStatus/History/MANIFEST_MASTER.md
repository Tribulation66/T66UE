# AccountStatus History Reference UI Manifest

Status: READY_FOR_CENTRAL_BUILD_AND_CAPTURE  
Target: AccountStatus History  
Base screen/modal: Screen  
Target state: History  
Coordinator worker mode: yes  
Phase: B, runtime asset promotion and state-gated source/layout wiring  
Pass count: 1 reset pass, 2 imagegen candidates, 1 Phase B slicing/wiring pass

## Source Inputs

| Item | Path | Status |
| --- | --- | --- |
| Master prompt | `C:\UE\T66\UI\MASTER_REFERENCE_UI_GENERATION_PROMPT.md` | Read |
| Task handoff | `C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md` | Read |
| Global process | `C:\UE\T66\Docs\UI\UI_GENERATION.md` | Read |
| Exact reference image | `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\AccountStatusHistory.png` | Exists, viewed |
| Capture script | `C:\UE\T66\Scripts\CaptureT66UIScreen.ps1` | Exists, not run |
| Local executable | `C:\UE\T66\Binaries\Win64\T66.exe` | Exists, not run |
| Target source cpp | `C:\UE\T66\Source\T66\UI\Screens\T66AccountStatusScreen.cpp` | Exists, edited in Phase B |
| Target source h | `C:\UE\T66\Source\T66\UI\Screens\T66AccountStatusScreen.h` | Exists, not edited |

## Reset Archive

| Scope | Archive path | Notes |
| --- | --- | --- |
| Active workspace generated candidates | `C:\UE\T66\UI\Reference\Screens\AccountStatus\History\Archive\PreRun_Reset_20260505_212218\WorkspaceGenerated` | Archived prior `Working` contents only |
| Active History runtime assets | `C:\UE\T66\UI\Reference\Screens\AccountStatus\History\Archive\PreRun_Reset_20260505_212218\RuntimeAssets` | Archived prior History runtime files/folders only |
| Preserved reference/proof/history | `C:\UE\T66\UI\Reference\Screens\AccountStatus\History\Proof` and existing manifests/rejected archive | Not reset |
| Protected same-screen common folder | `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Common` | Not touched |
| Protected sibling state/common screens | AccountStatus Overview, MainMenu, Shared, sibling screens | Not touched |

## Generated Candidate Paths

| Candidate | Path | Result | Reason |
| --- | --- | --- | --- |
| Pass 01 | `C:\UE\T66\UI\Reference\Screens\AccountStatus\History\Archive\Rejected\Pass_01\accountstatus_history_textfree_sheet_pass01_REJECTED_baked_table_composite.png` | Rejected | Text-free, but included baked table/header/row mini-layout that violated the atomic parent/child rule |
| Pass 02 | `C:\UE\T66\UI\Reference\Screens\AccountStatus\History\Working\Pass_01\Candidates\accountstatus_history_textfree_atomic_sheet_pass02_ACCEPTED.png` | Accepted for coordinator sprite review | Text-free isolated component sheet; parent panels are empty shells; rows, strips, controls, ornaments, and scrollbar pieces are separate |

Rejected note: `C:\UE\T66\UI\Reference\Screens\AccountStatus\History\Archive\Rejected\Pass_01\REJECTION_NOTE.txt`

## Runtime Assets

Runtime assets were sliced and promoted from the approved Pass 02 sheet into the History runtime folder. Outside pixels were flood-cleared to alpha 0 with transparent RGB set to 0,0,0.

`C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History`

Accepted runtime asset paths:

- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Buttons\accountstatus_history_pill_disabled.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Buttons\accountstatus_history_pill_hover.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Buttons\accountstatus_history_pill_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Buttons\accountstatus_history_pill_pressed.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Buttons\accountstatus_history_pill_selected.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Buttons\accountstatus_history_square_button_disabled.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Buttons\accountstatus_history_square_button_hover.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Buttons\accountstatus_history_square_button_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Buttons\accountstatus_history_square_button_pressed.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Buttons\accountstatus_history_square_button_selected.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Controls\accountstatus_history_dropdown_field_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Controls\accountstatus_history_dropdown_field_selected.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Controls\accountstatus_history_meter_track.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Controls\accountstatus_history_scrollbar_down_button.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Controls\accountstatus_history_scrollbar_thumb.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Controls\accountstatus_history_scrollbar_track.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Controls\accountstatus_history_scrollbar_up_button.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Controls\accountstatus_history_table_header_strip.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Controls\accountstatus_history_table_row_strip.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Dividers\accountstatus_history_horizontal_divider.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Dividers\accountstatus_history_vertical_divider.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Icons\accountstatus_history_dropdown_chevron.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Ornaments\accountstatus_history_corner_bottom_left.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Ornaments\accountstatus_history_corner_bottom_right.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Ornaments\accountstatus_history_corner_top_left.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Ornaments\accountstatus_history_corner_top_right.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Panels\accountstatus_history_content_panel_shell.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Panels\accountstatus_history_outer_panel_shell.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Panels\accountstatus_history_row_shell.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Panels\accountstatus_history_small_card_shell.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Panels\accountstatus_history_table_panel_shell.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\History\Slots\accountstatus_history_avatar_slot_frame.png`

## Atomic Component Table

| Component name | Atomic role | Parent/child status | Background/alpha expectation | Resize contract | Acceptance |
| --- | --- | --- | --- | --- | --- |
| history_outer_panel_shell | Empty parchment parent shell | Parent only, no baked children | Needs transparent/backgroundless PNG after slicing | 9-slice | Pass |
| history_content_panel_shell | Empty parchment content shell | Parent only, no baked children | Needs transparent/backgroundless PNG after slicing | 9-slice | Pass |
| history_small_card_shell | Empty small parchment card/row shell | Parent shell, no baked controls | Needs transparent/backgroundless PNG after slicing | 9-slice | Pass |
| history_table_header_strip | Single empty header strip | Child strip only | Needs transparent/backgroundless PNG after slicing | Horizontal 3-slice | Pass |
| history_table_row_strip | Single empty row background | Child row only | Needs transparent/backgroundless PNG after slicing | Horizontal 3-slice | Pass |
| history_horizontal_divider | Thin divider | Child ornament/line | Needs transparent/backgroundless PNG after slicing | Horizontal 3-slice or fixed caps with stretch center | Pass |
| history_vertical_divider | Thin vertical divider | Child line | Needs transparent/backgroundless PNG after slicing | Vertical 3-slice or fixed | Pass |
| history_dropdown_field_normal | Empty dropdown field | Child control only | Needs transparent/backgroundless PNG after slicing | Horizontal 3-slice | Pass |
| history_dropdown_field_selected | Empty selected/hover dropdown field | Child control only | Needs transparent/backgroundless PNG after slicing | Horizontal 3-slice | Pass |
| history_dropdown_chevron | Standalone chevron ornament | Child icon/ornament only | Needs transparent/backgroundless PNG after slicing | Fixed | Pass |
| history_tab_normal | Empty tab/button state | Child control only | Needs transparent/backgroundless PNG after slicing | Horizontal 3-slice | Pass |
| history_tab_selected | Empty selected tab/button state | Child control only | Needs transparent/backgroundless PNG after slicing | Horizontal 3-slice | Pass |
| history_square_button_normal | Empty compact square button | Child button state only | Needs transparent/backgroundless PNG after slicing | Fixed or 9-slice if resized | Pass |
| history_square_button_hover | Empty compact square button hover/selected | Child button state only | Needs transparent/backgroundless PNG after slicing | Fixed or 9-slice if resized | Pass |
| history_meter_track | Empty meter track | Child control only | Needs transparent/backgroundless PNG after slicing | Horizontal 3-slice | Pass |
| history_avatar_slot_frame | Empty square slot/frame | Child slot only | Needs transparent/backgroundless PNG after slicing | Fixed or 9-slice if resized | Pass |
| history_scrollbar_rail | Vertical rail | Child scrollbar part | Needs transparent/backgroundless PNG after slicing | Vertical 3-slice | Pass |
| history_scrollbar_thumb | Vertical thumb | Child scrollbar part | Needs transparent/backgroundless PNG after slicing | Vertical 3-slice | Pass |
| history_scrollbar_up_button | Standalone up arrow button | Child button/icon state | Needs transparent/backgroundless PNG after slicing | Fixed | Pass |
| history_scrollbar_down_button | Standalone down arrow button | Child button/icon state | Needs transparent/backgroundless PNG after slicing | Fixed | Pass |
| history_corner_ornament_fragments | Standalone corner trim pieces | Child ornaments only | Needs transparent/backgroundless PNG after slicing | Fixed | Pass |

Atomic component gate: PASS. No accepted parent shell contains baked tabs, rows, labels, icons, dropdowns, scrollbars, portraits, or mini-layouts.

## Sprite Quality Acceptance Table

| Check | Result | Notes |
| --- | --- | --- |
| Color temperature | PASS | Warm parchment, dark wood, and amber-gold trim match the reference family |
| Brightness | PASS | Not materially darker or brighter than the reference UI chrome |
| Paper tone | PASS | Parchment panels retain the tan/yellow paper tone |
| Wood tone | PASS | Button/dropdown/tab plates retain dark brown carved wood tone |
| Metal tone | PASS | Gold trim is amber/brass, not modern yellow or desaturated gray |
| Border thickness | PASS | Borders are close to reference thickness and remain pixel-crisp |
| Ornament density | PASS | Corner brackets and dividers are restrained, not over-ornate |
| Grain/noise | PASS | Subtle texture only; not excessively noisy or damaged |
| Bevel strength | PASS | Bevels are present but not upgraded into heavy embossed chrome |
| Contrast | PASS | Dark controls and parchment interiors retain readable contrast |
| Geometry/proportions | PASS | Component families correspond to reference panels, controls, tabs, and scrollbar pieces |
| Component correspondence | PASS | Sheet includes the visible History UI component families needed for later slicing |
| Text-free status | PASS | No labels, names, values, numbers, dashes, or placeholder text in accepted sheet |
| Atomic-component status | PASS | Accepted sheet is a parts board, not a screen/table layout |

Sprite sheet quality gate: PASS. Accepted sheet path: `C:\UE\T66\UI\Reference\Screens\AccountStatus\History\Working\Pass_01\Candidates\accountstatus_history_textfree_atomic_sheet_pass02_ACCEPTED.png`

## Owned And Protected Areas

Owned for later implementation:

| Area | Ownership |
| --- | --- |
| AccountStatus History tab selected state | Owned by History pass |
| AccountStatus Overview tab normal state where shown on History screen | Owned only as a duplicated target-state tab asset if needed |
| Right History filter dropdowns | Owned |
| Right Stat Record History table shell, header, rows, dividers | Owned |
| Right Unlock History table shell, header, rows, dividers | Owned |
| History-state content panel shells and row/card shells needed by this screen | Owned |
| Right vertical scrollbar parts | Owned |

Protected/shared:

| Area | Protection |
| --- | --- |
| Main top navigation/header/currency/power/settings | Shared top bar frozen, do not edit |
| AccountStatus Overview source/layout/runtime folder | Protected sibling state |
| AccountStatus Common runtime folder | Protected in Phase A |
| MainMenu, Shared, sibling screens/modals | Protected |
| `T66AccountStatusScreen.cpp` | Edited narrowly for History state asset/layout routing in Phase B |
| `T66AccountStatusScreen.h` | Not edited |

## Reference Hierarchy And 1920x1080 Occupancy Map

Reference source dimensions: 1672x941. Normalized target dimensions: 1920x1080.
scaleX = 1920 / 1672 = 1.148325.
scaleY = 1080 / 941 = 1.147715.

The implementation targets the normalized 1920x1080 rects. Native rects below are the original reference-image coordinate space, calculated as normalized rect / scale.

| Name | Role | Parent | Native rect x,y,w,h | Normalized 1920 rect x,y,w,h | Anchor | Padding/spacing | Resize contract | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| screen_frame | Background/frame | root | 0,0,1672,941 | 0,0,1920,1080 | Fill | n/a | Shared/protected | Shared top frame frozen |
| shared_top_nav | Shared top bar | screen_frame | 0,0,1672,116 | 0,0,1920,133 | Top fill | n/a | Shared/protected | Do not edit in this pass |
| account_title | Live title/text region | screen_frame | 710,124,253,58 | 815,142,290,67 | Top center | n/a | Live Slate | Not generated art |
| account_tabs_group | Tab group | screen_frame | 565,188,544,48 | 649,216,625,55 | Top center | 24 px gap between tabs | Horizontal 3-slice tabs | Contains Overview and History tabs |
| overview_tab_normal | Empty tab button | account_tabs_group | 565,188,261,48 | 649,216,300,55 | Left in group | 0 | Horizontal 3-slice | Text live |
| history_tab_selected | Empty selected tab button | account_tabs_group | 848,188,261,48 | 974,216,300,55 | Right in group | 0 | Horizontal 3-slice | Text live |
| left_account_column | Left account panels | screen_frame | 47,242,653,651 | 54,278,750,747 | Left/top | 12-16 px internal | 9-slice panel shells | Shared same screen visual but History-state screen-owned if duplicated |
| player_summary_card | Parchment card shell | left_account_column | 75,267,597,146 | 86,307,685,168 | Top | 23 px | 9-slice | Avatar slot and live text children |
| avatar_slot | Empty square slot | player_summary_card | 95,281,134,116 | 109,323,154,133 | Left | 24 px | Fixed/9-slice | Portrait/icon live or separate |
| status_card | Parchment card shell | left_account_column | 75,419,597,169 | 86,481,685,194 | Middle | 14 px vertical gap | 9-slice | Help button child |
| help_button | Empty square button | status_card | 607,446,48,48 | 697,512,55,55 | Right/top | 21 px right | Fixed | Text/icon live if used |
| progress_card | Parchment card shell | left_account_column | 75,599,597,265 | 86,688,685,304 | Bottom | 14 px vertical gap | 9-slice | Meter tracks and rows live |
| history_right_column | History content area | screen_frame | 711,242,866,651 | 816,278,994,747 | Right/top | 20-28 px | Parent content | Owned |
| filter_row | Controls row | history_right_column | 756,265,775,49 | 868,304,890,56 | Top | 57 px gap | Child controls | Owned |
| record_type_dropdown | Empty dropdown field | filter_row | 756,267,367,44 | 868,306,422,51 | Left | 0 | Horizontal 3-slice | Label and chevron live/separate |
| difficulty_dropdown | Empty dropdown field | filter_row | 1175,267,358,44 | 1349,306,411,51 | Right | 0 | Horizontal 3-slice | Label and chevron live/separate |
| stat_record_history_panel | Empty table panel shell | history_right_column | 738,329,811,259 | 847,378,931,297 | Top | 18 px interior | 9-slice | Owned parent, no baked rows |
| stat_record_title_region | Live title/text region | stat_record_history_panel | 756,345,274,30 | 868,396,315,34 | Top left | 0 | Live Slate | Not generated art |
| stat_header_strip | Empty table header strip | stat_record_history_panel | 747,372,790,45 | 858,427,907,52 | Top | 0 | Horizontal 3-slice | Column labels live |
| stat_row_01 | Empty table row strip | stat_record_history_panel | 747,417,790,35 | 858,479,907,40 | Stack | 0 | Horizontal 3-slice | Data live |
| stat_row_02 | Empty table row strip | stat_record_history_panel | 747,452,790,35 | 858,519,907,40 | Stack | 0 | Horizontal 3-slice | Data live |
| stat_row_03 | Empty table row strip | stat_record_history_panel | 747,487,790,35 | 858,559,907,40 | Stack | 0 | Horizontal 3-slice | Data live |
| stat_row_04 | Empty table row strip | stat_record_history_panel | 747,522,790,35 | 858,599,907,40 | Stack | 0 | Horizontal 3-slice | Data live |
| stat_row_05 | Empty table row strip | stat_record_history_panel | 747,557,790,31 | 858,639,907,36 | Stack | 0 | Horizontal 3-slice | Data live |
| stat_column_dividers | Thin vertical dividers | stat_record_history_panel | 900,372,513,209 | 1034,427,589,240 | Column anchors | Approx 184, 183, 184, 184 px spacing | Fixed/vertical line | Use separate line pieces |
| unlock_history_panel | Empty table panel shell | history_right_column | 738,599,811,265 | 847,687,931,304 | Bottom | 12 px from stat panel | 9-slice | Owned parent, no baked rows |
| unlock_title_region | Live title/text region | unlock_history_panel | 756,613,247,30 | 868,704,284,34 | Top left | 0 | Live Slate | Not generated art |
| unlock_header_strip | Empty table header strip | unlock_history_panel | 747,640,790,42 | 858,735,907,48 | Top | 0 | Horizontal 3-slice | Column labels live |
| unlock_row_01 | Empty table row strip | unlock_history_panel | 747,682,790,35 | 858,783,907,40 | Stack | 0 | Horizontal 3-slice | Data live |
| unlock_row_02 | Empty table row strip | unlock_history_panel | 747,717,790,35 | 858,823,907,40 | Stack | 0 | Horizontal 3-slice | Data live |
| unlock_row_03 | Empty table row strip | unlock_history_panel | 747,752,790,35 | 858,863,907,40 | Stack | 0 | Horizontal 3-slice | Data live |
| unlock_row_04 | Empty table row strip | unlock_history_panel | 747,787,790,35 | 858,903,907,40 | Stack | 0 | Horizontal 3-slice | Data live |
| unlock_row_05 | Empty table row strip | unlock_history_panel | 747,822,790,35 | 858,943,907,40 | Stack | 0 | Horizontal 3-slice | Data live |
| unlock_column_dividers | Thin vertical dividers | unlock_history_panel | 900,640,435,209 | 1034,735,499,240 | Column anchors | Approx 300, 200, 190 px spacing | Fixed/vertical line | Use separate line pieces |
| right_scrollbar_group | Vertical scrollbar | screen_frame | 1594,247,34,638 | 1830,284,39,732 | Right center | n/a | Separate parts | Owned if this screen handles it locally |
| scrollbar_up_button | Up arrow button | right_scrollbar_group | 1598,247,26,34 | 1835,284,30,39 | Top | 0 | Fixed | Standalone arrow button |
| scrollbar_rail | Vertical rail | right_scrollbar_group | 1601,283,20,572 | 1839,325,23,656 | Fill vertical | 0 | Vertical 3-slice | Separate rail |
| scrollbar_thumb | Vertical thumb | right_scrollbar_group | 1601,290,22,140 | 1838,333,25,161 | Top in rail | 0 | Vertical 3-slice | Separate thumb |
| scrollbar_down_button | Down arrow button | right_scrollbar_group | 1598,855,26,30 | 1835,981,30,35 | Bottom | 0 | Fixed | Standalone arrow button |

Hierarchy/containment/occupancy gate: PASS for Phase B reference mapping. Remaining implementation rect-delta issues: central capture pending by coordinator.

## Build And Capture

| Item | Result |
| --- | --- |
| Source files changed | `C:\UE\T66\Source\T66\UI\Screens\T66AccountStatusScreen.cpp` |
| Build attempts | 0, coordinator mode |
| Capture attempts | 0, coordinator mode |
| Current implementation screenshot | Not captured in coordinator mode |
| Central capture command for later | `powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Exe C:\UE\T66\Binaries\Win64\T66.exe -Screen AccountStatus -ResX 1920 -ResY 1080 -Output C:\UE\T66\UI\Reference\Screens\AccountStatus\History\Proof\AccountStatus_History_central_1920x1080.png -ExtraArgs "-T66AccountTab=History"` |

## Remaining Visual Differences

Central capture pending by coordinator. Worker did not build or capture.

## Approved Live Data And Shared Differences

Approved for later implementation/capture:

- Shared top navigation/header/currency/avatar/back/settings differences are out of scope for this target unless coordinator assigns shared top bar work.
- All labels, localized text, player names, stat values, dates, table values, dashes/placeholders, status text, balances, and progress numbers remain live Slate/UMG data, not baked art.

## Next Action

Coordinator should review the accepted sprite sheet at:

`C:\UE\T66\UI\Reference\Screens\AccountStatus\History\Working\Pass_01\Candidates\accountstatus_history_textfree_atomic_sheet_pass02_ACCEPTED.png`

Coordinator should run the central build/capture pass with the command above, then compare against the normalized 1920x1080 occupancy table.
