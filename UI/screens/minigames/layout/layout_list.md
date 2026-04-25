# Minigames Layout List

Canvas: 1920x1080.

Fixed structure:
- Full-width frontend top navigation bar across the top.
- Left back button under the top bar.
- Body is a vertical list of large feature rows.
- Each row has a large title and descriptive copy on the left, a framed action/status area on the right, and a status button or tag inside that action area.
- Visible rows include two available features followed by multiple coming-soon feature slots.
- A right-edge scrollbar with clean minimal handles indicates more content.

Runtime-owned regions:
- Feature titles, descriptions, availability labels, status labels, top navigation labels, currency amount, and scroll state.

Visual restyle target:
- Preserve the row heights, text/action split, and vertical list structure.
- Restyle the large row shells, right status sockets, and status button plates in the sleek main-menu red/black/charcoal style, with content treated as live data.
