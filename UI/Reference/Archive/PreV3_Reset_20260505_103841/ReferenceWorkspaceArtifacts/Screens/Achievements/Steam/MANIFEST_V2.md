# AchievementsSteam V2 Manifest

## Reference Gate

- Target: AchievementsSteam
- Base screen/modal: Achievements
- State: Steam achievements tab
- Preferred reference: `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\AchievementsSteam.png`
- Preferred reference status: missing
- Fallback reference: `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\Achievements.png`
- Fallback reference status: accepted because it visibly shows the Steam tab selected.

## Pre-V2 Reset

- Archived old active Achievements runtime art to `C:\UE\T66\UI\Reference\Screens\Achievements\Steam\Archive\PreV2_Reset_20260505_100101\`
- Cleared active target-owned folders: `Buttons`, `Panels`, `Controls`, `Slots`, `ScreenArt`

## Geometry Map - Reference 1920x1080

- Owned background/frame: x=0 y=128 w=1920 h=952; resize=fixed/fill image behind owned content
- Title row: x~586 y~149 w~745 h~77; resize=live text
- Steam tab: x~649 y~240 w~306 h~58; resize=horizontal 3-slice button
- Secret tab: x~972 y~240 w~306 h~58; resize=horizontal 3-slice button
- Total progress panel: x~77 y~313 w~1770 h~132; resize=9-slice panel
- Progress meter: x~481 y~390 w~961 h~31; resize=horizontal 3-slice meter
- Section label: x~108 y~462 w~442 h~38; resize=live text
- Achievement rows: x~79 y~503 w~1698 h~77, spacing~16; resize=9-slice row shell
- Icon slot: x~121 y~522 w~66 h~66; resize=fixed image
- Favorite plate: x~1660 y~520 w~68 h~68; resize=fixed image with live glyph
- Scrollbar: x~1822 y~241 w~42 h~781; resize=vertical 3-slice rail/thumb

## Geometry Map - Current Capture Before Editing

- Current proof: `C:\UE\T66\UI\Reference\Screens\Achievements\Steam\Proof\AchievementsSteam_pass00_current_packaged_1920x1080.png`
- Owned background/frame: x=0 y~147 w=1920 h~933; too intrusive at left/right
- Title row: x~709 y~196 w~584 h~65; too low
- Steam tab: x~513 y~299 w~438 h~79; too large and too low
- Secret tab: x~970 y~299 w~438 h~79; too large and too low
- Total progress panel: x~105 y~397 w~1717 h~157; too low and too tall
- Progress meter: x~265 y~482 w~1392 h~30; too wide
- Section label: x~134 y~580 w~363 h~38; too low
- Achievement rows: x~105 y~633 w~1683 h~82; too low and slightly too tall
- Icon slot: x~133 y~644 w~85 h~65; too large/wrong tint
- Favorite plate: x~1665 y~644 w~75 h~65; too large and glyph differs
- Scrollbar: x~1806 y~571 w~19 h~497; too low and missing arrow caps

## Pass 01

### Difference List Before Editing

- layout: title, tabs, progress panel, section label, rows, and scrollbar are lower than reference.
- layout: tab plates are too wide/tall versus the reference.
- layout: progress meter is too wide versus the reference.
- asset: active runtime folder contained pre-v2 Achievements assets, now archived.
- asset: favorite/star plate and icon slot art do not match the quieter reference proportions.
- resize-contract: progress/scrollbar art was routed through old sheet UVs instead of direct target-owned v2 assets.
- top-bar-shared: top bar button/currency/avatar differences are out of scope and were not edited.
- live-data: achievement names, descriptions, counts, reward values, and favorite glyphs remain live.

### Generated Candidates

- `C:\UE\T66\UI\Reference\Screens\Achievements\Steam\Working\Pass_01\Candidates\achievements_steam_pass01_reference_derived_sheet.png`
- `C:\UE\T66\UI\Reference\Screens\Achievements\Steam\Working\Pass_01\Candidates\achievements_steam_pass01_direct_components_chromakey.png`
- `C:\UE\T66\UI\Reference\Screens\Achievements\Steam\Working\Pass_01\Candidates\achievements_steam_pass01_direct_components_alpha.png`

### Accepted Runtime Assets

- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements\Buttons\Pill\*.png` - horizontal 3-slice button plates
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements\Buttons\SquareIcon\*.png` - fixed favorite button plates
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements\Panels\achievements_panels_reference_progress_panel_v2.png` - 9-slice progress panel
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements\Panels\achievements_panels_reference_row_shell_v2.png` - 9-slice achievement row shell
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements\Controls\achievements_controls_progress_track_v2.png` - horizontal 3-slice progress track
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements\Controls\achievements_controls_progress_fill_v2.png` - horizontal 3-slice progress fill
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements\Controls\achievements_controls_scrollbar_track_v2.png` - vertical 3-slice scrollbar rail
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements\Controls\achievements_controls_scrollbar_thumb_v2.png` - vertical 3-slice scrollbar thumb
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements\Controls\achievements_controls_scrollbar_arrow_up_v2.png` - fixed image, generated but not yet used by Slate scrollbar
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements\Controls\achievements_controls_scrollbar_arrow_down_v2.png` - fixed image, generated but not yet used by Slate scrollbar
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements\Slots\achievements_slots_reference_square_slot_frame_normal.png` - fixed icon slot
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements\ScreenArt\achievements_screen_art_dark_wood_frame_v2.png` - fixed/fill owned background art

### Source Files Changed

- `C:\UE\T66\Source\T66\UI\Screens\T66AchievementsScreen.cpp`

### Proof Captures

- `C:\UE\T66\UI\Reference\Screens\Achievements\Steam\Proof\AchievementsSteam_pass01_packaged_1920x1080.png`

### Difference List After Pass 01

- layout: title is clipped under the shared top bar because the owned content starts too high for the current packaged top-bar height.
- layout/resize-contract: tab plates remain taller than the reference.
- layout: progress meter is still far wider than the reference.
- layout: row shells are taller/lower than the reference and show fewer entries in the viewport.
- asset/top-bar-shared: shared top bar and side frame are heavier than the reference; top bar remains out of scope.
- live-data: achievement names, descriptions, counts, reward values, and favorite glyphs remain live.

## Pass 02

### Changes

- Reduced tab button height and text size.
- Narrowed the progress meter resize target.
- Reduced row height and row padding.
- Expanded owned content width by reducing side padding.

### Source Files Changed

- `C:\UE\T66\Source\T66\UI\Screens\T66AchievementsScreen.cpp`

### Proof Captures

- `C:\UE\T66\UI\Reference\Screens\Achievements\Steam\Proof\AchievementsSteam_pass02_packaged_1920x1080.png`

### Difference List After Pass 02

- layout: title/tab stack remains lower than reference.
- layout: list section and first row remain lower than reference; progress panel itself is close.
- layout: row height is closer but still slightly taller than reference.
- asset/top-bar-shared: shared top bar and side frame remain heavier than reference and were not edited.
- live-data: achievement names, descriptions, counts, reward values, and favorite glyphs remain live.

## Pass 03

### Changes

- Tightened vertical spacing above the tab row.
- Reduced the gap after the progress panel.
- Reduced section label bottom padding so rows start closer to the section header.

### Source Files Changed

- `C:\UE\T66\Source\T66\UI\Screens\T66AchievementsScreen.cpp`

### Proof Captures

- pending Pass 03 staged packaged capture
