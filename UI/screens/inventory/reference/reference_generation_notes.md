# Reference Generation Notes - inventory

- Chat number: 3
- Source current screenshot: `C:\UE\T66\UI\screens\inventory\current\current_state_1920x1080.png`
- Anchor path: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- Layout/component list: `C:\UE\T66\UI\screens\inventory\layout\layout_list.md`
- Source screenshot status: existed by the verification pass; no capture command was run by this chat
- Raw generated source kept at: `C:\UE\T66\UI\screens\inventory\reference\inventory_raw_imagegen_1672x941.png`
- Normalization: raw `1672x941` was deterministically resampled to `1920x1080` with `Scripts\InvokeDeterministicResample.py`
- Accepted output: `C:\UE\T66\UI\screens\inventory\reference\inventory_reference_1920x1080.png`
- Status: accepted for integrator review

## Runtime-Owned Regions To Preserve Later

- Gameplay scene, character, crosshair, score/time values, minimap content, stage number, hearts, idol slots, hero portrait, level badge, ability icons, inventory title, resource values, and item slots.
- Later runtime reconstruction must preserve gameplay visibility and keep the HUD regions as separate live apertures/socket wells.
