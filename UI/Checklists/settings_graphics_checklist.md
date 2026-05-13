# Settings Graphics Structural Preservation Checklist

Source inventory: `C:\UE\T66\UI\Geometry\settings_graphics_structural_inventory.md`

Baseline capture: `C:\UE\T66\Saved\Codex\UI\SettingsGraphics\baseline_capture.png`

Baseline dump: `C:\UE\T66\Saved\Codex\UI\SettingsGraphics\baseline_dump.json`

## Structure

- [ ] SettingsGraphics.Root | exists=true
- [ ] SettingsGraphics.Background | exists=true
- [ ] SettingsGraphics.SettingsTabs | exists=true
- [ ] SettingsGraphics.SettingsTabs.GameplayButton | exists=true
- [ ] SettingsGraphics.SettingsTabs.GraphicsButton | exists=true
- [ ] SettingsGraphics.SettingsTabs.ControlsButton | exists=true
- [ ] SettingsGraphics.SettingsTabs.HUDButton | exists=true
- [ ] SettingsGraphics.SettingsTabs.MediaViewerButton | exists=true
- [ ] SettingsGraphics.SettingsTabs.AudioButton | exists=true
- [ ] SettingsGraphics.SettingsTabs.CrashingButton | exists=true
- [ ] SettingsGraphics.SettingsTabs.RetroFXButton | exists=true
- [ ] SettingsGraphics.ContentScroll | exists=true
- [ ] SettingsGraphics.Rows.Monitor | exists=true
- [ ] SettingsGraphics.Rows.Resolution | exists=true
- [ ] SettingsGraphics.Rows.WindowMode | exists=true
- [ ] SettingsGraphics.Rows.DisplayMode | exists=true
- [ ] SettingsGraphics.Rows.UIStyle | exists=true
- [ ] SettingsGraphics.Rows.Quality | exists=true
- [ ] SettingsGraphics.Rows.FPSCap | exists=true
- [ ] SettingsGraphics.Rows.Fog | exists=true
- [ ] SettingsGraphics.ApplyButton | exists=true
- [ ] FrontendTopBar.SettingsButton | exists=true
- [ ] FrontendTopBar.TicketBadge.Value | exists=true

## Geometry

- [ ] SettingsGraphics.Root | x=0.000 | 0.020
- [ ] SettingsGraphics.Root | y=0.095 | 0.030
- [ ] SettingsGraphics.Root | w=1.000 | 0.020
- [ ] SettingsGraphics.Root | h=0.905 | 0.030
- [ ] SettingsGraphics.SettingsTabs | x=0.003 | 0.030
- [ ] SettingsGraphics.SettingsTabs | y=0.094 | 0.030
- [ ] SettingsGraphics.SettingsTabs | w=0.994 | 0.030
- [ ] SettingsGraphics.SettingsTabs | h=0.079 | 0.030
- [ ] SettingsGraphics.SettingsTabs.GameplayButton | x=0.003 | 0.030
- [ ] SettingsGraphics.SettingsTabs.GraphicsButton | x=0.129 | 0.030
- [ ] SettingsGraphics.SettingsTabs.ControlsButton | x=0.253 | 0.030
- [ ] SettingsGraphics.SettingsTabs.HUDButton | x=0.379 | 0.030
- [ ] SettingsGraphics.SettingsTabs.MediaViewerButton | x=0.503 | 0.030
- [ ] SettingsGraphics.SettingsTabs.AudioButton | x=0.628 | 0.030
- [ ] SettingsGraphics.SettingsTabs.CrashingButton | x=0.754 | 0.030
- [ ] SettingsGraphics.SettingsTabs.RetroFXButton | x=0.879 | 0.030
- [ ] SettingsGraphics.ContentScroll | x=0.002 | 0.040
- [ ] SettingsGraphics.ContentScroll | y=0.194 | 0.040
- [ ] SettingsGraphics.ContentScroll | w=0.978 | 0.040
- [ ] SettingsGraphics.ContentScroll | h=0.753 | 0.050
- [ ] SettingsGraphics.Rows.Monitor | y=0.194 | 0.050
- [ ] SettingsGraphics.Rows.Resolution | y=0.325 | 0.050
- [ ] SettingsGraphics.Rows.WindowMode | y=0.457 | 0.050
- [ ] SettingsGraphics.Rows.DisplayMode | y=0.589 | 0.050
- [ ] SettingsGraphics.Rows.UIStyle | y=0.721 | 0.050
- [ ] SettingsGraphics.Rows.Quality | y=0.802 | 0.060

