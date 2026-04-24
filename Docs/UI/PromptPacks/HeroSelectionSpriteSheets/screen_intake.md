# Hero Selection Screen Intake

## Identity

- Screen token: `HeroSelection`
- Display name: `Hero Selection`
- Owner: `UI reconstruction workflow`
- Target platform: `PC, controller-aware`

## Canonical Canvas

- Offline comparison target path: `C:/UE/T66/SourceAssets/UI/HeroSelectionReference/screen_master.png`
- Offline comparison target resolution: `1920x1080`
- Hi-res helper/reference path: `C:/UE/T66/SourceAssets/UI/HeroSelectionReference/screen_master_2x.png`
- Hi-res helper/reference resolution: `3840x2160`
- UI-free scene/background plate path: `TODO`
- Runtime target viewport: `1920x1080`
- Current packaged structure anchor: `C:/UE/T66/SourceAssets/UI/HeroSelectionReference/current_runtime_anchor.png`
- Current packaged helper anchor: `C:/UE/T66/SourceAssets/UI/HeroSelectionReference/current_runtime_anchor_2x.png`
- Approved style anchor: `C:/UE/T66/UI/screens/main_menu/reference/canonical_reference_1920x1080.png`

## Reference Variants

- Primary comparison frame: `C:/UE/T66/SourceAssets/UI/HeroSelectionReference/current_runtime_anchor.png`
- No-buttons variant for analysis/prompting only: `C:/UE/T66/SourceAssets/UI/HeroSelectionReference/screen_master_no_buttons.png`
- No-text variant for analysis/prompting only: `C:/UE/T66/SourceAssets/UI/HeroSelectionReference/screen_master_no_text.png`
- No-dynamic variant for analysis/prompting only: `C:/UE/T66/SourceAssets/UI/HeroSelectionReference/screen_master_no_dynamic.png`

## Regions

- `Top strip: back, currency badge, hero carousel` — `stateful | live`
- `Left panel: skins list / full stats switcher` — `live`
- `Center stage: preview model, companion CTA, body type toggles` — `live`
- `Right panel: title, preview/info panel, records, ult/passive or companion info` — `live`
- `Bottom row: party box and run controls` — `live`

## Dynamic And Live Content

- Live data sources: `selected hero, selected companion, 3D preview stage, preview media, skin ownership and prices, AC balance, temporary buff slots, party state, ready state, difficulty state`
- Freeze strategy for strict diffing: `single local-player packaged capture with default startup selections; use the current packaged anchor as the structural contract until a style-faithful screen master exists`
- Regions excluded from strict diff: `3D preview model, preview media frames, party portraits and party state, dynamic difficulty or ready text, any player-owned skin/economy values`
- Manual validation required for: `hero carousel readability, preview framing, localized hero and companion text zones, bottom run-controls spacing, party box live behavior`

## Family Boards Needed

- `TopBar`: `yes`
- `Center`: `yes`
- `LeftPanel`: `yes`
- `RightPanel`: `yes`
- `LeaderboardChrome`: `no`
- `CTAStack`: `yes`
- `Decor`: `yes`

## Runtime Rules

- Localizable controls must remain text-free in runtime art: `yes`
- Live numerals must remain text-free in runtime art: `yes`
- Full reference/buttonless/textless master may be used as runtime background: `no`
- Scene plate must be UI-free: `yes`
- Visible control must be the real button: `yes`
- Any plate expected to stretch must be authored as a true nine-slice: `yes`

## Layout Artifacts

- `reference_layout.json`: `C:/UE/T66/SourceAssets/UI/HeroSelectionReference/reference_layout.json`
- Generated layout header: `C:/UE/T66/Source/T66/UI/Style/T66HeroSelectionReferenceLayout.generated.h`
- Coordinate authority: `reference layout json and generated header after the offline target is approved; until then, the packaged anchor is the structure-only guide`

## Typography Notes

- Primary display font target: `match the approved main-menu Latin display treatment`
- Secondary/supporting font target: `match the approved main-menu supporting label treatment`
- Special casing or locale rules: `hero names, companion names, ability names, and difficulty labels must remain live text with room for localization expansion`

## Prompt Pack

- Offline target prompt: `C:/UE/T66/Docs/UI/PromptPacks/HeroSelectionSpriteSheets/master_frame_prompt.txt`
- Scene plate prompt: `TODO`
- `topbar_sheet_prompt.txt`: `C:/UE/T66/Docs/UI/PromptPacks/HeroSelectionSpriteSheets/topbar_sheet_prompt.txt`
- `center_sheet_prompt.txt`: `C:/UE/T66/Docs/UI/PromptPacks/HeroSelectionSpriteSheets/center_sheet_prompt.txt`
- `left_panel_sheet_prompt.txt`: `C:/UE/T66/Docs/UI/PromptPacks/HeroSelectionSpriteSheets/left_panel_sheet_prompt.txt`
- `right_panel_sheet_prompt.txt`: `C:/UE/T66/Docs/UI/PromptPacks/HeroSelectionSpriteSheets/right_panel_sheet_prompt.txt`
- `decor_sheet_prompt.txt`: `C:/UE/T66/Docs/UI/PromptPacks/HeroSelectionSpriteSheets/decor_sheet_prompt.txt`

## Validation

- Packaged screenshot command: `powershell -ExecutionPolicy Bypass -File C:/UE/T66/Scripts/CaptureT66UIScreen.ps1 -Screen HeroSelection -Output C:/UE/T66/UI/screens/hero_selection/outputs/2026-04-24/packaged_capture.png`
- Scene plate contamination check: `TODO`
- Foreground component ownership check: `TODO`
- Strict-diff regions: `panel silhouettes, top strip geometry, bottom bar geometry, companion CTA shell, right-panel shell breakup`
- Manual validation regions: `3D preview stage, portrait/media content, party box live content, all dynamic text and button labels`
- Acceptance bar: `blocked until a style-faithful screen master exists`

## Open Questions

- `How much of the main-menu top-bar family should carry into Hero Selection versus staying a simpler hero-strip treatment?`
- `Should the current hidden companion carousel remain collapsed in the approved reference, or should the reference present the full belt explicitly?`
