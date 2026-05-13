# Quit Confirmation Structural Preservation Checklist

Source inventory: `C:\UE\T66\UI\Geometry\quit_confirmation_structural_inventory.md`

Baseline capture: `C:\UE\T66\Saved\Codex\UI\QuitConfirmation\baseline_capture.png`

Baseline dump: `C:\UE\T66\Saved\Codex\UI\QuitConfirmation\baseline_dump.json`

## Structure

- [ ] QuitConfirmation.Root | exists=true
- [ ] QuitConfirmation.Scrim | exists=true
- [ ] QuitConfirmation.ModalPanel | exists=true
- [ ] QuitConfirmation.Title | exists=true
- [ ] QuitConfirmation.Message | exists=true
- [ ] QuitConfirmation.StayButton | exists=true
- [ ] QuitConfirmation.QuitButton | exists=true

## Geometry

- [ ] QuitConfirmation.Root | x=0.000 | 0.005
- [ ] QuitConfirmation.Root | y=0.000 | 0.005
- [ ] QuitConfirmation.Root | w=1.000 | 0.005
- [ ] QuitConfirmation.Root | h=1.000 | 0.005
- [ ] QuitConfirmation.ModalPanel | x=0.215 | 0.020
- [ ] QuitConfirmation.ModalPanel | y=0.326 | 0.020
- [ ] QuitConfirmation.ModalPanel | w=0.570 | 0.020
- [ ] QuitConfirmation.ModalPanel | h=0.348 | 0.020
- [ ] QuitConfirmation.Title | x=0.429 | 0.025
- [ ] QuitConfirmation.Title | y=0.374 | 0.025
- [ ] QuitConfirmation.Message | x=0.279 | 0.025
- [ ] QuitConfirmation.Message | y=0.470 | 0.025
- [ ] QuitConfirmation.StayButton | x=0.263 | 0.020
- [ ] QuitConfirmation.StayButton | y=0.544 | 0.020
- [ ] QuitConfirmation.QuitButton | x=0.508 | 0.020
- [ ] QuitConfirmation.QuitButton | y=0.544 | 0.020

## Colors

- [ ] QuitConfirmation.ModalPanel | button_state=Default
- [ ] QuitConfirmation.StayButton | button_state=Default
- [ ] QuitConfirmation.QuitButton | button_state=Selected
- [ ] QuitConfirmation.ModalPanel | border_color=DefaultBorder

## Content

- [ ] QuitConfirmation.Title | text=QUIT GAME?
- [ ] QuitConfirmation.Title | is_label=true
- [ ] QuitConfirmation.Message | text=Are you sure you want to quit?
- [ ] QuitConfirmation.Message | is_label=true
- [ ] QuitConfirmation.StayButton | text=NO, I WANT TO STAY
- [ ] QuitConfirmation.QuitButton | text=YES, I WANT TO QUIT

## Interactivity

- [ ] QuitConfirmation.StayButton | has_click_handler=true
- [ ] QuitConfirmation.StayButton | hover_capable=true
- [ ] QuitConfirmation.QuitButton | has_click_handler=true
- [ ] QuitConfirmation.QuitButton | hover_capable=true
