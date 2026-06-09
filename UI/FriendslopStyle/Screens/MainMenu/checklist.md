# FriendslopStyle Main Menu Checklist

Reference: `C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Current\main_menu_reference_04_foreground_right_panel_icon_filters_coupon_cli.png`

Geometry: `C:\UE\T66\UI\FriendslopStyle\Screens\MainMenu\geometry.md`

Slice spec: `C:\UE\T66\UI\FriendslopStyle\Screens\MainMenu\slice_specs.md`

This checklist is intentionally full-screen. A prior pass reported `PASS=94 FAIL=0`
while the runtime screen failed visually; do not weaken this file back to a
small structural smoke test.

## Structure

- [ ] MainMenu.Root | exists=true
- [ ] MainMenu.BackgroundRegion | exists=true
- [ ] FrontendTopBar.OuterContainer | exists=true
- [ ] FrontendTopBar.SettingsButton | exists=true
- [ ] FrontendTopBar.GlobeButton | exists=true
- [ ] FrontendTopBar.AccountButton | exists=true
- [ ] FrontendTopBar.ProfileButton | exists=true
- [ ] FrontendTopBar.PowerUpButton | exists=true
- [ ] FrontendTopBar.AchievementsButton | exists=true
- [ ] FrontendTopBar.TicketBadge | exists=true
- [ ] FrontendTopBar.PowerButton | exists=true
- [ ] MainMenu.Left.Panel | exists=true
- [ ] MainMenu.Left.ProfileButton | exists=true
- [ ] MainMenu.Left.ProfileAvatar | exists=true
- [ ] MainMenu.Left.ProfileName | exists=true
- [ ] MainMenu.Left.ProfileLevel | exists=true
- [ ] MainMenu.Left.ProfileProgress | exists=true
- [ ] MainMenu.Left.SearchField | exists=true
- [ ] MainMenu.Left.FriendsPanel | exists=true
- [ ] MainMenu.Left.OnlineToggle | exists=true
- [ ] MainMenu.Left.OnlineFriendRow01 | exists=true
- [ ] MainMenu.Left.OfflineToggle | exists=true
- [ ] MainMenu.Left.OfflineFriendRow01 | exists=true
- [ ] MainMenu.Left.PartyPanel | exists=true
- [ ] MainMenu.Left.PartySlot01 | exists=true
- [ ] MainMenu.Left.PartySlot04 | exists=true
- [ ] MainMenu.Center.TitleRegion | exists=true
- [ ] MainMenu.Center.Title | exists=true
- [ ] MainMenu.Center.Subtitle | exists=true
- [ ] MainMenu.Center.CtaStack | exists=true
- [ ] MainMenu.Center.EnterTribulationButton | exists=true
- [ ] MainMenu.Center.LoadGameButton | exists=true
- [ ] MainMenu.Right.Root | exists=true
- [ ] MainMenu.Right.FilterWorldButton | exists=true
- [ ] MainMenu.Right.FilterFriendsButton | exists=true
- [ ] MainMenu.Right.FilterStreamersButton | exists=true
- [ ] MainMenu.Right.LeaderboardPanel | exists=true
- [ ] MainMenu.Right.LeaderboardHeader | exists=true
- [ ] MainMenu.Right.TimeWeeklyButton | exists=true
- [ ] MainMenu.Right.TimeAllTimeButton | exists=true
- [ ] MainMenu.Right.PartySizeDropdown | exists=true
- [ ] MainMenu.Right.DifficultyDropdown | exists=true
- [ ] MainMenu.Right.HighScoreMetricButton | exists=true
- [ ] MainMenu.Right.SpeedRunMetricButton | exists=true
- [ ] MainMenu.Right.RankHeader | exists=true
- [ ] MainMenu.Right.NameHeader | exists=true
- [ ] MainMenu.Right.ScoreHeader | exists=true
- [ ] MainMenu.Right.LeaderboardRows | exists=true
- [ ] MainMenu.Right.RankingRowLocal | exists=true

## Full Screen Geometry

