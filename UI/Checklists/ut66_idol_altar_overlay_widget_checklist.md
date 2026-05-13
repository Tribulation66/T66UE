# UT66 Idol Altar Overlay Widget Structural Preservation Checklist

Source baseline dump: `C:\UE\T66\Saved\Codex\UI\HUDChromeMigration\UT66IdolAltarOverlayWidget\baseline_dump.json`

## Structure

- [ ] IdolAltar.Root | exists=true
- [ ] IdolAltar.Root | type=SOverlay
- [ ] IdolAltar.Backdrop | exists=true
- [ ] IdolAltar.Backdrop | type=SBorder
- [ ] IdolAltar.Panel | exists=true
- [ ] IdolAltar.Panel | type=SBorder
- [ ] IdolAltar.BackButton | exists=true
- [ ] IdolAltar.BackButton | type=SBox
- [ ] IdolAltar.Title | exists=true
- [ ] IdolAltar.Title | type=STextBlock
- [ ] IdolAltar.Card.0 | exists=true
- [ ] IdolAltar.Card.1 | exists=true
- [ ] IdolAltar.Card.2 | exists=true
- [ ] IdolAltar.Card.3 | exists=true
- [ ] IdolAltar.CardTitle.0 | exists=true
- [ ] IdolAltar.CardIcon.0 | exists=true
- [ ] IdolAltar.CardDescription.0 | exists=true
- [ ] IdolAltar.TakeButton.0 | exists=true
- [ ] IdolAltar.TakeButton.1 | exists=true
- [ ] IdolAltar.TakeButton.2 | exists=true
- [ ] IdolAltar.TakeButton.3 | exists=true
- [ ] IdolAltar.RerollButton | exists=true

## Geometry

- [ ] IdolAltar.Root | x=0.0000 | 0.005
- [ ] IdolAltar.Root | y=0.0000 | 0.005
- [ ] IdolAltar.Root | w=1.0000 | 0.005
- [ ] IdolAltar.Root | h=1.0000 | 0.005
- [ ] IdolAltar.Panel | x=0.0000 | 0.020
- [ ] IdolAltar.Panel | y=0.0000 | 0.020
- [ ] IdolAltar.Panel | w=1.0000 | 0.020
- [ ] IdolAltar.Panel | h=1.0000 | 0.020
- [ ] IdolAltar.BackButton | x=0.0225 | 0.020
- [ ] IdolAltar.BackButton | y=0.0496 | 0.020
- [ ] IdolAltar.BackButton | w=0.0825 | 0.020
- [ ] IdolAltar.BackButton | h=0.0613 | 0.020
- [ ] IdolAltar.Title | x=0.4253 | 0.020
- [ ] IdolAltar.Title | y=0.0400 | 0.020
- [ ] IdolAltar.Card.0 | x=0.0507 | 0.020
- [ ] IdolAltar.Card.0 | y=0.1653 | 0.020
- [ ] IdolAltar.Card.0 | w=0.2145 | 0.020
- [ ] IdolAltar.Card.0 | h=0.6800 | 0.020
- [ ] IdolAltar.Card.1 | x=0.2788 | 0.020
- [ ] IdolAltar.Card.2 | x=0.5067 | 0.020
- [ ] IdolAltar.Card.3 | x=0.7348 | 0.020
- [ ] IdolAltar.TakeButton.0 | x=0.0612 | 0.020
- [ ] IdolAltar.TakeButton.0 | y=0.7653 | 0.020
- [ ] IdolAltar.TakeButton.1 | x=0.2893 | 0.020
- [ ] IdolAltar.TakeButton.2 | x=0.5173 | 0.020
- [ ] IdolAltar.TakeButton.3 | x=0.7453 | 0.020
- [ ] IdolAltar.RerollButton | x=0.4325 | 0.020
- [ ] IdolAltar.RerollButton | y=0.8693 | 0.020

## Colors

