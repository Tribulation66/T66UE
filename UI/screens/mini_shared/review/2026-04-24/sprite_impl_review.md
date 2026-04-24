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
- Runtime captures: `UI/screens/<screen_slug>/outputs/2026-04-24/packaged_sprite_impl_final.png`
- Review sheet: `UI/screens/mini_scope_reference_runtime_diff_2026-04-24.png`
- Runtime contact sheet: `UI/screens/mini_scope_contact_sheet_2026-04-24_sprite_impl_final.png`

## Result

Verdict: close with blockers.

The generated component sprites are now staged and visible in packaged runtime. The regenerated clean stat-chip slice is present in the staged NonUFS tree and replaced the previous flat fallback on Character, Companion, Difficulty, and Idol-style compact wells.

## Remaining Blockers

- Asset: several generated slices still carry rectangular dark board backgrounds instead of clean transparent silhouettes, most visible around title plaques and compact chrome.
- Asset: current common board is a first-pass shared family; it does not match every screen-specific reference tightly enough for final comparison.
- Layout: Mini Idol Select remains dense and should get a screen-specific row/slot family with more generous live-text wells.
- Layout: Mini Run Summary is functional but sparse versus its reference and needs a dedicated summary-row/panel family before final polish.
- Text-fit: small live labels in Idol slots and compact stat chips remain barely legible at packaged scale.

## Next Stage

Route the remaining blockers back through `ui-sprite-families` for cleaner screen-specific transparent component families, then return to `ui-runtime-reconstruction` for a second placement pass.
