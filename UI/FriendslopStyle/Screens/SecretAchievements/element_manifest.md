# FriendslopStyle Secret Achievements Element Manifest

Reference visual: `UI/FriendslopStyle/Reference/SecretAchievements/Current/secretachievements_friendslop_reference_20260608.png`

Textless reference: `UI/FriendslopStyle/Reference/SecretAchievements/Current/secretachievements_textless_reference_20260608.png`

Current proof capture: `Saved/Codex/UI/FriendslopStyle/SixScreens/current_20260608/SecretAchievements_capture.png`

Current proof dump: `Saved/Codex/UI/FriendslopStyle/SixScreens/current_20260608/SecretAchievements_dump.json`

The shared frontend top bar is excluded from regeneration by user instruction.

## Visual Family Ledger

| Visual family | Owned region/elements | Status | Runtime evidence |
|---|---|---|---|
| `AchievementTabs` | Steam and Secret sub-tabs plus info icons; excludes shared frontend top bar. | Visual PASS | Generated and wired from `RuntimeDependencies/T66/UI/FriendslopStyle/SecretAchievements/secret_achievement_tab_selected.png` and `secret_achievement_tab_default.png`. |
| `SecretSummaryPanel` | Secret logo, summary panel shell, masked count, progress track/fill, and header containment. | Visual PASS | Generated and wired from `RuntimeDependencies/T66/UI/FriendslopStyle/SecretAchievements/secret_summary_panel.png`, `secret_summary_logo_plate.png`, `secret_summary_progress_track.png`, and `secret_summary_progress_fill.png`. |
| `SecretList` | Secret list shell, masked row surfaces, claim buttons, favorite buttons, reward icons, table dividers, and live row text/data slots. | Visual PASS | Generated and wired from `RuntimeDependencies/T66/UI/FriendslopStyle/SecretAchievements/secret_achievement_list_panel.png`, `secret_achievement_row_shell.png`, `secret_claim_button.png`, `secret_favorite_button.png`, and `secret_reward_slot.png`. |

Family count: 3.

Comparison evidence:
- `Saved/Codex/UI/FriendslopStyle/SixScreens/current_20260608/SecretAchievements_reference_vs_current_20260608.png`
- `Saved/Codex/UI/FriendslopStyle/SixScreens/current_20260608/SecretAchievements_previous_vs_current_20260608.png`
