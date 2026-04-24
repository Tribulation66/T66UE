# Player Summary Picker Layout List

Reference gate target: generated screen-specific reference for the Player Summary Picker modal.

Generate exactly one full-screen 1920x1080 runtime-layout target image for the current captured empty/no-player state. Do not generate a layout board, state matrix, sprite sheet, annotated spec, multi-variant comparison, callout sheet, or asset sheet. Required populated variants and states below are implementation requirements only; they must not appear as separate panels in the canonical reference image.

Canvas: canonical `1920x1080`.

Current target screenshot:
- `C:\UE\T66\UI\screens\player_summary_picker\current\2026-04-24\current_runtime_1920x1080.png`
- The file decodes at the required `1920x1080`. Use it as the exact current runtime structure capture.

Required image-generation inputs:
- Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
- Current target screenshot listed above.
- This layout list.

Source audited:
- `C:\UE\T66\Source\T66\UI\Screens\T66PlayerSummaryPickerScreen.cpp`
- `C:\UE\T66\Source\T66\UI\Screens\T66PlayerSummaryPickerScreen.h`

## Structural Contract

- Preserve the current modal hierarchy: a full-screen dimming scrim over the existing frontend scene, with a centered picker panel.
- Current visible state is the empty/no-player state.
- Do not redesign the picker into a full page, side panel, account dashboard, or character select screen.
- The modal should read as part of the main-menu fantasy family while staying simple and focused.

## Canvas And Bounds

- `canvas`: full-screen reference canvas, `1920x1080`.
- `modal_scrim`: full-canvas translucent dark overlay. Current capture dims the shared topbar, logo/title area, friends/party left panel, center CTA stack, and right edge panel behind the modal.
- `dimmed_underlay`: runtime parent screen visible behind the scrim. It is context only and not owned by the picker modal.
- `shared_topbar_under_scrim`: topbar remains visible but dimmed and non-interactive while the modal is open.
- `empty_state_modal_shell`: centered panel. Current decoded screenshot approximate bounds are `x 376-1088`, `y 371-704`; preserve this centered placement and compact vertical hierarchy in canonical reference.
- `empty_state_content_well`: centered stack inside the modal shell with title and empty message.
- `populated_state_modal_shell`: source variant uses a wider centered panel, logical width `900`, for horizontal player option cards.

## Current Empty-State Regions

- `full_screen_scrim`: modal-owned translucent dark overlay. It should dim all content behind it evenly.
- `modal_panel_shell`: generated fantasy panel frame/backplate, centered. Current styling is a plain dark panel; generated reference should add carved stone/bronze frame and deep interior without changing size or hierarchy.
- `modal_title_well`: centered runtime text well for `Pick the Player`.
- `empty_message_well`: centered runtime text well for `No players.`
- `underlay_topbar`: shared runtime topbar behind scrim; visible only as dimmed context.
- `underlay_main_menu_content`: shared/runtime parent screen behind scrim; visible only as dimmed context and not picker-owned.

## Populated-State Regions

When pending player snapshots exist, the modal changes to:

- `populated_modal_panel_shell`: centered wider panel, source logical width `900`.
- `populated_modal_title_well`: centered runtime title `Pick the Player`.
- `player_options_row`: horizontal row of player option cards. Source comment expects two or three options for duo/trio leaderboard rows; code iterates all pending snapshots.
- `player_option_card`: repeated generated row/card shell for each candidate player.
- `player_name_well`: centered runtime display name. Fallback text is `Player` if the snapshot has no display name.
- `hero_portrait_socket`: `96x96` runtime hero portrait well.
- `hero_avatar_frame`: generated foreground frame around the portrait; must not include the portrait pixels.
- `select_button_slot`: centered real button below the portrait. Label is `SELECT`.

## Controls And Required States

- `select_player_button`: real button on each populated player card.
  - Required states: `normal`, `hover`, `pressed`, `disabled`.
  - Click selects that snapshot, closes this modal, and opens Run Summary.
- `player_option_card`: visual card may also need interactive feedback if card-level selection is added later.
  - Required states: `normal`, `hover`, `pressed`, `disabled`.
- `modal_scrim`: blocks interaction with all underlay controls while open.
  - Required states: `active`, `closing`.
- There is no visible Cancel or Back button owned by this modal in the source. Do not invent one in the generated reference.

## Live Runtime Content And Ownership

- `modal_title_text`: `Pick the Player`; runtime/localizable.
- `empty_message_text`: `No players.`; runtime/localizable.
- `player_display_name`: runtime value from each pending snapshot.
- `hero_portrait`: runtime image loaded from hero/body/portrait data.
- `select_label`: `SELECT`; runtime/localizable.
- `pending_snapshot_count`: runtime data determining empty versus populated layout.
- `dimmed_underlay`: parent screen runtime composition; not picker-owned and not a source for picker sprites.
- `shared_topbar`: shared runtime/global UI behind the scrim.

## Variants

- `empty_current`: no pending snapshots; centered compact modal with title and `No players.`
- `two_player_picker`: two option cards across the populated modal.
- `three_player_picker`: three option cards across the populated modal.
- `many_player_picker`: code can iterate more snapshots; keep equal-width card behavior and preserve modal width unless a later source change introduces scrolling/wrapping.
- `portrait_loading`: portrait socket may be empty or placeholder while async texture loads.
- `missing_snapshot_name`: display name falls back to `Player`.
- `loading_error`: fallback source path can show `Error loading picker.` if the leaderboard subsystem or UI manager is unavailable.

## Reference Constraints

- Preserve the modal centered over the dimmed current frontend screen.
- Restyle only the modal shell, card shells, avatar frame, and button plates to match the main-menu fantasy style.
- Do not paint new foreground content into the dimmed underlay. The underlay is runtime-owned context behind the scrim.
- Do not bake the title, empty message, player names, hero portraits, or `SELECT` labels into runtime-intended art.
- The empty-state reference should not include player cards.
- The populated-state notes must be available for later generation even though the current screenshot is empty.
- Keep modal panel edges, text wells, and card rows stable between normal/hover/pressed states; button state art must not jump.
- Full-screen reference output is an offline comparison target only. Later runtime work must use a UI-free underlay/scrim treatment plus foreground modal/card/button families.
