# UT66 Collector Overlay Widget Structural Preservation Checklist

Source baseline dump: `C:\UE\T66\Saved\Codex\UI\HUDChromeMigration\UT66CollectorOverlayWidget\baseline_dump.json`

## Structure

- [ ] CollectorOverlay.Root | exists=true
- [ ] CollectorOverlay.Root | type=SBox
- [ ] CollectorOverlay.Backdrop | exists=true
- [ ] CollectorOverlay.Backdrop | type=SBorder
- [ ] CollectorOverlay.Panel | exists=true
- [ ] CollectorOverlay.Panel | type=SBorder
- [ ] CollectorOverlay.Title | exists=true
- [ ] CollectorOverlay.Title | type=STextBlock
- [ ] CollectorOverlay.Tab.0 | exists=true
- [ ] CollectorOverlay.Tab.1 | exists=true
- [ ] CollectorOverlay.Tab.2 | exists=true
- [ ] CollectorOverlay.Tab.3 | exists=true
- [ ] CollectorOverlay.ItemCard.01 | exists=true
- [ ] CollectorOverlay.ItemCard.02 | exists=true
- [ ] CollectorOverlay.ItemCard.03 | exists=true
- [ ] CollectorOverlay.ItemCard.04 | exists=true
- [ ] CollectorOverlay.ItemCard.05 | exists=true
- [ ] CollectorOverlay.ItemCard.06 | exists=true
- [ ] CollectorOverlay.ItemCard.07 | exists=true
- [ ] CollectorOverlay.ItemCard.08 | exists=true
- [ ] CollectorOverlay.ItemCard.09 | exists=true
- [ ] CollectorOverlay.ItemCard.10 | exists=true
- [ ] CollectorOverlay.ItemCard.11 | exists=true
- [ ] CollectorOverlay.ItemCard.12 | exists=true
- [ ] CollectorOverlay.ItemCard.13 | exists=true
- [ ] CollectorOverlay.ItemCard.14 | exists=true
- [ ] CollectorOverlay.ItemCard.15 | exists=true
- [ ] CollectorOverlay.ItemIcon.01 | exists=true
- [ ] CollectorOverlay.ItemTitle.01 | exists=true
- [ ] CollectorOverlay.ItemDescription.01 | exists=true
- [ ] CollectorOverlay.ItemAddButton.01 | exists=true
- [ ] CollectorOverlay.CloseButton | exists=true
- [ ] CollectorOverlay.ExitButton | exists=true

## Geometry

- [ ] CollectorOverlay.Root | x=0.0000 | 0.005
- [ ] CollectorOverlay.Root | y=0.0000 | 0.005
- [ ] CollectorOverlay.Root | w=1.0000 | 0.005
- [ ] CollectorOverlay.Root | h=1.0000 | 0.005
- [ ] CollectorOverlay.Panel | x=0.2885 | 0.020
- [ ] CollectorOverlay.Panel | y=0.1680 | 0.020
- [ ] CollectorOverlay.Panel | w=0.4230 | 0.020
- [ ] CollectorOverlay.Panel | h=0.7580 | 0.020
- [ ] CollectorOverlay.Title | x=0.3080 | 0.020
- [ ] CollectorOverlay.Title | y=0.2027 | 0.020
- [ ] CollectorOverlay.Tab.0 | x=0.3110 | 0.020
- [ ] CollectorOverlay.Tab.0 | y=0.2620 | 0.020
- [ ] CollectorOverlay.Tab.1 | x=0.3583 | 0.020
- [ ] CollectorOverlay.Tab.1 | y=0.2620 | 0.020
- [ ] CollectorOverlay.Tab.2 | x=0.4041 | 0.020
- [ ] CollectorOverlay.Tab.2 | y=0.2620 | 0.020
- [ ] CollectorOverlay.Tab.3 | x=0.4613 | 0.020
- [ ] CollectorOverlay.Tab.3 | y=0.2620 | 0.020
- [ ] CollectorOverlay.ItemCard.01 | x=0.3125 | 0.020
- [ ] CollectorOverlay.ItemCard.01 | y=0.3313 | 0.020
- [ ] CollectorOverlay.ItemCard.01 | w=0.3660 | 0.020
- [ ] CollectorOverlay.ItemCard.01 | h=0.1707 | 0.020
- [ ] CollectorOverlay.ItemAddButton.01 | x=0.3770 | 0.020
- [ ] CollectorOverlay.ItemAddButton.01 | y=0.4261 | 0.020
- [ ] CollectorOverlay.CloseButton | x=0.3080 | 0.020
- [ ] CollectorOverlay.CloseButton | y=0.8353 | 0.020
- [ ] CollectorOverlay.ExitButton | x=0.3830 | 0.020
- [ ] CollectorOverlay.ExitButton | y=0.8353 | 0.020

## Colors

