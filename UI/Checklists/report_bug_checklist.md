# Report Bug Structural Preservation Checklist

Source inventory: `C:\UE\T66\UI\Geometry\report_bug_structural_inventory.md`

Baseline capture: `C:\UE\T66\Saved\Codex\UI\ReportBug\baseline_capture.png`

Baseline dump: `C:\UE\T66\Saved\Codex\UI\ReportBug\baseline_dump.json`

## Structure

- [ ] ReportBug.Root | exists=true
- [ ] ReportBug.Scrim | exists=true
- [ ] ReportBug.ModalPanel | exists=true
- [ ] ReportBug.Title | exists=true
- [ ] ReportBug.TextFieldPanel | exists=true
- [ ] ReportBug.TextInput | exists=true
- [ ] ReportBug.SubmitButton | exists=true
- [ ] ReportBug.CancelButton | exists=true

## Geometry

- [ ] ReportBug.Root | x=0.000 | 0.005
- [ ] ReportBug.Root | y=0.000 | 0.005
- [ ] ReportBug.Root | w=1.000 | 0.005
- [ ] ReportBug.Root | h=1.000 | 0.005
- [ ] ReportBug.ModalPanel | x=0.236 | 0.020
- [ ] ReportBug.ModalPanel | y=0.134 | 0.020
- [ ] ReportBug.ModalPanel | w=0.529 | 0.020
- [ ] ReportBug.ModalPanel | h=0.694 | 0.020
- [ ] ReportBug.Title | x=0.416 | 0.025
- [ ] ReportBug.Title | y=0.201 | 0.025
- [ ] ReportBug.TextFieldPanel | x=0.277 | 0.020
- [ ] ReportBug.TextFieldPanel | y=0.282 | 0.020
- [ ] ReportBug.TextFieldPanel | w=0.446 | 0.020
- [ ] ReportBug.TextFieldPanel | h=0.371 | 0.020
- [ ] ReportBug.TextInput | x=0.299 | 0.020
- [ ] ReportBug.TextInput | y=0.314 | 0.020
- [ ] ReportBug.TextInput | w=0.401 | 0.020
- [ ] ReportBug.TextInput | h=0.307 | 0.020
- [ ] ReportBug.SubmitButton | x=0.284 | 0.020
- [ ] ReportBug.SubmitButton | y=0.652 | 0.020
- [ ] ReportBug.CancelButton | x=0.509 | 0.020
- [ ] ReportBug.CancelButton | y=0.652 | 0.020

## Colors

- [ ] ReportBug.ModalPanel | button_state=Default
- [ ] ReportBug.TextFieldPanel | button_state=Default
- [ ] ReportBug.SubmitButton | button_state=Default
- [ ] ReportBug.CancelButton | button_state=Default
- [ ] ReportBug.ModalPanel | border_color=DefaultBorder

## Content

- [ ] ReportBug.Title | text=REPORT BUG
- [ ] ReportBug.Title | is_label=true
- [ ] ReportBug.SubmitButton | text=SUBMIT
- [ ] ReportBug.CancelButton | text=CANCEL

## Interactivity

- [ ] ReportBug.SubmitButton | has_click_handler=true
- [ ] ReportBug.SubmitButton | hover_capable=true
- [ ] ReportBug.CancelButton | has_click_handler=true
- [ ] ReportBug.CancelButton | hover_capable=true
