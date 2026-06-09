# FriendslopStyle Steam Achievements Element Manifest

Reference visual: `UI/FriendslopStyle/Reference/SteamAchievements/Current/steamachievements_friendslop_reference_20260608.png`

Textless reference: `UI/FriendslopStyle/Reference/SteamAchievements/Current/steamachievements_textless_reference_20260608.png`

Current proof capture: `Saved/Codex/UI/FriendslopStyle/SixScreens/current_20260608/SteamAchievements_capture.png`

Current proof dump: `Saved/Codex/UI/FriendslopStyle/SixScreens/current_20260608/SteamAchievements_dump.json`

The shared frontend top bar is excluded from regeneration by user instruction.

## Visual Family Ledger

| Visual family | Owned region/elements | Status | Runtime evidence |
|---|---|---|---|
| `AchievementTabs` | Steam and Secret sub-tabs plus info icons; excludes shared frontend top bar. | Visual PASS | Generated and wired from `RuntimeDependencies/T66/UI/FriendslopStyle/SteamAchievements/steam_achievement_tab_selected.png` and `steam_achievement_tab_default.png`. |
| `SteamSummaryPanel` | Steam logo, summary panel shell, progress track/fill, count area, and header containment. | Visual PASS | Generated and wired from `RuntimeDependencies/T66/UI/FriendslopStyle/SteamAchievements/steam_summary_panel.png`, `steam_summary_logo_plate.png`, `steam_summary_progress_track.png`, and `steam_summary_progress_fill.png`. |
| `AchievementList` | Achievement list shell, row surfaces, claim buttons, favorite buttons, reward icons, table dividers, and live row text/data slots. | Visual PASS | Generated and wired from `RuntimeDependencies/T66/UI/FriendslopStyle/SteamAchievements/steam_achievement_list_panel.png`, `steam_achievement_row_shell.png`, `steam_claim_button.png`, `steam_favorite_button.png`, and `steam_reward_slot.png`. |

Family count: 3.

Comparison evidence:
- `Saved/Codex/UI/FriendslopStyle/SixScreens/current_20260608/SteamAchievements_reference_vs_current_20260608.png`
- `Saved/Codex/UI/FriendslopStyle/SixScreens/current_20260608/SteamAchievements_previous_vs_current_20260608.png`
