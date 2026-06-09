# FriendslopStyle Temporary Powerups Element Manifest

Reference visual: `UI/FriendslopStyle/Reference/Drugs/Current/drugs_friendslop_reference_20260608.png`

Textless reference: `UI/FriendslopStyle/Reference/Drugs/Current/drugs_textless_reference_20260608.png`

Current proof capture: `Saved/Codex/UI/FriendslopStyle/SixScreens/current_20260608/Drugs_capture.png`

Current proof dump: `Saved/Codex/UI/FriendslopStyle/SixScreens/current_20260608/Drugs_dump.json`

The shared frontend top bar is excluded from regeneration by user instruction.

## Visual Family Ledger

| Visual family | Owned region/elements | Status | Runtime evidence |
|---|---|---|---|
| `PowerUpTabs` | Relics/Permanent and Steroids/One Run Use sub-tabs; excludes shared frontend top bar. | Visual PASS | Generated and wired from `RuntimeDependencies/T66/UI/FriendslopStyle/Drugs/drugs_powerup_tab_selected.png` and `drugs_powerup_tab_default.png`. |
| `TemporaryInfoStrip` | One-run-use instruction strip and current-selection context. | Visual PASS | Generated and wired from `RuntimeDependencies/T66/UI/FriendslopStyle/Drugs/drugs_info_strip.png` and `drugs_info_icon_plate.png`. |
| `SteroidRows` | Primary stat row shells, row labels, and row backing surfaces. | Visual PASS | Generated and wired from `RuntimeDependencies/T66/UI/FriendslopStyle/Drugs/drugs_steroid_row_shell.png` and `drugs_steroid_row_label_plate.png`. |
| `TemporaryCardGrid` | Temporary buff card surfaces, icon wells, buy/equip buttons, coupon/cost areas, and card spacing. | Visual PASS | Generated and wired from `RuntimeDependencies/T66/UI/FriendslopStyle/Drugs/drugs_temp_card_panel.png`, `drugs_temp_icon_well.png`, `drugs_temp_buy_button.png`, and `drugs_temp_equipped_button.png`. |
| `TemporaryScrollSurface` | Scrollable body, scrollbar, row stacking, and vertical containment. | Visual PASS | Generated and wired from `RuntimeDependencies/T66/UI/FriendslopStyle/Drugs/drugs_scroll_track.png`, `drugs_scroll_thumb.png`, `drugs_scroll_arrow_up.png`, and `drugs_scroll_arrow_down.png`. |

Family count: 5.

Comparison evidence:
- `Saved/Codex/UI/FriendslopStyle/SixScreens/current_20260608/Drugs_reference_vs_current_20260608.png`
- `Saved/Codex/UI/FriendslopStyle/SixScreens/current_20260608/Drugs_previous_vs_current_20260608.png`
