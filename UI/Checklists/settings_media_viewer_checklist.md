# Settings Media Viewer Structural Preservation Checklist

Source inventory: `C:\UE\T66\UI\Geometry\settings_media_viewer_structural_inventory.md`

Baseline capture: `C:\UE\T66\Saved\Codex\UI\SettingsMediaViewer\baseline_capture.png`

Baseline dump: `C:\UE\T66\Saved\Codex\UI\SettingsMediaViewer\baseline_dump.json`

## Structure

- [ ] SettingsMediaViewer.Root | exists=true
- [ ] SettingsMediaViewer.Background | exists=true
- [ ] SettingsMediaViewer.SettingsTabs | exists=true
- [ ] SettingsMediaViewer.SettingsTabs.GameplayButton | exists=true
- [ ] SettingsMediaViewer.SettingsTabs.GraphicsButton | exists=true
- [ ] SettingsMediaViewer.SettingsTabs.ControlsButton | exists=true
- [ ] SettingsMediaViewer.SettingsTabs.HUDButton | exists=true
- [ ] SettingsMediaViewer.SettingsTabs.MediaViewerButton | exists=true
- [ ] SettingsMediaViewer.SettingsTabs.AudioButton | exists=true
- [ ] SettingsMediaViewer.SettingsTabs.CrashingButton | exists=true
- [ ] SettingsMediaViewer.SettingsTabs.RetroFXButton | exists=true
- [ ] SettingsMediaViewer.PrivacyBody | exists=true
- [ ] SettingsMediaViewer.EnableRow | exists=true
- [ ] SettingsMediaViewer.EnableRow.OnButton | exists=true
- [ ] SettingsMediaViewer.EnableRow.OffButton | exists=true
- [ ] SettingsMediaViewer.SourceRow | exists=true
- [ ] SettingsMediaViewer.SourceRow.TikTokButton | exists=true
- [ ] SettingsMediaViewer.SourceRow.ShortsButton | exists=true
- [ ] SettingsMediaViewer.SourceRow.ReelsButton | exists=true
- [ ] FrontendTopBar.SettingsButton | exists=true
- [ ] FrontendTopBar.TicketBadge.Value | exists=true

## Geometry

- [ ] SettingsMediaViewer.Root | x=0.000 | 0.020
- [ ] SettingsMediaViewer.Root | y=0.095 | 0.030
- [ ] SettingsMediaViewer.Root | w=1.000 | 0.020
- [ ] SettingsMediaViewer.Root | h=0.905 | 0.030
- [ ] SettingsMediaViewer.SettingsTabs | x=0.003 | 0.030
- [ ] SettingsMediaViewer.SettingsTabs | y=0.094 | 0.030
- [ ] SettingsMediaViewer.SettingsTabs | w=0.994 | 0.030
- [ ] SettingsMediaViewer.SettingsTabs | h=0.079 | 0.030
- [ ] SettingsMediaViewer.SettingsTabs.GameplayButton | x=0.003 | 0.030
- [ ] SettingsMediaViewer.SettingsTabs.GraphicsButton | x=0.129 | 0.030
- [ ] SettingsMediaViewer.SettingsTabs.ControlsButton | x=0.253 | 0.030
- [ ] SettingsMediaViewer.SettingsTabs.HUDButton | x=0.379 | 0.030
- [ ] SettingsMediaViewer.SettingsTabs.MediaViewerButton | x=0.503 | 0.030
- [ ] SettingsMediaViewer.SettingsTabs.AudioButton | x=0.628 | 0.030
- [ ] SettingsMediaViewer.SettingsTabs.CrashingButton | x=0.754 | 0.030
- [ ] SettingsMediaViewer.SettingsTabs.RetroFXButton | x=0.879 | 0.030
- [ ] SettingsMediaViewer.PrivacyBody | y=0.201 | 0.040
- [ ] SettingsMediaViewer.EnableRow | y=0.292 | 0.040
- [ ] SettingsMediaViewer.EnableRow | h=0.096 | 0.040
- [ ] SettingsMediaViewer.SourceRow | y=0.399 | 0.040
- [ ] SettingsMediaViewer.SourceRow | h=0.126 | 0.040
- [ ] SettingsMediaViewer.EnableRow.OnButton | x=0.824 | 0.050
- [ ] SettingsMediaViewer.EnableRow.OffButton | x=0.899 | 0.050
- [ ] SettingsMediaViewer.SourceRow.TikTokButton | x=0.614 | 0.060
- [ ] SettingsMediaViewer.SourceRow.ShortsButton | x=0.731 | 0.060
- [ ] SettingsMediaViewer.SourceRow.ReelsButton | x=0.849 | 0.060

