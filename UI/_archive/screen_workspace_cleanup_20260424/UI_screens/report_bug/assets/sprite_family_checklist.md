# Sprite Family Checklist - report_bug

Canonical reference: `C:\UE\T66\UI\screens\report_bug\reference\canonical_reference_1920x1080.png`

Required families:
- `modal-shell`: TODO generated board validation and slices
- `multiline-input-shell`: TODO generated board validation and slices
- `compact-buttons`: TODO generated board validation and slices

Validation requirements:
- no baked runtime/localizable text in runtime slices
- generated shells keep live wells empty
- button states share stable anchors
- nine-slice margins documented before runtime integration
- no full-screen reference image is used as a runtime overlay
