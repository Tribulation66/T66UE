# Hero Selection Stage 2 Content Differences

Stage 1 migrated Hero Selection chrome to `FT66FlatStyle` and preserved the current live content pipeline. These differences remain content or later-stage polish items, not Stage 1 helper/chrome blockers.

## Reference And Capture

- V3 target: `C:\UE\T66\UI\Screen References\Hero Selection.png`
- Stage 1 capture: `C:\UE\T66\UI\Screen References\Hero Selection.Stage1.png`

## Logged Differences

- The live center preview uses the existing in-world hero preview stage and current cooked hero mesh/material treatment. The V3 reference uses a cleaner painted hero composition and darker stage lighting.
- The live screen still reflects the existing global retro presentation pipeline over the whole frame. Stage 1 only bypasses the old PNG/glow chrome path for flat Hero Selection surfaces.
- The current live skin list only exposes the available skin rows from the existing skin subsystem. The V3 reference shows additional locked/purchasable skin portrait rows.
- Party card portraits and Steam/party presentation are preserved from the existing live data path. Any brand-correct Steam art cleanup should happen with the broader content reconciliation pass.
- Weapon and ultimate art are preserved from the existing live hero data, but their exact scale and spacing should be tuned with final V3 content assets in Stage 2.
