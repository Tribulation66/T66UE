# Mini Idol Select Layout List

Canvas: 1920x1080.

Required image-generation inputs:
- Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
- Current target screenshot: `C:\UE\T66\UI\screens\mini_idol_select\current\2026-04-24\current_1920x1080.png`
- This layout list.

## Image Generation Task

Generate a single 1920x1080 screen-specific canonical reference for this exact screen. Preserve the current target screenshot's layout and control positions, but reinterpret the panels, shells, buttons, equipped slots, offer rows, and background in the same fantasy material language as the canonical main-menu anchor. This reference is an offline comparison target only, not a runtime background.

## Regions

- Full-screen background: dark fantasy menu backdrop; no UI chrome or text baked into background.
- Top title: centered `Idol Selection`; runtime text.
- Equipped idol slots row: four top horizontal slot shells, each with slot index badge, slot title, and short instruction/value text.
- Idol offer list panel: large left panel with vertical scrollable list of offer rows.
- Loadout summary panel: large right panel with metrics, summary copy, and status copy.
- Back button: bottom-left blue navigation button.
- Reroll button: lower center purple primary/secondary button.
- Continue button: bottom-right green primary button.

## Panels And Shells

- Equipped slot shell: generated slot frame/backplate with icon/socket area or index badge area; text live.
- Slot index badge: generated small badge; numeric value can remain runtime text unless explicitly approved as nonlocalizable slot art.
- Offer list shell: generated scroll-panel frame/backplate.
- Offer row/card shell: generated row plate with icon socket, title/value text wells, and Take button area.
- Idol icon socket/frame: generated frame; runtime idol icon inside.
- Take plate inside each offer row: generated small visible plate with live label; the offer row itself is the current real button/control surface.
- Summary shell: generated panel frame/backplate.
- Summary metric well family: generated label/value backplates.
- Back/Reroll/Continue button plates: generated button art with live labels.
- Scrollbar track/thumb for offer list: generated or styled with runtime behavior.

## Controls And Required States

- Back button: states `normal`, `hover`, `pressed`, `disabled`.
- Reroll button: states `normal`, `hover`, `pressed`, `disabled`.
- Continue/Ready/Start Party Run button: states `normal`, `hover`, `pressed`; disabled state is optional/future-safe because current source keeps Continue enabled and reports missing-selection status in text.
- Take offer plate within offer row: visual states follow the owning offer row.
- Offer row button: states `normal`, `hover`, `pressed`, `selected/unavailable`.
- Equipped slot shell if interactive later: states `normal`, `hover`, `selected`, `disabled`.
- Offer scrollbar: states `normal`, `hover`, `dragged`, `disabled`.

## Live Runtime Content

- Page title and button labels: runtime/localizable.
- Equipped slot titles and instruction/copy: runtime/localizable/state.
- Slot indexes: runtime value unless later approved as static badge art.
- Idol icons: runtime-icon/image.
- Idol offer names, categories, base damage, property values: runtime text/value.
- Summary labels and values: runtime text/value.
- Summary/status copy: runtime/localizable.
- Online party ready/start labels: runtime-state/text.

## Variants

- Empty equipped slots.
- Equipped idol slots with icon and idol metadata.
- Offer list with different idol offers.
- Reroll available with changing reroll count/status.
- Continue available with missing-selection status copy when no idol is selected.
- Online host: `START PARTY RUN`.
- Online guest: `READY` / `UNREADY`.
- Intermission flow and fresh-run flow status copy.

## Reference Generation Constraints

- Preserve the current layout: four top slot cards, offer list left, loadout summary right, Back/Reroll/Continue along the bottom.
- Match the canonical main-menu style with fantasy frames and dark readable panels.
- Do not bake idol names, categories, stats, slot text, summary values, status copy, or button labels into runtime assets.
- Leave idol icon areas as sockets/open wells.
- Avoid a generic shop layout; this must read as a pre-run idol loadout screen in the same family as the main menu.
