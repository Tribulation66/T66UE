# Settings Gameplay Structural Preservation Checklist

Source inventory: `C:\UE\T66\UI\Geometry\settings_gameplay_structural_inventory.md`

Baseline capture: `C:\UE\T66\Saved\Codex\UI\SettingsGameplay\baseline_capture.png`

Baseline dump: `C:\UE\T66\Saved\Codex\UI\SettingsGameplay\baseline_dump.json`

## Structure

- [ ] SettingsGameplay.Root | exists=true
- [ ] SettingsGameplay.Background | exists=true
- [ ] SettingsGameplay.SettingsTabs | exists=true
- [ ] SettingsGameplay.SettingsTabs.GameplayButton | exists=true
- [ ] SettingsGameplay.SettingsTabs.GraphicsButton | exists=true
- [ ] SettingsGameplay.SettingsTabs.ControlsButton | exists=true
- [ ] SettingsGameplay.SettingsTabs.HUDButton | exists=true
- [ ] SettingsGameplay.SettingsTabs.MediaViewerButton | exists=true
- [ ] SettingsGameplay.SettingsTabs.AudioButton | exists=true
- [ ] SettingsGameplay.SettingsTabs.CrashingButton | exists=true
- [ ] SettingsGameplay.SettingsTabs.RetroFXButton | exists=true
- [ ] SettingsGameplay.ContentScroll | exists=true
- [ ] SettingsGameplay.Rows.PracticeMode | exists=true
- [ ] SettingsGameplay.Rows.SubmitAnonymous | exists=true
- [ ] SettingsGameplay.Rows.SpeedRunMode | exists=true
- [ ] SettingsGameplay.Rows.ShowTimeToBeat | exists=true
- [ ] SettingsGameplay.Rows.TimeToBeatSource | exists=true
- [ ] SettingsGameplay.Rows.ShowTimePacing | exists=true
- [ ] SettingsGameplay.Rows.ShowScoreToBeat | exists=true
- [ ] SettingsGameplay.Rows.ScoreToBeatSource | exists=true
- [ ] SettingsGameplay.Rows.ShowScorePacing | exists=true
- [ ] SettingsGameplay.Rows.GoonerMode | exists=true
- [ ] SettingsGameplay.Rows.NativeFogIntensity | exists=true
- [ ] FrontendTopBar.SettingsButton | exists=true
- [ ] FrontendTopBar.TicketBadge.Value | exists=true

## Geometry

- [ ] SettingsGameplay.Root | x=0.000 | 0.020
- [ ] SettingsGameplay.Root | y=0.095 | 0.030
- [ ] SettingsGameplay.Root | w=1.000 | 0.020
- [ ] SettingsGameplay.Root | h=0.905 | 0.030
- [ ] SettingsGameplay.SettingsTabs | x=0.003 | 0.030
- [ ] SettingsGameplay.SettingsTabs | y=0.094 | 0.030
- [ ] SettingsGameplay.SettingsTabs | w=0.994 | 0.030
- [ ] SettingsGameplay.SettingsTabs | h=0.079 | 0.030
- [ ] SettingsGameplay.SettingsTabs.GameplayButton | x=0.003 | 0.030
- [ ] SettingsGameplay.SettingsTabs.GameplayButton | y=0.094 | 0.030
- [ ] SettingsGameplay.SettingsTabs.GameplayButton | w=0.119 | 0.030
- [ ] SettingsGameplay.SettingsTabs.GraphicsButton | x=0.129 | 0.030
- [ ] SettingsGameplay.SettingsTabs.ControlsButton | x=0.253 | 0.030
- [ ] SettingsGameplay.SettingsTabs.HUDButton | x=0.379 | 0.030
- [ ] SettingsGameplay.SettingsTabs.MediaViewerButton | x=0.503 | 0.030
- [ ] SettingsGameplay.SettingsTabs.AudioButton | x=0.628 | 0.030
- [ ] SettingsGameplay.SettingsTabs.CrashingButton | x=0.754 | 0.030
- [ ] SettingsGameplay.SettingsTabs.RetroFXButton | x=0.879 | 0.030
- [ ] SettingsGameplay.ContentScroll | x=0.002 | 0.040
- [ ] SettingsGameplay.ContentScroll | y=0.194 | 0.040
- [ ] SettingsGameplay.ContentScroll | w=0.978 | 0.040
- [ ] SettingsGameplay.ContentScroll | h=0.753 | 0.050
- [ ] SettingsGameplay.Rows.PracticeMode | y=0.194 | 0.050
- [ ] SettingsGameplay.Rows.SubmitAnonymous | y=0.300 | 0.050
- [ ] SettingsGameplay.Rows.SpeedRunMode | y=0.406 | 0.050
- [ ] SettingsGameplay.Rows.ShowTimeToBeat | y=0.512 | 0.050
- [ ] SettingsGameplay.Rows.TimeToBeatSource | y=0.618 | 0.050
- [ ] SettingsGameplay.Rows.ShowTimePacing | y=0.724 | 0.050
- [ ] SettingsGameplay.Rows.ShowScoreToBeat | y=0.830 | 0.060

