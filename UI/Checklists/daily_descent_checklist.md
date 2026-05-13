# Daily Descent UI Fidelity Checklist

Source geometry: `C:\UE\T66\UI\Geometry\daily_descent_reference_geometry.md`

Reference notes: `Daily.png` is treated as the visual authority where it differs from broad spec prose. The reference shows two stat rows and ten leaderboard rows.

## Structure

- [ ] FrontendTopBar.Root | exists=true
- [ ] FrontendTopBar.OuterContainer | exists=true
- [ ] FrontendTopBar.SettingsButton | exists=true
- [ ] FrontendTopBar.SettingsButton.Icon | exists=true
- [ ] FrontendTopBar.GlobeButton | exists=true
- [ ] FrontendTopBar.GlobeButton.Icon | exists=true
- [ ] FrontendTopBar.BackToMainMenuButton | exists=true
- [ ] FrontendTopBar.BackToMainMenuButton.Label | exists=true
- [ ] FrontendTopBar.PowerButton | exists=true
- [ ] FrontendTopBar.PowerButton.Icon | exists=true
- [ ] DailyDescent.Root | exists=true
- [ ] DailyDescent.LeftPanel | exists=true
- [ ] DailyDescent.LeftPanel.Header | exists=true
- [ ] DailyDescent.LeftPanel.IntroPanel | exists=true
- [ ] DailyDescent.LeftPanel.IntroIcon | exists=true
- [ ] DailyDescent.LeftPanel.IntroText | exists=true
- [ ] DailyDescent.LeftPanel.HeroRow | exists=true
- [ ] DailyDescent.LeftPanel.HeroRow.Icon | exists=true
- [ ] DailyDescent.LeftPanel.HeroRow.Label | exists=true
- [ ] DailyDescent.LeftPanel.HeroRow.Value | exists=true
- [ ] DailyDescent.LeftPanel.DifficultyRow | exists=true
- [ ] DailyDescent.LeftPanel.DifficultyRow.Icon | exists=true
- [ ] DailyDescent.LeftPanel.DifficultyRow.Label | exists=true
- [ ] DailyDescent.LeftPanel.DifficultyRow.Value | exists=true
- [ ] DailyDescent.LeftPanel.ModifiersHeader | exists=true
- [ ] DailyDescent.LeftPanel.PocketDraftRow | exists=true
- [ ] DailyDescent.LeftPanel.IronParadeRow | exists=true
- [ ] DailyDescent.LeftPanel.DoubleDropRow | exists=true
- [ ] DailyDescent.CenterArt | exists=true
- [ ] DailyDescent.Title | exists=true
- [ ] DailyDescent.Subtitle | exists=true
- [ ] DailyDescent.StartButton | exists=true
- [ ] DailyDescent.StartButton.Label | exists=true
- [ ] DailyDescent.StartButton.Badge | exists=true
- [ ] DailyDescent.ContinueButton | exists=true
- [ ] DailyDescent.ContinueButton.Label | exists=true
- [ ] DailyDescent.LeaderboardTabs.GlobalButton | exists=true
- [ ] DailyDescent.LeaderboardTabs.FriendsButton | exists=true
- [ ] DailyDescent.LeaderboardTabs.BroadcastButton | exists=true
- [ ] DailyDescent.RightLeaderboardPanel | exists=true
- [ ] DailyDescent.RightLeaderboardPanel.Header | exists=true
- [ ] DailyDescent.RightLeaderboardPanel.Row01 | exists=true
- [ ] DailyDescent.RightLeaderboardPanel.Row05 | exists=true
- [ ] DailyDescent.RightLeaderboardPanel.Row10 | exists=true
- [ ] DailyDescent.RightLeaderboardPanel.PlayerSeparator | exists=true
- [ ] DailyDescent.RightLeaderboardPanel.PlayerRow | exists=true

## Geometry

