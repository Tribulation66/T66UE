# Pause Menu Layout List

Reference gate target: generated screen-specific Pause Menu modal reference. Preserve the current centered pause-command hierarchy and restyle it into the canonical main-menu fantasy material language. Do not redesign the modal into a different pause screen.

## Required Inputs

- Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
- Current target screenshot: `C:\UE\T66\UI\screens\pause_menu\current\2026-04-24\current_runtime_1920x1080.png`
- Source layout: `C:\UE\T66\Source\T66\UI\Screens\T66PauseMenuScreen.cpp`
- This layout list.

## Canvas And Capture Notes

- Canonical generated reference canvas: 1920x1080.
- Current screenshot decodes at the required 1920x1080. Use it as the exact current runtime structure capture.
- The screen is a modal (`bIsModal = true`) over dimmed gameplay/menu context.
- The modal content is a narrow centered command panel with one title and six vertical buttons.

## Screen And Backdrop Contract

- `canvas`: full 1920x1080 reference frame.
- `modal_scrim`: full-canvas translucent dark overlay. It dims the underlying scene and marks all background UI as inactive context.
- `dimmed_underlay`: runtime-owned parent screen/gameplay view behind the scrim. It may appear in the full-screen offline reference as context only.
- `pause_modal_shell`: centered fantasy dialog frame. Runtime assets later must not use a full-screen reference image as a background.
- Visual language: dark carved stone, aged bronze/gold bevels, worn dungeon material, subtle green/blue/purple accent glows consistent with main-menu controls.

## Modal Bounds And Structure

- `pause_modal_outer_slot`
  - Source uses `T66ScreenSlateHelpers::MakeCenteredScrimModal`.
  - Safe padding: `FT66Style::GetSafePadding(FMargin(24,24))`.
  - Horizontal alignment: center.
  - Vertical alignment: center.
  - Width override: 420 logical px in the current fantasy path.
  - Content is a shell with padding left 38, top 34, right 38, bottom 38.

- `pause_panel_shell`
  - Bounds: centered narrow vertical panel, approximately 420 px wide before shell padding.
  - Generated shell ownership: panel frame/backplate only.
  - Do not bake title, button labels, or button plate states into a single runtime background.
  - Needs enough inner height for title plus six 66 px command buttons with 12 px vertical rhythm.

- `title_well`
  - Text: `PAUSED`.
  - Source font: bold, about 44 px in fantasy path.
  - Position: centered at top of modal with 18 px bottom gap.
  - Ownership: runtime/localizable text. Do not bake into runtime-intended shell art.

## Controls

All controls must be real visible buttons with generated normal, hover, pressed, focused, and disabled states. Preserve button order and spacing.

- `resume_button`
  - Label: `RESUME GAME`.
  - Type: success/primary.
  - Source min width: 332.
  - Source height: 66.
  - Action: close modal, unpause, restore gameplay input.

- `save_and_quit_button`
  - Label: `SAVE AND QUIT`.
  - Type: neutral.
  - Source min width: 332.
  - Source height: 66.
  - Action: save run and return to frontend.

- `restart_button`
  - Label: `RESTART`.
  - Type: danger.
  - Source min width: 332.
  - Source height: 66.
  - Action: reset run state and reopen entry level.

- `settings_button`
  - Label: `SETTINGS`.
  - Type: neutral.
  - Source min width: 332.
  - Source height: 66.
  - Action: opens Settings modal.

- `achievements_button`
  - Label: `ACHIEVEMENTS`.
  - Type: neutral.
  - Source min width: 332.
  - Source height: 66.
  - Action: opens Achievements modal.

- `leaderboard_button`
  - Label: `LEADERBOARD`.
  - Type: neutral.
  - Source min width: 332.
  - Source height: 66.
  - Action: opens Leaderboard modal.

## Ownership And Live Data

- Runtime/localizable text: title and all six button labels.
- Real controls: all six buttons.
- Generated shell/chrome: modal frame, button plates, optional dividers or edge accents.
- Runtime-owned underlay: the dimmed game/menu content behind the scrim.
- No dynamic values are shown in the current pause menu body.

## Variants And Required States

- Normal pause modal state: all six buttons enabled.
- Hover/pressed/focus state for every button.
- Disabled state for every button family, even if not visible in the current capture.
- Navigation variants: Settings, Achievements, and Leaderboard buttons replace the current modal with another modal; do not include those child modals in this reference.
- Backdrop variants: may appear over gameplay or frontend context. The modal shell and layout stay unchanged.

## Reference Generation Constraints

- Preserve the narrow centered vertical hierarchy.
- Do not add side panels, icons, explanatory copy, keyboard shortcut text, or decorative cards.
- Do not bake localizable text into runtime-intended assets.
- Do not use the full generated reference as a runtime overlay.
- Generate fantasy material only for shell and component families; keep button labels live.
