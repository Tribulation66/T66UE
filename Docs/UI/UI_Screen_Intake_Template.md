# UI Screen Intake Template

Use this template before generating or reconstructing a new screen. Fill it in before runtime layout work starts.

## Identity

- Screen token: `[PascalCaseToken]`
- Display name: `[Human-readable screen name]`
- Owner: `[Name or team]`
- Target platform: `[PC / controller / other]`

## Canonical Canvas

- Offline comparison target path: `[path]`
- Authoring baseline resolution: `[width]x[height]`, normally `1920x1080` for 16:9
- Offline comparison target resolution: `[width]x[height]`
- Hi-res helper/reference path: `[path]`
- Hi-res helper/reference resolution: `[width]x[height]`
- UI-free scene/background plate path: `[path or TODO]`
- UI-free scene/background plate resolution: `[width]x[height]`
- Runtime target viewport(s): `[width]x[height list]`
- Packaged baseline capture resolution: `[width]x[height]`, normally `1920x1080` for 16:9
- Responsive validation matrix: `[16:9 / 16:10 / 21:9 / smaller-windowed]`
- Calibration anchor: `[main menu / other approved golden screen]`

## Blocking Style-Reference Gate

Do not begin sprite generation or runtime placement until this section is complete.

Use Codex-native `image_gen` only. Do not use legacy browser-automation image generation or request manifests.

- Canonical main-menu style anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- Current target runtime screenshot path: `[path in C:\UE\T66\UI\screens\<screen_slug>\current\YYYY-MM-DD\]`
- Layout list path: `[path in C:\UE\T66\UI\screens\<screen_slug>\layout\layout_list.md]`
- Image generation used all three required inputs: `[yes/no]`
- Raw imagegen source path: `[path or n/a]`
- Normalized with `InvokeDeterministicResample.py --target-width 1920 --target-height 1080`: `[yes/no/n/a]`
- Normalization accepted after visual inspection: `[yes/no/n/a]`
- Generated screen-specific reference path: `[C:\UE\T66\UI\screens\<screen_slug>\reference\<screen_slug>_reference_1920x1080.png]`
- Generated reference preserves target layout: `[yes/no]`
- Generated reference matches canonical main-menu style: `[yes/no]`
- Current screenshot used as layout/content authority: `[yes/no]`
- Main-menu reference used for style only: `[yes/no]`
- No invented slots/buttons/panels/icons/meters/currencies/menu entries/tabs/labels: `[yes/no]`
- Negative style guardrails applied: no grain, cracked stone, gemstone/crystal/beveled fantasy surfaces, noisy distressed panels, rubble texture, or micro-detail borders: `[yes/no]`
- Positive style direction applied: sleek, modern, minimalist, clean planar surfaces, crisp borders, flat/satin metallic accents, restrained gold, clean red/black/charcoal scheme, same font/layout/content: `[yes/no]`
- Target is not deprecated or stale: `[yes/no]`
- Gate verdict: `[blocked / approved for sprite generation]`

This gate does not complete the screen. It only allows the work to continue into the element checklist, sprite/component generation, runtime implementation, packaged capture, and review.

## Reference Variants

- Primary comparison frame: `[path]`
- No-buttons variant for analysis/prompting only: `[path or TODO]`
- No-text variant for analysis/prompting only: `[path or TODO]`
- No-dynamic variant for analysis/prompting only: `[path or TODO]`

## Regions

List each major region and classify it:

- `[RegionName]` — `static | stateful | live`
- `[RegionName]` — `static | stateful | live`
- `[RegionName]` — `static | stateful | live`

## Element Checklist

Fill this before image generation starts. Do not begin Unreal placement until every required element and state below is generated, validated, and sliceable.

