# TD Main Menu Layout List

Canvas: `1920x1080`
Screen slug: `td_main_menu`
Status: blocked until `reference/canonical_reference_1920x1080.png` exists and preserves this layout in the canonical main-menu visual style.

## Required Image-Generation Inputs

Use exactly these inputs for the screen-specific reference:

1. Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
2. Current runtime screenshot: `C:\UE\T66\UI\screens\td_main_menu\current\2026-04-24\current_runtime_1920x1080.png`
3. Layout list: `C:\UE\T66\UI\screens\td_main_menu\layout\layout_list.md`

Do not generate sprites or edit Unreal styling for this screen until the generated reference exists.

## Screen Contract

TD Main Menu is a three-zone launcher screen for Chadpocalypse TD. Preserve the runtime layout: top-left back button, centered title, left information panel, lower-center CTA stack, and right tier-preview panel. Restyle it into the main-menu fantasy visual language, but do not redesign the functional hierarchy.

## Regions

| Region | Approx Rect | Ownership | Notes |
| --- | --- | --- | --- |
| Full background scene | `0,0,1920,1080` | generated-scene-plate | Current capture is mostly black/flat. Reference should use main-menu-style dungeon fantasy atmosphere, but no baked foreground UI. |
| Back button | `34,34,434,60` visual, `20,20,304,44` logical | runtime button + generated plate | Label remains live: `BACK TO MENU`. Needs real button states. |
| Title well | `540,74,830,80` | runtime text | Text remains live: `CHADPOCALYPSE TD`. Keep safe empty/low-detail space behind it. |
| Left info panel shell | `37,255,540,790` | generated-shell | Large framed panel with title and three content cards. Shell must be text-free for runtime text. |
| Left card 1 | `85,334,470,136` | generated-shell + runtime text | Card title/body live: Format copy. |
| Left card 2 | `85,500,470,150` | generated-shell + runtime text | Card title/body live: Roster copy and hero count. |
| Left card 3 | `85,690,470,136` | generated-shell + runtime text | Card title/body live: Scope copy, difficulty count, map count. |
| CTA stack frame | `577,592,777,296` | generated-shell | Lower-center frame behind two real buttons. Do not bake labels. |
| CTA primary button | `587,613,746,118` | runtime button + generated plate | Label live: `REGULAR`. Green primary fantasy plate. |
| CTA map button | `587,754,746,118` | runtime button + generated plate | Label live: `DIFFICULTIES & MAPS`. Green primary fantasy plate. |
| Right tier panel shell | `1345,255,540,790` | generated-shell | Large framed panel with title and four content cards. Shell must be text-free. |
| Right card 1 | `1394,334,470,106` | generated-shell + runtime text | Live title/body: Easy to Dungeon tier. |
| Right card 2 | `1394,478,470,106` | generated-shell + runtime text | Live title/body: Medium to Forest tier. |
| Right card 3 | `1394,622,470,106` | generated-shell + runtime text | Live title/body: Hard to Ocean tier. |
| Right card 4 | `1394,766,470,126` | generated-shell + runtime text | Live title/body: Very Hard / Impossible tier. |

## Controls and Required States

| Control | States | Notes |
| --- | --- | --- |
| Back to Menu | normal, hover, pressed, disabled | Blue/neutral utility button plate; visible plate is the real button. |
| Regular | normal, hover, pressed, disabled | Wide green primary button; live label centered over plate. |
| Difficulties & Maps | normal, hover, pressed, disabled | Wide green primary button; live label centered over plate. |

## Live Data and Localizable Text

All visible strings remain live Slate text and must not be baked into runtime art:

- `BACK TO MENU`
- `CHADPOCALYPSE TD`
- `REGULAR MODE`
- `Format`, `Roster`, `Scope`
- body copy in all left cards
- `REGULAR`
- `DIFFICULTIES & MAPS`
- `TIER PREVIEW`
- all right card titles and descriptions
- hero count, difficulty count, map count, and any future status values

The full generated reference may show representative text for visual comparison, but downstream runtime assets must be generated text-free except for approved display art.

## Variants

- Default TD Main Menu view.
- Button states for back and CTA buttons.
- Future localization should fit the same title, card, and button wells.

## Current Capture Defects To Solve In Reference

- Background is mostly black and lacks the approved main-menu fantasy scene depth.
- Existing panel/button art is scaled beyond the logical widget slots.
- Back button plate is larger than its logical control.
- CTA stack and panels use reused chrome without a screen-specific composition.
- Text wells are tight in the right panel; generated reference should provide cleaner card padding.