## Colors

- [ ] SettingsGameplay.SettingsTabs.GameplayButton | button_state=Selected
- [ ] SettingsGameplay.SettingsTabs.GraphicsButton | button_state=Default
- [ ] SettingsGameplay.SettingsTabs.ControlsButton | button_state=Default
- [ ] SettingsGameplay.SettingsTabs.HUDButton | button_state=Default
- [ ] SettingsGameplay.SettingsTabs.MediaViewerButton | button_state=Default
- [ ] SettingsGameplay.SettingsTabs.AudioButton | button_state=Default
- [ ] SettingsGameplay.SettingsTabs.CrashingButton | button_state=Default
- [ ] SettingsGameplay.SettingsTabs.RetroFXButton | button_state=Default
- [ ] SettingsGameplay.Rows.PracticeMode | button_state=Default
- [ ] SettingsGameplay.Rows.SubmitAnonymous | button_state=Default
- [ ] SettingsGameplay.Rows.SpeedRunMode | button_state=Default
- [ ] SettingsGameplay.Rows.TimeToBeatSource.Dropdown | button_state=Selected
- [ ] SettingsGameplay.Rows.ScoreToBeatSource.Dropdown | button_state=Selected
- [ ] SettingsGameplay.Rows.NativeFogIntensity.Slider | button_state=Default

## Content

- [ ] SettingsGameplay.SettingsTabs.GameplayButton | text=GAMEPLAY
- [ ] SettingsGameplay.SettingsTabs.GraphicsButton | text=GRAPHICS
- [ ] SettingsGameplay.SettingsTabs.ControlsButton | text=CONTROLS
- [ ] SettingsGameplay.SettingsTabs.HUDButton | text=HUD
- [ ] SettingsGameplay.SettingsTabs.MediaViewerButton | text=MEDIA VIEWER
- [ ] SettingsGameplay.SettingsTabs.AudioButton | text=AUDIO
- [ ] SettingsGameplay.SettingsTabs.CrashingButton | text=CRASHING
- [ ] SettingsGameplay.SettingsTabs.RetroFXButton | text=RETRO FX
- [ ] SettingsGameplay.Rows.PracticeMode.Label | text=Practice Mode
- [ ] SettingsGameplay.Rows.PracticeMode.Label | is_label=true
- [ ] SettingsGameplay.Rows.SubmitAnonymous.Label | text=Submit Leaderboard as Anonymous
- [ ] SettingsGameplay.Rows.SubmitAnonymous.Label | is_label=true
- [ ] SettingsGameplay.Rows.SpeedRunMode.Label | text=Speed Run Mode
- [ ] SettingsGameplay.Rows.SpeedRunMode.Label | is_label=true
- [ ] SettingsGameplay.Rows.ShowTimeToBeat.Label | text=Show Time to Beat
- [ ] SettingsGameplay.Rows.ShowTimeToBeat.Label | is_label=true
- [ ] SettingsGameplay.Rows.TimeToBeatSource.Label | text=Time to Beat Source
- [ ] SettingsGameplay.Rows.TimeToBeatSource.Label | is_label=true
- [ ] SettingsGameplay.Rows.ShowTimePacing.Label | text=Show Time Pacing (Only for Global)
- [ ] SettingsGameplay.Rows.ShowScoreToBeat.Label | text=Show Score to Beat
- [ ] SettingsGameplay.Rows.ScoreToBeatSource.Label | text=Score to Beat Source
- [ ] SettingsGameplay.Rows.ShowScorePacing.Label | text=Show Score Pacing (Only for Global)
- [ ] SettingsGameplay.Rows.GoonerMode.Label | text=Gooner Mode
- [ ] SettingsGameplay.Rows.NativeFogIntensity.Label | text=Native Fog Intensity
- [ ] SettingsGameplay.Rows.NativeFogIntensity.Description | is_label=true
- [ ] FrontendTopBar.TicketBadge.Value | is_label=true

## Interactivity

