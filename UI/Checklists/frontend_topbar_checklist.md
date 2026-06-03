# Frontend Top Bar UI Fidelity Checklist

Source geometry: `C:\UE\T66\UI\Geometry\frontend_topbar_reference_geometry.md`

## Structure

- [ ] FrontendTopBar.Root | exists=true
- [ ] FrontendTopBar.OuterContainer | exists=true
- [ ] FrontendTopBar.SettingsButton | exists=true
- [ ] FrontendTopBar.SettingsButton.Icon | exists=true
- [ ] FrontendTopBar.GlobeButton | exists=true
- [ ] FrontendTopBar.GlobeButton.Icon | exists=true
- [ ] FrontendTopBar.AccountButton | exists=true
- [ ] FrontendTopBar.ProfileButton | exists=true
- [ ] FrontendTopBar.ProfileButton.Icon | exists=true
- [ ] FrontendTopBar.PowerUpButton | exists=true
- [ ] FrontendTopBar.AchievementsButton | exists=true
- [ ] FrontendTopBar.TicketBadge | exists=true
- [ ] FrontendTopBar.TicketBadge.Icon | exists=true
- [ ] FrontendTopBar.TicketBadge.Value | exists=true
- [ ] FrontendTopBar.PowerButton | exists=true
- [ ] FrontendTopBar.PowerButton.Icon | exists=true

## Geometry

- [ ] FrontendTopBar.OuterContainer | x=0.012 | 0.010
- [ ] FrontendTopBar.OuterContainer | y=0.018 | 0.010
- [ ] FrontendTopBar.OuterContainer | w=0.976 | 0.010
- [ ] FrontendTopBar.OuterContainer | h=0.077 | 0.010
- [ ] FrontendTopBar.SettingsButton | x=0.013 | 0.006
- [ ] FrontendTopBar.SettingsButton | y=0.018 | 0.006
- [ ] FrontendTopBar.SettingsButton | w=0.046 | 0.006
- [ ] FrontendTopBar.SettingsButton | h=0.077 | 0.006
- [ ] FrontendTopBar.GlobeButton | x=0.073 | 0.006
- [ ] FrontendTopBar.GlobeButton | y=0.018 | 0.006
- [ ] FrontendTopBar.GlobeButton | w=0.046 | 0.006
- [ ] FrontendTopBar.GlobeButton | h=0.077 | 0.006
- [ ] FrontendTopBar.AccountButton | x=0.133 | 0.006
- [ ] FrontendTopBar.AccountButton | y=0.018 | 0.006
- [ ] FrontendTopBar.AccountButton | w=0.156 | 0.006
- [ ] FrontendTopBar.AccountButton | h=0.077 | 0.006
- [ ] FrontendTopBar.ProfileButton | x=0.302 | 0.006
- [ ] FrontendTopBar.ProfileButton | y=0.018 | 0.006
- [ ] FrontendTopBar.ProfileButton | w=0.043 | 0.006
- [ ] FrontendTopBar.ProfileButton | h=0.077 | 0.006
- [ ] FrontendTopBar.PowerUpButton | x=0.359 | 0.006
- [ ] FrontendTopBar.PowerUpButton | y=0.018 | 0.006
- [ ] FrontendTopBar.PowerUpButton | w=0.142 | 0.006
- [ ] FrontendTopBar.PowerUpButton | h=0.077 | 0.006
- [ ] FrontendTopBar.AchievementsButton | x=0.513 | 0.006
- [ ] FrontendTopBar.AchievementsButton | y=0.018 | 0.006
- [ ] FrontendTopBar.AchievementsButton | w=0.153 | 0.006
- [ ] FrontendTopBar.AchievementsButton | h=0.077 | 0.006
- [ ] FrontendTopBar.TicketBadge | x=0.819 | 0.006
- [ ] FrontendTopBar.TicketBadge | y=0.018 | 0.006
- [ ] FrontendTopBar.TicketBadge | w=0.087 | 0.006
- [ ] FrontendTopBar.TicketBadge | h=0.077 | 0.006
- [ ] FrontendTopBar.TicketBadge.Icon | x=0.842 | 0.010
- [ ] FrontendTopBar.TicketBadge.Icon | y=0.041 | 0.010
- [ ] FrontendTopBar.TicketBadge.Icon | w=0.019 | 0.010
- [ ] FrontendTopBar.TicketBadge.Icon | h=0.032 | 0.010
- [ ] FrontendTopBar.TicketBadge.Value | x=0.870 | 0.012
- [ ] FrontendTopBar.TicketBadge.Value | y=0.043 | 0.012
- [ ] FrontendTopBar.TicketBadge.Value | w=0.025 | 0.012
- [ ] FrontendTopBar.TicketBadge.Value | h=0.028 | 0.012
- [ ] FrontendTopBar.PowerButton | x=0.923 | 0.006
- [ ] FrontendTopBar.PowerButton | y=0.018 | 0.006
- [ ] FrontendTopBar.PowerButton | w=0.063 | 0.006
- [ ] FrontendTopBar.PowerButton | h=0.077 | 0.006

