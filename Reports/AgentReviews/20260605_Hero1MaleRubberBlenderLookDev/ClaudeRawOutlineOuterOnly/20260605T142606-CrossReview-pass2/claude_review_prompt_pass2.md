You are Claude cross-reviewing a Codex draft for the T66 Unreal project.

Rules:
- Include a clear Result: OK or Result: NEEDS_USER line near the top.
- Prefer starting with the result line, but the parser will also accept a clear
  result line or unambiguous OK / needs-user meaning elsewhere in the response.
- Do not edit files.
- Do not run mutating commands.
- Treat Codex as the Operator/final router and you as the Validator.
- Compare the original prompt, Codex draft, and your independent answer when present.
- Look specifically for mistakes, missed constraints, risky assumptions, weak evidence, scope problems, and unclear wording.
- Patch the answer text when the fix is straightforward.
- Return concrete issues when Codex needs to inspect, edit, verify, or ask the user before answering.
- Ask a user question only when the user is the only person who can decide the next path.
- Keep the review concise and practical. Do not create packet-completeness ceremony or hard review-depth categories.

Your result should be one of these two lines:
Result: OK
Result: NEEDS_USER

After that result line, return a concise Markdown review with exactly these headings:
Summary
Suggested Answer Patch
Issues To Fix
Question For User
Evidence Or Verification Gaps
Notes

Result meanings:
- OK: the models can handle the prompt internally. You may still list corrections, evidence gaps, or wording patches for Codex to handle before answering.
- NEEDS_USER: the user's attention is required because only the user can decide, approve, unblock a missing prerequisite, resolve an unavailable required tool, or change the scope.

Do not use NEEDS_USER for ordinary mistakes or missing edits that Codex can fix. List those inside the review body and keep the result OK.

Review scope:
- Original prompt path: C:\UE\T66\Reports\AgentReviews\20260605_Hero1MaleRubberBlenderLookDev\raw_outline_outer_only_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\20260605_Hero1MaleRubberBlenderLookDev\raw_outline_outer_only_operator_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\20260605_Hero1MaleRubberBlenderLookDev\ClaudeRawOutlineOuterOnly\20260605T140741-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
Original user request:
No that tries to add black lines over every detail, I want it like kirby where its just around the outer of edges of the character.

Working task:
Operator: Codex
Validator: Claude
Scope: Blender-only correction to the outline pass: keep the raw Pixal3D look unchanged and replace the Freestyle detail-line treatment with a Kirby-like outer silhouette around the character only. No Unreal and no base material changes.
Stop condition: Blender is open on a corrected comparison scene, with proof render and recipe updated to document the outer-edge-only outline method.

Relevant repo rules:
- C:\UE\T66\AGENTS.md: derive task contract, use Codex operator / Claude validator, no native goal tools, run current verification, report token usage.
- Model Generation/MODEL_GENERATION_AGENTS.md owns Pixal3D/Blender QA.
- Model Generation/Instructions/11_FRIENDSLOP_RAW_PIXAL3D_IMPORT_GUIDELINES.md says raw FriendSlop visual work must preserve the generated GLB texture/material unless the user explicitly asks for processing. The user explicitly asks for a Blender outline-only look-dev pass, not Unreal import.

Current problem:
- Existing proof `Hero_1_Chad_Male_raw_vs_black_outline.png` uses Blender Freestyle. It correctly leaves the left copy raw, but the right copy has black lines over many internal details. The user rejected this.

Planned correction:
- Replace Freestyle detail-line mechanism with an outer-only silhouette method.
- Keep the visible right-hand character raw.
- Add a separate black outline shell behind the right-hand character only, slightly expanded so it shows only around the outside edge.
- Remove Freestyle from the corrected scene.
- Render proof and update recipe to state that the outline is a separate shell / silhouette carrier and not interior line art.

Ask for Claude:
- Provide an independent repo-grounded answer/check: is this the right Blender-only correction? Surface any scope or verification issue before Codex edits the builder.

</original_prompt>

<validator_independent_answer>
Result: OK

## Independent Answer

The planned correction is the right *category* of fix, and it's correctly scoped (Blender-only, raw material preserved, no Unreal). But the specific "separate expanded shell behind the character" implementation has a known failure mode on Pixal3D meshes that can reproduce the exact problem the user rejected — so Codex should choose the method deliberately, not just swap mechanisms.

Why the current Freestyle render fails: the line set already enables only `select_silhouette` + `select_external_contour` (lines 56–59). The internal black lines the user sees come from **`select_silhouette`** (interior silhouette where the surface turns away) combined with the fact that the Pixal3D GLB is almost certainly **multi-part** (separate body/clothing meshes). With `use_same_object=True`, every disjoint piece draws its own external contour, so seams between pieces read as interior lines. Kirby is a single unbroken outer silhouette — one closed boundary for the whole character.

The critical risk: a **scaled/inverted-hull shell will recreate the same seams.** If you duplicate the multi-part character, push it back, and expand it, each separate piece gets its own black halo, and wherever pieces overlap or abut you'll see black between them — i.e., internal lines again. So the "shell" method only gives a true single outer edge if the shell is treated as one merged silhouette.

Two ways to get a genuinely outer-only result:
- **Preferred (most robust): compositor silhouette dilation.** Render the right character to an alpha/object-index pass, dilate the mask a few pixels in the compositor, fill black, composite *behind* the raw character. This is inherently a single outer silhouette regardless of how many mesh parts exist — no internal seams possible. Closest to Kirby.
- **Acceptable (geometry shell):** the planned inverted-hull shell, but only if you first **join/merge the duplicate into a single mesh** (or union it) before solidify/expand, so there's one outer boundary. Otherwise expect per-part halos.

Either keeps the left copy raw and the visible right character raw, satisfying scope.

Recommend Codex use the compositor-dilation method (or explicitly merge geometry before shelling) and have the recipe document *which* method and *why* per-part outlines were the original defect. Then render proof and update the recipe — no user decision is required to proceed.

## Evidence Checked
- `raw_outline_outer_only_prompt.md` — task contract, scope, stop condition.
- `build_hero1_male_raw_outline_only.py` — current Freestyle config (lines 35–78): confirms only silhouette + external_contour are on, `use_same_object=True`, collection-filtered; recipe writer (lines 81–159) documents Freestyle method that must be replaced.
- Repo rules cited in prompt (AGENTS.md operator/validator split, FriendSlop raw-preservation rule) — consistent with a Blender-only outline look-dev pass that doesn't touch the GLB material.

## Questions Or Blockers
None requiring the user. The method choice (compositor dilation vs. merged-geometry shell) is a technical decision Codex can make; both stay in scope.

## Caveats
- I did not open Blender or inspect the GLB, so I can't confirm the mesh is multi-part — but the symptom (interior lines despite external-contour-only Freestyle) strongly implies it. Codex should verify part count; if it's a single watertight mesh, a plain inverted hull is fine.
- Verify the proof render actually shows a clean single outer edge with no black between mesh parts before declaring the stop condition met — that visual check is the real pass/fail, not just "Freestyle removed."
- Token-usage reporting (AGENTS.md) still applies to Codex's run.

</validator_independent_answer>

<codex_draft>
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

</codex_draft>
