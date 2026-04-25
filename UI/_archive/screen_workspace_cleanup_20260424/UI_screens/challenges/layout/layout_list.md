# Challenges Layout List

Reference gate target: generated screen-specific Challenges modal reference. Preserve the current Challenges/Mods catalog hierarchy and restyle it into the canonical main-menu fantasy material language. Do not redesign the modal into a new screen.

## Required Inputs

- Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
- Current target screenshot: `C:\UE\T66\UI\screens\challenges\current\2026-04-24\current_runtime_1920x1080.png`
- Source layout: `C:\UE\T66\Source\T66\UI\Screens\T66ChallengesScreen.cpp`
- This layout list.

## Canvas And Capture Notes

- Canonical generated reference canvas: 1920x1080.
- Current screenshot decodes at the required 1920x1080. Use it as the exact current runtime structure capture.
- Source-driven nominal 1920x1080 layout when the safe frame matches the canvas:
  - Modal width: min(safe frame width * 0.94, 1180), normally 1180.
  - Modal height: min(safe frame height * 0.92, 790), normally 790.
  - Modal anchor: centered with 24 left/right and 28 top/bottom safety padding available.
  - Nominal centered modal bounds: x 370, y 145, w 1180, h 790.
- Current screenshot structure shows a larger centered modal with the same hierarchy: one header/topbar panel, one left list panel, one right detail/editor panel, all over a dimmed game/menu backdrop.

## Screen And Backdrop Contract

- This is a modal screen (`bIsModal = true`), not a full page.
- Backdrop: dimmed main-menu/game context behind the modal. It may show low-contrast fantasy scenery and existing background chrome, but must read as inactive context.
- Backdrop ownership: generated offline reference may include the dim context for comparison, but no runtime UI controls, text, list rows, or modal chrome should be baked into a production background plate later.
- Modal shell: one centered fantasy dialog frame with dark carved stone/bronze/wood surfaces, green-gold accent trim, and a readable parchment-dark interior.
- Modal shell ownership: generated-shell. It must not contain baked live text, values, item names, author names, avatar images, badges, or button labels in runtime-intended assets.
- Backdrop interaction: no visible backdrop button. Closing is performed by the visible X button in the modal header.

## Modal Structure

- `challenges_modal_outer_shell`
  - Nominal bounds: centered 1180x790 on 1920x1080.
  - Source padding: 4 px around modal content.
  - Content direction: vertical stack.
  - Visual target: ornate main-menu fantasy frame with sturdy bevels, aged metal corners, dark inner surface, and subtle magical edge highlights.

- `header_panel`
  - Position: top of modal, full modal content width.
  - Source shell: row shell panel with 18 px horizontal and 12 px vertical content padding.
  - Contents:
    - Left title well.
    - Top mode tabs at upper right.
    - Close X button at far upper right.
    - Source tabs and create button on second row.
    - Status message line on third row.
  - Ownership: generated-shell plus live text and real buttons.

- `body_split`
  - Position: below header, fills remaining modal height.
  - Layout: horizontal two-column split.
  - Source spacing: body padding 14 px; column gap 12 px.
  - Left panel: list column.
  - Right panel: detail/editor column.

## Header And Topbar Elements

- `header_title`
  - Text: `Challenges` when Challenge tab active, `Mods` when Mods tab active.
  - Source font size: bold 28.
  - Ownership: runtime/localizable text. Do not bake into runtime art.
  - Visual target: large fantasy title inside the header, aligned left.

- `top_mode_tabs`
  - Buttons: `Challenges`, `Mods`.
  - Each button source size: 150x34.
  - Active state: ToggleOn family.
  - Inactive state: CompactNeutral family.
  - Required states for each tab plate: normal, hover, pressed, selected, disabled.
  - Ownership: real visible buttons with generated state plates and live labels.
  - Behavior variants:
    - Challenges selected: title reads `Challenges`, Challenge entries/rules are shown.
    - Mods selected: title reads `Mods`, Mod entries/rules are shown, create button reads `CREATE MOD`.

- `close_button`
  - Text/icon: `X`.
  - Source size: 38x38.
  - Family: ToggleOff.
  - Required states: normal, hover, pressed, disabled.
  - Ownership: real visible button. The X label/glyph is runtime-owned unless separately approved as icon art.

- `source_tabs`
  - Buttons: `Official`, `Community`.
  - Each button source size: 136x32.
  - Active state: ToggleOn family.
  - Inactive state: CompactNeutral family.
  - Required states: normal, hover, pressed, selected, disabled.
  - Ownership: real visible buttons with generated state plates and live labels.
  - Behavior variants:
    - Official selected: official entries shown.
    - Community selected: community browser/drafts shown.

- `create_button`
  - Text: `CREATE CHALLENGE` or `CREATE MOD`.
  - Source size: 188x32.
  - Family: ToggleOn.
  - Required states: normal, hover, pressed, disabled.
  - Ownership: real visible button with live/localizable label.

