# Sprite Family Checklist - daily_climb

Canonical reference: `C:\UE\T66\UI\screens\daily_climb\reference\canonical_reference_1920x1080.png`

Required families:
- `scene-plate`: TODO generated board validation and slices
- `rules-panel`: TODO generated board validation and slices
- `cta-stack`: TODO generated board validation and slices
- `leaderboard-shell`: TODO generated board validation and slices
- `row-card`: TODO generated board validation and slices
- `compact-back-button`: TODO generated board validation and slices
- `wide-cta-buttons`: TODO generated board validation and slices

Validation requirements:
- no baked runtime/localizable text in runtime slices
- generated shells keep live wells empty
- button states share stable anchors
- nine-slice margins documented before runtime integration
- no full-screen reference image is used as a runtime overlay
