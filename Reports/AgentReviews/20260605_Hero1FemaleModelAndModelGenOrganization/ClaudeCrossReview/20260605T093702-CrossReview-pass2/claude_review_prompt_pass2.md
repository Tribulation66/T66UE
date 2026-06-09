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
- Original prompt path: C:\UE\T66\Reports\AgentReviews\20260605_Hero1FemaleModelAndModelGenOrganization\original_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\20260605_Hero1FemaleModelAndModelGenOrganization\operator_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\20260605_Hero1FemaleModelAndModelGenOrganization\ClaudeIndependent\20260605T092654-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
# Original Prompt And Task Contract

## User Request

The user first asked to finish the information-gathering pass for the Blender-only Fall Guys/rubber material mission:

- Determine whether changing the Pixel3D/source-image inputs is likely to help more than Blender material/look-dev.
- Use the existing model for the next Blender work.
- Check which model the game currently uses for Hero 1 female.
- Hero 1 male is already confirmed to use the correct FriendSlop model.
- Avoid Unreal work; Blender material work is a later next step.

The user then expanded the request before final answer:

- Continue and finish the previous information gathering.
- Before answering, implement organization cleanup under Model Generation.
- Make the Model Generation README/docs clear that current work should use FriendSlop models, not the AccuRig ones.
- Put the newly generated FriendSlop Pixel3D models in the clear Model Generation location.
- Archive old/non-current model generations so future agents do not confuse them with active sources.
- Do not stop at reporting; organize and clean first, then answer.

## Working task

Operator: Codex
Validator: Claude
Scope: Finish the Hero 1 female/current-model and Pixel3D-vs-Blender investigation, then organize Model Generation so current FriendSlop/Pixal3D easy-difficulty models are clearly the active source set, unused legacy/generated model batches are archived, and README/process docs remove the AccuRig ambiguity. No Unreal work, no Blender look-dev edits yet, no deletion, no Git operations.
Stop condition: Provide the current model finding, the Blender/source-generation recommendation, exact organization changes, archive paths, docs updated, and verification/token counts.

## Repo Rules

- Root router: `AGENTS.md`.
- Role state: `.t66/operator-state.json` selects Codex Operator and Claude Validator.
- Operator/Validator process: `OPERATOR_VALIDATOR_PROTOCOL.md`.
- Model Generation router: `Model Generation/MODEL_GENERATION_AGENTS.md`.
- Read model-generation instructions and pending issue docs before editing.
- Do not call native goal tools.
- Do not run Unreal or Blender for this cleanup; this is organization and documentation before the later Blender material pass.
- Do not delete assets for this requested organization step. Archiving/moving under Model Generation is allowed when path-safe and documented.

</original_prompt>

<validator_independent_answer>
Result: OK

## Independent Answer

**1. Which model Hero 1 female currently uses.** Hero 1 female is "Stacy" (`Content/Characters/Heroes/Hero_1/Stacy`). Her shipped skins are the existing Pixal3D/AccuRig-lineage set: default `Pixal3DToonStyle/SM_Hero_1_Stacy` + outline, the rigged `AnimatedToonStyle/SK_Hero_1_Stacy`, plus DemoSkin, Beachgoer, and QuadRetro variants. **No FriendSlop-generated Hero 1 female model exists.** The new FriendSlop runs (`FriendSlopProbe_Hero1Male_20260604_1415` and `FriendSlopEasyBatch_20260604_1532`) cover only Hero 1 *male* among heroes — the batch is boosts, gates, dungeon enemies, NPCs, companions, pets, and props; Stacy/Hero_1 female is not in either run. So unlike male (confirmed FriendSlop), the female still rides the older Pixal3D source lineage.

**2. Pixel3D/source-image change vs. Blender material work.** For a Fall Guys rubber/jelly look, Blender material/look-dev is the higher-leverage lever. The rubber appearance is a shader-response property — roughness, specular/sheen, subsurface, clearcoat, normal softening — none of which the source image controls. Pixel3D/source-image regeneration mainly changes albedo and silhouette, not material response, and risks invalidating the already-correct male and the existing female mesh. Recommend: keep the existing meshes, do the rubber look in Blender material/look-dev, treat source-image regeneration as a last resort only if silhouette/topology blocks the material.