- [ ] FrontendTopBar.OuterContainer | x=0.010 | 0.010
- [ ] FrontendTopBar.OuterContainer | y=0.013 | 0.010
- [ ] FrontendTopBar.SettingsButton | x=0.023 | 0.008
- [ ] FrontendTopBar.SettingsButton | y=0.028 | 0.008
- [ ] FrontendTopBar.GlobeButton | x=0.083 | 0.008
- [ ] FrontendTopBar.GlobeButton | y=0.028 | 0.008
- [ ] FrontendTopBar.BackToMainMenuButton | x=0.322 | 0.008
- [ ] FrontendTopBar.BackToMainMenuButton | y=0.028 | 0.008
- [ ] FrontendTopBar.PowerButton | x=0.919 | 0.008
- [ ] FrontendTopBar.PowerButton | y=0.028 | 0.008
- [ ] DailyDescent.LeftPanel | x=0.013 | 0.010
- [ ] DailyDescent.LeftPanel | y=0.153 | 0.010
- [ ] DailyDescent.CenterArt | x=0.305 | 0.014
- [ ] DailyDescent.CenterArt | y=0.130 | 0.014
- [ ] DailyDescent.Title | x=0.345 | 0.012
- [ ] DailyDescent.Title | y=0.594 | 0.012
- [ ] DailyDescent.StartButton | x=0.321 | 0.010
- [ ] DailyDescent.StartButton | y=0.722 | 0.010
- [ ] DailyDescent.ContinueButton | x=0.321 | 0.010
- [ ] DailyDescent.ContinueButton | y=0.847 | 0.010
- [ ] DailyDescent.RightLeaderboardPanel | x=0.690 | 0.010
- [ ] DailyDescent.RightLeaderboardPanel | y=0.233 | 0.010
- [ ] DailyDescent.LeaderboardTabs.GlobalButton | x=0.696 | 0.010
- [ ] DailyDescent.LeaderboardTabs.GlobalButton | y=0.137 | 0.010
- [ ] DailyDescent.LeaderboardTabs.FriendsButton | x=0.792 | 0.010
- [ ] DailyDescent.LeaderboardTabs.BroadcastButton | x=0.895 | 0.010
- [ ] DailyDescent.RightLeaderboardPanel.PlayerRow | y=0.864 | 0.010

## Colors

- [ ] FrontendTopBar.SettingsButton | button_state=Default
- [ ] FrontendTopBar.GlobeButton | button_state=Default
- [ ] FrontendTopBar.BackToMainMenuButton | button_state=Default
- [ ] FrontendTopBar.PowerButton | button_state=Selected
- [ ] DailyDescent.LeftPanel | border_color=SelectedBorder
- [ ] DailyDescent.RightLeaderboardPanel | border_color=DefaultBorder
- [ ] DailyDescent.LeaderboardTabs.GlobalButton | button_state=Selected
- [ ] DailyDescent.LeaderboardTabs.FriendsButton | button_state=Default
- [ ] DailyDescent.LeaderboardTabs.BroadcastButton | button_state=Default
- [ ] DailyDescent.StartButton | button_state=Selected
- [ ] DailyDescent.ContinueButton | button_state=Default

## Content

