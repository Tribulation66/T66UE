# Report Bug Layout List

Reference gate target: generated screen-specific Report Bug modal reference. Preserve the current compact bug-report form hierarchy and restyle it into the canonical main-menu fantasy material language.

## Required Inputs

- Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
- Current target screenshot: `C:\UE\T66\UI\screens\report_bug\current\2026-04-24\current_runtime_1920x1080.png`
- Source layout: `C:\UE\T66\Source\T66\UI\Screens\T66ReportBugScreen.cpp`
- This layout list.

## Canvas And Capture Notes

- Canonical generated reference canvas: 1920x1080.
- Current screenshot decodes at the required 1920x1080. Use it as the exact current runtime structure capture.
- The screen is a modal (`bIsModal = true`) over a full-screen scrim.
- Current visible state is an empty text field with Submit disabled until text is entered.

## Screen And Backdrop Contract

- `canvas`: full 1920x1080 reference frame.
- `modal_scrim`: full-canvas dark translucent overlay, runtime or reference-only treatment.
- `dimmed_underlay`: inactive parent UI/gameplay visible behind the scrim only as context.
- `report_bug_modal_shell`: centered compact form panel.
- Visual target: dark fantasy frame with carved stone/bronze edges, readable dark inner surfaces, and restrained gold/green accents.

## Modal Bounds And Structure

- `report_bug_modal_outer_slot`
  - Source alignment: center/center.
  - Shell content padding: left 40, top 30, right 40, bottom 30.
  - Contents: title, multiline text field, two-button row.

- `report_bug_panel_shell`
  - Centered compact panel, approximately 500 px wide after padding.
  - Generated shell ownership: frame/backplate only.
  - Do not bake title, hint text, typed text, or button labels into runtime-intended shell art.

- `title_well`
  - Text: `REPORT BUG`.
  - Source font: bold 28.
  - Position: centered at top with 16 px bottom gap.
  - Ownership: runtime/localizable text.

- `bug_text_field_shell`
  - Source field size: width 400, height 120.
  - Source field padding: 12 left/right, 10 top/bottom.
  - Position: below title with 16 px bottom gap.
  - Generated shell ownership: text-entry frame/backplate only.
  - Runtime-owned content: hint text and typed multiline report text.

- `button_row`
  - Centered horizontal row below text field.
  - Source slot padding: 10 px horizontal on each button.
  - Buttons: Submit then Cancel.

## Controls

- `bug_report_textbox`
  - Real `SMultiLineEditableTextBox`.
  - Hint text: `Describe the bug...`.
  - Runtime value: player-entered text.
  - Required states: empty, focused, text-entered, disabled/read-only fallback.
  - Text must be readable on dark fantasy field interior.

- `submit_button`
  - Label: `SUBMIT`.
  - Type: success/primary.
  - Source min width: 140.
  - Source height: 52.
  - Enabled only when trimmed bug text is not empty.
  - Required states: disabled empty, normal, hover, pressed, focused.

- `cancel_button`
  - Label: `CANCEL`.
  - Type: neutral.
  - Source min width: 140.
  - Source height: 52.
  - Required states: normal, hover, pressed, focused, disabled.

## Ownership And Live Data

- Runtime/localizable text: title, hint text, Submit label, Cancel label.
- Runtime/live content: typed report text.
- Runtime action data: generated local bug report content and optional backend submission are not visible in the modal.
- Generated shell/chrome: modal frame, text field frame, button plates.
- Real controls: text box and both buttons.

## Variants And Required States

- Empty state: text box empty, Submit disabled.
- Typing state: text box focused with player text, Submit enabled.
- Submit state: modal closes after local/optional backend submission; no separate loading UI is currently visible.
- Cancel state: closes modal; if opened during gameplay pause, Pause Menu reopens.
- Field and button hover/focus states must be legible and not shift layout.

## Reference Generation Constraints

- Preserve compact centered form layout.
- Do not add attachment upload areas, category dropdowns, checkboxes, or explanatory paragraphs.
- Do not bake localizable or user-entered text into runtime-intended assets.
- Do not use the full generated reference as a runtime overlay.
- Keep generated reference as the offline full-screen comparison target only.
