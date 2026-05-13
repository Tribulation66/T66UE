# Settings Media Viewer Structural Inventory

Baseline capture: `C:\UE\T66\Saved\Codex\UI\SettingsMediaViewer\baseline_capture.png`

Baseline dump: `C:\UE\T66\Saved\Codex\UI\SettingsMediaViewer\baseline_dump.json`

## Named Regions

| Region | Baseline role | Normalized 1920x1080 box |
| --- | --- | --- |
| SettingsMediaViewer.Root | Full settings surface below verified top bar | x=0.000 y=0.095 w=1.000 h=0.905 |
| SettingsMediaViewer.SettingsTabs | Settings tab toggle row | x=0.003 y=0.094 w=0.994 h=0.079 |
| SettingsMediaViewer.SettingsTabs.GameplayButton | GAMEPLAY tab button | x=0.003 y=0.094 w=0.119 h=0.079 |
| SettingsMediaViewer.SettingsTabs.GraphicsButton | GRAPHICS tab button | x=0.129 y=0.094 w=0.118 h=0.079 |
| SettingsMediaViewer.SettingsTabs.ControlsButton | CONTROLS tab button | x=0.253 y=0.094 w=0.118 h=0.079 |
| SettingsMediaViewer.SettingsTabs.HUDButton | HUD tab button | x=0.379 y=0.094 w=0.118 h=0.079 |
| SettingsMediaViewer.SettingsTabs.MediaViewerButton | MEDIA VIEWER selected tab button | x=0.503 y=0.094 w=0.118 h=0.079 |
| SettingsMediaViewer.SettingsTabs.AudioButton | AUDIO tab button | x=0.628 y=0.094 w=0.118 h=0.079 |
| SettingsMediaViewer.SettingsTabs.CrashingButton | CRASHING tab button | x=0.754 y=0.094 w=0.118 h=0.079 |
| SettingsMediaViewer.SettingsTabs.RetroFXButton | RETRO FX tab button | x=0.879 y=0.094 w=0.118 h=0.079 |
| SettingsMediaViewer.PrivacyBody | Local-only privacy paragraph | x=0.002 y=0.201 w=0.978 h=0.062 |
| SettingsMediaViewer.EnableRow | Enable Media Viewer setting row | x=0.002 y=0.292 w=0.978 h=0.096 |
| SettingsMediaViewer.EnableRow.OnButton | ON toggle | x=0.824 y=0.311 w=0.067 h=0.059 |
| SettingsMediaViewer.EnableRow.OffButton | OFF toggle | x=0.899 y=0.311 w=0.067 h=0.059 |
| SettingsMediaViewer.SourceRow | Default Feed setting row | x=0.002 y=0.399 w=0.978 h=0.126 |
| SettingsMediaViewer.SourceRow.TikTokButton | TIKTOK feed button | x=0.614 y=0.433 w=0.111 h=0.059 |
| SettingsMediaViewer.SourceRow.ShortsButton | YOUTUBE SHORTS feed button | x=0.731 y=0.433 w=0.111 h=0.059 |
| SettingsMediaViewer.SourceRow.ReelsButton | INSTAGRAM REELS feed button | x=0.849 y=0.433 w=0.111 h=0.059 |

## Structural Notes

- The shared frontend top bar is already verified and remains outside this screen's structural region.
- Settings tab buttons form the `SettingsTabs` toggle group.
- Enable ON/OFF buttons form the `SettingsMediaViewer.Enable` toggle group.
- Feed source buttons form the `SettingsMediaViewer.Source` toggle group.
- `PrivacyBody`, row labels, and nested button labels are non-interactive text labels.
