Result: OK

## Independent Answer

This is a process/tooling pass only — do not re-implement the Main Menu screen. The three approved fixes map cleanly onto specific files. Here is the repo-grounded plan.

**1. Authored-plate instruction amendment** — `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`
- Sections 8 (ImageGen Rules), 9 (Slice Rules), and 11 Step E (Runtime Chrome Asset Authoring) currently allow "Unreal-reconstructed" sliced chrome and only *prefer* transparent backgrounds ("If the generator cannot produce reliable alpha, use a flat removable matte"). Tighten this: where a region needs real chrome quality, the runtime brush must come from an externally authored/cleaned transparent PNG plate, not a generic Slate-reconstructed atom. Make alpha-clean PNG the required input, not the fallback.
- Update the Section 2 Method Class line ("Unreal renders those assets as Slate Box/9-slice") and Section 10 to state the plate is the source of quality.
- No change needed to `UI/UI_AGENTS.md` lines 25–26 — the raster-chrome routing already scopes correctly to FriendslopStyle. Only the instructions file body needs the plate-quality amendment.

**2. Containment/fitting checks** — `Scripts/VerifyUIFidelity.py` + checklist support
- The verifier today has `absolute_x/y`, `absolute_width/height`, and `geometry.normalized.*` aliases (lines 46–49) but **no parent-child relationship and no overflow assertion**. A row can overflow its panel and every individual checklist item still PASSes. Add an explicit containment check: a new checklist property (e.g. `contained_in=<ParentTag>` with optional padding tolerance) that fails when the child's absolute rect escapes the parent's absolute rect on any edge.
- **Evidence gap Codex must verify before coding:** confirm the dump JSON actually exposes resolvable parent tags or absolute rects for both child and parent. The verifier indexes widgets flat `by_tag` (lines 319–323) and has no parent linkage. If the dump lacks parent references, the containment property must look up the named parent tag's absolute rect — that works with the existing flat index, so authoring `contained_in=<ParentTag>` is feasible without a dump schema change. Codex should confirm absolute rects are present in a real dump, not assume.
- `UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md` already states the rule ("no child control escapes its panel", "Do not accept clipped controls"); this pass gives it teeth in the verifier rather than changing the doc.

**3. Blocking visual scorecard gate**
- Today `main()` returns nonzero only on FAIL count (line 463–466); the checklist's "Holistic Visual Gate" section (lines 115–124) is explicitly *not evaluated*. Two viable designs — Codex should pick one:
  - (a) A separate scorecard step that requires a human/structured holistic verdict file to be present and marked PASS before the overall exit code can be 0; or
  - (b) Extend the verifier to treat an unsatisfied holistic-gate manifest as a blocking nonzero exit, so PASS counts alone can't green-light.
- Either way the deliverable is: PASS on the structured items can no longer stand in for visual fidelity — the gate must be able to block even with zero structured FAILs (mirrors Step J's "can fail even when individual checklist items pass").

**Verification that proves the pass (focused smoke test, not a Main Menu redo):**
- A tiny fixture dump + checklist where a child deliberately overflows its parent → verifier must report FAIL / nonzero exit on the new containment property.
- A fixture where all structured items PASS but the holistic gate is unsatisfied → verifier must still block (nonzero).
- A clean fixture → exit 0.
- Run the existing real Main Menu dump/checklist through the amended script to confirm no regression in current parsing.

## Evidence Checked
- `Scripts/VerifyUIFidelity.py` (full) — exit logic on FAIL count only (463–466); aliases incl. absolute rects (46–49); flat `by_tag` index, no parent linkage (319–323); `tag_prefix_count` is the only non-per-widget property.
- `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` (full) — Sections 8/9/10/11E on chrome authoring; matte-as-fallback language; Steps J/K/L holistic/responsive/manual gates already exist as prose.
- `UI/FriendslopStyle/Checklists/main_menu_checklist.md` — Holistic Visual Gate section (115–124) explicitly unevaluated.
- `UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md` — Parent Containment + Visual Gate rules already authored.
- `UI/UI_AGENTS.md` — raster-chrome routing already correctly scoped (25–26).

## Questions Or Blockers
None requiring the user. The path is decided and in scope; Codex can execute. One technical prerequisite Codex must confirm (not the user): that a real dump exposes absolute rects for both child and parent so the containment property is computable.

## Caveats
- Scope discipline: this is tooling/docs only. Resist adding new chrome assets or touching the Main Menu screen this pass.
- The holistic gate cannot be fully automated — design (a) vs (b) determines whether it blocks on a present-but-unsatisfied manifest or requires a human verdict artifact; either is acceptable per the stop condition, but the gate must be *capable of blocking with zero structured FAILs* or it doesn't satisfy fix #3.
- Don't regress existing checklist parsing — the smoke test must include a current real Main Menu run, not just synthetic fixtures.
