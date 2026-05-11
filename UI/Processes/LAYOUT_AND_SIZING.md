# Layout And Sizing Rules

The red/purple frontend pass uses reference screenshots for visual targets, but runtime screens must not behave like fixed screenshots. Every migrated screen needs an explicit layout contract before polish work starts.

## Screen Root

- Screens opened from the shared frontend top bar must fill the full viewport.
- The screen background must cover the full viewport, including the area behind and below content.
- Screen content must fill the area below the top bar through `T66ScreenSlateHelpers::MakeTopBarScreenRoot` or an equivalent top-bar-aware root.
- Do not center a fixed 1920x1080 child under the top bar and leave unused space around or beneath it.
- Modals may stay constrained, but full screens must use fill slots for the outer shell.
- Top-bar-accessible screens other than the main menu use a black full-viewport background unless a screen has an explicit approved art direction.

## Parent Containment

- A parent panel must be large enough for its children plus padding.
- If a child overflows a parent, resize the parent, reduce internal gaps, switch to a compact layout, or add a scroll area.
- Do not accept clipped controls as a visual match.
- Decorative raster chrome can be sliced or tiled, but live children must remain separate widgets inside the parent.
- Top-bar height is derived from the largest button/icon rect plus padding, not from the strip image alone.
- Top-bar rebuilds must preserve the top-bar viewport Z order. Do not route top-bar resize rebuilds through the base screen rebuild path.

## Standalone Capture State

- Screenshot and capture scripts may temporarily force windowed resolution, but they must restore the staged build's previous `GameUserSettings.ini` before exiting.
- Standalone staging should seed a sane default window size so the taskbar shortcut never inherits an ultrawide capture size.

## Responsive Structure

- Use the 1920x1080 reference as the composition source, not as the runtime viewport.
- Prefer `FillHeight` and `FillWidth` for top-level columns, panels, tables, and large content regions.
- Use fixed sizes only for atomic pieces that need stable proportions: icons, slots, compact buttons, row heights, and intentional reference-art slices.
- Wider displays can keep multi-column layouts. Narrow or short displays must reduce gutters, stack columns, or scroll content.
- Tables and histories should keep headers visible and scroll rows inside the table body.

## Breakpoints

Use the shared top-bar screen metrics:

- `bCompact`: reduce gutters, font sizes, and excess vertical gaps.
- `bStacked`: replace side-by-side columns with a vertical/scrolling layout.

These flags come from the available content area after the top bar, not the raw viewport.

## Visual Gate

Before a screen is marked migrated:

- Capture at 1920x1080, 1600x900, 1366x768, 1280x720, 2560x1440, and 3440x1440 when practical.
- Verify no top-level screen leaves blank space below its owned shell.
- Verify no child control escapes its panel, tab, row, or button.
- Verify all important text is readable or intentionally ellipsized.
- Screenshot mismatch still triggers another pass.
