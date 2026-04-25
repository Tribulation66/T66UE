# Reference Generation Notes

- Chat number: 5
- Screen slug: `collector_overlay`
- Source current screenshot: `C:\UE\T66\UI\screens\collector_overlay\current\current_state_1920x1080.png`
- Main-menu style anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- Source screenshot status: existed
- Reference status: accepted; screen-specific reference exists at `C:\UE\T66\UI\screens\collector_overlay\reference\collector_overlay_reference_1920x1080.png`
- Raw source status: screen-specific raw native file exists beside the accepted reference when available: `collector_overlay_reference_raw_native.png`

## Runtime-Owned Regions To Preserve Later

- Collector item, NPC, interactable, enemy, and boss card labels.
- Item icons if present.
- Unlock availability, counts, actions, exit text, and status footer.
- Card hover, pressed, disabled, selected, and unavailable states remain live.

## Integrator Notes

The restored reference is an offline comparison target only. Later sprite-family work should generate section/card shells and leave card contents live.
