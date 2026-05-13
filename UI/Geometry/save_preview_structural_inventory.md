# Save Preview Structural Inventory

Source baseline: `C:\UE\T66\Saved\Codex\UI\SavePreview\baseline_capture.png`

Source dump: `C:\UE\T66\Saved\Codex\UI\SavePreview\baseline_dump.json`

Normalized basis: 1920x1080.

Target state: `SavePreview`.

## Visible Structural Regions

| Element | Baseline source | Text | Role | x | y | w | h | Notes |
| --- | --- | --- | --- | ---: | ---: | ---: | ---: | --- |
| SavePreview.Root | viewport | PREVIEW | Root | 0.0000 | 0.0000 | 1.0000 | 1.0000 | Full-screen modal. |
| SavePreview.Background | widget 3 |  | Background | 0.0000 | 0.0000 | 1.0000 | 1.0000 | Baseline reference scene becomes flat background. |
| SavePreview.Scrim | widget 4 |  | Scrim | 0.0000 | 0.0000 | 1.0000 | 1.0000 | Full-screen dark overlay. |
| SavePreview.ModalPanel | widget 7 | PREVIEW | Panel | 0.2150 | 0.6458 | 0.5700 | 0.3169 | Bottom-centered modal shell. |
| SavePreview.Content | widget 8 | PREVIEW | Content cluster | 0.2465 | 0.6858 | 0.5070 | 0.2316 | Inner content column. |
| SavePreview.Title | widget 9 | PREVIEW | Label | 0.2465 | 0.6858 | 0.5070 | 0.0574 | Title label. |
| SavePreview.Subtitle | widget 10 | The run is paused for inspection. Back returns to Save Slots, Load resumes normally. | Label | 0.2465 | 0.7512 | 0.5070 | 0.0648 | Wrapped subtitle copy. |
| SavePreview.ButtonRow | widget 11 |  | Action row | 0.3380 | 0.8400 | 0.3240 | 0.0773 | Two-button row. |
| SavePreview.BackButton | widget 12 | BACK | Button | 0.3380 | 0.8400 | 0.1575 | 0.0773 | Returns to Save Slots. |
| SavePreview.LoadButton | widget 20 | LOAD | Button | 0.5045 | 0.8400 | 0.1575 | 0.0773 | Loads the previewed save. |

## Structural Rules

- Preserve each visible baseline region within +/- 0.02 normalized x/y/w/h, except full-viewport root/background/scrim which use +/- 0.005.
- Preserve title, subtitle, and button text.
- Replace the reference scene image, modal shell, retained surfaces, and sliced button chrome with `FT66FlatStyle` construction.
- `BACK` and `LOAD` remain single-action buttons with click handlers and no toggle group.
- No `top_bar` section is expected for this modal.
