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
