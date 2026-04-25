# Reference Generation Notes

- Chat number: 5
- Screen slug: `crate_overlay`
- Source current screenshot: `C:\UE\T66\UI\screens\crate_overlay\current\current_state_1920x1080.png`
- Main-menu style anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- Source screenshot status: existed
- Reference status: accepted; screen-specific reference exists at `C:\UE\T66\UI\screens\crate_overlay\reference\crate_overlay_reference_1920x1080.png`
- Raw source status: screen-specific raw native file exists beside the accepted reference when available: `crate_overlay_reference_raw_native.png`

## Runtime-Owned Regions To Preserve Later

- Scrolling item icons, rarity colors, winner state, scroll offset, and resolved draw result.
- Skip hint, result text, item name, and reward status.
- Tile highlight/winner state and crate strip animation remain runtime-owned.

## Integrator Notes

The restored reference is an offline comparison target only. Later runtime art must use separate strip, tile, marker, and shell assets rather than a flattened full-screen reference.
