# Mini Save Slots Layout List

Canvas: 1920x1080.

Required image-generation inputs:
- Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
- Current target screenshot: `C:\UE\T66\UI\screens\mini_save_slots\current\2026-04-24\current_1920x1080.png`
- This layout list.

## Image Generation Task

Generate a single 1920x1080 screen-specific canonical reference for this exact screen. Preserve the current target screenshot's layout and control positions, but reinterpret the panels, shells, buttons, and background in the same fantasy material language as the canonical main-menu anchor. This reference is an offline comparison target only, not a runtime background.

## Regions

- Full-screen background: dark fantasy menu backdrop; may be a generated scene/background plate, but must not include panels, buttons, text, save-slot content, or status text.
- Page title: centered top title region reading `Mini Save Slots`; runtime text, not baked into sprites.
- Save slot grid: six save-slot card shells arranged in two columns and three rows; static generated-shell frame with live text interior.
- Save slot card 1: upper-left card shell with title, save status/value lines, description/metadata, optional load control.
- Save slot card 2: upper-right card shell with the same structure.
- Save slot card 3: middle-left card shell with the same structure.
- Save slot card 4: middle-right card shell with the same structure.
- Save slot card 5: lower-left card shell with the same structure.
- Save slot card 6: lower-right card shell with the same structure.
- Footer status strip: bottom horizontal text backplate, positioned to the right of the Back button; shell may be generated, text stays runtime-owned.
- Back button area: bottom-left visible blue navigation button.

## Panels And Shells

- Save slot card shell: generated fantasy card frame, dark interior, no baked text; stretch strategy should be nine-slice or exact-size.
- Empty/occupied slot interior: neutral runtime text well inside each card.
- Footer status shell: generated long strip/backplate with no baked copy.
- Button plate family: blue neutral Back, green Load where occupied/enabled; no baked words in runtime art.

## Controls And Required States

- Back button: real visible button; states `normal`, `hover`, `pressed`, `disabled`.
- Load button per occupied slot: real visible button; states `normal`, `hover`, `pressed`, `disabled`.
- Save slot card selectable/clickable area if later made clickable: states `normal`, `hover`, `selected`, `disabled`.

## Live Runtime Content

- Page title text: runtime/localizable.
- Slot titles: `Mini Slot 1` through `Mini Slot 6`; runtime/localizable.
- Slot status: `Empty slot` or occupied metadata; runtime/localizable/value.
- Slot details: empty description or saved hero/difficulty/wave/last-updated data; runtime/localizable/value.
- Load button label: runtime/localizable.
- Footer status copy: runtime/localizable.

## Variants

- Empty local slots: cards show empty state and no visible Load button.
- Occupied local slots: cards show hero, difficulty, wave, timestamp, and enabled Load button.
- Online party host: occupied compatible slots can be loaded; footer text changes.
- Online party guest: load controls disabled or hidden; footer text explains host-only behavior.
- Error/status after attempted load: footer status copy changes.

## Reference Generation Constraints

- Preserve the six-card two-column layout and bottom footer/back-button contract from the current screenshot.
- Match the canonical main-menu material language: chunky stone/bronze frames, dark panel faces, jewel accents, green/blue/purple button family.
- Do not bake save-slot titles, status strings, metadata, button labels, or footer copy into any runtime-intended art.
- Leave enough interior space for multi-line save details without clipping.
- The generated reference may show representative readable text for planning, but production sprites derived later must be text-free except approved display art.
