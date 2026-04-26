# Achievements Layout List

Canvas: 1920x1080.

Fixed structure:
- Full-width frontend top navigation bar across the top.
- No standalone Back button. Reclaim the space below the global top bar without adding replacement navigation.
- Centered tab selector with Achievements selected and Secret inactive.
- Large progress bar strip beneath the tabs, with count on the left and percent on the right.
- Main scroll panel fills the lower body.
- Repeating achievement rows stack vertically: title and description on the left, progress value centered, reward value and star/favorite socket on the right.
- Vertical scrollbar with clean minimal handles on the right edge.

Runtime-owned regions:
- Achievement names, descriptions, progress counts, percent values, reward values, star state, tab labels, top navigation labels, currency amount, and scrollbar state.

Visual restyle target:
- Preserve the row list, progress strip, tab positions, and scrollable page structure.
- Restyle row frames, progress track, tab buttons, and scroll rails with the V2 main-menu style: sleek dark charcoal surfaces, crisp uniform purple borders, restrained gold accents, clean planar panels, rounded controls, and low-grain presentation. Avoid cracked stone, gemstones/crystals, bevel-heavy fantasy frames, noisy borders, screws, rivets, chipped edges, and distressed texture.