**3. Organization cleanup (Codex/Operator to execute — I cannot edit).** Path-safe plan:
- Active source set = the two FriendSlop runs under `Model Generation/Runs/Pixal3D/`. Surface them as the current source location and reference them in the README/Instructions.
- Archive legacy non-current generations, notably `Runs/Pixal3D/HeroDemoLineup_20260522_AccuRig` (and any other pre-FriendSlop hero batches), into an `Archive/` subtree with a short provenance note. Move only, no deletion.
- README/`Instructions` edit: state that current model work uses FriendSlop Pixal3D models; AccuRig batches are archived reference only. Note that `RR_accurig_textured_export` filenames inside the FriendSlop runs are just the export-stage naming, not the legacy AccuRig source set — call this out so future agents don't confuse them.

## Evidence Checked
- `.t66/operator-state.json` (Codex Operator, Claude Validator).
- `Model Generation/README.md`, `MODEL_GENERATION_AGENTS.md`, Pixal3D dir listing.
- `Content/Characters/Heroes/Hero_1/Stacy` skin tree (Pixal3D/Animated/DemoSkin/Beachgoer/QuadRetro).
- `Runs/Pixal3D/FriendSlopProbe_Hero1Male_*` and `FriendSlopEasyBatch_*` member lists — no Stacy/female hero present.

## Questions Or Blockers
None requiring the user. One thing Codex must state plainly rather than gloss: there is **no newly generated FriendSlop female hero model to place**, so the "put newly generated FriendSlop models in the clear location" step applies to male/the batch assets, not female. If the user actually wants a FriendSlop *female* regeneration, that conflicts with "use the existing model for next Blender work" — but the user already chose the existing model, so proceed.

## Caveats
- "Current female model" is inferred from on-disk skin folders; the authoritative wiring lives in the Stacy character Blueprint/skin data asset, which I did not open. Folder layout strongly implies `Pixal3DToonStyle` default + `AnimatedToonStyle` rig, but confirm in the BP if it matters.
- I did not enumerate every legacy run; Codex should sweep `Runs/Pixal3D/` for any other pre-FriendSlop hero batches before declaring the archive complete.
- Archiving is a move, not a delete — verify no script/manifest references the old `HeroDemoLineup` paths before relocating, or those references will break.

</validator_independent_answer>

<codex_draft>
# Operator Draft: Hero 1 Female Model Source and Model Generation Organization

## Task Contract

Working task:
- Operator: Codex
- Validator: Claude
- Scope: Finish the Hero 1 female/current-model and Pixal3D-vs-Blender investigation, then organize `Model Generation` so current FriendSlop/Pixal3D Easy-difficulty models are clearly the active source set, unused legacy/generated model batches are archived, and README/process docs remove the AccuRig ambiguity. No Unreal work, no Blender look-dev edits yet, no deletion, no Git operations.
- Stop condition: Provide the current model finding, the Blender/source-generation recommendation, exact organization changes, archive paths, docs updated, and verification/token counts.

## Current Hero 1 Model Finding

`Content/Data/CharacterVisuals.csv` shows:
- `Hero_1_Chad` and `Hero_1_Chad_DemoSkin` use the raw FriendSlop static mesh `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/SM_Hero_1_Chad_Male.SM_Hero_1_Chad_Male`.
- `Hero_1_Stacy` uses skeletal mesh `/Game/Characters/Heroes/Hero_1/Stacy/AnimatedToonStyle/SK_Hero_1_Stacy.SK_Hero_1_Stacy` and static fallback `/Game/Characters/Heroes/Hero_1/Stacy/Pixal3DToonStyle/SM_Hero_1_Stacy.SM_Hero_1_Stacy`.
- `Hero_1_Stacy_DemoSkin` uses skeletal mesh `/Game/Characters/Heroes/Hero_1/Stacy/DemoSkin/AnimatedToonStyle/SK_Hero_1_Stacy_DemoSkin.SK_Hero_1_Stacy_DemoSkin` and static fallback `/Game/Characters/Heroes/Hero_1/Stacy/Beachgoer/Pixal3DToonStyle/SM_Hero_1_Stacy_Beachgoer.SM_Hero_1_Stacy_Beachgoer`.

Runtime code in `Source/T66/Core/T66CharacterVisualSubsystem.cpp` loads both skeletal and static meshes from the row, then uses the static mesh only when there is no resolved skeletal mesh or no target skeletal component. If both a row skeletal mesh and target skeletal component exist, it hides the static mesh and applies the skeletal mesh. Therefore Hero 1 female is currently not using a FriendSlop raw model in normal skeletal-character usage; it is using the older Animated ToonStyle skeletal asset. Hero 1 male is already on FriendSlop raw static mesh.

There is no generated FriendSlop 3D female hero model in the current FriendSlop Pixal3D runs. The only located FriendSlop female Hero 1 asset is the 2D reference image `FriendSlop/ImageGen/Heroes/Demo/Hero_1_Chad_Female/FriendSlop_Hero1_Chad_Female_ReferenceRepro_v01.png`.

