# Mini Companion Select Layout List

Canvas: 1920x1080.

Required image-generation inputs:
- Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
- Current target screenshot: `C:\UE\T66\UI\screens\mini_companion_select\current\2026-04-24\current_1920x1080.png`
- This layout list.

## Image Generation Task

Generate a single 1920x1080 screen-specific canonical reference for this exact screen. Preserve the current target screenshot's layout and control positions, but reinterpret the panels, shells, buttons, scroll roster, card grid, and background in the same fantasy material language as the canonical main-menu anchor. This reference is an offline comparison target only, not a runtime background.

## Regions

- Full-screen background: dark fantasy menu backdrop; no UI chrome or text baked into background.
- Top title: centered `Companion Selection`; runtime text.
- Companion detail panel: upper-left wide panel with selected companion name, pairing line, portrait well, description, and metric wells.
- Run pairing panel: upper-right panel with selected hero, selected companion, roster count, and stages cleared.
- Companion roster panel: lower large scrollable grid with companion cards.
- Bottom footer/control band: bottom area reserved for Back and Continue buttons and roster scrolling clearance.
- Back button: bottom-left blue navigation button.
- Continue button: bottom-right green primary button.

## Panels And Shells

- Companion detail shell: generated panel shell with dark interior.
- Companion portrait frame: generated socket-frame; runtime companion image inside.
- Metric chip/well family: generated label/value backplates; text and values live.
- Run pairing shell: generated panel shell with stacked metric wells.
- Companion roster shell: generated scroll-panel frame/backplate.
- Companion card shell: generated selectable card plate with portrait socket, name label well, and status label well.
- Scrollbar track/thumb: generated or styled component with runtime scroll behavior.
- Button plate family: Back and Continue as real buttons with generated plates and live labels.

## Controls And Required States

- Back button: states `normal`, `hover`, `pressed`, `disabled`.
- Continue button: states `normal`, `hover`, `pressed`, `disabled`.
- Companion card button: states `normal`, `hover`, `pressed`, `selected`, `disabled/locked`.
- Roster scrollbar: states `normal`, `hover`, `dragged`, `disabled`.

## Live Runtime Content

- Page title and button labels: runtime/localizable.
- Companion name, pairing line, description: runtime/localizable.
- Companion portrait/icon: runtime-image.
- Metric labels and values: runtime text/value.
- Hero and companion pairing values: runtime text/value.
- Roster card portraits, names, unlock/status labels: runtime image/text/state.
- Scroll position/content: runtime.

## Variants

- Default unlocked companion selected.
- Locked companion card.
- Selected companion card.
- Hovered companion card.
- Ready/locked/stage requirement status.
- Continue disabled/no-selection state; no alternate online readiness label is currently source-visible on this screen.

## Reference Generation Constraints

- Preserve the current top two-panel layout and bottom roster grid with a visible scrollbar.
- Use main-menu fantasy material language: chunkier stone/bronze frames, dark wells, controlled gold/green/purple accents.
- Do not bake companion names, hero names, values, labels, status text, or button text into runtime-intended assets.
- Keep portrait wells open/neutral for runtime images.
- Reserve enough bottom clearance so controls do not cover roster content.
