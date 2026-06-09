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
- Original prompt path: C:\UE\T66\Reports\AgentReviews\20260605_Hero1MaleRubberBlenderLookDev\original_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\20260605_Hero1MaleRubberBlenderLookDev\operator_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\20260605_Hero1MaleRubberBlenderLookDev\ClaudeIndependent\20260605T094937-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
# Original Prompt: Hero 1 Male Rubber Blender Look-Dev

User confirmed to proceed with the male Hero 1 FriendSlop model and do the rubber/Fall Guys material look-dev in Blender.

Task contract:
- Operator: Codex
- Validator: Claude
- Scope: Use the confirmed Hero 1 male FriendSlop Pixal3D model as the Blender look-dev target; build the fixed soft/shadowless rig, create parametrized rubber material variations, render comparison stills and top-candidate turntables, and document a UE5-portable recipe. No Unreal work, no rigging/jiggle, no runtime import changes.
- Source GLB: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Outputs\Hero_1_Chad_Male.glb`
- Existing review scene: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\Hero_1_Chad_Male_Review.blend`
- Stop condition: Deliver Blender outputs and recipe, or stop only if a blocking source/Blender/tool issue prevents current proof.

Relevant repo rules:
- Do not use Unreal.
- Do not use native goal tools.
- Use the Operator/Validator process with Claude as Validator.
- For visual/material production tasks, use research-first replication, PPF check, artifact parity gate, mechanism manifest, proof artifacts, and documented recipe.
- Model Generation owns Pixal3D/Blender QA work; active source must be FriendSlop raw Pixal3D, not archived AccuRig.


</original_prompt>

<validator_independent_answer>
Blender MCP read access wasn't granted to me, so I couldn't confirm the live connection — but that's an execution-time concern for the Operator, not a user decision. I have enough to validate.

Result: OK

## Independent Answer
The prompt is executable internally by the Operator/Validator process — no user decision is outstanding. The user already made the one user-only call (proceed with the **male** Hero 1 FriendSlop Pixal3D model), and both named source assets exist on disk:
- `…\Outputs\Hero_1_Chad_Male.glb` (≈10.8 MB, 2026-06-04 14:47) ✓
- `…\Blender\Hero_1_Chad_Male_Review.blend` (≈11.8 MB, 2026-06-04 14:55) ✓

Recommended execution order for Codex:
1. Confirm Blender is live/connected before any scene work (look-dev cannot proceed otherwise).
2. Build the fixed soft/shadowless rig in the existing review .blend (or a copy), keeping the FriendSlop raw mesh as the target — not AccuRig.
3. Parametrize the rubber/Fall Guys material (base color, roughness, subsurface/translucency, specular, sheen/fresnel rim) so variations are reproducible.
4. Render comparison stills across the variation matrix, then turntables for top candidates only.
5. Document a **UE5-portable** recipe — map each Blender/Cycles-or-EEVEE parameter to its UE5 material equivalent, since UE won't read the Blender node graph directly.

Stay inside scope: no Unreal, no rigging/jiggle, no runtime import changes, no native goal tools.

## Evidence Checked
- Source GLB and review .blend both present with expected sizes/dates (read-only `ls`).
- Review folder contains `original_prompt.md` and `ClaudeIndependent`.
- Prompt scope, repo rules, and Operator/Validator roles read directly from the prompt.
- Blender MCP path-info/objects-summary tools loaded but **permission not granted**, so live scene state is unverified.

