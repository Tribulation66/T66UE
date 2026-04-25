# Reference Generation Notes

- Chat number: 5
- Screen slug: `idol_altar_overlay`
- Source current screenshot: `C:\UE\T66\UI\screens\idol_altar_overlay\current\current_state_1920x1080.png`
- Main-menu style anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- Source screenshot status: existed
- Reference status: accepted; screen-specific reference exists at `C:\UE\T66\UI\screens\idol_altar_overlay\reference\idol_altar_overlay_reference_1920x1080.png`
- Raw source status: screen-specific raw native file exists beside the accepted reference when available: `idol_altar_overlay_reference_raw_native.png`

## Runtime-Owned Regions To Preserve Later

- Title, idol names, idol descriptions, level/upgrade text, status messages, and button labels.
- Idol icons, card selected/disabled state, reroll availability/cost, and remaining selections.
- Take, return, reroll, and back controls remain real runtime buttons.

## Integrator Notes

The restored reference is an offline comparison target only. Later asset work should treat altar cards as socket frames with live idol icons and text.
