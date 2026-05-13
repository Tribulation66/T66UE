# Party Invite Modal Structural Preservation Checklist

Source inventory: `C:\UE\T66\UI\Geometry\party_invite_modal_structural_inventory.md`

Baseline capture: `C:\UE\T66\Saved\Codex\UI\PartyInviteModal\baseline_capture.png`

Baseline dump: `C:\UE\T66\Saved\Codex\UI\PartyInviteModal\baseline_dump.json`

## Structure

- [ ] PartyInviteModal.Root | exists=true
- [ ] PartyInviteModal.Scrim | exists=true
- [ ] PartyInviteModal.ModalPanel | exists=true
- [ ] PartyInviteModal.Title | exists=true
- [ ] PartyInviteModal.BodyRow | exists=true
- [ ] PartyInviteModal.AcceptButton | exists=true
- [ ] PartyInviteModal.RejectButton | exists=true

## Geometry

- [ ] PartyInviteModal.Root | x=0.000 | 0.005
- [ ] PartyInviteModal.Root | y=0.000 | 0.005
- [ ] PartyInviteModal.Root | w=1.000 | 0.005
- [ ] PartyInviteModal.Root | h=1.000 | 0.005
- [ ] PartyInviteModal.Scrim | x=0.000 | 0.005
- [ ] PartyInviteModal.Scrim | y=0.000 | 0.005
- [ ] PartyInviteModal.Scrim | w=1.000 | 0.005
- [ ] PartyInviteModal.Scrim | h=1.000 | 0.005
- [ ] PartyInviteModal.ModalPanel | x=0.241 | 0.020
- [ ] PartyInviteModal.ModalPanel | y=0.323 | 0.020
- [ ] PartyInviteModal.ModalPanel | w=0.519 | 0.020
- [ ] PartyInviteModal.ModalPanel | h=0.353 | 0.020
- [ ] PartyInviteModal.Title | x=0.427 | 0.025
- [ ] PartyInviteModal.Title | y=0.363 | 0.025
- [ ] PartyInviteModal.Title | w=0.146 | 0.025
- [ ] PartyInviteModal.Title | h=0.065 | 0.025
- [ ] PartyInviteModal.BodyRow | x=0.272 | 0.020
- [ ] PartyInviteModal.BodyRow | y=0.452 | 0.020
- [ ] PartyInviteModal.BodyRow | w=0.456 | 0.020
- [ ] PartyInviteModal.BodyRow | h=0.083 | 0.020
- [ ] PartyInviteModal.AcceptButton | x=0.283 | 0.020
- [ ] PartyInviteModal.AcceptButton | y=0.567 | 0.020
- [ ] PartyInviteModal.AcceptButton | w=0.210 | 0.020
- [ ] PartyInviteModal.AcceptButton | h=0.069 | 0.020
- [ ] PartyInviteModal.RejectButton | x=0.508 | 0.020
- [ ] PartyInviteModal.RejectButton | y=0.567 | 0.020
- [ ] PartyInviteModal.RejectButton | w=0.210 | 0.020
- [ ] PartyInviteModal.RejectButton | h=0.069 | 0.020

## Colors

- [ ] PartyInviteModal.ModalPanel | button_state=Default
- [ ] PartyInviteModal.ModalPanel | border_color=DefaultBorder
- [ ] PartyInviteModal.BodyRow | button_state=Default
- [ ] PartyInviteModal.AcceptButton | button_state=Disabled
- [ ] PartyInviteModal.RejectButton | button_state=Disabled

## Content

- [ ] PartyInviteModal.Title | text=PARTY INVITE
- [ ] PartyInviteModal.Title | is_label=true
- [ ] PartyInviteModal.BodyRow | text=No pending party invites.
- [ ] PartyInviteModal.AcceptButton | text=ACCEPT
- [ ] PartyInviteModal.RejectButton | text=REJECT

## Interactivity

- [ ] PartyInviteModal.AcceptButton | has_click_handler=true
- [ ] PartyInviteModal.AcceptButton | hover_capable=true
- [ ] PartyInviteModal.RejectButton | has_click_handler=true
- [ ] PartyInviteModal.RejectButton | hover_capable=true
