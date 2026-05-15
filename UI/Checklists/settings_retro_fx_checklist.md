# Settings Retro FX UI Fidelity Checklist

Source geometry: `C:\UE\T66\UI\Geometry\settings_retro_fx_reference_geometry.md`

Top bar dependency: `C:\UE\T66\UI\Checklists\frontend_topbar_checklist.md`

Session 2.5 simplification: Retro FX now intentionally exposes only two master toggles, Frontend Retro FX and Gameplay Retro FX. The old per-effect UI rows remain hidden for save compatibility and future advanced exposure.

## Structure

- [ ] FrontendTopBar.Root | exists=true
- [ ] FrontendTopBar.SettingsButton | exists=true
- [ ] FrontendTopBar.TicketBadge.Value | exists=true
- [ ] SettingsRetroFX.Root | exists=true
- [ ] SettingsRetroFX.Theme | exists=true
- [ ] SettingsRetroFX.Theme.SunButton | exists=true
- [ ] SettingsRetroFX.Theme.MoonButton | exists=true
- [ ] SettingsRetroFX.SettingsTabs | exists=true
- [ ] SettingsRetroFX.SettingsTabs.GameplayButton | exists=true
- [ ] SettingsRetroFX.SettingsTabs.GraphicsButton | exists=true
- [ ] SettingsRetroFX.SettingsTabs.ControlsButton | exists=true
- [ ] SettingsRetroFX.SettingsTabs.MediaViewerButton | exists=true
- [ ] SettingsRetroFX.SettingsTabs.AudioButton | exists=true
- [ ] SettingsRetroFX.SettingsTabs.RetroFXButton | exists=true
- [ ] SettingsRetroFX.FrontendPanel | exists=true
- [ ] SettingsRetroFX.FrontendPanel.Label | exists=true
- [ ] SettingsRetroFX.FrontendPanel.OnButton | exists=true
- [ ] SettingsRetroFX.FrontendPanel.OffButton | exists=true
- [ ] SettingsRetroFX.GameplayPanel | exists=true
- [ ] SettingsRetroFX.GameplayPanel.Label | exists=true
- [ ] SettingsRetroFX.GameplayPanel.OnButton | exists=true
- [ ] SettingsRetroFX.GameplayPanel.OffButton | exists=true
- [ ] SettingsRetroFX.ResetButton | exists=true
- [ ] SettingsRetroFX.CancelButton | exists=true
- [ ] SettingsRetroFX.ApplyButton | exists=true

## Geometry

- [ ] SettingsRetroFX.Root | x=0.013 | 0.012
- [ ] SettingsRetroFX.Root | y=0.157 | 0.012
- [ ] SettingsRetroFX.Root | w=0.974 | 0.012
- [ ] SettingsRetroFX.Root | h=0.821 | 0.012
- [ ] SettingsRetroFX.Theme.SunButton | x=0.014 | 0.008
- [ ] SettingsRetroFX.Theme.SunButton | y=0.157 | 0.008
- [ ] SettingsRetroFX.Theme.MoonButton | x=0.073 | 0.008
- [ ] SettingsRetroFX.Theme.MoonButton | y=0.157 | 0.008
- [ ] SettingsRetroFX.SettingsTabs.GameplayButton | x=0.154 | 0.008
- [ ] SettingsRetroFX.SettingsTabs.GameplayButton | y=0.157 | 0.008
- [ ] SettingsRetroFX.SettingsTabs.RetroFXButton | x=0.848 | 0.008
- [ ] SettingsRetroFX.SettingsTabs.RetroFXButton | y=0.157 | 0.008
- [ ] SettingsRetroFX.FrontendPanel | x=0.013 | 0.010
- [ ] SettingsRetroFX.FrontendPanel | y=0.247 | 0.010
- [ ] SettingsRetroFX.FrontendPanel | w=0.974 | 0.012
- [ ] SettingsRetroFX.FrontendPanel | h=0.160 | 0.012
- [ ] SettingsRetroFX.FrontendPanel.OnButton | x=0.598 | 0.010
- [ ] SettingsRetroFX.FrontendPanel.OnButton | y=0.284 | 0.010
- [ ] SettingsRetroFX.FrontendPanel.OffButton | x=0.774 | 0.010
- [ ] SettingsRetroFX.FrontendPanel.OffButton | y=0.284 | 0.010
- [ ] SettingsRetroFX.GameplayPanel | x=0.013 | 0.010
- [ ] SettingsRetroFX.GameplayPanel | y=0.435 | 0.010
- [ ] SettingsRetroFX.GameplayPanel | w=0.974 | 0.012
- [ ] SettingsRetroFX.GameplayPanel | h=0.160 | 0.012
- [ ] SettingsRetroFX.GameplayPanel.OnButton | x=0.598 | 0.010
- [ ] SettingsRetroFX.GameplayPanel.OnButton | y=0.472 | 0.010
- [ ] SettingsRetroFX.GameplayPanel.OffButton | x=0.774 | 0.010
- [ ] SettingsRetroFX.GameplayPanel.OffButton | y=0.472 | 0.010
- [ ] SettingsRetroFX.ResetButton | x=0.448 | 0.010
- [ ] SettingsRetroFX.ResetButton | y=0.860 | 0.010
- [ ] SettingsRetroFX.CancelButton | x=0.624 | 0.010
- [ ] SettingsRetroFX.CancelButton | y=0.860 | 0.010
- [ ] SettingsRetroFX.ApplyButton | x=0.800 | 0.010
- [ ] SettingsRetroFX.ApplyButton | y=0.860 | 0.010

