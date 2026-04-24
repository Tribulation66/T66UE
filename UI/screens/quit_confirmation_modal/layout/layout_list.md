# Quit Confirmation Layout List

Reference gate target: generated screen-specific Quit Confirmation modal reference. Preserve the current centered confirmation dialog hierarchy and restyle it into the canonical main-menu fantasy material language.

## Required Inputs

- Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
- Current target screenshot: `C:\UE\T66\UI\screens\quit_confirmation_modal\current\2026-04-24\current_runtime_1920x1080.png`
- Source layout: `C:\UE\T66\Source\T66\UI\Screens\T66QuitConfirmationModal.cpp`
- This layout list.

## Canvas And Capture Notes

- Canonical generated reference canvas: 1920x1080.
- Current screenshot decodes at the required 1920x1080. Use it as the exact current runtime structure capture.
- The screen is a modal (`bIsModal = true`) over a dimmed parent screen.
- Current visible state is the default confirm/cancel choice.

## Screen And Backdrop Contract

- `canvas`: full 1920x1080 reference frame.
- `modal_scrim`: full-canvas dark overlay. Parent screen remains dimmed and inactive.
- `dimmed_underlay`: parent UI behind scrim; context only.
- `quit_modal_shell`: centered wide confirmation panel.
- Visual target: fantasy warning dialog with dark carved frame, bronze/gold bevels, and danger/success button plates.

## Modal Bounds And Structure

- `quit_modal_outer_slot`
  - Source helper: `T66ScreenSlateHelpers::MakeCenteredScrimModal`.
  - Horizontal alignment: center.
  - Vertical alignment: center.
  - Width override: 760.
  - Safe padding: none beyond helper defaults.

- `quit_panel_shell`
  - Width: 760 logical px.
  - Shell content padding: left 48, top 36, right 48, bottom 40.
  - Contents: title, message text well, two-button row.
  - Generated shell ownership: frame/backplate only.

- `title_well`
  - Text: `QUIT GAME?`.
  - Source font: bold 38 with wide letter spacing.
  - Position: top centered; 18 px bottom gap.
  - Ownership: runtime/localizable text.

- `message_well`
  - Text: `Are you sure you want to quit?`.
  - Source width: 590.
  - Source font: regular 19, wraps and centers.
  - Position: below title with 28 px bottom gap.
  - Ownership: runtime/localizable text.

- `button_row`
  - Centered horizontal two-button row.
  - Buttons: Stay then Quit.
  - Do not reverse order.

## Controls

- `stay_button`
  - Label: `NO, I WANT TO STAY`.
  - Type: success.
  - Source min width: 306.
  - Source height: 58.
  - Source padding: 18 left/right, 8 top, 7 bottom.
  - Action: close modal and return to parent screen.
  - Required states: normal, hover, pressed, focused, disabled.

- `quit_button`
  - Label: `YES, I WANT TO QUIT`.
  - Type: danger.
  - Source min width: 306.
  - Source height: 58.
  - Source padding: 18 left/right, 8 top, 7 bottom.
  - Action: quit game.
  - Required states: normal, hover, pressed, focused, disabled.

## Ownership And Live Data

- Runtime/localizable text: title, message, both button labels.
- Real controls: both buttons.
- Generated shell/chrome: modal frame/backplate and button state plates.
- No live numeric values or player data are shown.
- Parent underlay remains runtime-owned context.

## Variants And Required States

- Default state: both buttons enabled.
- Hover/pressed/focus for both buttons.
- Disabled state for both button types, even if not normally shown.
- Parent underlay can be main menu or another frontend screen; modal placement and shell stay unchanged.

## Reference Generation Constraints

- Preserve the centered 760 px confirmation panel and two-button layout.
- Do not add extra copy, icons, checkboxes, keyboard shortcuts, or titlebar close controls.
- Do not bake localizable text into runtime-intended assets.
- Do not use the full generated reference as a runtime overlay.
