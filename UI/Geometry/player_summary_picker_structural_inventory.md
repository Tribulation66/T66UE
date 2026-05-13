# Player Summary Picker Structural Inventory

Source baseline: `C:\UE\T66\Saved\Codex\UI\PlayerSummaryPicker\baseline_capture.png`

Source dump: `C:\UE\T66\Saved\Codex\UI\PlayerSummaryPicker\baseline_dump.json`

Normalized basis: 1920x1080.

Target state: `PlayerSummaryPicker` / `SummaryPicker` with no pending picker snapshots.

## Visible Structural Regions

| Element | Baseline source | Text | Role | x | y | w | h | Notes |
| --- | --- | --- | --- | ---: | ---: | ---: | ---: | --- |
| PlayerSummaryPicker.Root | viewport | Pick the Player | Root | 0.0000 | 0.0000 | 1.0000 | 1.0000 | Full-screen modal surface. |
| PlayerSummaryPicker.Scrim | widget 2 | Pick the Player | Scrim | 0.0000 | 0.0000 | 1.0000 | 1.0000 | Full-screen dark overlay behind modal. |
| PlayerSummaryPicker.ModalPanel | widget 3 | Pick the Player | Panel | 0.2825 | 0.4165 | 0.4350 | 0.1669 | Centered picker shell. |
| PlayerSummaryPicker.Content | widget 4 | Pick the Player | Content cluster | 0.3050 | 0.4485 | 0.3900 | 0.1029 | Inner empty-state content area. |
| PlayerSummaryPicker.Title | widget 6 | Pick the Player | Label | 0.4271 | 0.4485 | 0.1458 | 0.0537 | Modal title. |
| PlayerSummaryPicker.EmptyLabel | widget 7 | No players. | Label | 0.4688 | 0.5209 | 0.0625 | 0.0306 | Empty-state body copy. |

## Structural Rules

- Preserve each visible baseline region within +/- 0.02 normalized x/y/w/h, except full-viewport root/scrim which use +/- 0.005.
- Preserve the empty-state text content exactly: `Pick the Player` and `No players.`.
- Replace all legacy reference chrome with `FT66FlatStyle` panels, labels, portrait slots, and buttons.
- Non-empty picker options remain dynamic runtime content: each option must keep a tagged option panel, name label, avatar slot, and `SELECT` button with click handler.
- The no-snapshots baseline has no interactive controls; interactivity assertions apply only to dynamic option buttons when snapshots exist.
