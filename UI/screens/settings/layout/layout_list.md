# Settings Layout List

Canvas: 1920x1080.

Fixed structure:
- Full-width frontend top navigation bar across the top.
- Horizontal category tab strip below the top bar with Gameplay selected and Graphics, Controls, HUD, Media Viewer, Audio, Crashing, Retro FX inactive.
- Left back button below the category strip.
- Main scrollable settings panel fills the body.
- Repeating settings rows run full width, with label on the left and control group on the right.
- Rows include two-state toggles, dropdowns, and additional settings below the fold.
- Vertical scrollbar with clean minimal handles on the right edge.

Runtime-owned regions:
- Category tab labels and states, setting labels, toggle labels and selected states, dropdown values/options, top navigation labels, currency amount, and scrollbar state.

Visual restyle target:
- Preserve the dense utility layout and row positions.
- Restyle tabs, row frames, toggles, dropdowns, and scroll rail with main-menu neon metal styling while keeping all text/value areas live and localizable.
