# Reference Generation Notes

Chat number: 4

Source current screenshot path: `C:\UE\T66\UI\screens\gameplay_hudinventory_inspect\current\current_state_1920x1080.png`

Anchor path: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`

Source screenshot status: existed at the canonical path before generation.

Generated source:
- Raw native imagegen output copied to `C:\UE\T66\UI\screens\gameplay_hudinventory_inspect\reference\gameplay_hudinventory_inspect_reference_raw_native.png`
- Raw output was deterministically normalized to `C:\UE\T66\UI\screens\gameplay_hudinventory_inspect\reference\gameplay_hudinventory_inspect_reference_1920x1080.png`

Runtime-owned regions to preserve later:
- Gameplay scene, player, crosshair, level geometry, minimap contents, stage text, score/time values, heart count, portrait, ability icons, passive icon, idol sockets, inventory inspect item slots, item icons, keybinds, cooldowns, currency values, and item counts.

Status: accepted for first-pass integrator review as an offline comparison target only.

Notes:
- The full-screen reference must not be used as a runtime background.
- Runtime reconstruction should replace live inventory contents and HUD values with real widgets/assets and mask gameplay/map/content interiors during strict diffing.