## Colors

- [ ] SettingsGraphics.SettingsTabs.GameplayButton | button_state=Default
- [ ] SettingsGraphics.SettingsTabs.GraphicsButton | button_state=Selected
- [ ] SettingsGraphics.SettingsTabs.ControlsButton | button_state=Default
- [ ] SettingsGraphics.SettingsTabs.HUDButton | button_state=Default
- [ ] SettingsGraphics.SettingsTabs.MediaViewerButton | button_state=Default
- [ ] SettingsGraphics.SettingsTabs.AudioButton | button_state=Default
- [ ] SettingsGraphics.SettingsTabs.CrashingButton | button_state=Default
- [ ] SettingsGraphics.SettingsTabs.RetroFXButton | button_state=Default
- [ ] SettingsGraphics.Rows.Monitor.Dropdown | button_state=Selected
- [ ] SettingsGraphics.Rows.Resolution.Dropdown | button_state=Selected
- [ ] SettingsGraphics.Rows.WindowMode.Dropdown | button_state=Selected
- [ ] SettingsGraphics.Rows.DisplayMode.Dropdown | button_state=Selected
- [ ] SettingsGraphics.Rows.UIStyle | button_state=Disabled
- [ ] SettingsGraphics.Rows.Quality.Slider | button_state=Default
- [ ] SettingsGraphics.Rows.FPSCap.Dropdown | button_state=Selected
- [ ] SettingsGraphics.ApplyButton | button_state=Selected

## Content

- [ ] SettingsGraphics.SettingsTabs.GameplayButton | text=GAMEPLAY
- [ ] SettingsGraphics.SettingsTabs.GraphicsButton | text=GRAPHICS
- [ ] SettingsGraphics.SettingsTabs.ControlsButton | text=CONTROLS
- [ ] SettingsGraphics.SettingsTabs.HUDButton | text=HUD
- [ ] SettingsGraphics.SettingsTabs.MediaViewerButton | text=MEDIA VIEWER
- [ ] SettingsGraphics.SettingsTabs.AudioButton | text=AUDIO
- [ ] SettingsGraphics.SettingsTabs.CrashingButton | text=CRASHING
- [ ] SettingsGraphics.SettingsTabs.RetroFXButton | text=RETRO FX
- [ ] SettingsGraphics.Rows.Monitor.Label | text=Monitor
- [ ] SettingsGraphics.Rows.Monitor.Label | is_label=true
- [ ] SettingsGraphics.Rows.Resolution.Label | text=Resolution
- [ ] SettingsGraphics.Rows.WindowMode.Label | text=Window Mode
- [ ] SettingsGraphics.Rows.DisplayMode.Label | text=Display Mode
- [ ] SettingsGraphics.Rows.UIStyle.Label | text=UI Style
- [ ] SettingsGraphics.Rows.UIStyle.Value | text=Unified
- [ ] SettingsGraphics.Rows.Quality.BestPerformanceLabel | text=Best Performance
- [ ] SettingsGraphics.Rows.Quality.BestQualityLabel | text=Best Quality
- [ ] SettingsGraphics.Rows.FPSCap.Label | text=FPS Cap
- [ ] SettingsGraphics.Rows.Fog.Label | text=Fog
- [ ] SettingsGraphics.ApplyButton | text=APPLY
- [ ] FrontendTopBar.TicketBadge.Value | is_label=true

## Interactivity

