# Load Game UI Fidelity Checklist

Source reference: `C:\UE\T66\UI\Screen References\LOAD.png`

Session 2.5 migration scope: `UT66SaveSlotsScreen` flat chrome migration. Save-slot content remains dynamic; the current automation fixture has empty slots, so slot action buttons are disabled.

## Structure

- [ ] SaveSlots.BackButton | exists=true
- [ ] SaveSlots.PartyFilterDropdown | exists=true
- [ ] SaveSlots.Slot1.PreviewButton | exists=true
- [ ] SaveSlots.Slot1.LoadButton | exists=true
- [ ] SaveSlots.Slot2.PreviewButton | exists=true
- [ ] SaveSlots.Slot2.LoadButton | exists=true
- [ ] SaveSlots.Slot3.PreviewButton | exists=true
- [ ] SaveSlots.Slot3.LoadButton | exists=true
- [ ] SaveSlots.Slot4.PreviewButton | exists=true
- [ ] SaveSlots.Slot4.LoadButton | exists=true
- [ ] SaveSlots.PrevButton | exists=true
- [ ] SaveSlots.NextButton | exists=true

## Geometry

- [ ] SaveSlots.BackButton | x=0.088 | 0.010
- [ ] SaveSlots.BackButton | y=0.056 | 0.010
- [ ] SaveSlots.PartyFilterDropdown | x=0.087 | 0.012
- [ ] SaveSlots.PartyFilterDropdown | y=0.146 | 0.012
- [ ] SaveSlots.Slot1.PreviewButton | x=0.115 | 0.012
- [ ] SaveSlots.Slot1.PreviewButton | y=0.495 | 0.012
- [ ] SaveSlots.Slot1.LoadButton | x=0.297 | 0.012
- [ ] SaveSlots.Slot1.LoadButton | y=0.495 | 0.012
- [ ] SaveSlots.Slot2.PreviewButton | x=0.529 | 0.012
- [ ] SaveSlots.Slot2.PreviewButton | y=0.495 | 0.012
- [ ] SaveSlots.Slot2.LoadButton | x=0.711 | 0.012
- [ ] SaveSlots.Slot2.LoadButton | y=0.495 | 0.012
- [ ] SaveSlots.Slot3.PreviewButton | x=0.115 | 0.012
- [ ] SaveSlots.Slot3.PreviewButton | y=0.845 | 0.012
- [ ] SaveSlots.Slot3.LoadButton | x=0.297 | 0.012
- [ ] SaveSlots.Slot3.LoadButton | y=0.845 | 0.012
- [ ] SaveSlots.Slot4.PreviewButton | x=0.529 | 0.012
- [ ] SaveSlots.Slot4.PreviewButton | y=0.845 | 0.012
- [ ] SaveSlots.Slot4.LoadButton | x=0.711 | 0.012
- [ ] SaveSlots.Slot4.LoadButton | y=0.845 | 0.012
- [ ] SaveSlots.PrevButton | x=0.091 | 0.012
- [ ] SaveSlots.PrevButton | y=0.926 | 0.012
- [ ] SaveSlots.NextButton | x=0.191 | 0.012
- [ ] SaveSlots.NextButton | y=0.926 | 0.012

## Colors

- [ ] SaveSlots.BackButton | button_state=Selected
- [ ] SaveSlots.PartyFilterDropdown | button_state=Selected
- [ ] SaveSlots.Slot1.PreviewButton | button_state=Disabled
- [ ] SaveSlots.Slot1.LoadButton | button_state=Disabled
- [ ] SaveSlots.Slot2.PreviewButton | button_state=Disabled
- [ ] SaveSlots.Slot2.LoadButton | button_state=Disabled
- [ ] SaveSlots.Slot3.PreviewButton | button_state=Disabled
- [ ] SaveSlots.Slot3.LoadButton | button_state=Disabled
- [ ] SaveSlots.Slot4.PreviewButton | button_state=Disabled
- [ ] SaveSlots.Slot4.LoadButton | button_state=Disabled
- [ ] SaveSlots.PrevButton | button_state=Disabled
- [ ] SaveSlots.NextButton | button_state=Disabled

## Content

- [ ] SaveSlots.BackButton | text=BACK
- [ ] SaveSlots.PartyFilterDropdown | text=Solo
- [ ] SaveSlots.Slot1.PreviewButton | text=PREVIEW
- [ ] SaveSlots.Slot1.LoadButton | text=LOAD
- [ ] SaveSlots.Slot2.PreviewButton | text=PREVIEW
- [ ] SaveSlots.Slot2.LoadButton | text=LOAD
- [ ] SaveSlots.Slot3.PreviewButton | text=PREVIEW
- [ ] SaveSlots.Slot3.LoadButton | text=LOAD
- [ ] SaveSlots.Slot4.PreviewButton | text=PREVIEW
- [ ] SaveSlots.Slot4.LoadButton | text=LOAD
- [ ] SaveSlots.PrevButton | text=PREV
- [ ] SaveSlots.NextButton | text=NEXT

## Interactivity

- [ ] SaveSlots.BackButton | has_click_handler=true
- [ ] SaveSlots.BackButton | hover_capable=true
- [ ] SaveSlots.PartyFilterDropdown | has_click_handler=true
- [ ] SaveSlots.PartyFilterDropdown | hover_capable=true
- [ ] SaveSlots.Slot1.PreviewButton | has_click_handler=true
- [ ] SaveSlots.Slot1.PreviewButton | hover_capable=false
- [ ] SaveSlots.Slot1.LoadButton | has_click_handler=true
- [ ] SaveSlots.Slot1.LoadButton | hover_capable=false
- [ ] SaveSlots.Slot2.PreviewButton | has_click_handler=true
- [ ] SaveSlots.Slot2.PreviewButton | hover_capable=false
- [ ] SaveSlots.Slot2.LoadButton | has_click_handler=true
- [ ] SaveSlots.Slot2.LoadButton | hover_capable=false
- [ ] SaveSlots.Slot3.PreviewButton | has_click_handler=true
- [ ] SaveSlots.Slot3.PreviewButton | hover_capable=false
- [ ] SaveSlots.Slot3.LoadButton | has_click_handler=true
- [ ] SaveSlots.Slot3.LoadButton | hover_capable=false
- [ ] SaveSlots.Slot4.PreviewButton | has_click_handler=true
- [ ] SaveSlots.Slot4.PreviewButton | hover_capable=false
- [ ] SaveSlots.Slot4.LoadButton | has_click_handler=true
- [ ] SaveSlots.Slot4.LoadButton | hover_capable=false
- [ ] SaveSlots.PrevButton | has_click_handler=true
- [ ] SaveSlots.PrevButton | hover_capable=false
- [ ] SaveSlots.NextButton | has_click_handler=true
- [ ] SaveSlots.NextButton | hover_capable=false