- `status_message_line`
  - Position: below source tabs, left aligned, wraps if needed.
  - Source font size: regular 10.
  - Text examples: `Community catalog refreshed (3 entries).`, backend/catalog status, empty string.
  - Ownership: runtime/live status text. Do not bake into shell art.

## Left List Panel

- `entry_list_panel_shell`
  - Position: left body column.
  - Source list content width: max(340, modal width - detail width - 72), nominally about 577 px at modal width 1180.
  - Source panel padding: 12 px.
  - Visual target: fantasy list well with a distinct generated frame, dark interior, and enough empty interior for a scroll list.
  - Ownership: generated-shell only.

- `entry_list_scroll_area`
  - Position: inside left shell.
  - Behavior: vertical scroll box.
  - Ownership: runtime list content.
  - Empty state: centered text `No community entries yet. Create the first one.` or `No official entries were found.`

- `entry_row`
  - Repeats for each official/community/draft entry.
  - Source row height: 74 px.
  - Source row vertical gap: 6 px.
  - Selected row family: ToggleOn.
  - Unselected row family: CompactNeutral.
  - Required row states: normal, hover, pressed, selected, disabled.
  - Row content:
    - Confirm marker square, 20x20.
    - Entry title text.
    - Origin/status badge at right of top row.
    - Reward summary text on lower row.
    - Author display name or draft submission status on lower right.
  - Ownership: row shell/button plate is generated; marker state, title, badge text, reward value, author/status strings are runtime-owned.
  - Visual target: list rows should become ornate but compact, readable fantasy selection plates. Preserve row density and two-line structure.

- `confirmed_marker`
  - Source: 20x20 square, shows `X` when the entry is already active/confirmed.
  - Ownership: runtime state well. Do not bake a permanent X into the shell.

- `origin_badge`
  - Text examples: `Official`, `Community`, `Draft`.
  - Ownership: runtime/localizable badge text. Badge backing may be a generated small plate family.

## Right Detail Panel

- `detail_panel_shell`
  - Position: right body column.
  - Source detail content width: max(380, modal width * 0.45), nominally about 531 px at modal width 1180.
  - Source panel padding: 16 px.
  - Visual target: larger fantasy reading panel with dark interior, not a new card layout.
  - Ownership: generated-shell only.

- `no_selection_detail`
  - Text: `Select an entry or create a new draft.`
  - Ownership: runtime/localizable text.

- `selected_entry_detail`
  - Scrollable content for selected non-editor entry.
  - Content order:
    - Entry title, source font bold 30.
    - Optional avatar 52x52 at left of origin/author line.
    - Origin/author line such as `Official by Tribulation 66`.
    - Description paragraph.
    - Detail list header: `Rules And Requirements` for challenges, `Rules` for mods.
    - Rule list panel with bullet rows.
    - Reward summary headline, source font bold 22.
    - Selection summary/status paragraph.
    - Confirm/selected action area for official/community entries, or draft action row for drafts.
  - Ownership: all text, avatar imagery, rule lines, reward values, and selection status are runtime/live content.
  - Rule list shell: generated inset row shell, but its bullet text is runtime-owned.

- `rule_constraint_row`
  - Repeated bullet row inside selected entry detail.
  - Content: `-` bullet plus one wrapped rule text.
  - Ownership: runtime/localizable text.

- `confirm_button`
  - Text: `CONFIRM` or `SELECTED`.
  - Source size: 174x40.
  - Family: ToggleOn.
  - Required states: normal, hover, pressed, selected/confirmed, disabled.
  - Ownership: real visible button with live label.

- `draft_action_row`
  - Visible when selected entry is a draft and editor is not open.
  - Buttons:
    - `EDIT`, CompactNeutral, 100x34.
    - `SUBMIT`, ToggleOn, 112x34.
    - `PLAY`, ToggleOn, 112x34.
    - `DELETE`, ToggleOff, 112x34.
  - Required states for each: normal, hover, pressed, disabled.
  - Ownership: real visible buttons with live/localizable labels.

## Draft Editor Variant

- `draft_editor_detail`
  - Replaces selected entry detail when draft editor is active.
  - Scrollable vertical form inside the right detail panel.
  - Header text: `Create Challenge` or `Create Mod`, source font bold 28.
  - Ownership: generated detail shell plus runtime form controls and live text.

- `draft_title_field`
  - Label: `Title`.
  - Control: editable text box wrapped in generated row shell.
  - Ownership: real editable text field; typed text is runtime-owned.

- `draft_description_field`
  - Label: `Description`.
  - Control: multi-line editable text box wrapped in generated row shell.
  - Ownership: real editable text field; typed text is runtime-owned.

- `suggested_reward_row`
  - Visible for Challenge drafts.
  - Label: `Suggested Chad Coupons`.
  - Controls: minus button, numeric value, plus button.
  - Minus/plus source size: 28x24, CompactNeutral.
  - Required button states: normal, hover, pressed, disabled.
  - Ownership: runtime label/value and real buttons.

