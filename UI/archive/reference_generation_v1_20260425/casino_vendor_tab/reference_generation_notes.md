# Reference Generation Notes

- Chat number: 5
- Screen slug: `casino_vendor_tab`
- Source current screenshot: `C:\UE\T66\UI\screens\casino_vendor_tab\current\current_state_1920x1080.png`
- Main-menu style anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- Source screenshot status: existed
- Reference status: accepted; screen-specific reference exists at `C:\UE\T66\UI\screens\casino_vendor_tab\reference\casino_vendor_tab_reference_1920x1080.png`
- Raw source status: screen-specific raw native file exists beside the accepted reference when available: `casino_vendor_tab_reference_raw_native.png`

## Runtime-Owned Regions To Preserve Later

- Score, timer, gold, debt, anger, prices, reroll costs, and other economy values.
- Tab labels and selected state.
- Offer names, item descriptions, item icons, rarity icons, inventory and buyback sockets, status messages, and button labels.
- Offer hover, selected, locked, sold, and disabled states remain runtime-owned.

## Integrator Notes

The restored reference is an offline comparison target only. Vendor/gambler standalone overlays remain excluded; this tab is the casino vendor surface for this pass.
