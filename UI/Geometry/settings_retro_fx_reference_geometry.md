# Settings Retro FX Reference Geometry

Reference image: `C:\UE\T66\UI\Screen References\Settings.png`

Native reference resolution: `1672x941`

Normalization basis: values below are `(x, y, w, h)` normalized to the same 1920x1080 basis used by `T66.UI.DumpScreen`; because the source image is 16:9, this is equivalent to `(x/1672, y/941, w/1672, h/941)`.

Measurement method: visual inspection of the Settings V3 reference image at native resolution. Top bar chrome is intentionally excluded because `UT66FrontendTopBarWidget` has its own Stage 2 geometry table.

## Overall Extents

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| SettingsRetroFX.Root | `(0.013, 0.157, 0.974, 0.821)` | `+/-0.012` | All Settings Retro FX-owned UI below the shared top bar. |
| SettingsRetroFX.Theme | `(0.014, 0.157, 0.109, 0.067)` | `+/-0.010` | Sun/Moon theme toggle group on the left. |
| SettingsRetroFX.SettingsTabs | `(0.154, 0.157, 0.824, 0.067)` | `+/-0.010` | Settings tab toggle group. |

## Theme Toggles

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| SettingsRetroFX.Theme.SunButton | `(0.014, 0.157, 0.050, 0.067)` | `+/-0.008` | Sun toggle, Default. |
| SettingsRetroFX.Theme.MoonButton | `(0.073, 0.157, 0.050, 0.067)` | `+/-0.008` | Moon toggle, Selected. |

## Settings Tabs

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| SettingsRetroFX.SettingsTabs.GameplayButton | `(0.154, 0.157, 0.127, 0.067)` | `+/-0.008` | GAMEPLAY tab, Default. |
| SettingsRetroFX.SettingsTabs.GraphicsButton | `(0.295, 0.157, 0.126, 0.067)` | `+/-0.008` | GRAPHICS tab, Default. |
| SettingsRetroFX.SettingsTabs.ControlsButton | `(0.434, 0.157, 0.124, 0.067)` | `+/-0.008` | CONTROLS tab, Default. |
| SettingsRetroFX.SettingsTabs.MediaViewerButton | `(0.572, 0.157, 0.132, 0.067)` | `+/-0.008` | MEDIA VIEWER tab, Default. |
| SettingsRetroFX.SettingsTabs.AudioButton | `(0.717, 0.157, 0.117, 0.067)` | `+/-0.008` | AUDIO tab, Default. |
| SettingsRetroFX.SettingsTabs.RetroFXButton | `(0.848, 0.157, 0.130, 0.067)` | `+/-0.008` | RETRO FX tab, Selected. |

## Master Panel

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| SettingsRetroFX.MasterPanel | `(0.013, 0.247, 0.974, 0.181)` | `+/-0.010` | RETRO FX MASTER ENABLE outer panel. |
| SettingsRetroFX.MasterPanel.Header | `(0.033, 0.268, 0.440, 0.040)` | `+/-0.012` | Header label. |
| SettingsRetroFX.MasterPanel.Description | `(0.033, 0.318, 0.615, 0.034)` | `+/-0.012` | Description label. |
| SettingsRetroFX.MasterPanel.StatusNote | `(0.033, 0.374, 0.520, 0.034)` | `+/-0.012` | Status note label. |
| SettingsRetroFX.MasterPanel.Controls | `(0.712, 0.268, 0.255, 0.143)` | `+/-0.012` | ON/OFF/APPLY control cluster. |
| SettingsRetroFX.MasterPanel.OnButton | `(0.712, 0.268, 0.128, 0.064)` | `+/-0.008` | ON button, Selected. |
| SettingsRetroFX.MasterPanel.OffButton | `(0.849, 0.268, 0.118, 0.064)` | `+/-0.008` | OFF button, Default. |
| SettingsRetroFX.MasterPanel.ApplyButton | `(0.712, 0.347, 0.255, 0.064)` | `+/-0.008` | APPLY button, Selected. |

## UI Panel

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| SettingsRetroFX.UIPanel | `(0.013, 0.442, 0.974, 0.159)` | `+/-0.010` | UI description panel. |
| SettingsRetroFX.UIPanel.Header | `(0.034, 0.462, 0.120, 0.042)` | `+/-0.012` | UI header label. |
| SettingsRetroFX.UIPanel.Description | `(0.034, 0.505, 0.860, 0.082)` | `+/-0.014` | Multi-line UI description. |

## UI Chrome Panel

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| SettingsRetroFX.UIChromePanel | `(0.013, 0.615, 0.974, 0.363)` | `+/-0.010` | UI CHROME outer panel. |
| SettingsRetroFX.UIChromePanel.Header | `(0.034, 0.636, 0.180, 0.042)` | `+/-0.012` | UI CHROME header label. |
| SettingsRetroFX.UIChromePanel.Description | `(0.034, 0.678, 0.560, 0.034)` | `+/-0.012` | UI chrome description label. |
| SettingsRetroFX.UIChromePanel.PixelationSubPanel | `(0.035, 0.706, 0.932, 0.110)` | `+/-0.010` | CHROME PIXELATION slider row panel. |
| SettingsRetroFX.UIChromePanel.Pixelation.Label | `(0.061, 0.727, 0.260, 0.036)` | `+/-0.012` | Pixelation label. |
| SettingsRetroFX.UIChromePanel.Pixelation.Description | `(0.061, 0.771, 0.300, 0.030)` | `+/-0.012` | Pixelation description. |
| SettingsRetroFX.UIChromePanel.Pixelation.Slider | `(0.374, 0.737, 0.566, 0.042)` | `+/-0.012` | Pixelation slider, handle/value at 0. |
| SettingsRetroFX.UIChromePanel.Pixelation.Caption | `(0.374, 0.788, 0.150, 0.026)` | `+/-0.012` | Pixelation slider caption. |
| SettingsRetroFX.UIChromePanel.DitheringSubPanel | `(0.035, 0.826, 0.932, 0.121)` | `+/-0.010` | CHROME DITHERING slider row panel. |
| SettingsRetroFX.UIChromePanel.Dithering.Label | `(0.061, 0.847, 0.260, 0.036)` | `+/-0.012` | Dithering label. |
| SettingsRetroFX.UIChromePanel.Dithering.Description | `(0.061, 0.891, 0.300, 0.030)` | `+/-0.012` | Dithering description. |
| SettingsRetroFX.UIChromePanel.Dithering.Slider | `(0.374, 0.858, 0.566, 0.042)` | `+/-0.012` | Dithering slider, handle/value at 0. |
| SettingsRetroFX.UIChromePanel.Dithering.Caption | `(0.374, 0.910, 0.150, 0.026)` | `+/-0.012` | Dithering slider caption. |