- [ ] MainMenu.Root | x=0.000 | 0.005
- [ ] MainMenu.Root | y=0.000 | 0.005
- [ ] MainMenu.Root | w=1.000 | 0.005
- [ ] MainMenu.Root | h=1.000 | 0.005
- [ ] FrontendTopBar.OuterContainer | x=0.000 | 0.002
- [ ] FrontendTopBar.OuterContainer | y=0.000 | 0.004
- [ ] FrontendTopBar.OuterContainer | w=1.000 | 0.002
- [ ] FrontendTopBar.OuterContainer | h=0.117 | 0.012
- [ ] MainMenu.Left.Panel | x=0.000 | 0.004
- [ ] MainMenu.Left.Panel | y=0.134 | 0.010
- [ ] MainMenu.Left.Panel | w=0.333 | 0.012
- [ ] MainMenu.Left.Panel | h=0.866 | 0.012
- [ ] MainMenu.Center.TitleRegion | x=0.271 | 0.020
- [ ] MainMenu.Center.TitleRegion | y=0.063 | 0.020
- [ ] MainMenu.Center.TitleRegion | w=0.458 | 0.020
- [ ] MainMenu.Center.TitleRegion | h=0.241 | 0.020
- [ ] MainMenu.Center.CtaStack | x=0.333 | 0.012
- [ ] MainMenu.Center.CtaStack | y=0.693 | 0.012
- [ ] MainMenu.Center.CtaStack | w=0.354 | 0.012
- [ ] MainMenu.Center.CtaStack | h=0.220 | 0.018
- [ ] MainMenu.Right.Root | x=0.698 | 0.012
- [ ] MainMenu.Right.Root | y=0.120 | 0.012
- [ ] MainMenu.Right.Root | w=0.302 | 0.012
- [ ] MainMenu.Right.Root | h=0.880 | 0.012
- [ ] MainMenu.Right.FilterPanel | x=0.698 | 0.012
- [ ] MainMenu.Right.FilterPanel | y=0.120 | 0.012
- [ ] MainMenu.Right.FilterPanel | w=0.302 | 0.012
- [ ] MainMenu.Right.FilterPanel | h=0.083 | 0.012
- [ ] MainMenu.Right.LeaderboardPanel | x=0.698 | 0.012
- [ ] MainMenu.Right.LeaderboardPanel | y=0.217 | 0.012
- [ ] MainMenu.Right.LeaderboardPanel | w=0.302 | 0.012
- [ ] MainMenu.Right.LeaderboardPanel | h=0.783 | 0.012

## Edge Anchor Geometry

- [ ] FrontendTopBar.OuterContainer | top_edge=0 | 1
- [ ] FrontendTopBar.OuterContainer | left_edge=0 | 1
- [ ] FrontendTopBar.OuterContainer | right_edge=1920 | 1
- [ ] MainMenu.Left.Panel | left_edge=0 | 1
- [ ] MainMenu.Left.Panel | bottom_edge=1 | 1
- [ ] MainMenu.Right.Root | right_edge=1 | 1
- [ ] MainMenu.Right.Root | bottom_edge=1 | 1

## Top Bar Geometry

- [ ] FrontendTopBar.SettingsButton | x=0.013 | 0.010
- [ ] FrontendTopBar.SettingsButton | y=0.022 | 0.010
- [ ] FrontendTopBar.SettingsButton | w=0.050 | 0.010
- [ ] FrontendTopBar.SettingsButton | h=0.069 | 0.010
- [ ] FrontendTopBar.GlobeButton | x=0.071 | 0.010
- [ ] FrontendTopBar.GlobeButton | y=0.022 | 0.010
- [ ] FrontendTopBar.GlobeButton | w=0.050 | 0.010
- [ ] FrontendTopBar.GlobeButton | h=0.069 | 0.010
- [ ] FrontendTopBar.AccountButton | x=0.131 | 0.012
- [ ] FrontendTopBar.AccountButton | w=0.152 | 0.012
- [ ] FrontendTopBar.ProfileButton | x=0.308 | 0.012
- [ ] FrontendTopBar.ProfileButton | w=0.169 | 0.012
- [ ] FrontendTopBar.PowerUpButton | x=0.486 | 0.012
- [ ] FrontendTopBar.PowerUpButton | w=0.152 | 0.012
- [ ] FrontendTopBar.AchievementsButton | x=0.647 | 0.012
- [ ] FrontendTopBar.AchievementsButton | w=0.156 | 0.012
- [ ] FrontendTopBar.TicketBadge | x=0.819 | 0.012
- [ ] FrontendTopBar.TicketBadge | w=0.090 | 0.012
- [ ] FrontendTopBar.PowerButton | x=0.925 | 0.010
- [ ] FrontendTopBar.PowerButton | w=0.050 | 0.010