## Colors

- [ ] FrontendTopBar.SettingsButton | border_color=DefaultBorder
- [ ] FrontendTopBar.GlobeButton | border_color=DefaultBorder
- [ ] FrontendTopBar.AccountButton | border_color=DefaultBorder
- [ ] FrontendTopBar.ProfileButton | border_color=DefaultBorder
- [ ] FrontendTopBar.PowerUpButton | border_color=DefaultBorder
- [ ] FrontendTopBar.AchievementsButton | border_color=DefaultBorder
- [ ] FrontendTopBar.TicketBadge | border_color=DefaultBorder
- [ ] FrontendTopBar.PowerButton | border_color=SelectedBorder

## Content

- [ ] FrontendTopBar.AccountButton | text=ACCOUNT
- [ ] FrontendTopBar.PowerUpButton | text=POWER UP
- [ ] FrontendTopBar.AchievementsButton | text=ACHIEVEMENTS
- [ ] FrontendTopBar.TicketBadge.Value | text=any | # Dynamic ticket balance; reference shows 10.
- [ ] FrontendTopBar.TicketBadge.Value | is_label=true
- [ ] FrontendTopBar.AccountButton | button_state=Default
- [ ] FrontendTopBar.PowerUpButton | button_state=Default
- [ ] FrontendTopBar.AchievementsButton | button_state=Default
- [ ] FrontendTopBar.PowerButton | button_state=Selected

## Interactivity

- [ ] FrontendTopBar.SettingsButton | has_click_handler=true
- [ ] FrontendTopBar.SettingsButton | hover_capable=true
- [ ] FrontendTopBar.GlobeButton | has_click_handler=true
- [ ] FrontendTopBar.GlobeButton | hover_capable=true
- [ ] FrontendTopBar.ProfileButton | has_click_handler=true
- [ ] FrontendTopBar.ProfileButton | hover_capable=true
- [ ] FrontendTopBar.AccountButton | has_click_handler=true
- [ ] FrontendTopBar.AccountButton | hover_capable=true
- [ ] FrontendTopBar.PowerUpButton | has_click_handler=true
- [ ] FrontendTopBar.PowerUpButton | hover_capable=true
- [ ] FrontendTopBar.AchievementsButton | has_click_handler=true
- [ ] FrontendTopBar.AchievementsButton | hover_capable=true
- [ ] FrontendTopBar.TicketBadge | has_click_handler=true
- [ ] FrontendTopBar.TicketBadge | hover_capable=true
- [ ] FrontendTopBar.PowerButton | has_click_handler=true
- [ ] FrontendTopBar.PowerButton | hover_capable=true
- [ ] FrontendTopBar.AccountButton | toggle_group=FrontendCategorySelection
- [ ] FrontendTopBar.PowerUpButton | toggle_group=FrontendCategorySelection
- [ ] FrontendTopBar.AchievementsButton | toggle_group=FrontendCategorySelection
- [ ] FrontendTopBar.SettingsButton | toggle_group=none
- [ ] FrontendTopBar.GlobeButton | toggle_group=none
- [ ] FrontendTopBar.ProfileButton | toggle_group=none
- [ ] FrontendTopBar.PowerButton | toggle_group=none
- [ ] FrontendTopBar.Root | exists=true | # SetActiveSection API contract: hosting screens drive active category; Overview renders Account as Default because sub-tabs own Selected state.