- [ ] CollectorOverlay.Panel | border_color=DefaultBorder
- [ ] CollectorOverlay.Panel | state=Default
- [ ] CollectorOverlay.Tab.0 | border_color=SelectedBorder
- [ ] CollectorOverlay.Tab.0 | state=Selected
- [ ] CollectorOverlay.Tab.1 | border_color=DefaultBorder
- [ ] CollectorOverlay.Tab.1 | state=Default
- [ ] CollectorOverlay.Tab.2 | border_color=DefaultBorder
- [ ] CollectorOverlay.Tab.2 | state=Default
- [ ] CollectorOverlay.Tab.3 | border_color=DefaultBorder
- [ ] CollectorOverlay.Tab.3 | state=Default
- [ ] CollectorOverlay.ItemCard.01 | border_color=DefaultBorder
- [ ] CollectorOverlay.ItemCard.01 | state=Default
- [ ] CollectorOverlay.ItemAddButton.01 | border_color=DefaultBorder
- [ ] CollectorOverlay.ItemAddButton.01 | state=Default
- [ ] CollectorOverlay.CloseButton | border_color=DefaultBorder
- [ ] CollectorOverlay.CloseButton | state=Default
- [ ] CollectorOverlay.ExitButton | border_color=SelectedBorder
- [ ] CollectorOverlay.ExitButton | state=Selected

## Content

- [ ] CollectorOverlay.Root | role=Overlay
- [ ] CollectorOverlay.Backdrop | role=Panel
- [ ] CollectorOverlay.Panel | role=Panel
- [ ] CollectorOverlay.Title | role=Label.Header
- [ ] CollectorOverlay.Title | text=The Collector
- [ ] CollectorOverlay.Title | font_size=20
- [ ] CollectorOverlay.Title | is_label=true
- [ ] CollectorOverlay.Tab.0 | role=Button
- [ ] CollectorOverlay.Tab.0 | text=Items
- [ ] CollectorOverlay.Tab.1 | role=Button
- [ ] CollectorOverlay.Tab.1 | text=NPCs
- [ ] CollectorOverlay.Tab.2 | role=Button
- [ ] CollectorOverlay.Tab.2 | text=Enemies
- [ ] CollectorOverlay.Tab.3 | role=Button
- [ ] CollectorOverlay.Tab.3 | text=Interactables
- [ ] CollectorOverlay.ItemCard.01 | role=Panel
- [ ] CollectorOverlay.ItemIcon.01 | role=Icon
- [ ] CollectorOverlay.ItemTitle.01 | role=Label.Body
- [ ] CollectorOverlay.ItemTitle.01 | text=Item_Stealing
- [ ] CollectorOverlay.ItemTitle.01 | is_label=true
- [ ] CollectorOverlay.ItemDescription.01 | role=Label.Caption
- [ ] CollectorOverlay.ItemDescription.01 | is_label=true
- [ ] CollectorOverlay.ItemAddButton.01 | role=Button
- [ ] CollectorOverlay.ItemAddButton.01 | text=ADD
- [ ] CollectorOverlay.CloseButton | role=Button
- [ ] CollectorOverlay.CloseButton | text=Close
- [ ] CollectorOverlay.ExitButton | role=Button
- [ ] CollectorOverlay.ExitButton | text=Exit The Lab

## Interactivity

- [ ] CollectorOverlay.Root | has_click_handler=false
- [ ] CollectorOverlay.Root | hover_capable=false
- [ ] CollectorOverlay.Backdrop | has_click_handler=false
- [ ] CollectorOverlay.Backdrop | hover_capable=false
- [ ] CollectorOverlay.Panel | has_click_handler=false
- [ ] CollectorOverlay.Panel | hover_capable=false
- [ ] CollectorOverlay.Tab.0 | has_click_handler=true
- [ ] CollectorOverlay.Tab.0 | hover_capable=true
- [ ] CollectorOverlay.Tab.0 | toggle_group=CollectorOverlayTabs
- [ ] CollectorOverlay.Tab.1 | has_click_handler=true
- [ ] CollectorOverlay.Tab.1 | hover_capable=true
- [ ] CollectorOverlay.Tab.1 | toggle_group=CollectorOverlayTabs
- [ ] CollectorOverlay.Tab.2 | has_click_handler=true
- [ ] CollectorOverlay.Tab.2 | hover_capable=true
- [ ] CollectorOverlay.Tab.2 | toggle_group=CollectorOverlayTabs
- [ ] CollectorOverlay.Tab.3 | has_click_handler=true
- [ ] CollectorOverlay.Tab.3 | hover_capable=true
- [ ] CollectorOverlay.Tab.3 | toggle_group=CollectorOverlayTabs
- [ ] CollectorOverlay.ItemCard.01 | has_click_handler=false
- [ ] CollectorOverlay.ItemCard.01 | hover_capable=false
- [ ] CollectorOverlay.ItemAddButton.01 | has_click_handler=true
- [ ] CollectorOverlay.ItemAddButton.01 | hover_capable=true
- [ ] CollectorOverlay.CloseButton | has_click_handler=true
- [ ] CollectorOverlay.CloseButton | hover_capable=true
- [ ] CollectorOverlay.ExitButton | has_click_handler=true
- [ ] CollectorOverlay.ExitButton | hover_capable=true
