# Main Menu Structural Inventory

Source baseline capture: `C:\UE\T66\Saved\Codex\UI\MainMenu\baseline_capture.png`

Source baseline dump: `C:\UE\T66\Saved\Codex\UI\MainMenu\baseline_dump.json`

Reference mode: no external V3 reference image exists for Main Menu. This inventory is the structural source for the Stage 2 no-reference migration.

Normalized basis: 1920x1080.

## Baseline Summary

- Screen: `MainMenu`
- Baseline widgets: `490`
- Baseline tagged widgets: `0`
- Baseline includes shared `top_bar` dump section: yes
- Structural target: preserve the baseline layout regions and visible text/interactive roles while replacing reachable legacy chrome with FT66FlatStyle surfaces.

## Screen Regions

| Region | Tag | x | y | w | h | Notes |
| --- | --- | ---: | ---: | ---: | ---: | --- |
| Root screen | `MainMenu.Root` | 0.000 | 0.000 | 1.000 | 1.000 | Full-screen screen-proper content below/behind the shared top bar. |
| Background region | `MainMenu.BackgroundRegion` | 0.000 | 0.000 | 1.000 | 1.000 | Flat background replaces reference-art plate. |
| Left social panel | `MainMenu.Left.Panel` | 0.008 | 0.137 | 0.242 | 0.819 | Profile, friend search, friend groups, party slots. |
| Center title region | `MainMenu.Center.TitleRegion` | 0.291 | 0.324 | 0.396 | 0.245 | Baseline title lockup/subtitle area. |
| Center CTA stack | `MainMenu.Center.CtaStack` | 0.312 | 0.602 | 0.375 | 0.320 | Enter, load, daily descent buttons. |
| Right leaderboard panel | `MainMenu.Right.Panel` | 0.742 | 0.137 | 0.248 | 0.819 | Filter controls and leaderboard rows. |

## Left Panel Elements

| Element | Tag | x | y | w | h | Text / role |
| --- | --- | ---: | ---: | ---: | ---: | --- |
| Profile button | `MainMenu.Left.ProfileButton` | 0.019 | 0.170 | 0.221 | 0.100 | Opens account status. |
| Profile name | `MainMenu.Left.ProfileName` | 0.076 | 0.172 | 0.164 | 0.035 | `Local Player` baseline fallback. |
| Profile level | `MainMenu.Left.ProfileLevel` | 0.076 | 0.210 | 0.164 | 0.024 | `Level 1/100`. |
| Profile progress | `MainMenu.Left.ProfileProgress` | 0.076 | 0.242 | 0.110 | 0.021 | Account progress bar. |
| Next level | `MainMenu.Left.ProfileNextLevel` | 0.213 | 0.242 | 0.027 | 0.021 | `Level 2`. |
| Search field | `MainMenu.Left.SearchField` | 0.019 | 0.282 | 0.221 | 0.056 | Editable friend search. |
| Friends panel | `MainMenu.Left.FriendsPanel` | 0.019 | 0.347 | 0.221 | 0.429 | Scrollable friend groups. |
| Online group toggle | `MainMenu.Left.OnlineToggle` | 0.019 | 0.347 | 0.221 | 0.027 | Collapses/expands online group. |
| Online label | `MainMenu.Left.OnlineLabel` | 0.032 | 0.347 | 0.034 | 0.027 | `ONLINE`. |
| Online count | `MainMenu.Left.OnlineCount` | 0.071 | 0.347 | 0.014 | 0.027 | `(0)`. |
| Offline group toggle | `MainMenu.Left.OfflineToggle` | 0.019 | 0.394 | 0.221 | 0.027 | Collapses/expands offline group. |
| Offline label | `MainMenu.Left.OfflineLabel` | 0.032 | 0.394 | 0.039 | 0.027 | `OFFLINE`. |
| Offline count | `MainMenu.Left.OfflineCount` | 0.075 | 0.394 | 0.014 | 0.027 | `(0)`. |
| Party panel | `MainMenu.Left.PartyPanel` | 0.019 | 0.783 | 0.221 | 0.152 | Party header and four slots. |
| Party label | `MainMenu.Left.PartyLabel` | 0.019 | 0.787 | 0.219 | 0.028 | `PARTY`. |
| Party slot 1 | `MainMenu.Left.PartySlot01` | 0.019 | 0.833 | 0.051 | 0.090 | Local/empty slot. |
| Party slot 4 | `MainMenu.Left.PartySlot04` | 0.178 | 0.833 | 0.051 | 0.090 | Empty slot. |

