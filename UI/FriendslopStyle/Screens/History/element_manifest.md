# FriendslopStyle Account History Element Manifest

Reference visual: `UI/FriendslopStyle/Reference/History/Current/history_friendslop_reference_20260608.png`

Textless reference: `UI/FriendslopStyle/Reference/History/Current/history_textless_reference_20260608.png`

Current proof capture: `Saved/Codex/UI/FriendslopStyle/SixScreens/current_20260608/History_capture.png`

Current proof dump: `Saved/Codex/UI/FriendslopStyle/SixScreens/current_20260608/History_dump.json`

The shared frontend top bar is excluded from regeneration by user instruction.

## Visual Family Ledger

| Visual family | Owned region/elements | Status | Runtime evidence |
|---|---|---|---|
| `AccountTabs` | Overview and History sub-tabs plus info icons; excludes shared frontend top bar. | Visual PASS | Generated and wired from `RuntimeDependencies/T66/UI/FriendslopStyle/History/history_account_tab_selected.png` and `history_account_tab_default.png`. |
| `HistoryFilterPanel` | Hero, difficulty, party size, status filters, daily descent checkbox, labels, and dropdown surfaces. | Visual PASS | Generated and wired from `RuntimeDependencies/T66/UI/FriendslopStyle/History/history_filter_panel.png`, `history_filter_dropdown.png`, `history_filter_checkbox_empty.png`, and `history_filter_checkbox_checked.png`. |
| `HistoryTable` | Run history panel, table header, row surfaces, sort controls, rank selector, dividers, and empty state. | Visual PASS | Generated and wired from `RuntimeDependencies/T66/UI/FriendslopStyle/History/history_table_panel.png`, `history_table_header_band.png`, and `history_table_row.png`. |

Family count: 3.

Comparison evidence:
- `Saved/Codex/UI/FriendslopStyle/SixScreens/current_20260608/History_reference_vs_current_20260608.png`
- `Saved/Codex/UI/FriendslopStyle/SixScreens/current_20260608/History_previous_vs_current_20260608.png`