## Left Panel Geometry

- [ ] MainMenu.Left.ProfileButton | x=0.021 | 0.008
- [ ] MainMenu.Left.ProfileButton | y=0.168 | 0.008
- [ ] MainMenu.Left.ProfileButton | w=0.240 | 0.008
- [ ] MainMenu.Left.ProfileButton | h=0.100 | 0.008
- [ ] MainMenu.Left.ProfileAvatar | x=0.027 | 0.008
- [ ] MainMenu.Left.ProfileAvatar | y=0.182 | 0.008
- [ ] MainMenu.Left.ProfileAvatar | w=0.040 | 0.008
- [ ] MainMenu.Left.SearchField | x=0.021 | 0.008
- [ ] MainMenu.Left.SearchField | y=0.279 | 0.008
- [ ] MainMenu.Left.SearchField | w=0.240 | 0.008
- [ ] MainMenu.Left.SearchField | h=0.056 | 0.008
- [ ] MainMenu.Left.FriendsPanel | x=0.021 | 0.008
- [ ] MainMenu.Left.FriendsPanel | y=0.344 | 0.008
- [ ] MainMenu.Left.FriendsPanel | w=0.240 | 0.008
- [ ] MainMenu.Left.FriendsPanel | h=0.431 | 0.010
- [ ] MainMenu.Left.OnlineToggle | x=0.021 | 0.008
- [ ] MainMenu.Left.OnlineToggle | w=0.240 | 0.008
- [ ] MainMenu.Left.OnlineFriendRow01 | x=0.021 | 0.008
- [ ] MainMenu.Left.OnlineFriendRow01 | w=0.240 | 0.008
- [ ] MainMenu.Left.OfflineToggle | x=0.021 | 0.008
- [ ] MainMenu.Left.OfflineToggle | w=0.240 | 0.008
- [ ] MainMenu.Left.OfflineFriendRow01 | x=0.021 | 0.008
- [ ] MainMenu.Left.OfflineFriendRow01 | w=0.240 | 0.008
- [ ] MainMenu.Left.PartyPanel | x=0.021 | 0.008
- [ ] MainMenu.Left.PartyPanel | y=0.790 | 0.010
- [ ] MainMenu.Left.PartyPanel | w=0.240 | 0.008
- [ ] MainMenu.Left.PartyPanel | h=0.144 | 0.010
- [ ] MainMenu.Left.PartySlot01 | x=0.027 | 0.010
- [ ] MainMenu.Left.PartySlot01 | y=0.838 | 0.010
- [ ] MainMenu.Left.PartySlot04 | x=0.199 | 0.010
- [ ] MainMenu.Left.PartySlot04 | y=0.838 | 0.010

## Center Geometry

- [ ] MainMenu.Center.Title | x=0.307 | 0.020
- [ ] MainMenu.Center.Title | y=0.115 | 0.020
- [ ] MainMenu.Center.Title | w=0.380 | 0.020
- [ ] MainMenu.Center.Title | h=0.093 | 0.020
- [ ] MainMenu.Center.Subtitle | x=0.354 | 0.018
- [ ] MainMenu.Center.Subtitle | y=0.198 | 0.018
- [ ] MainMenu.Center.Subtitle | w=0.292 | 0.020
- [ ] MainMenu.Center.EnterTribulationButton | x=0.333 | 0.010
- [ ] MainMenu.Center.EnterTribulationButton | y=0.693 | 0.010
- [ ] MainMenu.Center.EnterTribulationButton | w=0.354 | 0.010
- [ ] MainMenu.Center.EnterTribulationButton | h=0.096 | 0.010
- [ ] MainMenu.Center.EnterTribulationButton.Label | x=0.394 | 0.020
- [ ] MainMenu.Center.EnterTribulationButton.Label | y=0.726 | 0.020
- [ ] MainMenu.Center.LoadGameButton | x=0.339 | 0.010
- [ ] MainMenu.Center.LoadGameButton | y=0.819 | 0.010
- [ ] MainMenu.Center.LoadGameButton | w=0.344 | 0.010
- [ ] MainMenu.Center.LoadGameButton | h=0.087 | 0.010
- [ ] MainMenu.Center.LoadGameButton.Label | x=0.443 | 0.020
- [ ] MainMenu.Center.LoadGameButton.Label | y=0.843 | 0.020

