# Powerup Layout List

Canvas: 1920x1080.

Fixed structure:
- Full-width frontend top navigation bar across the top.
- Left back button below the top bar.
- Centered powerup category tabs near the top of the body: Permanent selected, Single Use inactive, plus currency cost block.
- Main body uses a three-column card grid.
- Cards contain category/name labels at top, stat label, progress track/value in the middle, and a large purchase button at bottom.
- A partial final row appears at the bottom, showing the grid continues.
- Right-edge scrollbar with clean minimal handles.

Runtime-owned regions:
- Item category/name labels, stat names, progress values/fills, prices, currency values, tab labels/states, top navigation labels, and scrollbar state.

Visual restyle target:
- Preserve the three-column card grid, tab controls, price strip, and scroll position.
- Restyle cards, progress tracks, purchase plates, and tabs in the sleek main-menu Chadpocalypse style while leaving card interiors as live-data sockets.
