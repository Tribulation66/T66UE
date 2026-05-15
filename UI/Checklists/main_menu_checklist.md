# Main Menu Structural Preservation Checklist

Source inventory: `C:\UE\T66\UI\Geometry\main_menu_structural_inventory.md`

Baseline capture: `C:\UE\T66\Saved\Codex\UI\MainMenu\baseline_capture.png`

Baseline dump: `C:\UE\T66\Saved\Codex\UI\MainMenu\baseline_dump.json`

No-reference rule:
- Preserve baseline structure within normalized tolerance.
- Replace reachable legacy chrome with FT66FlatStyle-tagged widgets.
- The shared top bar is verified via its existing `top_bar` dump section and is not rebuilt by Main Menu.

## Structure

- [ ] MainMenu.Root | exists=true
- [ ] MainMenu.BackgroundRegion | exists=true
- [ ] MainMenu.Left.Panel | exists=true
- [ ] MainMenu.Left.ProfileButton | exists=true
- [ ] MainMenu.Left.ProfileName | exists=true
- [ ] MainMenu.Left.ProfileLevel | exists=true
- [ ] MainMenu.Left.ProfileProgress | exists=true
- [ ] MainMenu.Left.ProfileNextLevel | exists=true
- [ ] MainMenu.Left.SearchField | exists=true
- [ ] MainMenu.Left.FriendsPanel | exists=true
- [ ] MainMenu.Left.OnlineToggle | exists=true
- [ ] MainMenu.Left.OnlineLabel | exists=true
- [ ] MainMenu.Left.OnlineCount | exists=true
- [ ] MainMenu.Left.OfflineToggle | exists=true
- [ ] MainMenu.Left.OfflineLabel | exists=true
- [ ] MainMenu.Left.OfflineCount | exists=true
- [ ] MainMenu.Left.PartyPanel | exists=true
- [ ] MainMenu.Left.PartyLabel | exists=true
- [ ] MainMenu.Left.PartySlot01 | exists=true
- [ ] MainMenu.Left.PartySlot04 | exists=true
- [ ] MainMenu.Center.TitleRegion | exists=true
- [ ] MainMenu.Center.Title | exists=true
- [ ] MainMenu.Center.Subtitle | exists=true
- [ ] MainMenu.Center.CtaStack | exists=true
- [ ] MainMenu.Center.EnterTribulationButton | exists=true
- [ ] MainMenu.Center.LoadGameButton | exists=true
- [ ] MainMenu.Center.DailyDescentButton | exists=true
- [ ] MainMenu.Right.Panel | exists=true
- [ ] MainMenu.Right.FilterWorldButton | exists=true
- [ ] MainMenu.Right.FilterFriendsButton | exists=true
- [ ] MainMenu.Right.FilterStreamersButton | exists=true
- [ ] MainMenu.Right.LeaderboardPanel | exists=true
- [ ] MainMenu.Right.LeaderboardHeader | exists=true
- [ ] MainMenu.Right.ScoreScopeButton | exists=true
- [ ] MainMenu.Right.SpeedrunScopeButton | exists=true
- [ ] MainMenu.Right.TypeDropdown | exists=true
- [ ] MainMenu.Right.ModeDropdown | exists=true
- [ ] MainMenu.Right.RankHeader | exists=true
- [ ] MainMenu.Right.NameHeader | exists=true
- [ ] MainMenu.Right.ScoreHeader | exists=true
- [ ] MainMenu.Right.LeaderboardRows | exists=true
- [ ] MainMenu.Right.RankingRow01 | exists=true
- [ ] MainMenu.Right.RankingRow10 | exists=true
- [ ] MainMenu.Right.RankingRowLocal | exists=true
- [ ] FrontendTopBar.OuterContainer | exists=true
- [ ] FrontendTopBar.ProfileButton | exists=true
- [ ] FrontendTopBar.AccountButton | exists=true
- [ ] FrontendTopBar.TicketBadge.Value | exists=true

## Geometry

