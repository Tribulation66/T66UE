# FriendslopStyle Account Overview Element Manifest

Reference visual: `UI/FriendslopStyle/Reference/Overview/Current/overview_friendslop_reference_20260608.png`

Textless reference: `UI/FriendslopStyle/Reference/Overview/Current/overview_textless_reference_20260608.png`

Current proof capture: `Saved/Codex/UI/FriendslopStyle/SixScreens/current_20260608/Overview_capture.png`

Current proof dump: `Saved/Codex/UI/FriendslopStyle/SixScreens/current_20260608/Overview_dump.json`

The shared frontend top bar is excluded from regeneration by user instruction.

## Visual Family Ledger

| Visual family | Owned region/elements | Status | Runtime evidence |
|---|---|---|---|
| `AccountTabs` | Overview and History sub-tabs plus info icons; excludes shared frontend top bar. | Visual PASS | Generated and wired from `RuntimeDependencies/T66/UI/FriendslopStyle/Overview/overview_account_tab_selected.png` and `overview_account_tab_default.png`. |
| `LeftAccountPanels` | Player block, account status panel, account progress panel, avatar, progress tracks, and status messaging. | Visual PASS | Generated and wired from `RuntimeDependencies/T66/UI/FriendslopStyle/Overview/overview_left_profile_panel.png`, `overview_left_status_panel.png`, `overview_left_progress_panel.png`, `overview_avatar_plate.png`, `overview_progress_track.png`, and `overview_progress_fill.png`. |
| `RightRecordsPanels` | Personal best filters, highest score panel, best speed run panel, table rows, dividers, and value cells. | Visual PASS | Generated and wired from `RuntimeDependencies/T66/UI/FriendslopStyle/Overview/overview_record_panel.png`, `overview_record_filter_dropdown.png`, `overview_record_header_band.png`, and `overview_record_table_row.png`. |

Family count: 3.

Comparison evidence:
- `Saved/Codex/UI/FriendslopStyle/SixScreens/current_20260608/Overview_reference_vs_current_20260608.png`
- `Saved/Codex/UI/FriendslopStyle/SixScreens/current_20260608/Overview_previous_vs_current_20260608.png`
