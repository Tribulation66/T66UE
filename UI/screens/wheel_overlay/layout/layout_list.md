# Wheel Overlay Layout List

Reference gate target: Wheel spin overlay/popup.

## Regions

- `wheel_overlay_shell` - centered popup frame; ownership: generated-shell.
- `wheel_title_status` - title/status line; ownership: runtime-text.
- `wheel_disk_socket` - circular/disk visual well; ownership: generated-shell plus runtime-state rotation/color.
- `spin_button` - real spin button; ownership: generated button states with live label.
- `back_button` - real close/back button; ownership: generated button states with live label.
- `reward_result_line` - payout/result text; ownership: runtime-text/runtime-value.

## Controls And States

- `spin_button` - required states: normal, hover, pressed, disabled/spinning.
- `back_button` - required states: normal, hover, pressed, disabled while resolving if applicable.
- `wheel_disk` - required states: idle, spinning, resolved.

## Live Runtime Content

- Live text: title, status/result, button labels.
- Live values: rarity, pending gold, spin angle/result.
- Live image/icon/avatar wells: none unless wheel symbols are added later.
- Live preview/media areas: spinning wheel disk is runtime-state.

## Variants

- Idle before spin.
- Spinning.
- Resolved reward.
