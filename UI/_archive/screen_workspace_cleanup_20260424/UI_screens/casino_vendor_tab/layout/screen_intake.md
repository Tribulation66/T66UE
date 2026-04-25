# Casino Vendor Tab Screen Intake

## Identity

- Screen token: `CasinoVendorTab`
- Display name: `Casino Vendor Tab`
- Owner: `[Name or team]`
- Target platform: `[PC / controller / other]`

## Canonical Canvas

- Canonical layout master path: `C:/UE/T66/UI/screens/casino_vendor_tab/reference/canonical_reference_1920x1080.png`
- Canonical layout master resolution: `1920x1080`
- Hi-res art master path: `C:/UE/T66/UI/screens/casino_vendor_tab/reference/helper_2x.png`
- Hi-res art master resolution: `3840x2160`
- Runtime target viewport: `1920x1080`

## Blocking Style-Reference Gate

- Canonical main-menu style anchor: `C:/UE/T66/UI/screens/main_menu/reference/canonical_reference_1920x1080.png`
- Current target screenshot path: `C:/UE/T66/UI/screens/casino_vendor_tab/current/YYYY-MM-DD/current.png`
- Layout list path: `C:/UE/T66/UI/screens/casino_vendor_tab/layout/layout_list.md`
- Image generation used all three required inputs: `[yes/no]`
- Generated screen-specific reference path: `C:/UE/T66/UI/screens/casino_vendor_tab/reference/canonical_reference_1920x1080.png`
- Generated reference preserves target layout: `[yes/no]`
- Generated reference matches canonical main-menu style: `[yes/no]`
- Gate verdict: `[blocked / approved for sprite generation]`

## Reference Variants

- Primary comparison frame: `C:/UE/T66/UI/screens/casino_vendor_tab/reference/canonical_reference_1920x1080.png`
- No-buttons variant: `[optional helper only]`
- No-text variant: `[optional helper only]`
- No-dynamic variant: `[optional helper only]`

## Regions

- `[RegionName]` — `static | stateful | live`
- `[RegionName]` — `static | stateful | live`
- `[RegionName]` — `static | stateful | live`

## Dynamic And Live Content

- Live data sources: `[leaderboards, social list, timers, store prices, etc.]`
- Freeze strategy for strict diffing: `[fixture, mock data, staged screenshot state, or mask]`
- Regions excluded from strict diff: `[list]`
- Manual validation required for: `[list]`

## Content Ownership Audit

- Ownership artifact: `C:/UE/T66/UI/screens/casino_vendor_tab/assets/content_ownership.json`
- Ownership audit completed: `[yes/no]`
- `generated-shell` regions: `[list]`
- `runtime-text` regions: `[list]`
- `runtime-image` / `runtime-avatar` regions: `[list]`
- `runtime-icon` / `runtime-media` / `runtime-3d-preview` regions: `[list]`
- Open-aperture regions: `[list]`
- Socket-frame regions: `[list]`
- Empty-backplate regions: `[list]`

## Family Boards Needed

- `TopBar`: `[yes/no]`
- `Center`: `[yes/no]`
- `LeftPanel`: `[yes/no]`
- `RightPanel`: `[yes/no]`
- `Decor`: `[yes/no]`

## Runtime Rules

- Localizable controls must remain text-free in runtime art: `[yes/no]`
- Live numerals must remain text-free in runtime art: `[yes/no]`
- Visible control must be the real button: `[yes/no]`
- Any plate expected to stretch must be authored as a true nine-slice: `[yes/no]`

## Layout Artifacts

- `reference_layout.json`: `C:/UE/T66/UI/screens/casino_vendor_tab/layout/reference_layout.json`
- Generated layout header: `C:/UE/T66/Source/T66/UI/Style/T66CasinoVendorTabReferenceLayout.generated.h`
- Coordinate authority: `[reference layout json / generated header / other]`

## Typography Notes

- Primary display font target: `[font]`
- Secondary/supporting font target: `[font]`
- Special casing or locale rules: `[notes]`

## Prompt Pack

- Master frame prompt: `C:/UE/T66/UI/screens/casino_vendor_tab/prompts/master_frame_prompt.txt`
- `topbar_sheet_prompt.txt`: `C:/UE/T66/UI/screens/casino_vendor_tab/prompts/topbar_sheet_prompt.txt`
- `center_sheet_prompt.txt`: `C:/UE/T66/UI/screens/casino_vendor_tab/prompts/center_sheet_prompt.txt`
- `left_panel_sheet_prompt.txt`: `C:/UE/T66/UI/screens/casino_vendor_tab/prompts/left_panel_sheet_prompt.txt`
- `right_panel_sheet_prompt.txt`: `C:/UE/T66/UI/screens/casino_vendor_tab/prompts/right_panel_sheet_prompt.txt`
- `decor_sheet_prompt.txt`: `C:/UE/T66/UI/screens/casino_vendor_tab/prompts/decor_sheet_prompt.txt`

## Validation

- Packaged screenshot command: `powershell -ExecutionPolicy Bypass -File C:/UE/T66/Scripts/CaptureT66UIScreen.ps1 -Screen CasinoVendorTab -Output C:/UE/T66/UI/screens/casino_vendor_tab/outputs/YYYY-MM-DD/packaged_capture.png`
- Strict-diff regions: `[list]`
- Manual validation regions: `[list]`
- Acceptance bar: `[exact enough / close with blockers / blocked]`

## Open Questions

- `[question]`
- `[question]`