## Right Panel Geometry

- [ ] MainMenu.Right.FilterWorldButton | x=0.716 | 0.010
- [ ] MainMenu.Right.FilterWorldButton | y=0.132 | 0.010
- [ ] MainMenu.Right.FilterWorldButton | w=0.078 | 0.010
- [ ] MainMenu.Right.FilterFriendsButton | x=0.803 | 0.010
- [ ] MainMenu.Right.FilterFriendsButton | y=0.132 | 0.010
- [ ] MainMenu.Right.FilterFriendsButton | w=0.078 | 0.010
- [ ] MainMenu.Right.FilterStreamersButton | x=0.891 | 0.010
- [ ] MainMenu.Right.FilterStreamersButton | y=0.132 | 0.010
- [ ] MainMenu.Right.FilterStreamersButton | w=0.078 | 0.010
- [ ] MainMenu.Right.LeaderboardHeader | x=0.767 | 0.020
- [ ] MainMenu.Right.LeaderboardHeader | y=0.233 | 0.020
- [ ] MainMenu.Right.TimeWeeklyButton | x=0.714 | 0.010
- [ ] MainMenu.Right.TimeWeeklyButton | y=0.289 | 0.010
- [ ] MainMenu.Right.TimeWeeklyButton | w=0.121 | 0.010
- [ ] MainMenu.Right.TimeAllTimeButton | x=0.841 | 0.010
- [ ] MainMenu.Right.TimeAllTimeButton | y=0.289 | 0.010
- [ ] MainMenu.Right.TimeAllTimeButton | w=0.121 | 0.010
- [ ] MainMenu.Right.PartySizeDropdown | x=0.714 | 0.010
- [ ] MainMenu.Right.PartySizeDropdown | y=0.344 | 0.010
- [ ] MainMenu.Right.DifficultyDropdown | x=0.841 | 0.010
- [ ] MainMenu.Right.DifficultyDropdown | y=0.344 | 0.010
- [ ] MainMenu.Right.HighScoreMetricButton | x=0.714 | 0.012
- [ ] MainMenu.Right.HighScoreMetricButton | y=0.404 | 0.012
- [ ] MainMenu.Right.SpeedRunMetricButton | x=0.841 | 0.012
- [ ] MainMenu.Right.SpeedRunMetricButton | y=0.404 | 0.012
- [ ] MainMenu.Right.RankHeader | x=0.720 | 0.018
- [ ] MainMenu.Right.RankHeader | y=0.492 | 0.018
- [ ] MainMenu.Right.NameHeader | x=0.773 | 0.018
- [ ] MainMenu.Right.NameHeader | y=0.492 | 0.018
- [ ] MainMenu.Right.ScoreHeader | x=0.925 | 0.018
- [ ] MainMenu.Right.ScoreHeader | y=0.492 | 0.018
- [ ] MainMenu.Right.LeaderboardRows | x=0.714 | 0.016
- [ ] MainMenu.Right.LeaderboardRows | y=0.526 | 0.016
- [ ] MainMenu.Right.LeaderboardRows | w=0.248 | 0.016
- [ ] MainMenu.Right.RankingRowLocal | x=0.714 | 0.012
- [ ] MainMenu.Right.RankingRowLocal | y=0.526 | 0.012
- [ ] MainMenu.Right.RankingRowLocal | w=0.248 | 0.012
- [ ] MainMenu.Right.RankingRowLocal | h=0.043 | 0.012

## Containment

