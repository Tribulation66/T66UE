# TD Difficulty Select Layout List

Canvas: `1920x1080`
Screen slug: `td_difficulty_select`
Status: blocked until `reference/canonical_reference_1920x1080.png` exists and preserves this layout in the canonical main-menu visual style.

## Required Image-Generation Inputs

Use exactly these inputs for the screen-specific reference:

1. Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
2. Current runtime screenshot: `C:\UE\T66\UI\screens\td_difficulty_select\current\2026-04-24\current_runtime_1920x1080.png`
3. Layout list: `C:\UE\T66\UI\screens\td_difficulty_select\layout\layout_list.md`

Do not generate sprites or edit Unreal styling for this screen until the generated reference exists.

## Screen Contract

TD Difficulty Select is a three-column decision screen. Preserve the left difficulty list, center 2x2 map-card grid, right selected-map summary, top title, and top-left back control. Restyle chrome and spacing into the main-menu fantasy family without replacing the live map imagery or baking data labels into runtime art.

## Regions

| Region | Approx Rect | Ownership | Notes |
| --- | --- | --- | --- |
| Full background | `0,0,1920,1080` | runtime-image + generated overlay allowance | Current screen uses the selected map background live. Reference should preserve the selected-map backdrop concept with a dark fantasy overlay. |
| Back button | `34,34,434,60` visual, `20,20,304,44` logical | runtime button + generated plate | Label live: `BACK TO TD`. Needs utility states. |
| Title well | `344,56,1235,58` | runtime text | Text live: `REGULAR DIFFICULTY AND MAP ROTATION`. Must avoid overlap with back button. |
| Main content envelope | `32,100,1856,956` | mixed | Three-column layout region. |
| Left difficulty column shell | `55,154,477,881` visual | generated-shell | Scrollable difficulty list. Shell must remain text-free. |
| Difficulty item: Easy | `80,180,410,170` | runtime button + generated selected/normal shell | Live name/body. Selected by default. |
| Difficulty item: Medium | `80,365,410,145` | runtime button + generated shell | Live name/body. |
| Difficulty item: Hard | `80,528,410,145` | runtime button + generated shell | Live name/body. |
| Difficulty item: Very Hard | `80,692,410,145` | runtime button + generated shell | Live name/body. |
| Difficulty item: Impossible | `80,858,410,165` | runtime button + generated shell | Live name/body. |
| Difficulty scrollbar | `493,181,14,802` | runtime control + generated track/thumb | Must be visible but not crowded. |
| Center map grid shell | `566,150,761,878` visual | generated-shell | Large framed map-card area, text-free frame. |
| Map card 1 | `620,190,310,390` | runtime button + runtime-image + generated card chrome | Selected map card. Preview image and text are live. |
| Map card 2 | `975,190,310,390` | runtime button + runtime-image + generated card chrome | Preview image and text are live. |
| Map card 3 | `620,612,310,390` | runtime button + runtime-image + generated card chrome | Preview image and text are live. |
| Map card 4 | `975,612,310,390` | runtime button + runtime-image + generated card chrome | Preview image and text are live. |
| Right summary panel shell | `1358,154,478,881` visual | generated-shell | Selected-map details and CTA. Shell must be text-free. |
| Summary title and map info wells | `1388,186,448,420` | runtime text | `REGULAR ROTATION`, map name, theme, description, scalar line, pressure notes, featured heroes. |
| Start Match button | `1388,626,448,66` | runtime button + generated plate | Green primary plate, live label. |
| Right lower empty summary space | `1388,704,448,300` | generated-shell | Keep as quiet panel interior for future text or variants. |

## Controls and Required States

| Control | States | Notes |
| --- | --- | --- |
| Back to TD | normal, hover, pressed, disabled | Blue/neutral utility button. |
| Difficulty items | normal, hover, pressed, selected, selected-hover, disabled | Selected state must be readable and distinct without broad muddy overlays. |
| Map cards | normal, hover, pressed, selected, selected-hover, disabled | Selected map needs clear border/accent; preview image remains live. |
| Start Match | normal, hover, pressed, disabled | Green primary fantasy plate with live label. |
| Scrollbar | normal, hover/drag where applicable | Track and thumb must fit inside left panel. |

## Live Data and Localizable Text

All visible strings and values remain live Slate text:

- `BACK TO TD`
- `REGULAR DIFFICULTY AND MAP ROTATION`
- difficulty names and descriptions
- map names, themes, lane counts, pad counts, boss-wave values
- `REGULAR ROTATION`
- selected map name, theme, description, scalar values, pressure notes, featured hero names
- `START MATCH`

Map preview images and the selected-map background are live images from TD map data. The generated reference may show representative current map art as comparison context, but runtime component assets must keep map-image apertures/sockets separate from chrome.

## Variants

- Default selected difficulty: Easy.
- Default selected map: Catacomb Spiral.
- Other difficulties reuse the same left-list and 2x2 map-grid structure.
- Map-card image wells must support different route shapes without changing card chrome.

## Current Capture Defects To Solve In Reference

- Top title is too large and crowds/overlaps the back-button area.
- Left and right shell art is wider than the code column slots.
- Center map cards are cramped and preview images are distorted/cropped.
- Selected state is too subtle.
- Difficulty descriptions wrap tightly near card edges.
- Summary panel has awkward vertical balance: dense text near top, large unused lower space.

