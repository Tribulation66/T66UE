# Save Preview Layout List

Reference gate target: generated screen-specific Save Preview modal reference. Preserve the current preview underlay with bottom action panel and restyle the modal chrome into the canonical main-menu fantasy material language.

Generate exactly one full-screen 1920x1080 runtime-layout target image for the current captured preview state. Do not generate a layout board, state matrix, sprite sheet, annotated spec, multi-variant comparison, callout sheet, or asset sheet. Required variants and states below are implementation requirements only; they must not appear as separate panels in the canonical reference image.

## Required Inputs

- Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
- Current target screenshot: `C:\UE\T66\UI\screens\save_preview\current\2026-04-24\current_runtime_1920x1080.png`
- Source layout: `C:\UE\T66\Source\T66\UI\Screens\T66SavePreviewScreen.cpp`
- This layout list.

## Canvas And Capture Notes

- Canonical generated reference canvas: 1920x1080.
- Current screenshot decodes at the required 1920x1080. Use it as the exact current runtime structure capture.
- The screen is a modal (`bIsModal = true`) over a live save/run preview.
- The visible UI owned by this modal is the full-screen dim overlay plus a bottom-centered action panel.

## Screen And Backdrop Contract

- `canvas`: full 1920x1080 reference frame.
- `preview_underlay`
  - Full-canvas runtime-owned gameplay/save preview scene.
  - It may contain live world visuals, character state, gameplay HUD remnants, or preview camera content depending on save data.
  - Generated reference may show it as dimmed context, but no runtime asset should bake preview pixels into modal art.

- `preview_scrim`
  - Full-canvas black overlay, source opacity about 0.58.
  - Purpose: lets bottom controls read over the live preview.
  - Ownership: runtime overlay or generated/reference treatment, not part of foreground shell.

- `bottom_action_panel`
  - Bottom-centered foreground panel.
  - Source width: 760.
  - Source slot padding: left 32, top 32, right 32, bottom 28 around outer panel slot.
  - Visual target: main-menu fantasy command panel with dark carved frame and gold title emphasis.

## Panel Bounds And Structure

- `save_preview_panel_outer_slot`
  - Horizontal alignment: center.
  - Vertical alignment: bottom.
  - Bottom padding: 28.
  - Width override: 760.

- `save_preview_shell`
  - Width: 760.
  - Shell content padding: left 42, top 30, right 42, bottom 34.
  - Contents: title, subtitle, two-button row.
  - Generated shell ownership: frame/backplate only.

- `title_well`
  - Text: `PREVIEW`.
  - Source font: bold 24.
  - Position: centered at top of bottom panel.
  - Ownership: runtime/localizable text.

- `subtitle_well`
  - Text: `The run is paused for inspection. Back returns to Save Slots, Load resumes normally.`
  - Source font: regular 13.
  - Auto-wrapped and centered.
  - Position: 6 px below title, 18 px bottom gap before buttons.
  - Ownership: runtime/localizable text.

- `button_row`
  - Centered horizontal row.
  - Buttons: Back then Load.
  - Source row gap: 12 px between buttons.

## Controls

- `back_button`
  - Label: `BACK`.
  - Type: neutral.
  - Source min width: 160.
  - Source height: 48.
  - Action: clears preview mode, returns to Save Slots/frontend.
  - Required states: normal, hover, pressed, focused, disabled.

- `load_button`
  - Label: `LOAD`.
  - Type: primary.
  - Source min width: 160.
  - Source height: 48.
  - Action: closes modal, clears preview mode, resumes loaded run.
  - Required states: normal, hover, pressed, focused, disabled.

## Ownership And Live Data

- Runtime-owned underlay: save preview/gameplay scene, any camera content, any gameplay-state visuals.
- Runtime/localizable text: title, subtitle, Back label, Load label.
- Real controls: Back and Load buttons.
- Generated shell/chrome: bottom action panel frame and button state plates.
- Do not bake preview underlay, labels, or live data into any runtime foreground art.

## Variants And Required States

- Default preview state: panel visible, both buttons enabled.
- Hover/pressed/focus for both buttons.
- Disabled button states required for component family completeness.
- Underlay variant: different save slots may show different gameplay/preview scenes; panel layout is unchanged.

## Reference Generation Constraints

- Preserve bottom-centered 760 px panel placement.
- Do not redesign into a full-screen dialog or add side panels.
- Do not obscure the preview underlay more than the current scrim/panel hierarchy does.
- Do not bake localizable text into runtime-intended assets.
- Do not use the full generated reference as a runtime overlay.
