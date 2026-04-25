# Reference Generation Notes

- Chat number: 5
- Screen slug: `wheel_overlay`
- Source current screenshot: `C:\UE\T66\UI\screens\wheel_overlay\current\current_state_1920x1080.png`
- Main-menu style anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- Source screenshot status: existed
- Reference status: accepted; screen-specific reference exists at `C:\UE\T66\UI\screens\wheel_overlay\reference\wheel_overlay_reference_1920x1080.png`
- Raw source status: screen-specific raw native file exists beside the accepted reference when available: `wheel_overlay_reference_raw_native.png`

## Runtime-Owned Regions To Preserve Later

- Wheel title, status/result text, spin/back labels, pending gold, rarity, spin angle, and result state.
- Wheel disk rotation, color/result state, idle/spinning/resolved state, and control enablement.
- Spin and back controls remain real runtime buttons.

## Integrator Notes

The restored reference is an offline comparison target only. Later runtime work should keep the wheel disk state live inside a generated socket/frame.
