# Player Summary Picker Structural Preservation Checklist

Source inventory: `C:\UE\T66\UI\Geometry\player_summary_picker_structural_inventory.md`

Baseline capture: `C:\UE\T66\Saved\Codex\UI\PlayerSummaryPicker\baseline_capture.png`

Baseline dump: `C:\UE\T66\Saved\Codex\UI\PlayerSummaryPicker\baseline_dump.json`

Capture state: `-Screen PlayerSummaryPicker` with no pending picker snapshots.

## Structure

- [ ] PlayerSummaryPicker.Root | exists=true
- [ ] PlayerSummaryPicker.Scrim | exists=true
- [ ] PlayerSummaryPicker.ModalPanel | exists=true
- [ ] PlayerSummaryPicker.Content | exists=true
- [ ] PlayerSummaryPicker.Title | exists=true
- [ ] PlayerSummaryPicker.EmptyLabel | exists=true

## Geometry

- [ ] PlayerSummaryPicker.Root | x=0.000 | 0.005
- [ ] PlayerSummaryPicker.Root | y=0.000 | 0.005
- [ ] PlayerSummaryPicker.Root | w=1.000 | 0.005
- [ ] PlayerSummaryPicker.Root | h=1.000 | 0.005
- [ ] PlayerSummaryPicker.Scrim | x=0.000 | 0.005
- [ ] PlayerSummaryPicker.Scrim | y=0.000 | 0.005
- [ ] PlayerSummaryPicker.Scrim | w=1.000 | 0.005
- [ ] PlayerSummaryPicker.Scrim | h=1.000 | 0.005
- [ ] PlayerSummaryPicker.ModalPanel | x=0.283 | 0.020
- [ ] PlayerSummaryPicker.ModalPanel | y=0.417 | 0.020
- [ ] PlayerSummaryPicker.ModalPanel | w=0.435 | 0.020
- [ ] PlayerSummaryPicker.ModalPanel | h=0.167 | 0.020
- [ ] PlayerSummaryPicker.Content | x=0.305 | 0.020
- [ ] PlayerSummaryPicker.Content | y=0.449 | 0.020
- [ ] PlayerSummaryPicker.Content | w=0.390 | 0.020
- [ ] PlayerSummaryPicker.Content | h=0.103 | 0.020
- [ ] PlayerSummaryPicker.Title | x=0.427 | 0.020
- [ ] PlayerSummaryPicker.Title | y=0.449 | 0.020
- [ ] PlayerSummaryPicker.Title | w=0.146 | 0.020
- [ ] PlayerSummaryPicker.Title | h=0.054 | 0.020
- [ ] PlayerSummaryPicker.EmptyLabel | x=0.469 | 0.020
- [ ] PlayerSummaryPicker.EmptyLabel | y=0.521 | 0.020
- [ ] PlayerSummaryPicker.EmptyLabel | w=0.063 | 0.020
- [ ] PlayerSummaryPicker.EmptyLabel | h=0.031 | 0.020

## Colors

- [ ] PlayerSummaryPicker.ModalPanel | button_state=Default
- [ ] PlayerSummaryPicker.ModalPanel | border_color=DefaultBorder
- [ ] PlayerSummaryPicker.Title | text_color=PrimaryText
- [ ] PlayerSummaryPicker.EmptyLabel | text_color=SecondaryText

## Content

- [ ] PlayerSummaryPicker.Title | text=Pick the Player
- [ ] PlayerSummaryPicker.Title | is_label=true
- [ ] PlayerSummaryPicker.EmptyLabel | text=No players.
- [ ] PlayerSummaryPicker.EmptyLabel | is_label=true

## Interactivity

- [ ] PlayerSummaryPicker.Title | has_click_handler=false
- [ ] PlayerSummaryPicker.Title | is_label=true
- [ ] PlayerSummaryPicker.EmptyLabel | has_click_handler=false
- [ ] PlayerSummaryPicker.EmptyLabel | is_label=true
