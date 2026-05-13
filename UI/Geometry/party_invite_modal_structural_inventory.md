# Party Invite Modal Structural Inventory

Source baseline capture: `C:\UE\T66\Saved\Codex\UI\PartyInviteModal\baseline_capture.png`

Source baseline dump: `C:\UE\T66\Saved\Codex\UI\PartyInviteModal\baseline_dump.json`

Reference mode: no external V3 reference image exists for Party Invite Modal. This inventory is the structural source for the Stage 2 no-reference migration.

Normalized basis: 1920x1080.

## Baseline Summary

- Screen: `PartyInvite`
- Baseline widgets: `40`
- Baseline tagged widgets: `0`
- Shared top bar: not present
- Visible layout: centered modal over gameplay/frontend backdrop.
- Current baseline state: no pending party invites; accept and reject actions are disabled.

## Regions And Elements

| Element | Tag | x | y | w | h | Text / role |
| --- | --- | ---: | ---: | ---: | ---: | --- |
| Root | `PartyInviteModal.Root` | 0.000 | 0.000 | 1.000 | 1.000 | Modal root. |
| Scrim | `PartyInviteModal.Scrim` | 0.000 | 0.000 | 1.000 | 1.000 | Full-screen modal scrim. |
| Modal panel | `PartyInviteModal.ModalPanel` | 0.241 | 0.323 | 0.519 | 0.353 | Centered party-invite panel. |
| Title | `PartyInviteModal.Title` | 0.427 | 0.363 | 0.146 | 0.065 | `PARTY INVITE`. |
| Body row | `PartyInviteModal.BodyRow` | 0.272 | 0.452 | 0.456 | 0.083 | Empty state message row. |
| Accept button | `PartyInviteModal.AcceptButton` | 0.283 | 0.567 | 0.210 | 0.069 | Disabled `ACCEPT` action. |
| Reject button | `PartyInviteModal.RejectButton` | 0.508 | 0.567 | 0.210 | 0.069 | Disabled `REJECT` action. |

## Structural Notes

- With a pending invite, the body row text changes to the host invite message and the action buttons become enabled.
- While an accept/reject action is in flight, a transient status row may appear between the message row and the action buttons.
- The no-invite baseline preserves disabled accept/reject buttons because the modal can be opened directly for automation even with an empty invite queue.
