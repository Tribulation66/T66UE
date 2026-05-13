# Settings Controls Structural Inventory

Baseline capture: `C:\UE\T66\Saved\Codex\UI\SettingsControls\baseline_capture.png`

Baseline dump: `C:\UE\T66\Saved\Codex\UI\SettingsControls\baseline_dump.json`

Normalization basis: `1920x1080`

Method: no-reference structural preservation from the routed Settings Controls baseline. Top bar chrome is the verified shared component and is only referenced through checklist assertions.

## Structural Regions

| Tag / Region | BBox `(x,y,w,h)` | Tolerance | Role | Text / State |
|---|---:|---:|---|---|
| SettingsControls.Root | `(0.000, 0.095, 1.000, 0.905)` | `+/-0.020` | Root | Settings tab content below shared top bar. |
| SettingsControls.SettingsTabs | `(0.003, 0.094, 0.994, 0.079)` | `+/-0.020` | Toggle group | Settings tab row. |
| SettingsControls.SettingsTabs.ControlsButton | `(0.253, 0.094, 0.118, 0.079)` | `+/-0.020` | Toggle button | `CONTROLS`, Selected. |
| SettingsControls.DeviceTabs.KeyboardMouseButton | `(0.004, 0.195, 0.134, 0.059)` | `+/-0.020` | Toggle button | `KEYBOARD & MOUSE`, Selected. |
| SettingsControls.DeviceTabs.ControllerButton | `(0.144, 0.195, 0.086, 0.059)` | `+/-0.020` | Toggle button | `CONTROLLER`, Default. |
| SettingsControls.RebindInstructions | `(0.002, 0.268, 0.620, 0.034)` | `+/-0.030` | Label | Rebind instructions. |
| SettingsControls.BindingList | `(0.002, 0.312, 0.978, 0.553)` | `+/-0.030` | Scroll content | Binding row list. |
| SettingsControls.Rows.MoveForward | `(0.002, 0.312, 0.978, 0.193)` | `+/-0.040` | Binding row | `Move Forward`. |
| SettingsControls.Rows.MoveBack | `(0.002, 0.510, 0.978, 0.193)` | `+/-0.040` | Binding row | `Move Back`. |
| SettingsControls.Rows.MoveLeft | `(0.002, 0.708, 0.978, 0.193)` | `+/-0.040` | Binding row | `Move Left`. |
| SettingsControls.Rows.MoveRight | `(0.002, 0.905, 0.978, 0.193)` | `+/-0.050` | Binding row | `Move Right`. |
| SettingsControls.RestoreDefaultsButton | `(0.871, 0.886, 0.126, 0.060)` | `+/-0.030` | Button | `RESTORE DEFAULTS`. |

## Interaction Inventory

| Tag / Element | Role | Expected behavior |
|---|---|---|
| SettingsControls.SettingsTabs.*Button | Toggle button | `SettingsTabs` group; click switches settings tab. |
| SettingsControls.DeviceTabs.*Button | Toggle button | `SettingsControls.DeviceTabs` group; switches keyboard/controller binding list. |
| SettingsControls.Rows.*.Primary.RebindButton | Button | Begins rebind for primary slot. |
| SettingsControls.Rows.*.Primary.ClearButton | Button | Clears primary slot binding. |
| SettingsControls.Rows.*.Secondary.RebindButton | Button | Begins rebind for secondary slot. |
| SettingsControls.Rows.*.Secondary.ClearButton | Button | Clears secondary slot binding. |
| SettingsControls.RestoreDefaultsButton | Button | First click arms confirmation, second click restores defaults for active device tab. |

