# TD Battle Layout List

Canvas: `1920x1080`
Screen slug: `td_battle`
Status: blocked until `reference/canonical_reference_1920x1080.png` exists and preserves this layout in the canonical main-menu visual style.

## Required Image-Generation Inputs

Use exactly these inputs for the screen-specific reference:

1. Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
2. Current runtime screenshot: `C:\UE\T66\UI\screens\td_battle\current\2026-04-24\current_runtime_1920x1080.png`
3. Layout list: `C:\UE\T66\UI\screens\td_battle\layout\layout_list.md`

Do not generate sprites or edit Unreal styling for this screen until the generated reference exists.

## Screen Contract

TD Battle is the live combat HUD for the tower-defense match. Preserve the left hero roster, centered title/stats, central gameplay board, bottom status, right match controls, and right selected-hero upgrade panel. The generated reference should make a fantasy TD HUD target in the main-menu visual family, but the gameplay board, hero roster content, stats, and button labels remain runtime-owned.

## Regions

| Region | Approx Rect | Ownership | Notes |
| --- | --- | --- | --- |
| Full background | `0,0,1920,1080` | runtime-image/generated-scene context | Current selected-map background sits behind HUD. Reference should keep dark readable map ambience. |
| Back button | `34,34,370,59` visual | runtime button + generated plate | Label live: `BACK TO MAPS`. |
| Title well | `638,42,644,68` | runtime text | Live selected map and difficulty: `Catacomb Spiral | Easy`. |
| Main content envelope | `22,84,1876,974` | mixed | Contains roster, board, panels, status. |
| Left hero roster shell | `39,130,450,909` | generated-shell | Large scrollable roster panel; shell must be text-free. |
| Roster header/body well | `66,164,390,124` | runtime text | Title and instructions live. |
| Roster list well | `65,309,392,710` | runtime list + generated row chrome | Hero rows contain live portrait, name, metadata, and cost. |
| Roster row | `76,315,370,103` repeated | runtime button/drag source + runtime image/text + generated row plate | Needs portrait socket, name line, meta/cost line. |
| Roster scrollbar | `448,310,14,685` | runtime control + generated track/thumb | Must not clip bottom item. |
| Stats bar shell | `522,125,817,69` | generated-shell + runtime text | Live materials, hearts, wave. |
| Center gameplay board frame | `522,221,818,797` visual | runtime gameplay board + generated frame | Code board is fixed `960x540`; board content is live. Reference must preserve board as live viewport, not baked combat UI. |
| Gameplay board interior | approx `540,238,780,760` visual, logical `960x540` | runtime board | Lanes, pads, towers, enemies, effects, hover, drag/drop, range rings live. |
| Bottom status well | `518,1048,815,24` | runtime text | Live status line; reference needs readable bottom band or safe text zone. |
| Right match-control shell | `1374,131,500,362` | generated-shell | Top right control panel. |
| Primary wave button | `1403,232,448,76` | runtime button + generated plate | Live label: start/wave running/restart. Green primary. |
| Speed button | `1403,326,448,57` | runtime button + generated plate | Live speed value. Blue/neutral. |
| Threat/status wells | `1403,405,448,72` | runtime text | Enemy notes and wave threat text live. |
| Right selected/upgrade shell | `1373,533,501,465` | generated-shell | Selected hero panel and upgrade controls. |
| Selected hero title/body | `1402,565,450,136` | runtime text | Live selected hero name/body or no-selection copy. |
| Power upgrade button | `1403,720,448,52` | runtime button + generated plate | Disabled by default in capture; label/cost live. |
| Range upgrade button | `1403,788,448,52` | runtime button + generated plate | Disabled by default; label/cost live. |
| Tempo upgrade button | `1403,852,448,52` | runtime button + generated plate | Disabled by default; label/cost live. |
| Sell hero button | `1403,920,448,52` | runtime button + generated plate | Disabled/no selection by default; label live. Purple/danger/sell variant. |

## Controls and Required States

| Control | States | Notes |
| --- | --- | --- |
| Back to Maps | normal, hover, pressed, disabled | Blue/neutral utility plate. |
| Hero roster rows | normal, hover, pressed, selected/dragging, disabled | Row plate, portrait socket, live text wells. |
| Battle board pads | normal, hover, occupied, invalid/drop-denied, selected | Board remains runtime-drawn; art target can clarify pad sockets and lane readability. |
| Start Wave / Wave Running / Restart | normal, hover, pressed, disabled | Green primary plate. Label and enabled state live. |
| Speed | normal, hover, pressed, disabled | Blue/neutral plate. Speed value live. |
| Power upgrade | normal, hover, pressed, disabled, maxed | Green/success plate, readable disabled state. |
| Range upgrade | normal, hover, pressed, disabled, maxed | Blue/neutral plate, readable disabled state. |
| Tempo upgrade | normal, hover, pressed, disabled, maxed | Green/success plate, readable disabled state. |
| Sell hero | normal, hover, pressed, disabled | Purple/danger sell plate, readable disabled state. |
| Scrollbar | normal, hover/drag where applicable | Track and thumb must fit the roster panel. |

## Live Data and Localizable Text

All visible text, values, and gameplay content remain live:

- `BACK TO MAPS`
- selected map name and difficulty
- materials, hearts, wave values
- hero roster title/body
- hero names, sprites, cost values, metadata
- board paths, pads, towers, enemies, projectiles/effects, range rings, drag/drop state
- match-control title/body, wave labels, speed value, threat text
- selected hero title/body
- upgrade labels/costs/max state and sell value
- bottom status line

The generated reference may include representative text for comparison, but runtime assets must be generated as text-free shells, sockets, plates, and wells.

## Variants

- Default pre-wave, no hero selected.
- Wave running.
- Victory/defeat.
- Hero selected with enabled/disabled upgrades.
- Dragging hero over valid and invalid board pads.
- Different maps with different board route geometry.

## Current Capture Defects To Solve In Reference

- Reused frame art is mismatched and crowded for battle HUD.
- Center gameplay board reads muddy; lane and pad readability need a stronger target.
- Roster rows are noisy and bottom item clips under the panel.
- Upgrade disabled states are too low contrast.
- Right-side selected panel text and buttons are cramped.
- Overall hierarchy mixes ornate frames with utilitarian live overlays; reference should unify materials and spacing while preserving live ownership.

