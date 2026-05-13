# AT66 World Interactable Base Structural Preservation Checklist

Source baseline dump: `C:\UE\T66\Saved\Codex\UI\HUDChromeMigration\AT66WorldInteractableBase\baseline_dump.json`

## Structure

- [ ] WorldInteractablePrompt.Root | exists=true
- [ ] WorldInteractablePrompt.Root | type=SBox
- [ ] WorldInteractablePrompt.Panel | exists=true
- [ ] WorldInteractablePrompt.Panel | type=SBorder
- [ ] WorldInteractablePrompt.Text | exists=true
- [ ] WorldInteractablePrompt.Text | type=STextBlock

## Geometry

- [ ] WorldInteractablePrompt.Root | x=0.3350 | 0.020
- [ ] WorldInteractablePrompt.Root | y=0.9120 | 0.020
- [ ] WorldInteractablePrompt.Root | w=0.3300 | 0.020
- [ ] WorldInteractablePrompt.Root | h=0.0720 | 0.020
- [ ] WorldInteractablePrompt.Panel | x=0.3350 | 0.020
- [ ] WorldInteractablePrompt.Panel | y=0.9120 | 0.020
- [ ] WorldInteractablePrompt.Panel | w=0.3300 | 0.020
- [ ] WorldInteractablePrompt.Panel | h=0.0720 | 0.020
- [ ] WorldInteractablePrompt.Text | x=0.3470 | 0.020
- [ ] WorldInteractablePrompt.Text | y=0.9280 | 0.020
- [ ] WorldInteractablePrompt.Text | w=0.3060 | 0.020
- [ ] WorldInteractablePrompt.Text | h=0.0400 | 0.020

## Colors

- [ ] WorldInteractablePrompt.Root | border_color=DefaultBorder
- [ ] WorldInteractablePrompt.Root | state=Default
- [ ] WorldInteractablePrompt.Panel | border_color=DefaultBorder
- [ ] WorldInteractablePrompt.Panel | state=Default
- [ ] WorldInteractablePrompt.Text | text_color=PrimaryText

## Content

- [ ] WorldInteractablePrompt.Root | role=InWorldPrompt
- [ ] WorldInteractablePrompt.Panel | role=Panel
- [ ] WorldInteractablePrompt.Text | role=Label.Body
- [ ] WorldInteractablePrompt.Text | text=Press F to interact with Chest
- [ ] WorldInteractablePrompt.Text | is_label=true

## Interactivity

- [ ] WorldInteractablePrompt.Root | has_click_handler=false
- [ ] WorldInteractablePrompt.Root | hover_capable=false
- [ ] WorldInteractablePrompt.Panel | has_click_handler=false
- [ ] WorldInteractablePrompt.Panel | hover_capable=false
- [ ] WorldInteractablePrompt.Text | has_click_handler=false
- [ ] WorldInteractablePrompt.Text | hover_capable=false
