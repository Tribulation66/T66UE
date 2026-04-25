# TD Battle Runtime Review - 2026-04-24

Packaged capture: `C:\UE\T66\UI\screens\td_battle\outputs\2026-04-24\packaged_capture_v2.png`

Verdict: close with blockers.

## What Improved

- Battle HUD keeps real hero roster rows, drag/drop board, stats, status text, and action buttons.
- Right-side no-selection title now matches the generated reference casing.
- Column sizing is back to the less-crowded layout after the first failed adjustment.

## Remaining Deltas

- Gameplay board still uses the current live schematic map image and custom runtime board paint, so it does not match the painterly board in the generated reference.
- Roster bottom row remains close to the panel edge and should get a dedicated row-family pass.
- Upgrade disabled states are readable but still reuse generic button plates instead of a battle-specific disabled family.

Next stage: battle-specific board frame/row/button sprite family, then a separate live TD map-art decision.

## Sprite-Family Runtime Pass

Packaged capture: `C:\UE\T66\UI\screens\td_battle\outputs\2026-04-24\packaged_capture_final.png`

Verdict: first-pass mechanical gate passed; still not final 1:1.

What changed:

- Roster shell/rows, stats bar, board frame, status bar, right-side panels, and action buttons now use the TD battle component family under `SourceAssets/TD/UI/td_battle/Components`.
- The battle board now displays the regenerated live Easy-tier map background art under `SourceAssets/TD/Maps/Backgrounds`.
- The bottom status message was constrained into a fixed-height generated status bar so it no longer clips off the packaged 1920x1080 capture.

Remaining risks:

- Hero portraits/sprites, hero names, costs, wave values, materials, hearts, and status values remain runtime-owned, so they should be manually validated rather than strict-diffed.
- The runtime board still owns pads, hit testing, tower placement, hover, and drag/drop behavior over the generated map art.
- Some generated component edges still show slight green/despill tinting at high zoom.
