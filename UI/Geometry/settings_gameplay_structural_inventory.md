# Settings Gameplay Structural Inventory

Baseline capture: `C:\UE\T66\Saved\Codex\UI\SettingsGameplay\baseline_capture.png`

Baseline dump: `C:\UE\T66\Saved\Codex\UI\SettingsGameplay\baseline_dump.json`

Normalization basis: `1920x1080`

Method: no-reference structural preservation from the routed Settings Gameplay baseline. The top bar is treated as the already-verified shared component and is not restated here except through checklist assertions.

## Structural Regions

| Tag / Region | BBox `(x,y,w,h)` | Tolerance | Role | Text / State |
|---|---:|---:|---|---|
| SettingsGameplay.Root | `(0.000, 0.095, 1.000, 0.905)` | `+/-0.020` | Root | Settings tab content below shared top bar. |
| SettingsGameplay.Background | `(0.000, 0.000, 1.000, 1.000)` | `+/-0.005` | Background | Flat black page backing. |
| SettingsGameplay.SettingsTabs | `(0.003, 0.094, 0.994, 0.079)` | `+/-0.020` | Toggle group | Settings tab row. |
| SettingsGameplay.SettingsTabs.GameplayButton | `(0.003, 0.094, 0.119, 0.079)` | `+/-0.020` | Toggle button | `GAMEPLAY`, Selected. |
| SettingsGameplay.SettingsTabs.GraphicsButton | `(0.129, 0.094, 0.118, 0.079)` | `+/-0.020` | Toggle button | `GRAPHICS`, Default. |
| SettingsGameplay.SettingsTabs.ControlsButton | `(0.253, 0.094, 0.118, 0.079)` | `+/-0.020` | Toggle button | `CONTROLS`, Default. |
| SettingsGameplay.SettingsTabs.HUDButton | `(0.379, 0.094, 0.118, 0.079)` | `+/-0.020` | Toggle button | `HUD`, Default. |
| SettingsGameplay.SettingsTabs.MediaViewerButton | `(0.503, 0.094, 0.118, 0.079)` | `+/-0.020` | Toggle button | `MEDIA VIEWER`, Default. |
| SettingsGameplay.SettingsTabs.AudioButton | `(0.628, 0.094, 0.118, 0.079)` | `+/-0.020` | Toggle button | `AUDIO`, Default. |
| SettingsGameplay.SettingsTabs.CrashingButton | `(0.754, 0.094, 0.118, 0.079)` | `+/-0.020` | Toggle button | `CRASHING`, Default. |
| SettingsGameplay.SettingsTabs.RetroFXButton | `(0.879, 0.094, 0.118, 0.079)` | `+/-0.020` | Toggle button | `RETRO FX`, Default. |
| SettingsGameplay.ContentScroll | `(0.002, 0.194, 0.978, 0.753)` | `+/-0.030` | Scroll content | Gameplay options list. |
| SettingsGameplay.Rows.PracticeMode | `(0.002, 0.194, 0.958, 0.095)` | `+/-0.030` | Toggle row | `Practice Mode`. |
| SettingsGameplay.Rows.SubmitAnonymous | `(0.002, 0.300, 0.958, 0.095)` | `+/-0.030` | Toggle row | `Submit Leaderboard as Anonymous`. |
| SettingsGameplay.Rows.SpeedRunMode | `(0.002, 0.406, 0.958, 0.095)` | `+/-0.030` | Toggle row | `Speed Run Mode`. |
| SettingsGameplay.Rows.ShowTimeToBeat | `(0.002, 0.512, 0.958, 0.095)` | `+/-0.030` | Toggle row | `Show Time to Beat`. |
| SettingsGameplay.Rows.TimeToBeatSource | `(0.002, 0.618, 0.958, 0.095)` | `+/-0.030` | Dropdown row | `Time to Beat Source`. |
| SettingsGameplay.Rows.ShowTimePacing | `(0.002, 0.724, 0.958, 0.095)` | `+/-0.030` | Toggle row | `Show Time Pacing (Only for Global)`. |
| SettingsGameplay.Rows.ShowScoreToBeat | `(0.002, 0.830, 0.958, 0.095)` | `+/-0.030` | Toggle row | `Show Score to Beat`. |
| SettingsGameplay.Rows.ScoreToBeatSource | `(0.002, 0.936, 0.958, 0.095)` | `+/-0.030` | Dropdown row | `Score to Beat Source`. |
| SettingsGameplay.Rows.ShowScorePacing | `(0.002, 1.042, 0.958, 0.095)` | `+/-0.040` | Toggle row | `Show Score Pacing (Only for Global)`. |
| SettingsGameplay.Rows.GoonerMode | `(0.002, 1.148, 0.958, 0.095)` | `+/-0.040` | Toggle row | `Gooner Mode`. |
| SettingsGameplay.Rows.NativeFogIntensity | `(0.002, 1.254, 0.958, 0.120)` | `+/-0.050` | Slider row | `Native Fog Intensity`. |

## Interaction Inventory

| Tag / Element | Role | Expected behavior |
|---|---|---|
| SettingsGameplay.SettingsTabs.*Button | Toggle button | `SettingsTabs` group; click switches to the requested settings tab. |
| SettingsGameplay.Rows.*.OnButton / OffButton | Toggle button | Per-row two-choice toggle group; click updates `UT66PlayerSettingsSubsystem` and rebuilds state. |
| SettingsGameplay.Rows.TimeToBeatSource.Dropdown | Dropdown | Opens target-source choices and updates time target selection. |
| SettingsGameplay.Rows.ScoreToBeatSource.Dropdown | Dropdown | Opens target-source choices and updates score target selection. |
| SettingsGameplay.Rows.NativeFogIntensity.Slider | Slider | Updates native fog intensity in player settings. |

