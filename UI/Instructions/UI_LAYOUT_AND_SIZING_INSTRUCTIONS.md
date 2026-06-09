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
- Reference-driven checklists must assert containment with
  `contained_in=<ParentTag>` for simple parent bounds or
  `contained_in=<ParentTag> inset=<left>,<top>,<right>,<bottom>` when a child
  must stay inside the parent's content area rather than its outer chrome.
- Flat chrome must not use decorative raster slices. Content artwork may be fixed, aspect-preserved, or intentionally tiled, but live children must remain separate widgets inside the parent.
- Top-bar height is derived from the largest button/icon rect plus padding, not from the strip image alone.
- Top-bar rebuilds must preserve the top-bar viewport Z order. Do not route top-bar resize rebuilds through the base screen rebuild path.

### Framed Panel Content Budget

- Framed panels with visible chrome must define one outer `PanelWidth` and one
  explicit per-side `ContentInset`.
- The live content width is derived, not hand-typed:
  `ContentWidth = PanelWidth - (ContentInset * 2)`.
- Child rows, columns, filters, tabs, dropdowns, tables, and slot groups inside
  the panel derive their widths from `ContentWidth` and named gaps.
- No child control may consume the full `PanelWidth`; doing so removes the
  breathing room around the chrome and reads as squished even when the parent is
  technically large enough.
- For a squished framed-panel pass, keep two independent knobs: first increase
  or preserve the outer panel shell when the reference/direction needs more
  breathing room, then reduce row/control/slot footprint inside the content
  area. Do not fix squish by shrinking the parent panel.
- If derived children still overflow, reduce child width/gap, use a compact
  state, or add scrolling inside the content area. Do not keep widening the
  panel past the reference target without an explicit visual-direction decision.

### Primitive Fit Gate

Reusable UI primitives with visible chrome, such as modals, dialogs, popovers,
tooltips, drawers, confirmation panels, and overlay cards, need an explicit
fit gate before visual handoff.

The primitive contract must define:

- outer plate size;
- per-side chrome/content insets;
- named live content rects;
- minimum chrome-to-content padding;
- minimum sibling gaps;
- allowed wrap, scale-down, ellipsis, compact, or scroll behavior;
- centering/alignment tolerances for title, body, row groups, and button rows;
- raster/tint sanity sample rects and maximum source-vs-capture luminance delta
  when textured chrome is used.

The fit gate is binary:

- `SIZING/POSITIONING PASS`: every live child is contained in the declared
  content rect, required padding/gaps are met, no unintended overlap occurs,
  text fits by an allowed behavior, textured chrome remains within declared
  tint tolerance, and all state variants still fit.
- `SIZING/POSITIONING FAIL`: any child escapes, clips, overlaps, touches chrome
  too closely, crowds the notch/corner/button cap, uses an undeclared wrap or
  ellipsis, renders textured chrome materially darker/lighter than the source
  outside the declared tolerance, or fits only because important text becomes
  unreadable.

On fail, adjust in this order:

1. Recalculate child sizes and positions from the primitive's content rect and
   named gaps instead of hand-moving individual pixels.
2. Reduce child widths, gaps, or font sizes only within the declared readable
   bounds.
3. Switch to a declared compact or scroll layout when available.
4. Increase the primitive outer size or author a size-specific plate only when
   the content cannot fit within minimum readable bounds.
5. Recapture and rerun the fit gate.

If the same fail set repeats twice, stop and create a review packet with the
capture, dump, unchanged failure list, and the next proposed sizing knob.

### Edge-Anchored Screen Chrome

- When a screen's approved composition uses edge chrome, encode that as hard
  geometry instead of leaving approximate gutters.
- Top-bar outer chrome must touch the top edge of the screen or scaled reference
  canvas: `TopBarY = 0`.
- Left-side chrome panels must touch the left edge: `PanelX = 0`.
- Right-side chrome panels must touch the right edge:
  `PanelX + PanelWidth = CanvasWidth`.
- Bottom-anchored side panels must touch the bottom edge:
  `PanelY + PanelHeight = CanvasHeight`.
- On scaled fixed-reference screens, these rules apply to the scaled reference
  frame. Off-aspect viewports may still letterbox or pillarbox unless the screen
  has a separate aspect-fill layout.

## Standalone Capture State

- Screenshot and capture scripts may temporarily force windowed resolution, but they must restore the staged build's previous `GameUserSettings.ini` before exiting.
- Standalone staging should seed a sane default window size so the taskbar shortcut never inherits an ultrawide capture size.

## Responsive Structure

- Use the 1920x1080 reference as the composition source, not as the runtime viewport.
- Prefer `FillHeight` and `FillWidth` for top-level columns, panels, tables, and large content regions.
- Use fixed sizes only for atomic pieces that need stable proportions: icons, slots, compact buttons, row heights, and intentional reference-art slices.
- Buttons and compact controls need explicit minimum widths and stable heights so labels, hover state, selection state, or localization changes cannot resize or collapse the control.
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