- [ ] MainMenu.Root | x=0.000 | 0.005
- [ ] MainMenu.Root | y=0.000 | 0.005
- [ ] MainMenu.Root | w=1.000 | 0.005
- [ ] MainMenu.Root | h=1.000 | 0.005
- [ ] MainMenu.Left.Panel | x=0.008 | 0.020
- [ ] MainMenu.Left.Panel | y=0.137 | 0.020
- [ ] MainMenu.Left.Panel | w=0.242 | 0.020
- [ ] MainMenu.Left.Panel | h=0.819 | 0.020
- [ ] MainMenu.Left.ProfileButton | x=0.019 | 0.020
- [ ] MainMenu.Left.ProfileButton | y=0.170 | 0.020
- [ ] MainMenu.Left.ProfileButton | w=0.221 | 0.020
- [ ] MainMenu.Left.ProfileButton | h=0.100 | 0.020
- [ ] MainMenu.Left.SearchField | y=0.282 | 0.020
- [ ] MainMenu.Left.FriendsPanel | y=0.347 | 0.020
- [ ] MainMenu.Left.PartyPanel | y=0.783 | 0.020
- [ ] MainMenu.Center.TitleRegion | x=0.291 | 0.020
- [ ] MainMenu.Center.TitleRegion | y=0.324 | 0.030
- [ ] MainMenu.Center.CtaStack | x=0.312 | 0.020
- [ ] MainMenu.Center.CtaStack | y=0.602 | 0.020
- [ ] MainMenu.Center.EnterTribulationButton | x=0.312 | 0.020
- [ ] MainMenu.Center.EnterTribulationButton | y=0.602 | 0.020
- [ ] MainMenu.Center.EnterTribulationButton | w=0.375 | 0.020
- [ ] MainMenu.Center.LoadGameButton | x=0.373 | 0.020
- [ ] MainMenu.Center.LoadGameButton | y=0.739 | 0.020
- [ ] MainMenu.Center.DailyDescentButton | x=0.373 | 0.020
- [ ] MainMenu.Center.DailyDescentButton | y=0.837 | 0.020
- [ ] MainMenu.Right.Panel | x=0.742 | 0.020
- [ ] MainMenu.Right.Panel | y=0.137 | 0.020
- [ ] MainMenu.Right.Panel | w=0.248 | 0.020
- [ ] MainMenu.Right.Panel | h=0.819 | 0.020
- [ ] MainMenu.Right.FilterWorldButton | x=0.747 | 0.020
- [ ] MainMenu.Right.FilterWorldButton | y=0.138 | 0.020
- [ ] MainMenu.Right.FilterFriendsButton | x=0.826 | 0.020
- [ ] MainMenu.Right.FilterStreamersButton | x=0.904 | 0.020
- [ ] MainMenu.Right.LeaderboardPanel | x=0.751 | 0.020
- [ ] MainMenu.Right.LeaderboardPanel | y=0.231 | 0.020

## Colors

- [ ] MainMenu.Left.Panel | button_state=Default
- [ ] MainMenu.Right.Panel | button_state=Default
- [ ] MainMenu.Center.EnterTribulationButton | button_state=Selected
- [ ] MainMenu.Center.LoadGameButton | button_state=Default
- [ ] MainMenu.Center.DailyDescentButton | button_state=Default
- [ ] MainMenu.Right.FilterWorldButton | button_state=Selected
- [ ] MainMenu.Right.FilterFriendsButton | button_state=Default
- [ ] MainMenu.Right.FilterStreamersButton | button_state=Default
- [ ] MainMenu.Left.Panel | border_color=DefaultBorder
- [ ] MainMenu.Right.Panel | border_color=DefaultBorder

## Content