- [ ] MainMenu.Left.ProfileButton | contained_in=MainMenu.Left.Panel inset=60,0,60,0 | 2
- [ ] MainMenu.Left.SearchField | contained_in=MainMenu.Left.Panel inset=60,0,60,0 | 2
- [ ] MainMenu.Left.FriendsPanel | contained_in=MainMenu.Left.Panel inset=60,0,60,0 | 2
- [ ] MainMenu.Left.PartyPanel | contained_in=MainMenu.Left.Panel inset=60,0,60,0 | 2
- [ ] MainMenu.Left.OnlineFriendRow01 | contained_in=MainMenu.Left.FriendsPanel inset=0,0,0,0 | 2
- [ ] MainMenu.Left.OnlineFriendRow01.ActionButton | contained_in=MainMenu.Left.OnlineFriendRow01 inset=0,0,6,0 | 2
- [ ] MainMenu.Left.OnlineFriendRow01.ActionText | contained_in=MainMenu.Left.OnlineFriendRow01.ActionButton inset=4,0,4,0 | 2
- [ ] MainMenu.Left.OfflineFriendRow01 | contained_in=MainMenu.Left.FriendsPanel inset=0,0,0,0 | 2
- [ ] MainMenu.Left.OfflineFriendRow01.ActionButton | contained_in=MainMenu.Left.OfflineFriendRow01 inset=0,0,6,0 | 2
- [ ] MainMenu.Left.OfflineFriendRow01.ActionText | contained_in=MainMenu.Left.OfflineFriendRow01.ActionButton inset=4,0,4,0 | 2
- [ ] MainMenu.Left.PartySlot01 | contained_in=MainMenu.Left.PartyPanel inset=0,0,0,0 | 2
- [ ] MainMenu.Left.PartySlot04 | contained_in=MainMenu.Left.PartyPanel inset=0,0,0,0 | 2
- [ ] MainMenu.Center.Title | contained_in=MainMenu.Center.TitleRegion inset=40,24,40,72 | 2
- [ ] MainMenu.Center.Subtitle | contained_in=MainMenu.Center.TitleRegion inset=80,0,80,18 | 2
- [ ] MainMenu.Center.EnterTribulationButton | contained_in=MainMenu.Center.CtaStack | 0
- [ ] MainMenu.Center.EnterTribulationButton.Label | contained_in=MainMenu.Center.EnterTribulationButton inset=104,0,104,0 | 2
- [ ] MainMenu.Center.EnterTribulationButton.LeftIcon | contained_in=MainMenu.Center.EnterTribulationButton inset=24,0,0,0 | 2
- [ ] MainMenu.Center.EnterTribulationButton.RightIcon | contained_in=MainMenu.Center.EnterTribulationButton inset=0,0,24,0 | 2
- [ ] MainMenu.Center.LoadGameButton | contained_in=MainMenu.Center.CtaStack | 0
- [ ] MainMenu.Center.LoadGameButton.Label | contained_in=MainMenu.Center.LoadGameButton inset=80,0,80,0 | 2
- [ ] MainMenu.Right.FilterPanel | contained_in=MainMenu.Right.Root inset=0,0,0,804 | 2
- [ ] MainMenu.Right.LeaderboardPanel | contained_in=MainMenu.Right.Root inset=0,104,0,0 | 2
- [ ] MainMenu.Right.TimeWeeklyButton | contained_in=MainMenu.Right.LeaderboardPanel inset=60,0,60,0 | 2
- [ ] MainMenu.Right.TimeAllTimeButton | contained_in=MainMenu.Right.LeaderboardPanel inset=60,0,60,0 | 2
- [ ] MainMenu.Right.PartySizeDropdown | contained_in=MainMenu.Right.LeaderboardPanel inset=60,0,60,0 | 2
- [ ] MainMenu.Right.DifficultyDropdown | contained_in=MainMenu.Right.LeaderboardPanel inset=60,0,60,0 | 2
- [ ] MainMenu.Right.RankingRowLocal | contained_in=MainMenu.Right.LeaderboardPanel inset=60,0,60,0 | 2
- [ ] MainMenu.Right.RankingRowLocal.Avatar | contained_in=MainMenu.Right.RankingRowLocal inset=0,0,0,0 | 2

## Content

- [ ] FrontendTopBar.ProfileButton | text=HOME
- [ ] FrontendTopBar.AccountButton | text=ACCOUNT
- [ ] FrontendTopBar.PowerUpButton | text=POWER UP
- [ ] FrontendTopBar.AchievementsButton | text=ACHIEVEMENTS
- [ ] MainMenu.Center.Subtitle | text=If you're not Chad it's over
- [ ] MainMenu.Center.Subtitle | is_label=true
- [ ] MainMenu.Center.EnterTribulationButton | text=ENTER TRIBULATION
- [ ] MainMenu.Center.LoadGameButton | text=LOAD GAME
- [ ] MainMenu.Left.OnlineLabel | text=ONLINE
- [ ] MainMenu.Left.OfflineLabel | text=OFFLINE
- [ ] MainMenu.Left.PartyLabel | text=PARTY
- [ ] MainMenu.Right.LeaderboardHeader | text=GLOBAL CHAD RANKING
- [ ] MainMenu.Right.TimeWeeklyButton | text=WEEKLY
- [ ] MainMenu.Right.TimeAllTimeButton | text=ALL TIME
- [ ] MainMenu.Right.PartySizeDropdown | text=SOLO
- [ ] MainMenu.Right.DifficultyDropdown | text=EASY

