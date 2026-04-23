# Hero Selection Screen Intake

## Identity

- Screen token: `HeroSelection`
- Display name: `Hero Selection`
- Owner: `UI reconstruction workflow`
- Target platform: `PC, controller-aware`

## Canonical Canvas

- Canonical layout master path: `C:/UE/T66/SourceAssets/UI/HeroSelectionReference/screen_master.png`
- Canonical layout master resolution: `1920x1080`
- Hi-res art master path: `C:/UE/T66/SourceAssets/UI/HeroSelectionReference/screen_master_2x.png`
- Hi-res art master resolution: `3840x2160`
- Runtime target viewport: `1920x1080`
- Current packaged structure anchor: `C:/UE/T66/SourceAssets/UI/HeroSelectionReference/current_runtime_anchor.png`
- Current packaged helper anchor: `C:/UE/T66/SourceAssets/UI/HeroSelectionReference/current_runtime_anchor_2x.png`
- Approved style anchor: `C:/UE/T66/SourceAssets/UI/MainMenuReference/reference_main_menu_master.png`

## Reference Variants

- Primary comparison frame: `C:/UE/T66/SourceAssets/UI/HeroSelectionReference/current_runtime_anchor.png`
- No-buttons variant: `C:/UE/T66/SourceAssets/UI/HeroSelectionReference/screen_master_no_buttons.png`
- No-text variant: `C:/UE/T66/SourceAssets/UI/HeroSelectionReference/screen_master_no_text.png`
- No-dynamic variant: `C:/UE/T66/SourceAssets/UI/HeroSelectionReference/screen_master_no_dynamic.png`

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
- `Decor`: `yes`

## Runtime Rules

- Localizable controls must remain text-free in runtime art: `yes`
- Live numerals must remain text-free in runtime art: `yes`
- Visible control must be the real button: `yes`
- Any plate expected to stretch must be authored as a true nine-slice: `yes`

## Layout Artifacts

- `reference_layout.json`: `C:/UE/T66/SourceAssets/UI/HeroSelectionReference/reference_layout.json`
- Generated layout header: `C:/UE/T66/Source/T66/UI/Style/T66HeroSelectionReferenceLayout.generated.h`
- Coordinate authority: `reference layout json and generated header after the screen master is approved; until then, the packaged anchor is the structure-only guide`

## Typography Notes

- Primary display font target: `match the approved main-menu Latin display treatment`
- Secondary/supporting font target: `match the approved main-menu supporting label treatment`
- Special casing or locale rules: `hero names, companion names, ability names, and difficulty labels must remain live text with room for localization expansion`

## Prompt Pack

- Master frame prompt: `C:/UE/T66/Docs/UI/PromptPacks/HeroSelectionSpriteSheets/master_frame_prompt.txt`
- `topbar_sheet_prompt.txt`: `C:/UE/T66/Docs/UI/PromptPacks/HeroSelectionSpriteSheets/topbar_sheet_prompt.txt`
- `center_sheet_prompt.txt`: `C:/UE/T66/Docs/UI/PromptPacks/HeroSelectionSpriteSheets/center_sheet_prompt.txt`
- `left_panel_sheet_prompt.txt`: `C:/UE/T66/Docs/UI/PromptPacks/HeroSelectionSpriteSheets/left_panel_sheet_prompt.txt`
- `right_panel_sheet_prompt.txt`: `C:/UE/T66/Docs/UI/PromptPacks/HeroSelectionSpriteSheets/right_panel_sheet_prompt.txt`
- `decor_sheet_prompt.txt`: `C:/UE/T66/Docs/UI/PromptPacks/HeroSelectionSpriteSheets/decor_sheet_prompt.txt`

## Validation

- Packaged screenshot command: `"C:/UE/T66/Saved/StagedBuilds/Windows/T66/Binaries/Win64/T66.exe" -T66FrontendScreen=HeroSelection -T66AutoScreenshot="C:/UE/T66/output/hero_selection_workflow_anchor_fixed.png" -T66AutoScreenshotDelay=2`
- Strict-diff regions: `panel silhouettes, top strip geometry, bottom bar geometry, companion CTA shell, right-panel shell breakup`
- Manual validation regions: `3D preview stage, portrait/media content, party box live content, all dynamic text and button labels`
- Acceptance bar: `blocked until a style-faithful screen master exists`

## Open Questions

- `How much of the main-menu top-bar family should carry into Hero Selection versus staying a simpler hero-strip treatment?`
- `Should the current hidden companion carousel remain collapsed in the approved reference, or should the reference present the full belt explicitly?`