- [ ] FrontendTopBar.BackToMainMenuButton.Label | text=BACK TO MAIN MENU
- [ ] FrontendTopBar.BackToMainMenuButton.Label | is_label=true
- [ ] DailyDescent.LeftPanel.Header | text=RULES OF THE DAY
- [ ] DailyDescent.LeftPanel.Header | is_label=true
- [ ] DailyDescent.LeftPanel.IntroText | text=Everyone gets the same seed, hero, and rules. One attempt only.
- [ ] DailyDescent.LeftPanel.IntroText | is_label=true
- [ ] DailyDescent.LeftPanel.HeroRow.Label | text=Hero Selected
- [ ] DailyDescent.LeftPanel.HeroRow.Label | is_label=true
- [ ] DailyDescent.LeftPanel.HeroRow.Value | text=Hero_14
- [ ] DailyDescent.LeftPanel.HeroRow.Value | is_label=true
- [ ] DailyDescent.LeftPanel.DifficultyRow.Label | text=Difficulty
- [ ] DailyDescent.LeftPanel.DifficultyRow.Value | text=Hard
- [ ] DailyDescent.LeftPanel.ModifiersHeader | text=MODIFIERS
- [ ] DailyDescent.LeftPanel.PocketDraftRow.Name | text=Pocket Draft
- [ ] DailyDescent.LeftPanel.PocketDraftRow.Description | text=Begin with 2 random items.
- [ ] DailyDescent.LeftPanel.IronParadeRow.Name | text=Iron Parade
- [ ] DailyDescent.LeftPanel.IronParadeRow.Description | text=All enemies have 50% more HP.
- [ ] DailyDescent.LeftPanel.DoubleDropRow.Name | text=Double Drop
- [ ] DailyDescent.LeftPanel.DoubleDropRow.Description | text=Enemies that drop loot bags drop twice as many.
- [ ] DailyDescent.Title | text=DAILY DESCENT
- [ ] DailyDescent.Title | is_label=true
- [ ] DailyDescent.Subtitle | text=One seed. One attempt. Same puzzle for everyone.
- [ ] DailyDescent.StartButton.Label | text=START DESCENT
- [ ] DailyDescent.StartButton.Badge | text=1
- [ ] DailyDescent.ContinueButton.Label | text=CONTINUE DESCENT
- [ ] DailyDescent.RightLeaderboardPanel.Header | text=DAILY GLOBAL CHAD RANKINGS
- [ ] DailyDescent.RightLeaderboardPanel.Row01.Name | text=CROWNED CHAD
- [ ] DailyDescent.RightLeaderboardPanel.Row01.Score | text=184250
- [ ] DailyDescent.RightLeaderboardPanel.Row05.Name | text=NO HIT NATE
- [ ] DailyDescent.RightLeaderboardPanel.Row10.Name | text=SKULL FARMER
- [ ] DailyDescent.RightLeaderboardPanel.PlayerRow.Rank | text=#42
- [ ] DailyDescent.RightLeaderboardPanel.PlayerRow.Name | text=DOPRA
- [ ] DailyDescent.RightLeaderboardPanel.PlayerRow.Score | text=118700

## Interactivity

- [ ] FrontendTopBar.SettingsButton | has_click_handler=true
- [ ] FrontendTopBar.SettingsButton | hover_capable=true
- [ ] FrontendTopBar.GlobeButton | has_click_handler=true
- [ ] FrontendTopBar.GlobeButton | hover_capable=true
- [ ] FrontendTopBar.BackToMainMenuButton | has_click_handler=true
- [ ] FrontendTopBar.BackToMainMenuButton | hover_capable=true
- [ ] FrontendTopBar.PowerButton | has_click_handler=true
- [ ] FrontendTopBar.PowerButton | hover_capable=true
- [ ] DailyDescent.LeaderboardTabs.GlobalButton | has_click_handler=true
- [ ] DailyDescent.LeaderboardTabs.GlobalButton | hover_capable=true
- [ ] DailyDescent.LeaderboardTabs.GlobalButton | toggle_group=DailyLeaderboardTabs
- [ ] DailyDescent.LeaderboardTabs.FriendsButton | has_click_handler=true
- [ ] DailyDescent.LeaderboardTabs.FriendsButton | hover_capable=true
- [ ] DailyDescent.LeaderboardTabs.FriendsButton | toggle_group=DailyLeaderboardTabs
- [ ] DailyDescent.LeaderboardTabs.BroadcastButton | has_click_handler=true
- [ ] DailyDescent.LeaderboardTabs.BroadcastButton | hover_capable=true
- [ ] DailyDescent.LeaderboardTabs.BroadcastButton | toggle_group=DailyLeaderboardTabs
- [ ] DailyDescent.StartButton | has_click_handler=true
- [ ] DailyDescent.StartButton | hover_capable=true
- [ ] DailyDescent.ContinueButton | has_click_handler=true
- [ ] DailyDescent.ContinueButton | hover_capable=true
- [ ] DailyDescent.LeftPanel.IntroText | is_label=true
- [ ] DailyDescent.LeftPanel.PocketDraftRow.Description | is_label=true
- [ ] DailyDescent.RightLeaderboardPanel.PlayerRow.Name | is_label=true
