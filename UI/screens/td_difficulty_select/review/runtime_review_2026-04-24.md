# TD Difficulty Select Runtime Review - 2026-04-24

Packaged capture: `C:\UE\T66\UI\screens\td_difficulty_select\outputs\2026-04-24\packaged_capture_v2.png`

Verdict: close with blockers.

## What Improved

- Top title no longer clips or overlaps the back button area.
- Map preview wells now scale through a fill container instead of relying on raw image desired size.
- Live map names, difficulty text, map stats, and start button remain runtime-owned.

## Remaining Deltas

- Live TD map art is still the existing schematic map plates, not the richer painterly map art shown in the generated reference.
- Difficulty cards are still crowded compared with the target reference.
- Panel and card chrome uses shared first-pass generated chrome rather than a dedicated screen-local family.

Next stage: dedicated text-free difficulty/map-card sprite family and a decision on whether TD map backgrounds should be regenerated as live map assets.
