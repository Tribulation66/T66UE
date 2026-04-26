# Account Status Layout List

Canvas: 1920x1080.

Fixed structure:
- Full-width frontend top navigation bar across the top, with settings/social buttons on the left, Account, Power Up, Achievements, Minigames tabs, currency block, and power button on the right.
- No standalone Back button. Reclaim the space below the global top bar without adding replacement navigation.
- Secondary account tabs centered below the global top bar: Overview selected, History inactive.
- Main content splits into a left progress column and a right records column inside heavy framed panels.
- Left column contains Account Status panel at top and Account Progress list below with stacked progress rows.
- Right column contains top filter dropdowns, then Highest Score table and Best Speed Run table.
- Vertical scroll rails with clean minimal handles on the right edge of each major scrollable region.

Runtime-owned regions:
- All headings, labels, account standing text, appeal state, progress values, progress fills, dropdown values, table headings, difficulty rows, ranks, scores, times, and empty-state text.
- The top navigation labels, icon meanings, currency amount, and power button state.

Visual restyle target:
- Preserve the layout and row/table hierarchy exactly.
- Reinterpret shells, row plates, dropdowns, table chrome, scroll rails, and progress tracks in the V2 main-menu style: sleek dark charcoal surfaces, crisp uniform purple borders, restrained gold accents, clean planar panels, rounded controls, and readable socketed live-data areas. Avoid grain, cracked stone, gemstone/crystal shapes, bevel-heavy fantasy frames, noisy borders, screws, rivets, chipped edges, and distressed texture.
