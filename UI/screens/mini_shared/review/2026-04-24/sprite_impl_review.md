# Mini Screen Sprite Implementation Review - 2026-04-24

## Scope

Reviewed packaged 1920x1080 captures for:
- Mini Save Slots
- Mini Character Select
- Mini Companion Select
- Mini Difficulty Select
- Mini Idol Select
- Mini Run Summary

## Inputs

- Screen references: `UI/screens/<screen_slug>/reference/canonical_reference_1920x1080.png`
- Runtime captures: `UI/screens/<screen_slug>/outputs/2026-04-24/packaged_sprite_impl_alpha2.png`
- Review sheet: `UI/screens/mini_scope_reference_runtime_diff_2026-04-24_alpha2.png`
- Runtime contact sheet: `UI/screens/mini_scope_contact_sheet_2026-04-24_sprite_impl_alpha2.png`

## Result

Verdict: close with smaller blockers.

The generated component sprites are staged and visible in packaged runtime. A second built-in Codex `image_gen` pass produced a chroma-key component board, converted locally to alpha, then deterministically sliced into runtime `*_alpha.png` assets. The earlier rectangular board backgrounds are no longer the dominant failure, and custom screen-local generated buttons now avoid the old flat rectangular Slate backing while remaining real buttons with live labels.

## Remaining Blockers

- Asset: current common transparent board is still a first-pass shared family; it does not match every screen-specific reference tightly enough for final comparison.
- Asset: title plaques are cleaner but too thin/simple versus several references and need screen-specific plaque variants.
- Layout: Mini Idol Select remains dense and should get a screen-specific row/slot family with more generous live-text wells.
- Layout: Mini Run Summary is more structured after the added metric row but still needs a dedicated summary-row/panel family before final polish.
- Text-fit: small live labels in Idol slots, compact stat chips, and bottom-row controls remain barely legible at packaged scale.

## Next Stage

Route the remaining blockers back through `ui-sprite-families` for screen-specific transparent component families, then return to `ui-runtime-reconstruction` for tighter reference-specific placement.
