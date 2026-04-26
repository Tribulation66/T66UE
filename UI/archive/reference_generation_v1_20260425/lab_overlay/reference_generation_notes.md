# Reference Generation Notes

- Chat number: 5
- Screen slug: `lab_overlay`
- Source current screenshot: `C:\UE\T66\UI\screens\lab_overlay\current\current_state_1920x1080.png`
- Main-menu style anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- Source screenshot status: existed
- Reference status: accepted; screen-specific reference exists at `C:\UE\T66\UI\screens\lab_overlay\reference\lab_overlay_reference_1920x1080.png`
- Raw source status: screen-specific raw native file exists beside the accepted reference when available: `lab_overlay_reference_raw_native.png`

## Runtime-Owned Regions To Preserve Later

- Lab title, tabs, item and enemy names, counts, status messages, and action labels.
- Item icons if present.
- Grant, spawn, reset, exit, selected tab, empty, and expanded/collapsed state remain runtime-owned.

## Integrator Notes

The restored reference is an offline comparison target only. Later sprite-family work should create shell, panel, tab, and button plates while preserving live item/enemy lists.