## Colors

- [ ] SettingsMediaViewer.SettingsTabs.GameplayButton | button_state=Default
- [ ] SettingsMediaViewer.SettingsTabs.GraphicsButton | button_state=Default
- [ ] SettingsMediaViewer.SettingsTabs.ControlsButton | button_state=Default
- [ ] SettingsMediaViewer.SettingsTabs.HUDButton | button_state=Default
- [ ] SettingsMediaViewer.SettingsTabs.MediaViewerButton | button_state=Selected
- [ ] SettingsMediaViewer.SettingsTabs.AudioButton | button_state=Default
- [ ] SettingsMediaViewer.SettingsTabs.CrashingButton | button_state=Default
- [ ] SettingsMediaViewer.SettingsTabs.RetroFXButton | button_state=Default
- [ ] SettingsMediaViewer.EnableRow.OnButton | button_state=Selected
- [ ] SettingsMediaViewer.EnableRow.OffButton | button_state=Default
- [ ] SettingsMediaViewer.SourceRow.TikTokButton | button_state=Selected
- [ ] SettingsMediaViewer.SourceRow.ShortsButton | button_state=Default
- [ ] SettingsMediaViewer.SourceRow.ReelsButton | button_state=Default

## Content

- [ ] SettingsMediaViewer.SettingsTabs.GameplayButton | text=GAMEPLAY
- [ ] SettingsMediaViewer.SettingsTabs.GraphicsButton | text=GRAPHICS
- [ ] SettingsMediaViewer.SettingsTabs.ControlsButton | text=CONTROLS
- [ ] SettingsMediaViewer.SettingsTabs.HUDButton | text=HUD
- [ ] SettingsMediaViewer.SettingsTabs.MediaViewerButton | text=MEDIA VIEWER
- [ ] SettingsMediaViewer.SettingsTabs.AudioButton | text=AUDIO
- [ ] SettingsMediaViewer.SettingsTabs.CrashingButton | text=CRASHING
- [ ] SettingsMediaViewer.SettingsTabs.RetroFXButton | text=RETRO FX
- [ ] SettingsMediaViewer.PrivacyBody | is_label=true
- [ ] SettingsMediaViewer.EnableRow.Label | text=Enable Media Viewer
- [ ] SettingsMediaViewer.EnableRow.Label | is_label=true
- [ ] SettingsMediaViewer.EnableRow.OnButton | text=ON
- [ ] SettingsMediaViewer.EnableRow.OffButton | text=OFF
- [ ] SettingsMediaViewer.SourceRow.Label | text=Default Feed
- [ ] SettingsMediaViewer.SourceRow.Label | is_label=true
- [ ] SettingsMediaViewer.SourceRow.TikTokButton | text=TIKTOK
- [ ] SettingsMediaViewer.SourceRow.ShortsButton | text=YOUTUBE SHORTS
- [ ] SettingsMediaViewer.SourceRow.ReelsButton | text=INSTAGRAM REELS
- [ ] FrontendTopBar.TicketBadge.Value | is_label=true

## Interactivity

