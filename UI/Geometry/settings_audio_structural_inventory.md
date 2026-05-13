# Settings Audio Structural Inventory

Baseline capture: `C:\UE\T66\Saved\Codex\UI\SettingsAudio\baseline_capture.png`

Baseline dump: `C:\UE\T66\Saved\Codex\UI\SettingsAudio\baseline_dump.json`

## Named Regions

| Region | Baseline role | Normalized 1920x1080 box |
| --- | --- | --- |
| SettingsAudio.Root | Full settings surface below verified top bar | x=0.000 y=0.095 w=1.000 h=0.905 |
| SettingsAudio.SettingsTabs | Settings tab toggle row | x=0.003 y=0.094 w=0.994 h=0.079 |
| SettingsAudio.SettingsTabs.GameplayButton | GAMEPLAY tab button | x=0.003 y=0.094 w=0.119 h=0.079 |
| SettingsAudio.SettingsTabs.GraphicsButton | GRAPHICS tab button | x=0.129 y=0.094 w=0.118 h=0.079 |
| SettingsAudio.SettingsTabs.ControlsButton | CONTROLS tab button | x=0.253 y=0.094 w=0.118 h=0.079 |
| SettingsAudio.SettingsTabs.HUDButton | HUD tab button | x=0.379 y=0.094 w=0.118 h=0.079 |
| SettingsAudio.SettingsTabs.MediaViewerButton | MEDIA VIEWER tab button | x=0.503 y=0.094 w=0.118 h=0.079 |
| SettingsAudio.SettingsTabs.AudioButton | AUDIO selected tab button | x=0.628 y=0.094 w=0.118 h=0.079 |
| SettingsAudio.SettingsTabs.CrashingButton | CRASHING tab button | x=0.754 y=0.094 w=0.118 h=0.079 |
| SettingsAudio.SettingsTabs.RetroFXButton | RETRO FX tab button | x=0.879 y=0.094 w=0.118 h=0.079 |
| SettingsAudio.Rows.MasterVolume | Master Volume slider row | x=0.002 y=0.193 w=0.978 h=0.108 |
| SettingsAudio.Rows.MusicVolume | Music Volume slider row | x=0.002 y=0.313 w=0.978 h=0.108 |
| SettingsAudio.Rows.SFXVolume | SFX Volume slider row | x=0.002 y=0.433 w=0.978 h=0.108 |
| SettingsAudio.Rows.MuteWhenUnfocused | Mute when unfocused toggle row | x=0.002 y=0.567 w=0.978 h=0.092 |
| SettingsAudio.Rows.OutputDevice | Output Device dropdown row | x=0.002 y=0.672 w=0.978 h=0.120 |
| SettingsAudio.Rows.SubtitlesAlwaysOn | Subtitles always on toggle row | x=0.002 y=0.802 w=0.978 h=0.092 |

## Structural Notes

- The shared frontend top bar is already verified and remains outside this screen's structural region.
- Settings tab buttons form the `SettingsTabs` toggle group.
- Master, Music, and SFX sliders are interactive and write through `UT66PlayerSettingsSubsystem`.
- Mute and subtitles use single toggle buttons matching the baseline structure.
- Output Device is a dropdown with a Default option in the current implementation.