- `[ElementName]` — family: `[ScenePlate/TopBar/Center/LeftPanel/RightPanel/LeaderboardChrome/CTAStack/Decor/Other]` — ownership: `[static/stateful/live aperture]` — states required: `[normal, hover, pressed, disabled, selected, or n/a]` — status: `[TODO/generated/validated]`
- `[ElementName]` — family: `[ScenePlate/TopBar/Center/LeftPanel/RightPanel/LeaderboardChrome/CTAStack/Decor/Other]` — ownership: `[static/stateful/live aperture]` — states required: `[normal, hover, pressed, disabled, selected, or n/a]` — status: `[TODO/generated/validated]`
- `[ElementName]` — family: `[ScenePlate/TopBar/Center/LeftPanel/RightPanel/LeaderboardChrome/CTAStack/Decor/Other]` — ownership: `[static/stateful/live aperture]` — states required: `[normal, hover, pressed, disabled, selected, or n/a]` — status: `[TODO/generated/validated]`

Asset gate: keep generating or regenerating assets until the checklist is complete. Generated assets must come correct from image generation; do not manually pixel-clean, mask, erase/fill, cover-patch, clone, repaint, or screenshot-repair bad pixels. Deterministic slicing/cropping from an accepted board is allowed.

## Dynamic And Live Content

- Live text sources: `[labels, taglines, names, keybinds, tabs, tooltips, etc.]`
- Live value sources: `[scores, ranks, timers, store prices, percentages, settings values, etc.]`
- Live image/media sources: `[avatars, icons, thumbnails, videos, 3D previews, async brushes, etc.]`
- Freeze strategy for strict diffing: `[fixture, mock data, staged screenshot state, or mask]`
- Regions excluded from strict diff: `[list]`
- Manual validation required for: `[list]`

## Family Boards Needed

- `TopBar`: `[yes/no]`
- `Center`: `[yes/no]`
- `LeftPanel`: `[yes/no]`
- `RightPanel`: `[yes/no]`
- `LeaderboardChrome`: `[yes/no]`
- `CTAStack`: `[yes/no]`
- `Decor`: `[yes/no]`

Add additional families if the screen requires them.

## Runtime Rules

- Localizable controls must remain text-free in runtime art: `[yes/no]`
- Live numerals must remain text-free in runtime art: `[yes/no]`
- Full reference/buttonless/textless master may be used as runtime background: `no`
- Scene plate must be UI-free: `[yes/no]`
- Visible control must be the real button: `[yes/no]`
- Any plate expected to stretch must be authored as a true nine-slice: `[yes/no]`
- Baked display art allowed only for: `[title art / logo / none / list]`
- Tagline/subtitle remains live/localizable: `[yes/no]`

## Layout Artifacts

- `reference_layout.json`: `[path]`
- Generated layout header: `[path]`
- Coordinate authority: `[reference layout json / generated header / other]`
- Shell rect list: `[path or names]`
- Visible control rect list: `[path or names]`
- Live-content rect list: `[path or names]`

## Asset Validation

- Asset manifest: `[path]`
- Normalization metadata: `[raw size / crop rect / target size / resampling method / n/a]`
- Scene plate ownership validation: `[passed/TODO]`
- Dimension validation: `[passed/TODO]`
- Alpha validation: `[passed/TODO]`
- Nine-slice margin validation: `[passed/TODO/not applicable]`
- State anchor validation: `[passed/TODO/not applicable]`

## Typography Notes

- Primary display font target: `[font]`
- Secondary/supporting font target: `[font]`
- Special casing or locale rules: `[notes]`

## Prompt Pack

- Offline target prompt: `[path or n/a]`
- Scene plate prompt: `[path]`
- Family board prompts: `[paths]`
- Source images attached to family prompts: `[list]`

## Validation

- Packaged screenshot command: `powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Screen [ScreenKey] -Output C:\UE\T66\UI\screens\<screen_slug>\outputs\YYYY-MM-DD\packaged_capture.png`
- Baseline packaged capture path: `[path]`
- Aspect validation capture paths or notes: `[16:9 / 16:10 / 21:9 / smaller-windowed]`
- Scene plate contamination check: `[passed/TODO]`
- Foreground component ownership check: `[passed/TODO]`
- Strict-diff regions: `[list]`
- Diff mask paths or notes: `[list]`
- Manual validation regions: `[list]`
- Acceptance bar: `[exact enough / close with blockers / blocked]`

## Open Questions

- `[question]`
- `[question]`
