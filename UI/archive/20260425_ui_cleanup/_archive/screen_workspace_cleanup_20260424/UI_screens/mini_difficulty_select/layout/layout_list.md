# Mini Difficulty Select Layout List

Canvas: 1920x1080.

Required image-generation inputs:
- Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
- Current target screenshot: `C:\UE\T66\UI\screens\mini_difficulty_select\current\2026-04-24\current_1920x1080.png`
- This layout list.

## Image Generation Task

Generate a single 1920x1080 screen-specific canonical reference for this exact screen. Preserve the current target screenshot's layout and control positions, but reinterpret the panels, shells, buttons, difficulty cards, and background in the same fantasy material language as the canonical main-menu anchor. This reference is an offline comparison target only, not a runtime background.

## Regions

- Full-screen background: dark fantasy menu backdrop; no UI chrome or text baked into background.
- Top title: centered `Difficulty Selection`; runtime text.
- Hero summary panel: upper-left panel with selected hero name, archetype/pairing line, portrait well, description, and hero stat wells.
- Difficulty detail panel: upper-right panel with selected difficulty title, description, ladder modifier wells, and next-screen hint.
- Difficulty option strip: wide lower-middle panel containing five difficulty cards in one row.
- Back button: bottom-left blue navigation button.
- Continue button: bottom-right green primary button.

## Panels And Shells

- Hero summary shell: generated panel frame/backplate with open portrait socket and stat wells.
- Hero portrait frame: generated socket-frame; runtime hero sprite inside.
- Difficulty detail shell: generated panel frame/backplate with modifier wells.
- Modifier/stat well family: generated small backplates; labels and values live.
- Difficulty strip shell: generated frame/backplate holding option cards.
- Difficulty option card: generated selectable card plate with title and tier text wells.
- Selected difficulty card: selected/highlighted state.
- Button plate family: Back and Continue as real buttons with live labels.

## Controls And Required States

- Back button: states `normal`, `hover`, `pressed`, `disabled`.
- Continue button: states `normal`, `hover`, `pressed`, `disabled`.
- Difficulty card button: states `normal`, `hover`, `pressed`, `selected`, `disabled/host-locked`.

## Live Runtime Content

- Page title and button labels: runtime/localizable.
- Hero name, archetype/pairing line, description: runtime/localizable.
- Hero portrait/sprite: runtime-image.
- Hero stat labels and values: runtime text/value.
- Difficulty name, description, modifier labels/values: runtime text/value.
- Difficulty card names and tier values: runtime text/value.
- Host-lock/readiness messaging only if source-visible later; current source disables difficulty cards for online guests without a dedicated message.

## Variants

- Five difficulty cards: Easy, Medium, Hard, Very Hard, Impossible.
- Selected card state.
- Hovered card state.
- Host-locked disabled state for online guests.
- Different difficulty accent colors while preserving the main-menu style family.

## Reference Generation Constraints

- Preserve the current composition: two upper panels, one lower option strip with five cards, Back and Continue at the bottom corners.
- Match the canonical main-menu style, not flat placeholder panels.
- Do not bake difficulty names, tier numbers, values, descriptions, or button labels into runtime sprites.
- Keep option-card proportions stable and readable for the longest label, `Impossible`.
- Do not add extra panels or change the screen flow.
