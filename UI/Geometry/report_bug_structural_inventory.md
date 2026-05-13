# Report Bug Structural Inventory

Source baseline capture: `C:\UE\T66\Saved\Codex\UI\ReportBug\baseline_capture.png`

Source baseline dump: `C:\UE\T66\Saved\Codex\UI\ReportBug\baseline_dump.json`

Reference mode: no external V3 reference image exists for Report Bug. This inventory is the structural source for the Stage 2 no-reference migration.

Normalized basis: 1920x1080.

## Baseline Summary

- Screen: `ReportBug`
- Baseline widgets: `59`
- Baseline tagged widgets: `0`
- Shared top bar: not present

## Regions And Elements

| Element | Tag | x | y | w | h | Text / role |
| --- | --- | ---: | ---: | ---: | ---: | --- |
| Root | `ReportBug.Root` | 0.000 | 0.000 | 1.000 | 1.000 | Modal root over gameplay background. |
| Scrim | `ReportBug.Scrim` | 0.000 | 0.000 | 1.000 | 1.000 | Transparent modal backdrop. |
| Modal panel | `ReportBug.ModalPanel` | 0.236 | 0.134 | 0.529 | 0.694 | Centered report modal panel. |
| Title | `ReportBug.Title` | 0.416 | 0.201 | 0.168 | 0.081 | `REPORT BUG`. |
| Text field panel | `ReportBug.TextFieldPanel` | 0.277 | 0.282 | 0.446 | 0.371 | Multiline report field frame. |
| Text input | `ReportBug.TextInput` | 0.299 | 0.314 | 0.401 | 0.307 | Multiline editable bug report input; hint `Describe the bug...`. |
| Submit button | `ReportBug.SubmitButton` | 0.284 | 0.652 | 0.210 | 0.101 | `SUBMIT`; disabled while text is empty. |
| Cancel button | `ReportBug.CancelButton` | 0.509 | 0.652 | 0.210 | 0.101 | `CANCEL`. |
