# HeroSelection V2 Manifest

## Target

- Base screen/modal: HeroSelection
- State: default screen/modal state
- Reference: `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\HeroSelection.png`
- Runtime asset folder: `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection`

## Pre-V2 Reset

- Archived active target-owned generated art to `C:\UE\T66\UI\Reference\Screens\HeroSelection\Archive\PreV2_Reset_20260505_095930\`
- Cleared active target-owned folders: `Buttons`, `Panels`, `Controls`, `Slots`, `Icons`

## Geometry Map - Reference 1920x1080

- Left upper owned panel: x 0, y 22, w 608, h 738
- Center preview area: x 610, y 25, w 781, h 734
- Right upper owned panel: x 1400, y 23, w 504, h 737
- Left footer party panel: x 0, y 762, w 738, h 153
- Center footer companion panel: x 742, y 762, w 499, h 153
- Right footer run panel: x 1248, y 763, w 672, h 152
- Top hero carousel: x 626, y 29, w 754, h 92

## Geometry Map - Pass 00 Current Packaged 1920x1080

- Left upper owned panel: x 27, y 27, w 586, h 862
- Center preview area: x 614, y 28, w 776, h 835
- Right upper owned panel: x 1399, y 27, w 493, h 862
- Left footer party panel: x 34, y 925, w 572, h 116
- Center footer companion panel: x 828, y 957, w 348, h 98
- Right footer run panel: x 1390, y 927, w 501, h 116
- Top hero carousel: x 623, y 27, w 756, h 85

## Difference List Before Editing

- layout: Upper side panels were about 124 px taller than the reference and pushed the footer row too low.
- layout: Bottom controls were compressed inward and did not span the same reference bands.
- asset: Active HeroSelection panels/buttons/slots were ornate and denser than the reference.
- asset/staging: New generated assets were not visible until copied/staged into the packaged tree.
- live-data: Hero name, hero model, hero stats, currency values, and party metadata differ from the reference.
- top-bar-shared: Global top/header button chrome and currency/header elements are shared and were not edited.

## Pass 01

- Generated reference-derived sheet: `C:\UE\T66\UI\Reference\Screens\HeroSelection\Working\Pass_01\Candidates\HeroSelection_reference_derived_component_sheet_pass01.png`
- Rejected to: `C:\UE\T66\UI\Reference\Screens\HeroSelection\Archive\Rejected\Pass_01\HeroSelection_reference_derived_component_sheet_pass01.png`
- Rejection reasons: too ornate/bright, green chroma-key edge artifacts in packaged capture, mismatched component proportions.
- Proof: `C:\UE\T66\UI\Reference\Screens\HeroSelection\Proof\HeroSelection_pass01_packaged_stagedassets_1920x1080.png`

## Pass 02

- Generated reference-derived sheet: `C:\UE\T66\UI\Reference\Screens\HeroSelection\Working\Pass_02\Candidates\HeroSelection_reference_derived_component_sheet_pass02.png`
- Accepted sliced runtime folders:
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Buttons\`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Panels\`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Controls\`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Slots\`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Icons\`
- Resize contracts:
  - side and content panels: 9-slice `FSlateBrush` box rendering
  - parchment rows/dropdowns/scrollbars: 9-slice or vertical/horizontal sliced box rendering
  - non-square buttons: existing sliced button path with nearest filtering and live text
  - square buttons, slots, and icons: fixed image rendering
- Source changed: `C:\UE\T66\Source\T66\UI\Screens\HeroSelection\T66HeroSelectionScreen_Build.cpp`
- Proof: `C:\UE\T66\UI\Reference\Screens\HeroSelection\Proof\HeroSelection_pass02_packaged_stagedassets_1920x1080.png`

## Remaining Differences After Pass 02

- layout: packaged proof after the layout source patch could not be captured because full standalone staging is blocked by an active AutomationTool/cook mutex from another live `C:\UE\T66` package operation.
- asset: Pass 02 generated art is closer and cleaner than Pass 01, but still needs final staged screenshot review after the source patch is cooked.
- live-data: hero name/model/stats/currency/party values remain live-data differences.
- top-bar-shared: global top/header chrome remains out of scope.

## Staging Blocker

- Command attempted: `powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\StageStandaloneBuild.ps1`
- First failure: `A conflicting instance of AutomationTool is already running.`
- Retry after 35 seconds: same AutomationTool singleton error.
- Process evidence after retry: active `cmd.exe` RunUAT, `dotnet.exe AutomationTool.dll BuildCookRun`, and later `UnrealEditor-Cmd.exe` command lines for `C:\UE\T66`.
- Recovery decision: not terminated because the process was active and had progressed into cook/editor commandlet work, not a stale unattended singleton.
