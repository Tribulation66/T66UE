# Mini Character Select Layout List

Canvas: 1920x1080.

Required image-generation inputs:
- Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
- Current target screenshot: `C:\UE\T66\UI\screens\mini_character_select\current\2026-04-24\current_1920x1080.png`
- This layout list.

## Image Generation Task

Generate a single 1920x1080 screen-specific canonical reference for this exact screen. Preserve the current target screenshot's layout and control positions, but reinterpret the panels, shells, buttons, card grid, and background in the same fantasy material language as the canonical main-menu anchor. This reference is an offline comparison target only, not a runtime background.

## Regions

- Full-screen background: dark fantasy menu backdrop; no UI chrome or text baked into the background plate.
- Top title: centered `Character Selection`; runtime text.
- Back button: top-left blue navigation button.
- Hero detail panel: wide top-left panel containing hero name, archetype, portrait well, description, and stat wells.
- Records panel: top-right panel containing best wave, fastest clear, completed runs, failed runs, and unlocked hero count.
- Hero roster panel: large bottom panel containing a two-row, eight-column grid of hero cards.
- Continue button: bottom-right green primary button.

## Panels And Shells

- Hero detail shell: generated frame/backplate with open portrait well and neutral stat wells; no baked live text.
- Hero portrait frame: socket-frame around runtime hero sprite/portrait.
- Stat well family: generated small backplates for label/value pairs; text and values are live.
- Records shell: generated frame/backplate with text/value wells.
- Hero roster shell: generated frame/backplate for scrollable or fixed grid.
- Hero card shell: generated selectable card plate with portrait socket and label area; no baked hero names.
- Selected hero card state: highlighted generated shell/state, label still live.
- Back and Continue button plates: generated button art, real buttons, no baked labels.

## Controls And Required States

- Back button: states `normal`, `hover`, `pressed`, `disabled`.
- Continue button: states `normal`, `hover`, `pressed`, `disabled`.
- Hero card button: states `normal`, `hover`, `pressed`, `selected`; optional `disabled/locked` only as a future-safe visual state, not required by current source behavior.
- Roster scroll area if content exceeds visible rows: scrollbar states `normal`, `hover`, `dragged`, `disabled`.

## Live Runtime Content

- Page title and button labels: runtime/localizable.
- Hero name, archetype, and description: runtime/localizable.
- Hero portrait/sprite: runtime-image.
- Stat labels and values: runtime text/value.
- Records labels and values: runtime text/value.
- Hero roster portraits and names: runtime-image and runtime text.
- Unlock/selected state labels if added later: runtime text/state.

## Variants

- Default selected hero.
- Hovered hero card.
- Selected hero card.
- Optional locked/disabled hero card if future progression gates apply.
- Empty/fallback sprite when an image cannot load.
- Records with no clears versus existing profile records.

## Reference Generation Constraints

- Preserve the current layout: title at top, hero-detail panel and records panel across the upper half, roster grid across the bottom, Back top-left, Continue bottom-right.
- Match the canonical main-menu style with heavy fantasy frames and dark readable interiors.
- Do not bake hero names, records, stats, labels, or button text into runtime assets.
- Leave portrait/card interiors open for runtime character sprites.
- The reference should improve the visual quality but keep the current information hierarchy and approximate region sizes.