## Colors

- [ ] FrontendTopBar.SettingsButton | button_state=Selected
- [ ] FrontendTopBar.TicketBadge.Value | is_label=true
- [ ] SettingsRetroFX.Theme.SunButton | button_state=Default
- [ ] SettingsRetroFX.Theme.MoonButton | button_state=Selected
- [ ] SettingsRetroFX.SettingsTabs.GameplayButton | button_state=Default
- [ ] SettingsRetroFX.SettingsTabs.GraphicsButton | button_state=Default
- [ ] SettingsRetroFX.SettingsTabs.ControlsButton | button_state=Default
- [ ] SettingsRetroFX.SettingsTabs.MediaViewerButton | button_state=Default
- [ ] SettingsRetroFX.SettingsTabs.AudioButton | button_state=Default
- [ ] SettingsRetroFX.SettingsTabs.RetroFXButton | button_state=Selected
- [ ] SettingsRetroFX.FrontendPanel | border_color=DefaultBorder
- [ ] SettingsRetroFX.FrontendPanel.OnButton | button_state=Selected
- [ ] SettingsRetroFX.FrontendPanel.OffButton | button_state=Default
- [ ] SettingsRetroFX.GameplayPanel | border_color=DefaultBorder
- [ ] SettingsRetroFX.GameplayPanel.OnButton | button_state=Selected
- [ ] SettingsRetroFX.GameplayPanel.OffButton | button_state=Default
- [ ] SettingsRetroFX.ResetButton | button_state=Default
- [ ] SettingsRetroFX.CancelButton | button_state=Default
- [ ] SettingsRetroFX.ApplyButton | button_state=Selected

## Content

- [ ] FrontendTopBar.TicketBadge.Value | text=any | # Dynamic ticket balance.
- [ ] SettingsRetroFX.SettingsTabs.GameplayButton.Label | text=GAMEPLAY
- [ ] SettingsRetroFX.SettingsTabs.GraphicsButton.Label | text=GRAPHICS
- [ ] SettingsRetroFX.SettingsTabs.ControlsButton.Label | text=CONTROLS
- [ ] SettingsRetroFX.SettingsTabs.MediaViewerButton.Label | text=MEDIA VIEWER
- [ ] SettingsRetroFX.SettingsTabs.AudioButton.Label | text=AUDIO
- [ ] SettingsRetroFX.SettingsTabs.RetroFXButton.Label | text=RETRO FX
- [ ] SettingsRetroFX.FrontendPanel.Label | text=FRONTEND RETRO FX
- [ ] SettingsRetroFX.FrontendPanel.OnButton.Label | text=ON
- [ ] SettingsRetroFX.FrontendPanel.OffButton.Label | text=OFF
- [ ] SettingsRetroFX.GameplayPanel.Label | text=GAMEPLAY RETRO FX
- [ ] SettingsRetroFX.GameplayPanel.OnButton.Label | text=ON
- [ ] SettingsRetroFX.GameplayPanel.OffButton.Label | text=OFF
- [ ] SettingsRetroFX.ResetButton.Label | text=RESET
- [ ] SettingsRetroFX.CancelButton.Label | text=CANCEL
- [ ] SettingsRetroFX.ApplyButton.Label | text=APPLY
- [ ] SettingsRetroFX.FrontendPanel.Label | is_label=true
- [ ] SettingsRetroFX.GameplayPanel.Label | is_label=true