- `gameplay_rules_section`
  - Header: `Gameplay Rules`.
  - Rows:
    - `Start Level` stepper.
    - `Max Hero Stats` toggle button, label `Enabled` or `Disabled`.
    - Bonus stat steppers: `Damage`, `Attack Speed`, `Attack Scale`, `Accuracy`, `Armor`, `Evasion`, `Luck`, `Speed`.
    - Cycle rows: `Starting Item`, `Passive Override`, `Ultimate Override`.
  - Stepper controls: minus and plus buttons, 28x24, CompactNeutral.
  - Cycle controls: previous `<` and next `>` buttons, 28x24, CompactNeutral.
  - Max Hero Stats toggle size: 100x24, ToggleOn when enabled, CompactNeutral when disabled.
  - Required states: normal, hover, pressed, selected/toggled, disabled.
  - Ownership: runtime labels, values, enum names, and real buttons.

- `completion_requirements_section`
  - Visible for Challenge drafts.
  - Header: `Completion Requirements`.
  - Checkboxes:
    - `Require full clear`.
    - `Require no damage`.
  - Steppers:
    - `Minimum Stage Reached`.
    - `Max Run Time (Seconds)`.
  - Ownership: real checkbox and button controls; labels/values runtime-owned.
  - Required checkbox states: unchecked, checked, hover, disabled.

- `draft_editor_footer_buttons`
  - Buttons:
    - `SAVE DRAFT`, ToggleOn, 136x34.
    - `SUBMIT`, ToggleOn, 120x34.
    - `PLAY DRAFT`, CompactNeutral, 120x34.
    - `CANCEL`, ToggleOff, 112x34.
  - Required states for each: normal, hover, pressed, disabled.
  - Ownership: real visible buttons with live/localizable labels.

## Button Families And State Requirements

- `compact_neutral_button`
  - Used by inactive top/source tabs, inactive rows, neutral action buttons, steppers, cycle arrows, and `PLAY DRAFT`.
  - Required states: normal, hover, pressed, disabled.

- `toggle_on_button`
  - Used by active tabs, selected rows, create, confirm, submit, save, enabled toggle, and selected-like buttons.
  - Required states: normal, hover, pressed, selected, disabled.

- `toggle_off_button`
  - Used by close, cancel, delete, and negative/destructive controls.
  - Required states: normal, hover, pressed, disabled.

- `toggle_inactive_button`
  - Source family exists as inactive/disabled treatment.
  - Required states: normal, hover if applicable, pressed if applicable, disabled.

## Live Runtime Ownership

- All screen labels, tab labels, button labels, section headers, status messages, titles, descriptions, rule lines, rewards, author names, origin badges, draft labels, enum labels, numeric values, checkbox labels, and empty states are runtime/localizable text.
- Entry list data is live: number of rows, selected entry, confirmed entry, origin, reward, author, draft status, and order can change.
- Detail panel content is live: selected title, author/origin, optional avatar, description, rules, reward summary, selection summary, draft status, and action availability.
- Avatar images are runtime-owned 52x52 image wells fetched from author avatar URLs. The generated reference may show a placeholder only.
- Scroll positions are runtime-owned and must not be implied by baked art.
- Generated runtime art later must be limited to shell frames, row plates, badge plates, button state plates, and decorative trim.

## Required Variants

- Challenges tab active, Official source active, selected official entry with Confirm button.
- Challenges tab active, Community source active, selected draft entry with Edit/Submit/Play/Delete buttons.
- Mods tab active, Official source active, selected mod entry with rules detail.
- Mods tab active, Community source active, create button reading `CREATE MOD`.
- Empty official list.
- Empty community list.
- Draft editor active for Challenge draft.
- Draft editor active for Mod draft.
- Selected entry already confirmed, with confirm button label `SELECTED` and marker shown in list row.
- Backend/catalog status line populated, empty, and long enough to wrap.

## Reference Generation Constraints

- Preserve the modal hierarchy exactly: header/topbar, source controls, status line, left scroll list, right detail/editor panel.
- Restyle the existing modal to match the main-menu fantasy language: carved dark stone, bronze/gold trim, subtle green magic accents, jewel-like buttons, readable dark insets.
- Do not convert this modal into a full-screen page or add new major panels.
- Do not remove the two-column body split.
- Do not move the close button, top tabs, source tabs, create button, entry list, or detail/editor area outside their current hierarchy.
- The full generated reference may include representative readable text for visual planning, but runtime-derived art must be text-free except approved icon/display art.
- Do not bake live/localizable labels, names, rewards, rules, author data, draft values, status messages, badges, avatars, or selected-state markers into runtime-intended art.
- Leave enough room for wrapped status messages, long entry titles, long author names, long rule lines, and long draft enum values without clipping.
- Later sprite work must keep shell rects, visible control rects, and live-content rects distinct.
- Bad generated structure or contaminated runtime-owned regions should be fixed by regenerating the screen-specific reference, not by manual pixel repair.
