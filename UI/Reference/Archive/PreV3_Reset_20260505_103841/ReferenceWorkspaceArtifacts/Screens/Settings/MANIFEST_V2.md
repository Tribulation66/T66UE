# Settings V2 Manifest

Target: Settings
State: default screen/modal state
Reference: `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\Settings.png`

## Pass 01

### Reference Geometry Map At 1920x1080

Top bar/header is shared and frozen for this target.

| Owned family | X | Y | W | H | Spacing / role | Resize contract |
| --- | ---: | ---: | ---: | ---: | --- | --- |
| Settings tab strip | 28 | 146 | 1834 | 82 | eight equal tabs, 13-15 px gaps | horizontal 3-slice tab/button plates |
| Content parchment panel | 67 | 232 | 1804 | 805 | fills owned content below tabs | 9-slice panel frame |
| Page heading row | 132 | 274 | 470 | 44 | decorative rule around live title | fixed dividers, live text |
| Intro body text | 132 | 332 | 1416 | 80 | wraps to 4 lines | live text over panel |
| Master toggle row shell | 104 | 467 | 1620 | 126 | left text, right two buttons, 20 px button gap | 9-slice row shell |
| ON/OFF toggle buttons | 1337 | 493 | 354 | 71 | two equal 163x70 plates | horizontal 3-slice buttons |
| Apply button | 1447 | 619 | 278 | 69 | right aligned below master row | horizontal 3-slice button |
| PS1 section heading | 102 | 711 | 344 | 39 | decorative rule around live title | fixed dividers, live text |
| Slider row 1 shell | 104 | 753 | 1621 | 114 | label left, slider right | 9-slice row shell |
| Slider track 1 | 901 | 796 | 742 | 25 | value above, help below | horizontal 3-slice progress/track |
| Slider row 2 shell | 104 | 882 | 1621 | 114 | label left, slider right | 9-slice row shell |
| Scrollbar | 1782 | 279 | 60 | 699 | rail with arrow caps | vertical 3-slice/fixed arrows |

### Current Packaged Geometry Map At 1920x1080

Current packaged capture: `C:\UE\T66\UI\Reference\Screens\Settings\Proof\settings_pass1_packaged_1920x1080.png`

| Owned family | X | Y | W | H | Spacing / role | Resize contract observed |
| --- | ---: | ---: | ---: | ---: | --- | --- |
| Settings tab strip | 88 | 153 | 1745 | 86 | tabs too tall, lower than reference | horizontal sliced buttons |
| Content parchment panel | 31 | 147 | 1858 | 933 | content starts too high and extends below viewport | 9-slice panel frame |
| Page heading | 84 | 282 | 115 | 30 | missing reference decorative heading treatment | live text only |
| Intro body text | 84 | 341 | 1703 | 74 | too wide, starts too far left | live text over panel |
| Master toggle row shell | 94 | 443 | 1704 | 142 | too wide/tall and lower border heavier | 9-slice row shell |
| ON/OFF toggle buttons | 1480 | 477 | 292 | 74 | buttons too far right, unequal reference spacing | horizontal sliced buttons |
| Apply button | 1540 | 646 | 258 | 64 | too far right/down | horizontal sliced button |
| PS1 section heading | 84 | 749 | 109 | 29 | heading too low and missing rule | live text only |
| Slider row 1 shell | 94 | 793 | 1704 | 203 | too tall; text column and slider too low | 9-slice row shell |
| Slider track 1 | 884 | 889 | 877 | 37 | too long and lower than reference | horizontal sliced progress |
| Scrollbar | 1815 | 274 | 34 | 735 | too narrow and extends lower | vertical brush pieces |

### Difference List Before Editing

- Layout: owned content panel begins at y=147 instead of y=232 and extends below the viewport; major vertical anchoring mismatch.
- Layout: tabs start x=88/y=153 and are taller/lower than reference x=28/y=146.
- Layout: rows are wider/taller than reference; first row is 1704x142 instead of about 1620x126.
- Layout: second slider row is far too tall and too low compared with the reference.
- Asset: old active runtime art was pre-v2 and not from a new reference-derived sheet for this prompt.
- Asset: section heading divider/chrome is missing in current packaged capture.
- Resize-contract: progress/slider art uses stale generated-sheet UVs and does not match the new v2 sheet.
- Top-bar-shared: header chrome, avatar, tickets/currency, language/settings/power buttons are out of scope.
- Live-data: all labels, values, descriptions, and ON/OFF/APPLY text remain live.

### Generated Candidates

- `C:\UE\T66\UI\Reference\Screens\Settings\Working\Pass_01\Candidates\settings_v2_reference_derived_sheet_pass01.png` - accepted, reference-derived text-free component sheet from built-in imagegen.

### Accepted Runtime Assets

- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Panels\settings_panels_reference_scroll_paper_frame.png` - 9-slice panel frame.
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Panels\settings_panels_fullscreen_fullscreen_panel_wide.png` - 9-slice panel frame.
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Panels\settings_panels_fullscreen_row_shell_quiet.png` - 9-slice row shell.
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Buttons\settings_buttons_pill_normal.png` - horizontal 3-slice button.
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Buttons\settings_buttons_pill_hover.png` - horizontal 3-slice button.
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Buttons\settings_buttons_pill_pressed.png` - horizontal 3-slice button.
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Buttons\settings_buttons_pill_selected.png` - horizontal 3-slice toggle selected plate.
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Buttons\settings_buttons_pill_disabled.png` - horizontal 3-slice disabled plate.
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Buttons\settings_buttons_tab_selected_gem.png` - horizontal 3-slice selected tab.
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Buttons\settings_buttons_tab_normal_generated.png` - horizontal 3-slice tab/button.
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Controls\settings_controls_reference_dropdown_field_normal.png` - horizontal 3-slice dropdown field.
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Controls\settings_controls_reference_progress_meter_sheet.png` - progress/slider source sheet with UV regions.
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Controls\settings_controls_controls_sheet.png` - slider thumb source sheet with UV region.
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Controls\settings_controls_scrollbar_track_generated.png` - vertical 3-slice scrollbar track.
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Controls\settings_controls_scrollbar_thumb_generated.png` - vertical 3-slice scrollbar thumb.
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Controls\settings_controls_scrollbar_arrow_up.png` - fixed image arrow.
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Controls\settings_controls_scrollbar_arrow_down.png` - fixed image arrow.
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Dividers\settings_dividers_section_heading_rule.png` - fixed divider rule.

### Rejected Candidates

- None in pass 01.

### Archived / Reset Assets

- `C:\UE\T66\UI\Reference\Screens\Settings\Archive\PreV2_Reset_20260505_100107\`

### Source Files Changed

- `C:\UE\T66\Source\T66\UI\Screens\Settings\T66SettingsScreen_Private.h`

### Proof Captures

- BLOCKED before packaged Settings capture. Staging command attempted twice:
  - `powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\StageStandaloneBuild.ps1 -ClientConfig Development -SkipShortcutRefresh`
  - Both attempts failed before compile/cook with `System.Exception: A conflicting instance of AutomationTool is already running`.
- Recovery performed:
  - Waited 35 seconds and retried exact stage command.
  - Inspected processes matching `AutomationTool`, `UnrealBuildTool`, `BuildCookRun`, `RunUAT`, and `C:\UE\T66`.
  - Found active, non-stale concurrent UAT/capture work for this repo, including PowerUp capture/stage processes and `T66.exe` automation capture. Did not terminate active user/agent work.
  - Waited an additional 15 minutes; active repo automation still remained.
- Packaged Settings proof capture remains blocked by the active AutomationTool singleton, not by the Settings asset/source change.