## Interactivity

- [ ] SettingsRetroFX.Theme.SunButton | has_click_handler=true
- [ ] SettingsRetroFX.Theme.SunButton | hover_capable=true
- [ ] SettingsRetroFX.Theme.SunButton | toggle_group=ThemeMode
- [ ] SettingsRetroFX.Theme.MoonButton | has_click_handler=true
- [ ] SettingsRetroFX.Theme.MoonButton | hover_capable=true
- [ ] SettingsRetroFX.Theme.MoonButton | toggle_group=ThemeMode
- [ ] SettingsRetroFX.SettingsTabs.GameplayButton | has_click_handler=true
- [ ] SettingsRetroFX.SettingsTabs.GameplayButton | hover_capable=true
- [ ] SettingsRetroFX.SettingsTabs.GameplayButton | toggle_group=SettingsTabs
- [ ] SettingsRetroFX.SettingsTabs.GraphicsButton | has_click_handler=true
- [ ] SettingsRetroFX.SettingsTabs.GraphicsButton | hover_capable=true
- [ ] SettingsRetroFX.SettingsTabs.GraphicsButton | toggle_group=SettingsTabs
- [ ] SettingsRetroFX.SettingsTabs.ControlsButton | has_click_handler=true
- [ ] SettingsRetroFX.SettingsTabs.ControlsButton | hover_capable=true
- [ ] SettingsRetroFX.SettingsTabs.ControlsButton | toggle_group=SettingsTabs
- [ ] SettingsRetroFX.SettingsTabs.MediaViewerButton | has_click_handler=true
- [ ] SettingsRetroFX.SettingsTabs.MediaViewerButton | hover_capable=true
- [ ] SettingsRetroFX.SettingsTabs.MediaViewerButton | toggle_group=SettingsTabs
- [ ] SettingsRetroFX.SettingsTabs.AudioButton | has_click_handler=true
- [ ] SettingsRetroFX.SettingsTabs.AudioButton | hover_capable=true
- [ ] SettingsRetroFX.SettingsTabs.AudioButton | toggle_group=SettingsTabs
- [ ] SettingsRetroFX.SettingsTabs.RetroFXButton | has_click_handler=true
- [ ] SettingsRetroFX.SettingsTabs.RetroFXButton | hover_capable=true
- [ ] SettingsRetroFX.SettingsTabs.RetroFXButton | toggle_group=SettingsTabs
- [ ] SettingsRetroFX.FrontendPanel.OnButton | has_click_handler=true
- [ ] SettingsRetroFX.FrontendPanel.OnButton | hover_capable=true
- [ ] SettingsRetroFX.FrontendPanel.OnButton | toggle_group=RetroFXFrontendEnable
- [ ] SettingsRetroFX.FrontendPanel.OffButton | has_click_handler=true
- [ ] SettingsRetroFX.FrontendPanel.OffButton | hover_capable=true
- [ ] SettingsRetroFX.FrontendPanel.OffButton | toggle_group=RetroFXFrontendEnable
- [ ] SettingsRetroFX.GameplayPanel.OnButton | has_click_handler=true
- [ ] SettingsRetroFX.GameplayPanel.OnButton | hover_capable=true
- [ ] SettingsRetroFX.GameplayPanel.OnButton | toggle_group=RetroFXGameplayEnable
- [ ] SettingsRetroFX.GameplayPanel.OffButton | has_click_handler=true
- [ ] SettingsRetroFX.GameplayPanel.OffButton | hover_capable=true
- [ ] SettingsRetroFX.GameplayPanel.OffButton | toggle_group=RetroFXGameplayEnable
- [ ] SettingsRetroFX.ResetButton | has_click_handler=true
- [ ] SettingsRetroFX.ResetButton | hover_capable=true
- [ ] SettingsRetroFX.CancelButton | has_click_handler=true
- [ ] SettingsRetroFX.CancelButton | hover_capable=true
- [ ] SettingsRetroFX.ApplyButton | has_click_handler=true
- [ ] SettingsRetroFX.ApplyButton | hover_capable=true
