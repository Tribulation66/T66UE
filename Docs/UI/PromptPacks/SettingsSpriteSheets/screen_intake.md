# Settings Screen Intake

## Identity

- Screen token: `Settings`
- Display name: `Settings`
- Owner: `UI reconstruction workflow`
- Target platform: `PC, controller-aware`

## Canonical Canvas

- Offline comparison target path: `C:/UE/T66/SourceAssets/UI/SettingsReference/screen_master.png`
- Offline comparison target resolution: `1920x1080`
- Hi-res helper/reference path: `C:/UE/T66/SourceAssets/UI/SettingsReference/screen_master_2x.png`
- Hi-res helper/reference resolution: `3840x2160`
- UI-free scene/background plate path: `TODO`
- Runtime target viewport: `1920x1080`
- Current packaged structure anchor: `C:/UE/T66/SourceAssets/UI/SettingsReference/current_runtime_anchor.png`
- Current packaged helper anchor: `C:/UE/T66/SourceAssets/UI/SettingsReference/current_runtime_anchor_2x.png`
- Approved style anchor: `C:/UE/T66/UI/screens/main_menu/reference/main_menu_reference_1920x1080.png`

## Reference Variants

- Primary comparison frame: `C:/UE/T66/SourceAssets/UI/SettingsReference/current_runtime_anchor.png`
- No-buttons variant for analysis/prompting only: `C:/UE/T66/SourceAssets/UI/SettingsReference/screen_master_no_buttons.png`
- No-text variant for analysis/prompting only: `C:/UE/T66/SourceAssets/UI/SettingsReference/screen_master_no_text.png`
- No-dynamic variant for analysis/prompting only: `C:/UE/T66/SourceAssets/UI/SettingsReference/screen_master_no_dynamic.png`

## Regions

- `Frontend top bar and close/back affordances` — `stateful | live`
- `Settings tab strip` — `stateful`
- `Primary content shell and scroll viewport` — `stateful`
- `Settings row stack: toggles, dropdowns, sliders, text rows` — `live`
- `Graphics confirm overlay and controls rebinding states` — `stateful | live`

## Dynamic And Live Content

- Live data sources: `player settings values, dropdown selections, on/off state, slider percentages, keybind labels, confirm timers, retro-fx values, favorites-driven pacing labels`
- Freeze strategy for strict diffing: `packaged default startup on the Gameplay tab using current saved settings; use the current packaged anchor as the structure-only guide until the approved style-faithful master exists`
- Regions excluded from strict diff: `exact live toggle state, dropdown selected values, slider fill values, keybind text, monitor or graphics values, transient confirm timers`
- Manual validation required for: `tab readability, row spacing consistency, toggle hit areas, dropdown geometry, controls-rebinding row density, any modal confirm overlay state`

## Family Boards Needed

- `TopBar`: `yes`
- `Center`: `yes`
- `LeftPanel`: `no`
- `RightPanel`: `no`
- `LeaderboardChrome`: `no`
- `CTAStack`: `no`
- `Decor`: `yes`

## Runtime Rules

- Localizable controls must remain text-free in runtime art: `yes`
- Live numerals must remain text-free in runtime art: `yes`
- Full reference/buttonless/textless master may be used as runtime background: `no`
- Scene plate must be UI-free: `yes`
- Visible control must be the real button: `yes`
- Any plate expected to stretch must be authored as a true nine-slice: `yes`

## Layout Artifacts

- `reference_layout.json`: `C:/UE/T66/SourceAssets/UI/SettingsReference/reference_layout.json`
- Generated layout header: `C:/UE/T66/Source/T66/UI/Style/T66SettingsReferenceLayout.generated.h`
- Coordinate authority: `reference layout json and generated header after the offline target is approved; until then, the packaged anchor is the structure-only guide`

## Typography Notes

- Primary display font target: `match the approved main-menu Latin display treatment for tabs and major headings`
- Secondary/supporting font target: `match the approved main-menu supporting treatment for rows and values without sacrificing settings density`
- Special casing or locale rules: `all tab labels, row labels, values, and keybind text remain live FText or live runtime value fields`

## Prompt Pack

- Offline target prompt: `C:/UE/T66/Docs/UI/PromptPacks/SettingsSpriteSheets/master_frame_prompt.txt`
- Scene plate prompt: `TODO`
- `topbar_sheet_prompt.txt`: `C:/UE/T66/Docs/UI/PromptPacks/SettingsSpriteSheets/topbar_sheet_prompt.txt`
- `center_sheet_prompt.txt`: `C:/UE/T66/Docs/UI/PromptPacks/SettingsSpriteSheets/center_sheet_prompt.txt`
- `left_panel_sheet_prompt.txt`: `C:/UE/T66/Docs/UI/PromptPacks/SettingsSpriteSheets/left_panel_sheet_prompt.txt`
- `right_panel_sheet_prompt.txt`: `C:/UE/T66/Docs/UI/PromptPacks/SettingsSpriteSheets/right_panel_sheet_prompt.txt`
- `decor_sheet_prompt.txt`: `C:/UE/T66/Docs/UI/PromptPacks/SettingsSpriteSheets/decor_sheet_prompt.txt`

## Validation

- Packaged screenshot command: `powershell -ExecutionPolicy Bypass -File C:/UE/T66/Scripts/CaptureT66UIScreen.ps1 -Screen Settings -Output C:/UE/T66/UI/screens/settings/outputs/2026-04-24/packaged_capture.png`
- Scene plate contamination check: `TODO`
- Foreground component ownership check: `TODO`
- Strict-diff regions: `tab strip silhouette, body-shell geometry, row-card family, dropdown shell, toggle pair shell, top-bar integration`
- Manual validation regions: `live values, keybind rows, confirm overlays, scroll behavior, exact on/off state values`
- Acceptance bar: `blocked until a style-faithful screen master exists`

## Open Questions

- `Should the final approved settings reference keep the inherited frontend top bar, or should the settings shell fully own the top region below the global header?`
- `Do we want a separate approved modal reference for the Graphics keep/revert confirm state, or is the base Gameplay-tab screen enough for the first pass?`
