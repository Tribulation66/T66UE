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
- Original prompt path: C:\UE\T66\Reports\AgentReviews\20260605_Hero1MaleRubberBlenderLookDev\raw_outline_only_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\20260605_Hero1MaleRubberBlenderLookDev\raw_outline_only_operator_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\20260605_Hero1MaleRubberBlenderLookDev\ClaudeRawOutlineOnly\20260605T134959-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
Original user request:
To be honest the original pixal3D output is still the best looking, lets try to just add the black outline around him, so no other changes.

Working task:
Operator: Codex
Validator: Claude
Scope: Blender-only look-dev update: keep the original raw Pixal3D material unchanged and add only a black outline around a duplicate/comparison version of the same male Hero 1 model. No Unreal, no material recolor, no rubber shader changes.
Stop condition: Blender is open on a comparison scene showing raw Pixal3D and raw Pixal3D + black outline, with rendered proof and a short recipe, or a blocker is reported.

Source asset:
- `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Outputs\Hero_1_Chad_Male.glb`

Constraints:
- Preserve the raw Pixal3D look as the base.
- Add black outline only.
- Do not touch Unreal.


</original_prompt>

<validator_independent_answer>
Result: OK

## Independent Answer

This is a self-contained Blender look-dev task that Codex (Operator) can execute internally — no user decision is required. The request is unambiguous: keep the raw Pixal3D material exactly as-is and add **only** a black outline, shown as a side-by-side comparison (raw vs. raw + outline).

Recommended approach (standard, non-destructive, preserves the original look):
1. Import the GLB into a fresh scene.
2. Duplicate the mesh so one copy stays untouched (raw Pixal3D) and the second gets the outline. Place them side by side for the comparison render.
3. On the outline copy, use the **inverted-hull** method: add a Solidify modifier (Flip Normals on, thickness small/negative) with a second material slot assigned to backfaces — a pure black emission/diffuse material with Backface Culling/normals flipped — OR add the outline as a separate shell. The original material slot stays unchanged. This adds a black silhouette without altering the base shader.
4. Render both side by side, save the proof PNG into the run's report folder, and write a short recipe (modifier settings + outline thickness).

Crucially: do not recolor, do not swap to a rubber shader, do not touch Unreal — all of which the prompt explicitly forbids.

## Evidence Checked
- Source GLB exists: `...\FriendSlopProbe_Hero1Male_20260604_1415\Outputs\Hero_1_Chad_Male.glb` (10.8 MB, dated 2026-06-04). ✓
- Prompt file confirms scope: Blender-only, black outline only, no Unreal. ✓
- Attempted to read live Blender scene state (`get_objects_summary`) but the MCP tool is not yet permission-granted in this session, so I could not confirm whether Blender is open/connected.

## Questions Or Blockers
- None requiring the user. The only operational unknown is whether Blender is running with the MCP add-on connected — that's an Operator-side prerequisite, not a user decision. If Blender is not connected, Codex should report that as the blocker per the stop condition.

## Caveats
- I could not verify the live Blender connection (permission not granted to me as read-only Validator); Codex must confirm the add-on is connected before claiming the stop condition is met.
- "Black outline" thickness/style is subjective — the inverted-hull approach gives a clean toon silhouette, but the user may want to eyeball the thickness afterward. Deliver the comparison render so they can judge.
- Ensure the proof render genuinely shows the **original** material untouched on both copies (only the outline differs), so the comparison is honest.

</validator_independent_answer>

<codex_draft>
Working task:
Operator: Codex
Validator: Claude
Scope: Blender-only look-dev update: keep the original raw Pixal3D material unchanged and add only a black outline around a duplicate/comparison version of the same male Hero 1 model. No Unreal, no material recolor, no rubber shader changes.
Stop condition: Blender is open on a comparison scene showing raw Pixal3D and raw Pixal3D + black outline, with rendered proof and a short recipe, or I report a blocker.

Operator draft:
- Created a new outline-only Blender builder at `C:\UE\T66\Reports\AgentReviews\20260605_Hero1MaleRubberBlenderLookDev\build_hero1_male_raw_outline_only.py`.
- Created and saved `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Hero_1_Chad_Male_RawOutlineOnly.blend`.
- Rendered proof to `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Renders\Hero_1_Chad_Male_raw_vs_black_outline.png`.
- Wrote recipe files:
  - `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\raw_black_outline_recipe.md`
  - `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\raw_black_outline_recipe.json`
- Opened Blender with the new `.blend`; process `27156` command line points to `Hero_1_Chad_Male_RawOutlineOnly.blend`.
- Existing older Blender process `24848` still points to the previous rejected comparison file; I did not close it to avoid discarding any manual state. The correct judging file is the new outline-only blend.

Implementation details:
- Both left and right characters are raw duplicates of the imported GLB.
- No character materials, textures, Principled BSDF inputs, colors, roughness, or mesh bevel/material modifiers were changed.
- The black outline is Blender Freestyle, collection-filtered only to the outlined duplicate collection `RawOutline_Outlined_Pixal3D`.
- Freestyle line set is `Black_Outline_Only`; line style is `Black_FallGuys_Kirby_Outline`; thickness is `3.0 px`; edge types are silhouette and external contour.

Verification:
- `blender.exe --background --factory-startup --python build_hero1_male_raw_outline_only.py` completed with exit code 0.
- Blender saved `Hero_1_Chad_Male_RawOutlineOnly.blend`.
- Blender saved `Hero_1_Chad_Male_raw_vs_black_outline.png`.
- I visually inspected the PNG: left side is raw Pixal3D; right side is the same raw look with black line art; full body is visible.
- Blender emitted many GLB mesh topology warnings (`edge appears twice`, `degenerated triangle detected`) during render, but saved the proof and blend successfully.

PPF close:
Process used: Raw Pixal3D duplicate plus transcript-derived black outline / toon line-art mechanism.
Matches declared process: YES.
Evidence: raw material untouched in recipe and builder; rendered side-by-side proof; open Blender process pointing to the new outline-only file.

Mechanism close:
Preserve raw material: PRESENT. Evidence: no material edits or material builder applied to duplicates.
Add black outline only: PRESENT. Evidence: Freestyle collection-filtered line set on only the right duplicate.
Compare raw vs outline: PRESENT. Evidence: side-by-side render and Blender scene.

</codex_draft>
