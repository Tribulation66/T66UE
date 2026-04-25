# Mini Run Summary Layout List

Canvas: 1920x1080.

Required image-generation inputs:
- Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
- Current target screenshot: `C:\UE\T66\UI\screens\mini_run_summary\current\2026-04-24\current_1920x1080.png`
- This layout list.

## Image Generation Task

Generate a single 1920x1080 screen-specific canonical reference for this exact screen. Preserve the current target screenshot's layout and control positions, but reinterpret the panels, result ornament, buttons, and background in the same fantasy material language as the canonical main-menu anchor. This reference is an offline comparison target only, not a runtime background.

## Regions

- Full-screen background: dark fantasy menu backdrop; no UI chrome or text baked into background.
- Result title: large centered top result text, `RUN ENDED` or `RUN CLEARED`; runtime text/state.
- Run details panel: upper-left summary panel with hero/difficulty/wave/materials/time values.
- Loadout snapshot panel: upper-right summary panel with idols/items and replay guidance.
- Bottom action bar: three large buttons across the bottom.
- Play Again button: bottom-left green primary button.
- Mini Menu button: bottom-center blue neutral button.
- Full Main Menu button: bottom-right blue neutral button.

## Panels And Shells

- Run details shell: generated panel frame/backplate; no baked result values.
- Loadout snapshot shell: generated panel frame/backplate; no baked labels or values.
- Bottom button plates: generated button family plates; real buttons with live labels.
- Optional result-title ornament/frame: may be generated display chrome, but result words stay live/localizable.

## Controls And Required States

- Play Again button: states `normal`, `hover`, `pressed`, `disabled`.
- Mini Menu button: states `normal`, `hover`, `pressed`, `disabled`.
- Full Main Menu button: states `normal`, `hover`, `pressed`, `disabled`.

## Live Runtime Content

- Result title and result label: runtime/localizable/state.
- Hero display name: runtime text.
- Difficulty display name: runtime text/value.
- Wave reached, materials banked, run time: runtime values.
- Idols equipped and items owned counts: runtime values.
- Replay guidance copy: runtime/localizable.
- Button labels: runtime/localizable.

## Variants

- Defeat state: `RUN ENDED`, red/danger accent.
- Victory state: `RUN CLEARED`, green/success accent.
- Missing summary/fallback state.
- Play Again available in current source behavior.

## Reference Generation Constraints

- Preserve the sparse current layout: result title top-center, two summary panels in the upper half, three action buttons along the bottom.
- Match the canonical main-menu visual language with stronger fantasy frames and readable dark panel interiors.
- Do not bake labels, values, result words, guidance copy, or button labels into runtime assets.
- Leave enough panel interior padding for localized strings and longer difficulty names.
- Do not add leaderboard, portrait, or shop content; this screen is a compact post-run summary.