## Pixal3D Versus Blender Recommendation

For the Fall Guys rubber target, Blender is the higher-value place to do the work. The required read is driven by lighting and shader response: roughness, specular/clearcoat, subsurface tint/radius, base color saturation, normal/bevel softness, and shadowless soft presentation. Those are portable to UE5 as material parameters.

Changing the source image or Pixal3D generation parameters can help only if the base mesh/albedo blocks the target, for example if the silhouette is too sharp, the material texture is too noisy, or baked highlights conflict with the shader. It should not be the primary lever for this mission because it risks changing identity/topology and does not produce the documented UE5-portable material recipe.

## Organization Changes Made

Moved the old AccuRig hero lineup out of active source discovery:
- From `Model Generation/Runs/Pixal3D/HeroDemoLineup_20260522_AccuRig`
- To `Model Generation/Runs/Pixal3D/Archive/DeprecatedHeroLineups/HeroDemoLineup_20260522_AccuRig`

Added/updated docs:
- `Model Generation/README.md`: adds the current FriendSlop source set, lists the two active runs, explains the 49-asset Easy batch, calls out the Hero 1 male/female difference, and warns against using archived AccuRig/ToonStyle lineups.
- `Model Generation/Runs/Pixal3D/README.md`: states that only `FriendSlopEasyBatch_20260604_1532` and `FriendSlopProbe_Hero1Male_20260604_1415` are active in the root runs folder.
- `Model Generation/Runs/Pixal3D/Archive/README.md`: documents archive semantics.
- `Model Generation/Runs/Pixal3D/FriendSlopEasyBatch_20260604_1532/README.md`: documents the 49-asset active Easy batch.
- `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/README.md`: documents the active Hero 1 male FriendSlop probe.
- `Model Generation/Runs/Pixal3D/Archive/DeprecatedHeroLineups/HeroDemoLineup_20260522_AccuRig/README_ARCHIVED.md`: marks the archived AccuRig lineup as not current.
- `Model Generation/Instructions/README.md`
- `Model Generation/Instructions/00_MODEL_GENERATION_ROUTING_INSTRUCTIONS.md`
- `Model Generation/Instructions/11_FRIENDSLOP_RAW_PIXAL3D_IMPORT_GUIDELINES.md`
- `Model Generation/Instructions/12_MODEL_CLEANUP_AND_ORGANIZATION_INSTRUCTIONS.md`
- `Model Generation/Rigging and Animation/README.md`
- `Model Generation/Rigging and Animation/07_ANIMATED_TOONSTYLE_HERO_PIPELINE_INSTRUCTIONS.md`
- `Model Generation/Rigging and Animation/Tools/create_animated_toonstyle_hero_sources.py`
- `Model Generation/Rigging and Animation/Tools/render_animated_toonstyle_facing_diagnostic.py`
- Archived `ACCURIG_HANDOFF.md`

Important wording added: `AccuRig_Textured` inside active FriendSlop raw runs is exporter-stage naming only. It is not approval to use the archived AccuRig hero lineup.

## Verification Performed

- Checked `.t66/operator-state.json`: Codex is Operator, Claude is Validator.
- Verified `ANTHROPIC_API_KEY` is unset in Process/User/Machine before invoking Claude.
- Read `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `Model Generation/MODEL_GENERATION_AGENTS.md`, and model-generation instruction routers.
- Inspected `Content/Data/CharacterVisuals.csv` Hero 1 rows and `Source/T66/Core/T66CharacterVisualSubsystem.cpp` mesh-selection logic.
- Verified the 2D FriendSlop female reference image exists.
- Verified root Pixal3D run directory now contains only `Archive`, `FriendSlopEasyBatch_20260604_1532`, `FriendSlopProbe_Hero1Male_20260604_1415`, and `README.md`.
- Verified `FriendSlopEasyBatch_20260604_1532` manifest asset count = 49, generated `Outputs/*.glb` count = 49, and `SourceAssets/Import/FriendSlop/Pixal3D/FriendSlopEasyBatch_20260604_1532/*.glb` count = 49.
- Verified archived AccuRig destination exists and old root AccuRig path no longer exists.
- Ran `python -m py_compile` on the two updated rigging helper scripts.
- Ran targeted `rg` excluding the archive; the only active `HumanoidGuidelineTest_20260522_100k` hit is a legacy-doc warning that the cleaned non-durable run must be explicitly restored/provided if that bridge is revived.

No Unreal editor work, no Blender look-dev, no runtime data import/reload, no deletion, and no Git operations were performed.


</codex_draft>
