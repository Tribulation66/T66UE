# UI Screen Intake Template

Use this template before generating or reconstructing a new screen. Fill it in before runtime layout work starts.

## Identity

- Screen token: `[PascalCaseToken]`
- Display name: `[Human-readable screen name]`
- Owner: `[Name or team]`
- Target platform: `[PC / controller / other]`

## Canonical Canvas

- Canonical layout master path: `[path]`
- Canonical layout master resolution: `[width]x[height]`
- Hi-res art master path: `[path]`
- Hi-res art master resolution: `[width]x[height]`
- Runtime target viewport: `[width]x[height]`

## Reference Variants

- Primary comparison frame: `[path]`
- No-buttons variant: `[path or TODO]`
- No-text variant: `[path or TODO]`
- No-dynamic variant: `[path or TODO]`

## Regions

List each major region and classify it:

- `[RegionName]` — `static | stateful | live`
- `[RegionName]` — `static | stateful | live`
- `[RegionName]` — `static | stateful | live`

## Dynamic And Live Content

- Live data sources: `[leaderboards, social list, timers, store prices, etc.]`
- Freeze strategy for strict diffing: `[fixture, mock data, staged screenshot state, or mask]`
- Regions excluded from strict diff: `[list]`
- Manual validation required for: `[list]`

## Family Boards Needed

- `TopBar`: `[yes/no]`
- `Center`: `[yes/no]`
- `LeftPanel`: `[yes/no]`
- `RightPanel`: `[yes/no]`
- `Decor`: `[yes/no]`

Add additional families if the screen requires them.

## Runtime Rules

- Localizable controls must remain text-free in runtime art: `[yes/no]`
- Live numerals must remain text-free in runtime art: `[yes/no]`
- Visible control must be the real button: `[yes/no]`
- Any plate expected to stretch must be authored as a true nine-slice: `[yes/no]`

## Layout Artifacts

- `reference_layout.json`: `[path]`
- Generated layout header: `[path]`
- Coordinate authority: `[reference layout json / generated header / other]`

## Typography Notes

- Primary display font target: `[font]`
- Secondary/supporting font target: `[font]`
- Special casing or locale rules: `[notes]`

## Prompt Pack

- Master frame prompt: `[path]`
- Family board prompts: `[paths]`
- Source images attached to family prompts: `[list]`

## Validation

- Packaged screenshot command: `[command]`
- Strict-diff regions: `[list]`
- Manual validation regions: `[list]`
- Acceptance bar: `[exact enough / close with blockers / blocked]`

## Open Questions

- `[question]`
- `[question]`

