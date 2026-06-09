# FriendslopStyle Permanent Powerups Element Manifest

Reference visual: `UI/FriendslopStyle/Reference/Diplomas/Current/diplomas_friendslop_reference_20260608.png`

Textless reference: `UI/FriendslopStyle/Reference/Diplomas/Current/diplomas_textless_reference_20260608.png`

Current proof capture: `Saved/Codex/UI/FriendslopStyle/SixScreens/current_20260608/Diplomas_capture.png`

Current proof dump: `Saved/Codex/UI/FriendslopStyle/SixScreens/current_20260608/Diplomas_dump.json`

The shared frontend top bar is excluded from regeneration by user instruction.

## Visual Family Ledger

| Visual family | Owned region/elements | Status | Runtime evidence |
|---|---|---|---|
| `PowerUpTabs` | Relics/Permanent and Steroids/One Run Use sub-tabs; excludes shared frontend top bar. | Visual PASS | Generated and wired from `RuntimeDependencies/T66/UI/FriendslopStyle/Diplomas/diplomas_powerup_tab_selected.png` and `diplomas_powerup_tab_default.png`. |
| `PermanentInfoStrip` | Permanent powerup instruction strip and coupon balance context. | Visual PASS | Generated and wired from `RuntimeDependencies/T66/UI/FriendslopStyle/Diplomas/diplomas_info_strip.png` and `diplomas_info_icon_plate.png`. |
| `RelicCardGrid` | Relic card shells, art wells, buy/owned buttons, cost pills, and card row spacing. | Visual PASS | Generated and wired from `RuntimeDependencies/T66/UI/FriendslopStyle/Diplomas/diplomas_relic_card_available.png`, `diplomas_relic_card_owned.png`, `diplomas_relic_icon_well.png`, `diplomas_relic_buy_button.png`, and `diplomas_relic_owned_button.png`. |
| `RelicScrollSurface` | Scrollable body, scrollbar, card-grid backing, and vertical containment. | Visual PASS | Generated and wired from `RuntimeDependencies/T66/UI/FriendslopStyle/Diplomas/diplomas_scroll_track.png`, `diplomas_scroll_thumb.png`, `diplomas_scroll_arrow_up.png`, and `diplomas_scroll_arrow_down.png`. |

Family count: 4.

Comparison evidence:
- `Saved/Codex/UI/FriendslopStyle/SixScreens/current_20260608/Diplomas_reference_vs_current_20260608.png`
- `Saved/Codex/UI/FriendslopStyle/SixScreens/current_20260608/Diplomas_previous_vs_current_20260608.png`