- [ ] MainMenu.Center.Title | text=TRIBULATION 66
- [ ] MainMenu.Center.Title | is_label=true
- [ ] MainMenu.Center.Subtitle | text=If you're not Chad it's over
- [ ] MainMenu.Center.Subtitle | is_label=true
- [ ] MainMenu.Center.EnterTribulationButton | text=ENTER TRIBULATION
- [ ] MainMenu.Center.LoadGameButton | text=LOAD GAME
- [ ] MainMenu.Center.DailyDescentButton | text=DAILY DESCENT
- [ ] MainMenu.Left.ProfileName | text=Local Player | # content fallback; live Steam name may replace this outside baseline automation
- [ ] MainMenu.Left.ProfileName | is_label=true
- [ ] MainMenu.Left.ProfileLevel | text=Level 1/100
- [ ] MainMenu.Left.ProfileLevel | is_label=true
- [ ] MainMenu.Left.ProfileNextLevel | text=Level 2
- [ ] MainMenu.Left.ProfileNextLevel | is_label=true
- [ ] MainMenu.Left.OnlineLabel | text=ONLINE
- [ ] MainMenu.Left.OnlineLabel | is_label=true
- [ ] MainMenu.Left.OnlineCount | text=(0)
- [ ] MainMenu.Left.OnlineCount | is_label=true
- [ ] MainMenu.Left.OfflineLabel | text=OFFLINE
- [ ] MainMenu.Left.OfflineLabel | is_label=true
- [ ] MainMenu.Left.OfflineCount | text=(0)
- [ ] MainMenu.Left.OfflineCount | is_label=true
- [ ] MainMenu.Left.PartyLabel | text=PARTY
- [ ] MainMenu.Left.PartyLabel | is_label=true
- [ ] MainMenu.Right.LeaderboardHeader | text=GLOBAL CHAD RANKING
- [ ] MainMenu.Right.LeaderboardHeader | is_label=true
- [ ] MainMenu.Right.RankHeader | text=RANK
- [ ] MainMenu.Right.NameHeader | text=NAME
- [ ] MainMenu.Right.ScoreHeader | text=SCORE
- [ ] FrontendTopBar.TicketBadge.Value | is_label=true

## Interactivity

- [ ] MainMenu.Left.ProfileButton | has_click_handler=true
- [ ] MainMenu.Left.ProfileButton | hover_capable=true
- [ ] MainMenu.Left.SearchField | has_click_handler=true
- [ ] MainMenu.Left.SearchField | hover_capable=true
- [ ] MainMenu.Left.OnlineToggle | has_click_handler=true
- [ ] MainMenu.Left.OnlineToggle | hover_capable=true
- [ ] MainMenu.Left.OfflineToggle | has_click_handler=true
- [ ] MainMenu.Left.OfflineToggle | hover_capable=true
- [ ] MainMenu.Center.EnterTribulationButton | has_click_handler=true
- [ ] MainMenu.Center.EnterTribulationButton | hover_capable=true
- [ ] MainMenu.Center.LoadGameButton | has_click_handler=true
- [ ] MainMenu.Center.LoadGameButton | hover_capable=true
- [ ] MainMenu.Center.DailyDescentButton | has_click_handler=true
- [ ] MainMenu.Center.DailyDescentButton | hover_capable=true
- [ ] MainMenu.Right.FilterWorldButton | has_click_handler=true
- [ ] MainMenu.Right.FilterWorldButton | hover_capable=true
- [ ] MainMenu.Right.FilterWorldButton | toggle_group=MainMenuLeaderboardFilter
- [ ] MainMenu.Right.FilterFriendsButton | has_click_handler=true
- [ ] MainMenu.Right.FilterFriendsButton | hover_capable=true
- [ ] MainMenu.Right.FilterFriendsButton | toggle_group=MainMenuLeaderboardFilter
- [ ] MainMenu.Right.FilterStreamersButton | has_click_handler=true
- [ ] MainMenu.Right.FilterStreamersButton | hover_capable=true
- [ ] MainMenu.Right.FilterStreamersButton | toggle_group=MainMenuLeaderboardFilter
- [ ] MainMenu.Right.ScoreScopeButton | has_click_handler=true
- [ ] MainMenu.Right.ScoreScopeButton | hover_capable=true
- [ ] MainMenu.Right.ScoreScopeButton | toggle_group=MainMenuLeaderboardScope
- [ ] MainMenu.Right.SpeedrunScopeButton | has_click_handler=true
- [ ] MainMenu.Right.SpeedrunScopeButton | hover_capable=true
- [ ] MainMenu.Right.SpeedrunScopeButton | toggle_group=MainMenuLeaderboardScope
- [ ] MainMenu.Right.TypeDropdown | has_click_handler=true
- [ ] MainMenu.Right.TypeDropdown | hover_capable=true
- [ ] MainMenu.Right.ModeDropdown | has_click_handler=true
- [ ] MainMenu.Right.ModeDropdown | hover_capable=true
- [ ] FrontendTopBar.ProfileButton | button_state=Selected
- [ ] FrontendTopBar.ProfileButton | has_click_handler=true
- [ ] FrontendTopBar.ProfileButton | hover_capable=true
- [ ] FrontendTopBar.AccountButton | button_state=Default
- [ ] FrontendTopBar.AccountButton | toggle_group=FrontendCategorySelection