- [ ] SettingsGraphics.SettingsTabs.GameplayButton | has_click_handler=true
- [ ] SettingsGraphics.SettingsTabs.GameplayButton | hover_capable=true
- [ ] SettingsGraphics.SettingsTabs.GameplayButton | toggle_group=SettingsTabs
- [ ] SettingsGraphics.SettingsTabs.GraphicsButton | has_click_handler=true
- [ ] SettingsGraphics.SettingsTabs.GraphicsButton | hover_capable=true
- [ ] SettingsGraphics.SettingsTabs.GraphicsButton | toggle_group=SettingsTabs
- [ ] SettingsGraphics.SettingsTabs.ControlsButton | has_click_handler=true
- [ ] SettingsGraphics.SettingsTabs.ControlsButton | hover_capable=true
- [ ] SettingsGraphics.SettingsTabs.ControlsButton | toggle_group=SettingsTabs
- [ ] SettingsGraphics.SettingsTabs.HUDButton | has_click_handler=true
- [ ] SettingsGraphics.SettingsTabs.HUDButton | hover_capable=true
- [ ] SettingsGraphics.SettingsTabs.HUDButton | toggle_group=SettingsTabs
- [ ] SettingsGraphics.SettingsTabs.MediaViewerButton | has_click_handler=true
- [ ] SettingsGraphics.SettingsTabs.MediaViewerButton | hover_capable=true
- [ ] SettingsGraphics.SettingsTabs.MediaViewerButton | toggle_group=SettingsTabs
- [ ] SettingsGraphics.SettingsTabs.AudioButton | has_click_handler=true
- [ ] SettingsGraphics.SettingsTabs.AudioButton | hover_capable=true
- [ ] SettingsGraphics.SettingsTabs.AudioButton | toggle_group=SettingsTabs
- [ ] SettingsGraphics.SettingsTabs.CrashingButton | has_click_handler=true
- [ ] SettingsGraphics.SettingsTabs.CrashingButton | hover_capable=true
- [ ] SettingsGraphics.SettingsTabs.CrashingButton | toggle_group=SettingsTabs
- [ ] SettingsGraphics.SettingsTabs.RetroFXButton | has_click_handler=true
- [ ] SettingsGraphics.SettingsTabs.RetroFXButton | hover_capable=true
- [ ] SettingsGraphics.SettingsTabs.RetroFXButton | toggle_group=SettingsTabs
- [ ] SettingsGraphics.Rows.Monitor.Dropdown | has_click_handler=true
- [ ] SettingsGraphics.Rows.Monitor.Dropdown | hover_capable=true
- [ ] SettingsGraphics.Rows.Resolution.Dropdown | has_click_handler=true
- [ ] SettingsGraphics.Rows.Resolution.Dropdown | hover_capable=true
- [ ] SettingsGraphics.Rows.WindowMode.Dropdown | has_click_handler=true
- [ ] SettingsGraphics.Rows.WindowMode.Dropdown | hover_capable=true
- [ ] SettingsGraphics.Rows.DisplayMode.Dropdown | has_click_handler=true
- [ ] SettingsGraphics.Rows.DisplayMode.Dropdown | hover_capable=true
- [ ] SettingsGraphics.Rows.Quality.Slider | has_click_handler=true
- [ ] SettingsGraphics.Rows.Quality.Slider | hover_capable=true
- [ ] SettingsGraphics.Rows.FPSCap.Dropdown | has_click_handler=true
- [ ] SettingsGraphics.Rows.FPSCap.Dropdown | hover_capable=true
- [ ] SettingsGraphics.Rows.Fog.OnButton | has_click_handler=true
- [ ] SettingsGraphics.Rows.Fog.OnButton | hover_capable=true
- [ ] SettingsGraphics.Rows.Fog.OnButton | toggle_group=SettingsGraphics.Rows.Fog
- [ ] SettingsGraphics.Rows.Fog.OffButton | has_click_handler=true
- [ ] SettingsGraphics.Rows.Fog.OffButton | hover_capable=true
- [ ] SettingsGraphics.Rows.Fog.OffButton | toggle_group=SettingsGraphics.Rows.Fog
- [ ] SettingsGraphics.ApplyButton | has_click_handler=true
- [ ] SettingsGraphics.ApplyButton | hover_capable=true

