Working task:
Operator: Codex
Validator: Claude
Scope: Blender-only correction to the outline pass: keep the raw Pixal3D look unchanged and replace the Freestyle detail-line treatment with a Kirby-like outer silhouette around the character only. No Unreal and no base material changes.
Stop condition: Blender is open on a corrected comparison scene, with proof render and recipe updated to document the outer-edge-only outline method.

Operator draft:
- The previous Freestyle line-art approach was replaced because it drew black lines over clothing/details.
- The corrected builder is `C:\UE\T66\Reports\AgentReviews\20260605_Hero1MaleRubberBlenderLookDev\build_hero1_male_raw_outline_only.py`.
- The corrected `.blend` is `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Hero_1_Chad_Male_RawOutlineOnly.blend`.
- The corrected final proof render is `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Renders\Hero_1_Chad_Male_raw_vs_black_outline.png`.
- The recipe files are:
  - `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\raw_black_outline_recipe.md`
  - `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\raw_black_outline_recipe.json`
- The correct live Blender process to judge is `45256`; it opens the corrected blend and switches to scene `Raw_Pixal3D_OuterOnly_Final_Proof_Image`.
- Older Blender processes are still present:
  - `24848`: old rejected rubber/video comparison blend.
  - `27156`: old instance of the same outline blend, potentially stale in memory.

Implementation details:
- Visible left and right characters remain raw Pixal3D renders.
- No base material/shader/texture changes were made to the raw GLB model.
- Freestyle is disabled for the corrected final target.
- The right character is rendered to a white luminance mask.
- The mask is converted to an outer silhouette overlay with a 7 px radius.
- That overlay is composited pixel-for-pixel over a raw-vs-raw base render.
- A final proof image scene is added to the `.blend` so opening Blender shows the corrected visual target.

Verification:
- `blender.exe --background --factory-startup --python build_hero1_male_raw_outline_only.py` completed successfully.
- Saved mask: `Hero_1_Chad_Male_outline_mask.png`.
- Saved overlay: `Hero_1_Chad_Male_outer_only_outline_overlay.png`.
- Saved base raw comparison: `Hero_1_Chad_Male_raw_vs_raw_base.png`.
- Saved final proof: `Hero_1_Chad_Male_raw_vs_black_outline.png`.
- Visual inspection: the final proof no longer has black lines over jacket folds, cravat, buttons, or material details. The black treatment is restricted to the outside silhouette/visible separated silhouette edges, matching the user's Kirby-like correction.

PPF close:
Process used: raw Pixal3D preservation plus outer-only silhouette overlay generated from the right character mask.
Matches declared process: YES.

Mechanism close:
- Preserve raw material: PRESENT. Evidence: base raw render and no visible-material edits in builder.
- Draw only outside silhouette: PRESENT. Evidence: final proof has no Freestyle-style internal detail strokes.
- Side-by-side comparison: PRESENT. Evidence: final proof render and proof image scene in Blender.
