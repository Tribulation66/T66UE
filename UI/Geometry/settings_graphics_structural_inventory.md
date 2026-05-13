# Settings Graphics Structural Inventory

Baseline capture: `C:\UE\T66\Saved\Codex\UI\SettingsGraphics\baseline_capture.png`

Baseline dump: `C:\UE\T66\Saved\Codex\UI\SettingsGraphics\baseline_dump.json`

Normalization basis: `1920x1080`

Method: no-reference structural preservation from the routed Settings Graphics baseline. Top bar chrome is the verified shared component and is only referenced through checklist assertions.

## Structural Regions

| Tag / Region | BBox `(x,y,w,h)` | Tolerance | Role | Text / State |
|---|---:|---:|---|---|
| SettingsGraphics.Root | `(0.000, 0.095, 1.000, 0.905)` | `+/-0.020` | Root | Settings tab content below shared top bar. |
| SettingsGraphics.Background | `(0.000, 0.000, 1.000, 1.000)` | `+/-0.005` | Background | Flat black backing. |
| SettingsGraphics.SettingsTabs | `(0.003, 0.094, 0.994, 0.079)` | `+/-0.020` | Toggle group | Settings tab row. |
| SettingsGraphics.SettingsTabs.GameplayButton | `(0.003, 0.094, 0.119, 0.079)` | `+/-0.020` | Toggle button | `GAMEPLAY`, Default. |
| SettingsGraphics.SettingsTabs.GraphicsButton | `(0.129, 0.094, 0.118, 0.079)` | `+/-0.020` | Toggle button | `GRAPHICS`, Selected. |
| SettingsGraphics.SettingsTabs.ControlsButton | `(0.253, 0.094, 0.118, 0.079)` | `+/-0.020` | Toggle button | `CONTROLS`, Default. |
| SettingsGraphics.SettingsTabs.HUDButton | `(0.379, 0.094, 0.118, 0.079)` | `+/-0.020` | Toggle button | `HUD`, Default. |
| SettingsGraphics.SettingsTabs.MediaViewerButton | `(0.503, 0.094, 0.118, 0.079)` | `+/-0.020` | Toggle button | `MEDIA VIEWER`, Default. |
| SettingsGraphics.SettingsTabs.AudioButton | `(0.628, 0.094, 0.118, 0.079)` | `+/-0.020` | Toggle button | `AUDIO`, Default. |
| SettingsGraphics.SettingsTabs.CrashingButton | `(0.754, 0.094, 0.118, 0.079)` | `+/-0.020` | Toggle button | `CRASHING`, Default. |
| SettingsGraphics.SettingsTabs.RetroFXButton | `(0.879, 0.094, 0.118, 0.079)` | `+/-0.020` | Toggle button | `RETRO FX`, Default. |
| SettingsGraphics.ContentScroll | `(0.002, 0.194, 0.978, 0.753)` | `+/-0.030` | Scroll content | Graphics options list. |
| SettingsGraphics.Rows.Monitor | `(0.002, 0.194, 0.958, 0.121)` | `+/-0.030` | Dropdown row | `Monitor`. |
| SettingsGraphics.Rows.Resolution | `(0.002, 0.325, 0.958, 0.121)` | `+/-0.030` | Dropdown row | `Resolution`. |
| SettingsGraphics.Rows.WindowMode | `(0.002, 0.457, 0.958, 0.121)` | `+/-0.030` | Dropdown row | `Window Mode`. |
| SettingsGraphics.Rows.DisplayMode | `(0.002, 0.589, 0.958, 0.121)` | `+/-0.030` | Dropdown row | `Display Mode`. |
| SettingsGraphics.Rows.UIStyle | `(0.002, 0.721, 0.958, 0.070)` | `+/-0.030` | Static row | `UI Style`, `Unified`. |
| SettingsGraphics.Rows.Quality | `(0.002, 0.802, 0.958, 0.115)` | `+/-0.030` | Slider row | `Best Performance`, `Best Quality`. |
| SettingsGraphics.Rows.FPSCap | `(0.002, 0.927, 0.958, 0.121)` | `+/-0.040` | Dropdown row | `FPS Cap`. |
| SettingsGraphics.Rows.Fog | `(0.002, 1.059, 0.958, 0.095)` | `+/-0.050` | Toggle row | `Fog`. |
| SettingsGraphics.ApplyButton | `(0.840, 1.166, 0.120, 0.067)` | `+/-0.050` | Button | `APPLY`. |

## Interaction Inventory

| Tag / Element | Role | Expected behavior |
|---|---|---|
| SettingsGraphics.SettingsTabs.*Button | Toggle button | `SettingsTabs` group; click switches to the requested settings tab. |
| SettingsGraphics.Rows.Monitor.Dropdown | Dropdown | Selects target monitor and marks pending graphics dirty. |
| SettingsGraphics.Rows.Resolution.Dropdown | Dropdown | Selects staged resolution and marks pending graphics dirty. |
| SettingsGraphics.Rows.WindowMode.Dropdown | Dropdown | Selects staged window mode and marks pending graphics dirty. |
| SettingsGraphics.Rows.DisplayMode.Dropdown | Dropdown | Selects staged display mode and marks pending graphics dirty. |
| SettingsGraphics.Rows.Quality.Slider | Slider | Updates quality notch 0-3. |
| SettingsGraphics.Rows.FPSCap.Dropdown | Dropdown | Selects staged FPS cap. |
| SettingsGraphics.Rows.Fog.OnButton / OffButton | Toggle button | Updates persisted fog setting. |
| SettingsGraphics.ApplyButton | Button | Applies pending graphics settings. |