- [ ] SettingsGameplay.SettingsTabs.GameplayButton | has_click_handler=true
- [ ] SettingsGameplay.SettingsTabs.GameplayButton | hover_capable=true
- [ ] SettingsGameplay.SettingsTabs.GameplayButton | toggle_group=SettingsTabs
- [ ] SettingsGameplay.SettingsTabs.GraphicsButton | has_click_handler=true
- [ ] SettingsGameplay.SettingsTabs.GraphicsButton | hover_capable=true
- [ ] SettingsGameplay.SettingsTabs.GraphicsButton | toggle_group=SettingsTabs
- [ ] SettingsGameplay.SettingsTabs.ControlsButton | has_click_handler=true
- [ ] SettingsGameplay.SettingsTabs.ControlsButton | hover_capable=true
- [ ] SettingsGameplay.SettingsTabs.ControlsButton | toggle_group=SettingsTabs
- [ ] SettingsGameplay.SettingsTabs.HUDButton | has_click_handler=true
- [ ] SettingsGameplay.SettingsTabs.HUDButton | hover_capable=true
- [ ] SettingsGameplay.SettingsTabs.HUDButton | toggle_group=SettingsTabs
- [ ] SettingsGameplay.SettingsTabs.MediaViewerButton | has_click_handler=true
- [ ] SettingsGameplay.SettingsTabs.MediaViewerButton | hover_capable=true
- [ ] SettingsGameplay.SettingsTabs.MediaViewerButton | toggle_group=SettingsTabs
- [ ] SettingsGameplay.SettingsTabs.AudioButton | has_click_handler=true
- [ ] SettingsGameplay.SettingsTabs.AudioButton | hover_capable=true
- [ ] SettingsGameplay.SettingsTabs.AudioButton | toggle_group=SettingsTabs
- [ ] SettingsGameplay.SettingsTabs.CrashingButton | has_click_handler=true
- [ ] SettingsGameplay.SettingsTabs.CrashingButton | hover_capable=true
- [ ] SettingsGameplay.SettingsTabs.CrashingButton | toggle_group=SettingsTabs
- [ ] SettingsGameplay.SettingsTabs.RetroFXButton | has_click_handler=true
- [ ] SettingsGameplay.SettingsTabs.RetroFXButton | hover_capable=true
- [ ] SettingsGameplay.SettingsTabs.RetroFXButton | toggle_group=SettingsTabs
- [ ] SettingsGameplay.Rows.PracticeMode.OnButton | has_click_handler=true
- [ ] SettingsGameplay.Rows.PracticeMode.OnButton | hover_capable=true
- [ ] SettingsGameplay.Rows.PracticeMode.OnButton | toggle_group=SettingsGameplay.Rows.PracticeMode
- [ ] SettingsGameplay.Rows.PracticeMode.OffButton | has_click_handler=true
- [ ] SettingsGameplay.Rows.PracticeMode.OffButton | hover_capable=true
- [ ] SettingsGameplay.Rows.PracticeMode.OffButton | toggle_group=SettingsGameplay.Rows.PracticeMode
- [ ] SettingsGameplay.Rows.SubmitAnonymous.OnButton | has_click_handler=true
- [ ] SettingsGameplay.Rows.SubmitAnonymous.OnButton | hover_capable=true
- [ ] SettingsGameplay.Rows.SubmitAnonymous.OnButton | toggle_group=SettingsGameplay.Rows.SubmitAnonymous
- [ ] SettingsGameplay.Rows.SubmitAnonymous.OffButton | has_click_handler=true
- [ ] SettingsGameplay.Rows.SubmitAnonymous.OffButton | hover_capable=true
- [ ] SettingsGameplay.Rows.SubmitAnonymous.OffButton | toggle_group=SettingsGameplay.Rows.SubmitAnonymous
- [ ] SettingsGameplay.Rows.SpeedRunMode.OnButton | has_click_handler=true
- [ ] SettingsGameplay.Rows.SpeedRunMode.OnButton | hover_capable=true
- [ ] SettingsGameplay.Rows.SpeedRunMode.OnButton | toggle_group=SettingsGameplay.Rows.SpeedRunMode
- [ ] SettingsGameplay.Rows.SpeedRunMode.OffButton | has_click_handler=true
- [ ] SettingsGameplay.Rows.SpeedRunMode.OffButton | hover_capable=true
- [ ] SettingsGameplay.Rows.SpeedRunMode.OffButton | toggle_group=SettingsGameplay.Rows.SpeedRunMode
- [ ] SettingsGameplay.Rows.TimeToBeatSource.Dropdown | has_click_handler=true
- [ ] SettingsGameplay.Rows.TimeToBeatSource.Dropdown | hover_capable=true
- [ ] SettingsGameplay.Rows.ScoreToBeatSource.Dropdown | has_click_handler=true
- [ ] SettingsGameplay.Rows.ScoreToBeatSource.Dropdown | hover_capable=true
- [ ] SettingsGameplay.Rows.NativeFogIntensity.Slider | has_click_handler=true
- [ ] SettingsGameplay.Rows.NativeFogIntensity.Slider | hover_capable=true
- [ ] SettingsGameplay.Rows.NativeFogIntensity.Value | is_label=true
