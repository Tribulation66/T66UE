# Reference Generation Notes

Chat number: 4

Source current screenshot path: `C:\UE\T66\UI\screens\gameplay_hudfull_map\current\current_state_1920x1080.png`

Anchor path: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`

Source screenshot status: existed at the canonical path before generation.

Generated source:
- Raw native imagegen output copied to `C:\UE\T66\UI\screens\gameplay_hudfull_map\reference\gameplay_hudfull_map_reference_raw_native.png`
- Raw output was deterministically normalized to `C:\UE\T66\UI\screens\gameplay_hudfull_map\reference\gameplay_hudfull_map_reference_1920x1080.png`

Runtime-owned regions to preserve later:
- Full gameplay scene behind the modal, underlying HUD values, map contents, fog/route data, player marker, close keybind/label, minimap content, and stage label.

Status: accepted for first-pass integrator review as an offline comparison target only.

Notes:
- The full-screen reference must not be used as a runtime background.
- Runtime reconstruction should preserve the large map well as a live-content socket and mask it during strict diffing.