## Questions Or Blockers
None requiring the user. Operator-side items to resolve at execution time:
- Verify Blender is running with the MCP add-on connected before rig/material work.
- Prompt does not pin an output directory for stills/turntables/recipe — Codex should place them under the run folder (e.g. `…\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\`) per Model-Generation convention.

## Caveats
- I did not open the .blend; I can't confirm the imported mesh inside is the FriendSlop raw Pixal3D (vs. an archived AccuRig variant) or that materials/UVs survived GLB import. Operator should confirm in-scene before look-dev.
- "UE5-portable recipe" means a parameter mapping, not a node-graph transfer — Blender shading won't 1:1 reproduce in UE5; flag any look that depends on engine-specific effects.
- Apply the stated visual-task process (research-first replication, PPF check, artifact parity gate, mechanism manifest, proof artifacts) as documentation discipline, not gating ceremony.

</validator_independent_answer>

<codex_draft>
# Operator Draft: Hero 1 Male Rubber Blender Look-Dev

## Scope

Codex used the confirmed Hero 1 male FriendSlop source and stayed entirely in Blender/Blender CLI. No Unreal work, rigging, jiggle, runtime import, DataTable, or Git operation was performed.

Source assets:
- GLB: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Outputs\Hero_1_Chad_Male.glb`
- Source image: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Sources\Hero_1_Chad_Male.png`
- Existing review scene preserved: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\Hero_1_Chad_Male_Review.blend`

## PPF Close

Process used: Blender look-dev from the raw FriendSlop Pixal3D GLB, fixed soft/shadowless lighting, parameter-swept Principled BSDF rubber/vinyl material, non-destructive bevel/weighted normals, comparison grid, and turntable evidence.

Matches declared process: YES

Evidence:
- New Blender file: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Hero_1_Chad_Male_RubberLookDev.blend`
- Reproducible build script: `C:\UE\T66\Reports\AgentReviews\20260605_Hero1MaleRubberBlenderLookDev\build_hero1_male_rubber_lookdev.py`
- Recipe Markdown: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\rubber_recipe.md`
- Recipe JSON: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\rubber_recipe.json`

## Mechanism Close

Mechanism: Fixed soft flat lighting
- Status: PRESENT
- Evidence: Recipe records white world, Standard view transform, weak AO, and four shadow-disabled area lights.
- Discriminator test: Rendered grid and turntable frames use the same soft rig; no Unreal or desktop screenshot substitute.
- Reported status: FULL

Mechanism: Shader-driven rubber response
- Status: PRESENT
- Evidence: Six named material variations use Principled BSDF parameters for roughness, specular IOR, coat, coat roughness, subsurface, saturation/value, contrast flattening, bevel, and bump.
- Discriminator test: Raw Pixel3D material is present next to rubber variants; the variants are not just relit raw output.
- Reported status: FULL

Mechanism: Soft silhouette/edge catch
- Status: PRESENT
- Evidence: Rubber copies include non-destructive bevel modifiers and weighted normals; values are in the variation table.
- Discriminator test: Edge softness is geometry/normal driven and portable to UE5 as bevelled geometry/weighted normals.
- Reported status: FULL

Mechanism: Comparison under fixed viewing condition
- Status: PRESENT
- Evidence: `Hero_1_Chad_Male_rubber_comparison_grid.png` and `Hero_1_Chad_Male_rubber_comparison_grid_preview_1300.png` show reference image, raw GLB, and six variants in one rig.
- Discriminator test: The final grid is front-facing, readable, and includes the source reference panel after UV/emission correction.
- Reported status: FULL

Mechanism: Multi-angle judgment
- Status: PRESENT
- Evidence: Four top-candidate turntables are encoded as MP4 from 72 rendered Blender frames each.
- Discriminator test: `ffprobe` shows 1280x1280, 72 frames, 3 seconds, 24 fps for all four MP4s.
- Reported status: FULL

## Rendered Artifacts

- Comparison grid: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Renders\Hero_1_Chad_Male_rubber_comparison_grid.png`
- Preview grid: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Renders\Hero_1_Chad_Male_rubber_comparison_grid_preview_1300.png`
- V02 turntable: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Renders\Hero_1_Chad_Male_V02_rubber_pop_turntable.mp4`
- V03 turntable: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Renders\Hero_1_Chad_Male_V03_vinyl_bounce_turntable.mp4`
- V04 turntable: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Renders\Hero_1_Chad_Male_V04_candy_rubber_turntable.mp4`
- V06 turntable: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Renders\Hero_1_Chad_Male_V06_toy_vinyl_gloss_turntable.mp4`

## Top Candidate

Suggested first candidate for Pablo review: `V04_candy_rubber`.

Reason: It is the softest static read in the matrix, with more subsurface, stronger edge rounding, and broad coat highlights without going as toy-plastic as `V06_toy_vinyl_gloss`.

This is not declared the final locked target until Pablo reviews and either accepts it or gives a critique such as "more rubbery", "less glossy", "too plastic", or "more Fall Guys".

## Verification Performed

- `python -m py_compile` passed for the Blender build script.
- Blender CLI 5.1 imported the raw GLB, rendered the grid, rendered all turntable frame sequences, saved the `.blend`, and wrote recipe files.
- Visual check: final grid preview is readable and includes reference, raw, and six variants.
- Visual check: `V04_candy_rubber_0001.png` shows a front-facing full character under the fixed rig.
- Frame counts: V02/V03/V04/V06 each have 72 PNG frames.
- MP4 validation: all four MP4s are 1280x1280, 72 frames, 3.0 seconds, 24 fps.
- Blender MCP live connection was unavailable, so Blender work used installed local Blender CLI at `C:\Program Files\Blender Foundation\Blender 5.1\blender.exe`.


</codex_draft>
