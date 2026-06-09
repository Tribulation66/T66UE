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