- [ ] SettingsMediaViewer.SettingsTabs.GameplayButton | has_click_handler=true
- [ ] SettingsMediaViewer.SettingsTabs.GameplayButton | hover_capable=true
- [ ] SettingsMediaViewer.SettingsTabs.GameplayButton | toggle_group=SettingsTabs
- [ ] SettingsMediaViewer.SettingsTabs.GraphicsButton | has_click_handler=true
- [ ] SettingsMediaViewer.SettingsTabs.GraphicsButton | hover_capable=true
- [ ] SettingsMediaViewer.SettingsTabs.GraphicsButton | toggle_group=SettingsTabs
- [ ] SettingsMediaViewer.SettingsTabs.ControlsButton | has_click_handler=true
- [ ] SettingsMediaViewer.SettingsTabs.ControlsButton | hover_capable=true
- [ ] SettingsMediaViewer.SettingsTabs.ControlsButton | toggle_group=SettingsTabs
- [ ] SettingsMediaViewer.SettingsTabs.HUDButton | has_click_handler=true
- [ ] SettingsMediaViewer.SettingsTabs.HUDButton | hover_capable=true
- [ ] SettingsMediaViewer.SettingsTabs.HUDButton | toggle_group=SettingsTabs
- [ ] SettingsMediaViewer.SettingsTabs.MediaViewerButton | has_click_handler=true
- [ ] SettingsMediaViewer.SettingsTabs.MediaViewerButton | hover_capable=true
- [ ] SettingsMediaViewer.SettingsTabs.MediaViewerButton | toggle_group=SettingsTabs
- [ ] SettingsMediaViewer.SettingsTabs.AudioButton | has_click_handler=true
- [ ] SettingsMediaViewer.SettingsTabs.AudioButton | hover_capable=true
- [ ] SettingsMediaViewer.SettingsTabs.AudioButton | toggle_group=SettingsTabs
- [ ] SettingsMediaViewer.SettingsTabs.CrashingButton | has_click_handler=true
- [ ] SettingsMediaViewer.SettingsTabs.CrashingButton | hover_capable=true
- [ ] SettingsMediaViewer.SettingsTabs.CrashingButton | toggle_group=SettingsTabs
- [ ] SettingsMediaViewer.SettingsTabs.RetroFXButton | has_click_handler=true
- [ ] SettingsMediaViewer.SettingsTabs.RetroFXButton | hover_capable=true
- [ ] SettingsMediaViewer.SettingsTabs.RetroFXButton | toggle_group=SettingsTabs
- [ ] SettingsMediaViewer.EnableRow.OnButton | has_click_handler=true
- [ ] SettingsMediaViewer.EnableRow.OnButton | hover_capable=true
- [ ] SettingsMediaViewer.EnableRow.OnButton | toggle_group=SettingsMediaViewer.Enable
- [ ] SettingsMediaViewer.EnableRow.OffButton | has_click_handler=true
- [ ] SettingsMediaViewer.EnableRow.OffButton | hover_capable=true
- [ ] SettingsMediaViewer.EnableRow.OffButton | toggle_group=SettingsMediaViewer.Enable
- [ ] SettingsMediaViewer.SourceRow.TikTokButton | has_click_handler=true
- [ ] SettingsMediaViewer.SourceRow.TikTokButton | hover_capable=true
- [ ] SettingsMediaViewer.SourceRow.TikTokButton | toggle_group=SettingsMediaViewer.Source
- [ ] SettingsMediaViewer.SourceRow.ShortsButton | has_click_handler=true
- [ ] SettingsMediaViewer.SourceRow.ShortsButton | hover_capable=true
- [ ] SettingsMediaViewer.SourceRow.ShortsButton | toggle_group=SettingsMediaViewer.Source
- [ ] SettingsMediaViewer.SourceRow.ReelsButton | has_click_handler=true
- [ ] SettingsMediaViewer.SourceRow.ReelsButton | hover_capable=true
- [ ] SettingsMediaViewer.SourceRow.ReelsButton | toggle_group=SettingsMediaViewer.Source