- [ ] IdolAltar.Panel | border_color=DefaultBorder
- [ ] IdolAltar.Panel | state=Default
- [ ] IdolAltar.BackButton | border_color=DefaultBorder
- [ ] IdolAltar.BackButton | state=Default
- [ ] IdolAltar.Card.0 | border_color=DefaultBorder
- [ ] IdolAltar.Card.0 | state=Default
- [ ] IdolAltar.Card.1 | border_color=DefaultBorder
- [ ] IdolAltar.Card.1 | state=Default
- [ ] IdolAltar.Card.2 | border_color=DefaultBorder
- [ ] IdolAltar.Card.2 | state=Default
- [ ] IdolAltar.Card.3 | border_color=DefaultBorder
- [ ] IdolAltar.Card.3 | state=Default
- [ ] IdolAltar.TakeButton.0 | border_color=DefaultBorder
- [ ] IdolAltar.TakeButton.0 | state=Default
- [ ] IdolAltar.RerollButton | border_color=DefaultBorder
- [ ] IdolAltar.RerollButton | state=Default

## Content

- [ ] IdolAltar.Root | role=Overlay
- [ ] IdolAltar.Backdrop | role=Panel
- [ ] IdolAltar.Panel | role=Panel
- [ ] IdolAltar.BackButton | role=ToggleButton
- [ ] IdolAltar.BackButton | text=BACK
- [ ] IdolAltar.Title | role=Label.Title
- [ ] IdolAltar.Title | text=IDOL ALTAR
- [ ] IdolAltar.Title | font_size=42
- [ ] IdolAltar.Title | is_label=true
- [ ] IdolAltar.Card.0 | role=Panel
- [ ] IdolAltar.CardTitle.0 | role=Label.Header
- [ ] IdolAltar.CardTitle.0 | text=LIGHT IDOL
- [ ] IdolAltar.CardTitle.0 | is_label=true
- [ ] IdolAltar.CardIcon.0 | role=Icon
- [ ] IdolAltar.CardDescription.0 | role=Label.Body
- [ ] IdolAltar.CardDescription.0 | is_label=true
- [ ] IdolAltar.TakeButton.0 | role=ToggleButton
- [ ] IdolAltar.TakeButton.0 | text=TAKE
- [ ] IdolAltar.TakeButton.1 | text=TAKE
- [ ] IdolAltar.TakeButton.2 | text=TAKE
- [ ] IdolAltar.TakeButton.3 | text=TAKE
- [ ] IdolAltar.RerollButton | role=ToggleButton
- [ ] IdolAltar.RerollButton | text=REROLL

## Interactivity

- [ ] IdolAltar.Root | has_click_handler=false
- [ ] IdolAltar.Root | hover_capable=false
- [ ] IdolAltar.Backdrop | has_click_handler=false
- [ ] IdolAltar.Backdrop | hover_capable=false
- [ ] IdolAltar.Panel | has_click_handler=false
- [ ] IdolAltar.Panel | hover_capable=false
- [ ] IdolAltar.BackButton | has_click_handler=true
- [ ] IdolAltar.BackButton | hover_capable=true
- [ ] IdolAltar.Card.0 | has_click_handler=false
- [ ] IdolAltar.Card.0 | hover_capable=false
- [ ] IdolAltar.CardIcon.0 | has_click_handler=false
- [ ] IdolAltar.CardIcon.0 | hover_capable=false
- [ ] IdolAltar.TakeButton.0 | has_click_handler=true
- [ ] IdolAltar.TakeButton.0 | hover_capable=true
- [ ] IdolAltar.TakeButton.1 | has_click_handler=true
- [ ] IdolAltar.TakeButton.1 | hover_capable=true
- [ ] IdolAltar.TakeButton.2 | has_click_handler=true
- [ ] IdolAltar.TakeButton.2 | hover_capable=true
- [ ] IdolAltar.TakeButton.3 | has_click_handler=true
- [ ] IdolAltar.TakeButton.3 | hover_capable=true
- [ ] IdolAltar.RerollButton | has_click_handler=true
- [ ] IdolAltar.RerollButton | hover_capable=true
