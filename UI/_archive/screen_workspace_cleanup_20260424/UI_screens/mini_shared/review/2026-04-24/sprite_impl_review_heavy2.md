# Mini Screen Sprite Implementation Review - Heavy2 - 2026-04-24

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
- Runtime captures: `UI/screens/<screen_slug>/outputs/2026-04-24/packaged_sprite_impl_heavy2.png`
- Runtime contact sheet: `UI/screens/mini_scope_contact_sheet_2026-04-24_sprite_impl_heavy2.png`
- Reference/runtime diff sheet: `UI/screens/mini_scope_reference_runtime_diff_2026-04-24_heavy2.png`
- Diff metrics: `UI/screens/mini_shared/review/2026-04-24/sprite_impl_heavy2_metrics.txt`

## Result

Verdict: close with blockers.

The heavy title/footer/button component board is staged and visible in packaged runtime. Title plaques, footer/action bars, metric chips, and generated buttons now resolve from individual promoted sprites instead of falling back to flat Slate rectangles. Visible controls remain real Slate buttons with live/localizable labels layered over generated plates.

## Remaining Blockers

- Asset: this is still a shared component family, so it does not match each screen-specific reference tightly enough for final visual parity.
- Asset: the heavy title and footer plates are usable but should be replaced by screen-specific plaque/button families before final polish.
- Layout: Mini Idol Select remains the densest screen and needs dedicated idol slot and offer row assets with larger live-text wells.
- Layout: Mini Run Summary is readable after the added metric row, but still needs a dedicated result-summary panel family.
- Text-fit: Mini Save Slots bottom helper copy and compact stat labels remain small at 1920x1080 packaged scale.

## Next Stage

Route the remaining blockers back through screen-specific `ui-sprite-families`, then return to `ui-runtime-reconstruction` for tighter per-screen placement and state coverage.
