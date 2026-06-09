# Main Menu Round06 Production Slice Specs

Status: current slice record, revised 2026-06-06. Older inpaint/clean-sheet
slice specs are archived under `UI/FriendslopStyle/Archive/DeprecatedSliceSpecs/`.

Reference measurement crops are comparison targets only. Runtime plates must not
come from full-reference crops, hard-cleared live text/data regions, or manual
patching. If a listed runtime plate came from a superseded pass and has not been
revalidated under the current process, it is a runtime candidate, not accepted
proof.

| Asset | Runtime path | Intended size | Notes |
|---|---|---:|---|
| `topbar_strip_round06.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/topbar_strip_round06.png` | `1888x100` | live-overlay chrome plate. |
| `topbar_icon_dark_round06.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/topbar_icon_dark_round06.png` | `96x74` | pass17 imagegen-authored blank dark topbar icon plate; settings/globe glyphs are live Slate overlays. |
| `topbar_tab_dark_round06.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/topbar_tab_dark_round06.png` | `292x74` | live-overlay chrome plate. |
| `topbar_tab_red_round06.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/topbar_tab_red_round06.png` | `325x80` | live-overlay chrome plate. |
| `topbar_ticket_round06.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/topbar_ticket_round06.png` | `172x74` | live-overlay chrome plate. |
| `topbar_power_red_round06.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/topbar_power_red_round06.png` | `96x74` | blank red topbar power plate; power glyph is a live Slate overlay. |
| `left_panel_round06.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/left_panel_round06.png` | `500x892` | live-overlay chrome plate. |
| `profile_row_round06.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/profile_row_round06.png` | `460x108` | live-overlay chrome plate. |
| `search_field_round06.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/search_field_round06.png` | `460x60` | live-overlay chrome plate. |
| `section_header_round06.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/section_header_round06.png` | `460x42` | live-overlay chrome plate. |
| `friend_row_round06.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/friend_row_round06.png` | `460x58` | live-overlay chrome plate. |
| `invite_button_green_round06.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/invite_button_green_round06.png` | `80x44` | live-overlay chrome plate. |
| `offline_button_dark_round06.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/offline_button_dark_round06.png` | `80x42` | live-overlay chrome plate. |
| `party_slot_round06.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/party_slot_round06.png` | `94x94` | live-overlay chrome plate. |
| `title_logo_round06.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/title_logo_round06.png` | `730x100` | title-only generated asset or live Slate title; no full-reference crop. |
| `cta_primary_round06.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/cta_primary_round06.png` | `680x104` | live-overlay chrome plate. |
| `cta_secondary_round06.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/cta_secondary_round06.png` | `660x94` | live-overlay chrome plate. |
| `filter_global_red_round06.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/filter_global_red_round06.png` | `76x70` | static control icon plate. |
| `filter_friends_dark_round06.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/filter_friends_dark_round06.png` | `76x70` | static control icon plate. |
| `filter_stream_dark_round06.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/filter_stream_dark_round06.png` | `76x70` | static control icon plate. |
| `leaderboard_panel_round06.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/leaderboard_panel_round06.png` | `422x884` | live-overlay chrome plate. |
| `leaderboard_tab_red_round06.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/leaderboard_tab_red_round06.png` | `190x52` | live-overlay chrome plate. |
| `leaderboard_tab_dark_round06.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/leaderboard_tab_dark_round06.png` | `190x52` | live-overlay chrome plate. |
| `dropdown_dark_round06.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/dropdown_dark_round06.png` | `190x52` | live-overlay chrome plate. |
| `checkbox_checked_round06.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/checkbox_checked_round06.png` | `28x28` | live-overlay chrome plate. |
| `checkbox_empty_round06.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/checkbox_empty_round06.png` | `28x28` | live-overlay chrome plate. |
| `ranking_row_red_round06.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/ranking_row_red_round06.png` | `406x46` | live-overlay chrome plate. |

## Historical Loose Files Not Current Runtime Topbar Brushes

These PNGs still exist in `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu`
from earlier passes, but current `FT66FriendslopStyle` does not bind them for
the topbar icon buttons:

- `topbar_settings_round06.png`
- `topbar_language_round06.png`
- `topbar_power_icon_round06.png`
