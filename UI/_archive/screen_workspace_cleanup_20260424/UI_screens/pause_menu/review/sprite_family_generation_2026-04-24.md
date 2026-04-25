# Sprite Family Generation - 2026-04-24

Attempted family: `modal-shell`, `vertical-command-buttons`.

Inputs:
- `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
- `C:\UE\T66\UI\screens\pause_menu\reference\canonical_reference_1920x1080.png`
- `C:\UE\T66\UI\screens\pause_menu\layout\reference_layout_overlay.png`

Generated helper:
- `C:\UE\T66\UI\screens\pause_menu\assets\generated\20260424-131534-create-one-production-sprite-family-board-for-t6.png`
- `C:\UE\T66\UI\screens\pause_menu\assets\generated\20260424-113151-native-imagegen-pause-sprite-family-board-helper-only.png`

Verdict: rejected for production.

Reason:
- Both tested family boards were generated at `1672x941`.
- The screen workflow forbids promoting non-canonical generated outputs as production sprite sheets, slices, scene plates, or runtime assets.

Next required action:
- Regenerate this family through a backend that can produce acceptable canonical/native-proportion production output, then slice only the accepted board.
