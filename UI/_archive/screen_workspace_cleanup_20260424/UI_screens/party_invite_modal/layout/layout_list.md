# Party Invite Layout List

Reference gate target: generated screen-specific Party Invite modal reference. Preserve the current invite/empty-state hierarchy and restyle it into the canonical main-menu fantasy material language.

Generate exactly one full-screen 1920x1080 runtime-layout target image for the current captured state. Do not generate a layout board, state matrix, sprite sheet, annotated spec, multi-variant comparison, callout sheet, or asset sheet. Required variants and states below are implementation requirements only; they must not appear as separate panels in the canonical reference image.

## Required Inputs

- Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
- Current target screenshot: `C:\UE\T66\UI\screens\party_invite_modal\current\2026-04-24\current_runtime_1920x1080.png`
- Source layout: `C:\UE\T66\Source\T66\UI\Screens\T66PartyInviteModal.cpp`
- This layout list.

## Canvas And Capture Notes

- Canonical generated reference canvas: 1920x1080.
- Current screenshot decodes at the required 1920x1080. Use it as the exact current runtime structure capture.
- The screen is a modal (`bIsModal = true`) over a full-screen scrim.
- Current visible state may be empty if no pending invite exists; populated and action-in-flight states must also be represented in the layout contract.

## Screen And Backdrop Contract

- `canvas`: full 1920x1080 reference frame.
- `modal_scrim`: full-canvas dark translucent overlay.
- `dimmed_underlay`: inactive parent screen context only.
- `party_invite_modal_shell`: centered dialog frame.
- Visual target: fantasy party prompt with dark stone/bronze shell, readable row panels, success and danger button plates.

## Modal Bounds And Structure

- `party_invite_outer_slot`
  - Source alignment: center/center.
  - Content shell padding: left 42, top 30, right 42, bottom 30.
  - Contents: title, invite body row, optional status row, accept/reject row.

- `party_invite_shell`
  - Centered compact-to-medium panel. Width is content-driven by two 280 px buttons plus gaps and shell padding.
  - Generated shell ownership: frame/backplate only.
  - Must support both empty and populated invite body text without resizing unpredictably.

- `title_well`
  - Text: `PARTY INVITE`.
  - Source font: bold 34.
  - Centered with 18 px bottom gap.
  - Ownership: runtime/localizable text.

- `invite_body_row_shell`
  - Source row padding: 24 left/right, 18 top/bottom.
  - Position: below title with 24 px bottom gap.
  - Contains centered wrapped body text.
  - Generated row ownership: row frame/backplate only.

- `status_row_shell`
  - Optional row visible only when `ActionStatusText` is non-empty.
  - Source row padding: 18 left/right, 10 top/bottom.
  - Position: between invite body and buttons with 18 px bottom gap when visible.
  - Generated row ownership: row frame/backplate only.

- `button_row`
  - Centered horizontal row.
  - Source button slot horizontal padding: 10 px each.
  - Buttons: Accept then Reject.

## Text And Data Wells

- `invite_body_text`
  - Empty-state text: `No pending party invites.`
  - Populated text: `{HostDisplayName or HostSteamId} invited you to join their party.`
  - Ownership: runtime/localizable text with live backend data.
  - Must not be baked into runtime-intended art.

- `action_status_text`
  - Hidden in current idle state unless action status exists.
  - Possible live values: `Accepting invite...`, `Joining party...`, `Rejecting invite...`, backend failure/success message.
  - Ownership: runtime/live text.

## Controls

- `accept_button`
  - Label: `ACCEPT`; in-flight label: `WORKING...`.
  - Type: success.
  - Source min width: 280.
  - Source height: 52.
  - Enabled only when an invite exists and no action is in flight.
  - Required states: disabled empty, disabled in-flight, normal, hover, pressed, focused.

- `reject_button`
  - Label: `REJECT`; in-flight label: `PLEASE WAIT`.
  - Type: danger.
  - Source min width: 280.
  - Source height: 52.
  - Enabled only when an invite exists and no action is in flight.
  - Required states: disabled empty, disabled in-flight, normal, hover, pressed, focused.

## Ownership And Live Data

- Runtime/live data: host display name, host Steam ID fallback, lobby/app/invite action state.
- Runtime/localizable text: title, empty/body copy, status copy, button labels.
- Generated shell/chrome: modal frame, row frames, button state plates.
- Real controls: Accept and Reject buttons.
- Parent underlay remains runtime-owned context.

## Variants And Required States

- Empty state: body row says no pending invites; Accept and Reject disabled.
- Populated state: body row names host; Accept and Reject enabled.
- In-flight accept state: Accept label changes to `WORKING...`, Reject to `PLEASE WAIT`, status row visible.
- In-flight reject state: status row visible, both buttons disabled.
- Failure state: status row displays backend message and buttons may re-enable.

## Reference Generation Constraints

- Preserve centered compact modal hierarchy and two-button row.
- Do not invent avatars, party member cards, chat panels, or lobby details not present in source.
- Do not bake host names, status messages, or button labels into runtime-intended assets.
- Do not use the full generated reference as a runtime overlay.