## Center Elements

| Element | Tag | x | y | w | h | Text / role |
| --- | --- | ---: | ---: | ---: | ---: | --- |
| Main title | `MainMenu.Center.Title` | 0.291 | 0.324 | 0.396 | 0.160 | Flat text title replacing untagged title art. |
| Subtitle | `MainMenu.Center.Subtitle` | 0.291 | 0.500 | 0.396 | 0.055 | `If you're not Chad it's over`. |
| Enter Tribulation button | `MainMenu.Center.EnterTribulationButton` | 0.312 | 0.602 | 0.375 | 0.122 | Primary navigation to Hero Selection. |
| Load Game button | `MainMenu.Center.LoadGameButton` | 0.373 | 0.739 | 0.253 | 0.085 | Navigation to Load Game. |
| Daily Descent button | `MainMenu.Center.DailyDescentButton` | 0.373 | 0.837 | 0.253 | 0.085 | Navigation to Daily Descent. |

## Right Panel Elements

| Element | Tag | x | y | w | h | Text / role |
| --- | --- | ---: | ---: | ---: | ---: | --- |
| Filter world button | `MainMenu.Right.FilterWorldButton` | 0.747 | 0.138 | 0.071 | 0.067 | Leaderboard filter. |
| Filter friends button | `MainMenu.Right.FilterFriendsButton` | 0.826 | 0.138 | 0.071 | 0.067 | Leaderboard filter. |
| Filter streamers button | `MainMenu.Right.FilterStreamersButton` | 0.904 | 0.138 | 0.071 | 0.067 | Leaderboard filter. |
| Leaderboard content | `MainMenu.Right.LeaderboardPanel` | 0.751 | 0.231 | 0.229 | 0.707 | Ranking header, toggles, table. |
| Header | `MainMenu.Right.LeaderboardHeader` | 0.790 | 0.239 | 0.152 | 0.041 | `GLOBAL CHAD RANKING`. |
| Score scope button | `MainMenu.Right.ScoreScopeButton` | 0.752 | 0.283 | 0.112 | 0.053 | Selected scope control. |
| Speedrun scope button | `MainMenu.Right.SpeedrunScopeButton` | 0.868 | 0.283 | 0.112 | 0.053 | Default scope control. |
| Type dropdown | `MainMenu.Right.TypeDropdown` | 0.752 | 0.344 | 0.112 | 0.053 | Dropdown role. |
| Mode dropdown | `MainMenu.Right.ModeDropdown` | 0.868 | 0.344 | 0.112 | 0.053 | Dropdown role. |
| Rank header | `MainMenu.Right.RankHeader` | 0.752 | 0.446 | 0.043 | 0.024 | `RANK`. |
| Name header | `MainMenu.Right.NameHeader` | 0.795 | 0.446 | 0.141 | 0.024 | `NAME`. |
| Score header | `MainMenu.Right.ScoreHeader` | 0.935 | 0.446 | 0.044 | 0.024 | `SCORE`. |
| Ranking row 1 | `MainMenu.Right.RankingRow01` | 0.755 | 0.486 | 0.222 | 0.034 | `#1 CROWNED CHAD 184250`. |
| Ranking row 8 | `MainMenu.Right.RankingRow08` | 0.755 | 0.804 | 0.222 | 0.034 | `#8 STAGE SKIP 133910`. |
| Local ranking row | `MainMenu.Right.RankingRowLocal` | 0.755 | 0.902 | 0.222 | 0.038 | `#42 DOPRA 118700`. |

## Shared Top Bar Assertions

The shared top bar is not owned by Main Menu, but the dump includes it in the additive `top_bar` section. Main Menu checklist assertions verify presence only so this screen cannot regress the shared chrome visibility.

| Element | Tag | Expected |
| --- | --- | --- |
| Top bar outer container | `FrontendTopBar.OuterContainer` | Exists in `top_bar`. |
| Profile button | `FrontendTopBar.ProfileButton` | Selected state for Main Menu. |
| Account button | `FrontendTopBar.AccountButton` | Default category state. |
| Ticket value | `FrontendTopBar.TicketBadge.Value` | Label-only text. |