## State And Interactivity

- [ ] FrontendTopBar.SettingsButton | has_click_handler=true
- [ ] FrontendTopBar.GlobeButton | has_click_handler=true
- [ ] FrontendTopBar.AccountButton | has_click_handler=true
- [ ] FrontendTopBar.ProfileButton | has_click_handler=true
- [ ] FrontendTopBar.PowerUpButton | has_click_handler=true
- [ ] FrontendTopBar.AchievementsButton | has_click_handler=true
- [ ] FrontendTopBar.PowerButton | has_click_handler=true
- [ ] MainMenu.Left.ProfileButton | has_click_handler=true
- [ ] MainMenu.Left.ProfileButton | hover_capable=true
- [ ] MainMenu.Left.SearchField | has_click_handler=true
- [ ] MainMenu.Left.SearchField | hover_capable=true
- [ ] MainMenu.Left.OnlineToggle | has_click_handler=true
- [ ] MainMenu.Left.OfflineToggle | has_click_handler=true
- [ ] MainMenu.Left.OnlineFriendRow01.ActionButton | has_click_handler=true
- [ ] MainMenu.Center.EnterTribulationButton | has_click_handler=true
- [ ] MainMenu.Center.EnterTribulationButton | hover_capable=true
- [ ] MainMenu.Center.EnterTribulationButton | button_state=Selected
- [ ] MainMenu.Center.LoadGameButton | has_click_handler=true
- [ ] MainMenu.Center.LoadGameButton | hover_capable=true
- [ ] MainMenu.Center.LoadGameButton | button_state=Default
- [ ] MainMenu.Right.FilterWorldButton | has_click_handler=true
- [ ] MainMenu.Right.FilterWorldButton | toggle_group=MainMenuLeaderboardFilter
- [ ] MainMenu.Right.FilterFriendsButton | has_click_handler=true
- [ ] MainMenu.Right.FilterFriendsButton | toggle_group=MainMenuLeaderboardFilter
- [ ] MainMenu.Right.FilterStreamersButton | has_click_handler=true
- [ ] MainMenu.Right.FilterStreamersButton | toggle_group=MainMenuLeaderboardFilter
- [ ] MainMenu.Right.TimeWeeklyButton | has_click_handler=true
- [ ] MainMenu.Right.TimeWeeklyButton | toggle_group=MainMenuLeaderboardTime
- [ ] MainMenu.Right.TimeAllTimeButton | has_click_handler=true
- [ ] MainMenu.Right.TimeAllTimeButton | toggle_group=MainMenuLeaderboardTime
- [ ] MainMenu.Right.HighScoreMetricButton | has_click_handler=true
- [ ] MainMenu.Right.HighScoreMetricButton | toggle_group=MainMenuLeaderboardMetric
- [ ] MainMenu.Right.SpeedRunMetricButton | has_click_handler=true
- [ ] MainMenu.Right.SpeedRunMetricButton | toggle_group=MainMenuLeaderboardMetric
- [ ] MainMenu.Right.RankingRowLocal | has_click_handler=true

## Wiring And Functionality Gate Notes

This checklist is technical evidence, not the Friendslop visual acceptance gate.
It supports the final wiring/functionality gate by proving live widgets,
handlers, toggle groups, text/data ownership, containment that affects
functionality, and dump metadata.

The Main Menu pass report must still include the five-family manifest ledger
summary from `element_manifest.md`: all five visual families, visual
`PASS`/`FAIL` counts used for generation planning, per-element visual
`PASS`/`FAIL` counts inside every failed family, a family-by-family element
breakdown with bold family headings and two-column `Element` / `Visual
PASS/FAIL` tables, one CLI worker record per failed family, generated asset
implementation paths, sizing/fitting work performed, and wiring/functionality
`PASS`/`FAIL`.

Do not add a `visual_gate=PASS` row for Friendslop Main Menu unless the user
explicitly asks to restore a Codex-owned visual gate. The produced
reference/capture/contact evidence is for user visual review.

