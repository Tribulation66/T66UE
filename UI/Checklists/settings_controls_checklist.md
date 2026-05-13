# Settings Controls Structural Preservation Checklist

Source inventory: `C:\UE\T66\UI\Geometry\settings_controls_structural_inventory.md`

Baseline capture: `C:\UE\T66\Saved\Codex\UI\SettingsControls\baseline_capture.png`

Baseline dump: `C:\UE\T66\Saved\Codex\UI\SettingsControls\baseline_dump.json`

## Structure

- [ ] SettingsControls.Root | exists=true
- [ ] SettingsControls.Background | exists=true
- [ ] SettingsControls.SettingsTabs | exists=true
- [ ] SettingsControls.SettingsTabs.GameplayButton | exists=true
- [ ] SettingsControls.SettingsTabs.GraphicsButton | exists=true
- [ ] SettingsControls.SettingsTabs.ControlsButton | exists=true
- [ ] SettingsControls.SettingsTabs.HUDButton | exists=true
- [ ] SettingsControls.SettingsTabs.MediaViewerButton | exists=true
- [ ] SettingsControls.SettingsTabs.AudioButton | exists=true
- [ ] SettingsControls.SettingsTabs.CrashingButton | exists=true
- [ ] SettingsControls.SettingsTabs.RetroFXButton | exists=true
- [ ] SettingsControls.DeviceTabs.KeyboardMouseButton | exists=true
- [ ] SettingsControls.DeviceTabs.ControllerButton | exists=true
- [ ] SettingsControls.RebindInstructions | exists=true
- [ ] SettingsControls.BindingList | exists=true
- [ ] SettingsControls.Rows.MoveForward | exists=true
- [ ] SettingsControls.Rows.MoveBack | exists=true
- [ ] SettingsControls.Rows.MoveLeft | exists=true
- [ ] SettingsControls.Rows.MoveRight | exists=true
- [ ] SettingsControls.RestoreDefaultsButton | exists=true
- [ ] FrontendTopBar.SettingsButton | exists=true
- [ ] FrontendTopBar.TicketBadge.Value | exists=true

## Geometry

- [ ] SettingsControls.Root | x=0.000 | 0.020
- [ ] SettingsControls.Root | y=0.095 | 0.030
- [ ] SettingsControls.Root | w=1.000 | 0.020
- [ ] SettingsControls.Root | h=0.905 | 0.030
- [ ] SettingsControls.SettingsTabs | x=0.003 | 0.030
- [ ] SettingsControls.SettingsTabs | y=0.094 | 0.030
- [ ] SettingsControls.SettingsTabs | w=0.994 | 0.030
- [ ] SettingsControls.SettingsTabs | h=0.079 | 0.030
- [ ] SettingsControls.SettingsTabs.ControlsButton | x=0.253 | 0.030
- [ ] SettingsControls.DeviceTabs.KeyboardMouseButton | x=0.004 | 0.030
- [ ] SettingsControls.DeviceTabs.KeyboardMouseButton | y=0.195 | 0.040
- [ ] SettingsControls.DeviceTabs.ControllerButton | x=0.144 | 0.040
- [ ] SettingsControls.DeviceTabs.ControllerButton | y=0.195 | 0.040
- [ ] SettingsControls.RebindInstructions | y=0.268 | 0.040
- [ ] SettingsControls.BindingList | y=0.312 | 0.050
- [ ] SettingsControls.Rows.MoveForward | y=0.312 | 0.050
- [ ] SettingsControls.Rows.MoveBack | y=0.510 | 0.060
- [ ] SettingsControls.Rows.MoveLeft | y=0.708 | 0.060

## Colors

- [ ] SettingsControls.SettingsTabs.ControlsButton | button_state=Selected
- [ ] SettingsControls.SettingsTabs.GameplayButton | button_state=Default
- [ ] SettingsControls.SettingsTabs.GraphicsButton | button_state=Default
- [ ] SettingsControls.DeviceTabs.KeyboardMouseButton | button_state=Selected
- [ ] SettingsControls.DeviceTabs.ControllerButton | button_state=Default
- [ ] SettingsControls.Rows.MoveForward | button_state=Disabled
- [ ] SettingsControls.Rows.MoveBack | button_state=Disabled
- [ ] SettingsControls.Rows.MoveLeft | button_state=Disabled
- [ ] SettingsControls.RestoreDefaultsButton | button_state=Selected

## Content

