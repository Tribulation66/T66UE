# TD Main Menu Runtime Review - 2026-04-24

Packaged capture: `C:\UE\T66\UI\screens\td_main_menu\outputs\2026-04-24\packaged_capture_v2.png`

Verdict: close with blockers.

## What Improved

- Runtime no longer falls back to a black background in packaged capture.
- Back button, side panels, CTA frame, and CTA buttons remain real Slate controls/live text.
- Layout remains the TD launcher layout from the generated reference gate.

## Remaining Deltas

- Background scene uses the staged main-menu fantasy plate rather than a TD-main-menu-specific UI-free scene plate from the generated TD target.
- Title remains live Slate text, not the fully rendered gold wordmark look of the reference.
- Panel/card chrome uses shared first-pass generated chrome rather than a screen-local TD sprite family.

Next stage: `ui-sprite-families` for a TD-main-menu-specific UI-free scene plate and text-free panel/button family.

## Sprite-Family Runtime Pass

Packaged capture: `C:\UE\T66\UI\screens\td_main_menu\outputs\2026-04-24\packaged_capture_final.png`

Verdict: first-pass mechanical gate passed; still not final 1:1.

What changed:

- Runtime now uses the generated TD dungeon scene plate at `SourceAssets/TD/UI/td_main_menu/Scene/scene_plate.png`.
- Side panel, CTA, and button chrome are loaded from the TD component family under `SourceAssets/TD/UI/td_main_menu/Components`.
- The title remains live Slate text but was restyled and repositioned to avoid clipping while moving closer to the generated gold reference.

Remaining risks:

- The live title font is still not the baked stone wordmark in the screen reference.
- Runtime text line wrapping differs from the reference skeleton because labels and body copy remain live/localizable.
- Some generated component edges still show slight green/despill tinting at high zoom.
