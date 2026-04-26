# gameplay_hudfull_map Layout List

Canvas: 1920x1080.

Fixed structure:
- Gameplay HUD remains dimmed behind a large centered map overlay.
- Map overlay starts near x=145, y=3 and spans approximately w=1630, h=1077.
- Header row contains large MAP title at upper-left and close affordance at upper-right.
- Main map well is a very large black/dark live-data region inset inside the overlay, with a small player marker near center and a short horizontal route segment.
- Underlying HUD elements remain visible only as darkened context at the screen edges.

Runtime-owned regions:
- Full gameplay scene behind the modal, all underlying HUD data, map contents, player marker, route/fog data, keybind text, and close label.

Style target:
- Preserve the modal size, map well position, background dimming, and map/content region.
- Reinterpret only the shell, border, title bar, and inset frame in the accepted V2 main-menu language: sleek dark charcoal panels, smooth satin surfaces, crisp uniform purple outlines, cream text treatment, restrained gold accents only where useful, and a clean inner map socket.
- Do not add map controls, tabs, legends, icons, meters, rows, explanatory labels, or side panels beyond what is visible in the current screenshot.
- Avoid grain, cracked stone, rubble, gem/crystal surfaces, distressed metal, chipped edges, screws, rivets, bolts, heavy bevel stacks, noisy borders, and extra glow clutter.