- [ ] SettingsControls.SettingsTabs.ControlsButton | text=CONTROLS
- [ ] SettingsControls.DeviceTabs.KeyboardMouseButton | text=KEYBOARD & MOUSE
- [ ] SettingsControls.DeviceTabs.ControllerButton | text=CONTROLLER
- [ ] SettingsControls.RebindInstructions | text=Click REBIND, then press a key/button (Esc cancels).
- [ ] SettingsControls.RebindInstructions | is_label=true
- [ ] SettingsControls.Rows.MoveForward.Label | text=Move Forward
- [ ] SettingsControls.Rows.MoveForward.Primary.SlotLabel | text=PRIMARY
- [ ] SettingsControls.Rows.MoveForward.Primary.Key | text=W
- [ ] SettingsControls.Rows.MoveForward.Primary.RebindButton | text=REBIND
- [ ] SettingsControls.Rows.MoveForward.Primary.ClearButton | text=CLEAR
- [ ] SettingsControls.Rows.MoveForward.Secondary.SlotLabel | text=SECONDARY
- [ ] SettingsControls.Rows.MoveBack.Label | text=Move Back
- [ ] SettingsControls.Rows.MoveLeft.Label | text=Move Left
- [ ] SettingsControls.RestoreDefaultsButton | text=RESTORE DEFAULTS
- [ ] FrontendTopBar.TicketBadge.Value | is_label=true

## Interactivity

- [ ] SettingsControls.SettingsTabs.GameplayButton | has_click_handler=true
- [ ] SettingsControls.SettingsTabs.GameplayButton | hover_capable=true
- [ ] SettingsControls.SettingsTabs.GameplayButton | toggle_group=SettingsTabs
- [ ] SettingsControls.SettingsTabs.GraphicsButton | has_click_handler=true
- [ ] SettingsControls.SettingsTabs.GraphicsButton | hover_capable=true
- [ ] SettingsControls.SettingsTabs.GraphicsButton | toggle_group=SettingsTabs
- [ ] SettingsControls.SettingsTabs.ControlsButton | has_click_handler=true
- [ ] SettingsControls.SettingsTabs.ControlsButton | hover_capable=true
- [ ] SettingsControls.SettingsTabs.ControlsButton | toggle_group=SettingsTabs
- [ ] SettingsControls.SettingsTabs.HUDButton | has_click_handler=true
- [ ] SettingsControls.SettingsTabs.HUDButton | hover_capable=true
- [ ] SettingsControls.SettingsTabs.HUDButton | toggle_group=SettingsTabs
- [ ] SettingsControls.SettingsTabs.MediaViewerButton | has_click_handler=true
- [ ] SettingsControls.SettingsTabs.MediaViewerButton | hover_capable=true
- [ ] SettingsControls.SettingsTabs.MediaViewerButton | toggle_group=SettingsTabs
- [ ] SettingsControls.SettingsTabs.AudioButton | has_click_handler=true
- [ ] SettingsControls.SettingsTabs.AudioButton | hover_capable=true
- [ ] SettingsControls.SettingsTabs.AudioButton | toggle_group=SettingsTabs
- [ ] SettingsControls.SettingsTabs.CrashingButton | has_click_handler=true
- [ ] SettingsControls.SettingsTabs.CrashingButton | hover_capable=true
- [ ] SettingsControls.SettingsTabs.CrashingButton | toggle_group=SettingsTabs
- [ ] SettingsControls.SettingsTabs.RetroFXButton | has_click_handler=true
- [ ] SettingsControls.SettingsTabs.RetroFXButton | hover_capable=true
- [ ] SettingsControls.SettingsTabs.RetroFXButton | toggle_group=SettingsTabs
- [ ] SettingsControls.DeviceTabs.KeyboardMouseButton | has_click_handler=true
- [ ] SettingsControls.DeviceTabs.KeyboardMouseButton | hover_capable=true
- [ ] SettingsControls.DeviceTabs.KeyboardMouseButton | toggle_group=SettingsControls.DeviceTabs
- [ ] SettingsControls.DeviceTabs.ControllerButton | has_click_handler=true
- [ ] SettingsControls.DeviceTabs.ControllerButton | hover_capable=true
- [ ] SettingsControls.DeviceTabs.ControllerButton | toggle_group=SettingsControls.DeviceTabs
- [ ] SettingsControls.Rows.MoveForward.Primary.RebindButton | has_click_handler=true
- [ ] SettingsControls.Rows.MoveForward.Primary.RebindButton | hover_capable=true
- [ ] SettingsControls.Rows.MoveForward.Primary.ClearButton | has_click_handler=true
- [ ] SettingsControls.Rows.MoveForward.Primary.ClearButton | hover_capable=true
- [ ] SettingsControls.Rows.MoveForward.Secondary.RebindButton | has_click_handler=true
- [ ] SettingsControls.Rows.MoveForward.Secondary.RebindButton | hover_capable=true
- [ ] SettingsControls.Rows.MoveForward.Secondary.ClearButton | has_click_handler=true
- [ ] SettingsControls.Rows.MoveForward.Secondary.ClearButton | hover_capable=true
- [ ] SettingsControls.Rows.MoveBack.Primary.RebindButton | has_click_handler=true
- [ ] SettingsControls.Rows.MoveBack.Primary.RebindButton | hover_capable=true
- [ ] SettingsControls.Rows.MoveBack.Primary.ClearButton | has_click_handler=true
- [ ] SettingsControls.Rows.MoveBack.Primary.ClearButton | hover_capable=true
- [ ] SettingsControls.RestoreDefaultsButton | has_click_handler=true
- [ ] SettingsControls.RestoreDefaultsButton | hover_capable=true

