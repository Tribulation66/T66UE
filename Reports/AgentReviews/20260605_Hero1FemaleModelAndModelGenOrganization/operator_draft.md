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

That means the request to keep the recently generated FriendSlop models in a clear active location applies to the 49-asset Easy batch and the Hero 1 male probe. There is no generated FriendSlop female 3D model to place there yet.

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
- `Model Generation/pending_issues_model_generation.md`: records the missing FriendSlop female Hero 1 3D model as an out-of-scope content gap.

Important wording added: `AccuRig_Textured` inside active FriendSlop raw runs is exporter-stage naming only. It is not approval to use the archived AccuRig hero lineup.

The two legacy rigging helper scripts were updated because they hard-referenced the old active AccuRig run location as default source roots. After the archive move, leaving those defaults pointed at the old root would recreate the same confusion or fail with a stale path. They now resolve through `Archive/DeprecatedHeroLineups` and are marked as legacy bridge tooling.

## Verification Performed

- Checked `.t66/operator-state.json`: Codex is Operator, Claude is Validator.
- Verified `ANTHROPIC_API_KEY` is unset in Process/User/Machine before invoking Claude.
- Read `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `Model Generation/MODEL_GENERATION_AGENTS.md`, and model-generation instruction routers.
- Inspected `Content/Data/CharacterVisuals.csv` Hero 1 rows and `Source/T66/Core/T66CharacterVisualSubsystem.cpp` mesh-selection logic.
- Verified the 2D FriendSlop female reference image exists.
- Verified root Pixal3D run directory now contains only `Archive`, `FriendSlopEasyBatch_20260604_1532`, `FriendSlopProbe_Hero1Male_20260604_1415`, and `README.md`.
- Verified `FriendSlopEasyBatch_20260604_1532` manifest asset count = 49, generated `Outputs/*.glb` count = 49, and `SourceAssets/Import/FriendSlop/Pixal3D/FriendSlopEasyBatch_20260604_1532/*.glb` count = 49.
- Verified archived AccuRig destination exists and old root AccuRig path no longer exists.
- Ran a targeted `rg --fixed-strings` check for the old non-archived AccuRig path forms under active `Model Generation` content excluding the archive; old root path hit count = 0.
- Ran a broader active `HeroDemoLineup_20260522_AccuRig` search excluding the archive; remaining hits are warning text or paths rebuilt through `Archive/DeprecatedHeroLineups`.
- Ran `python -m py_compile` on the two updated rigging helper scripts.
- Ran targeted `rg` excluding the archive; the only active `HumanoidGuidelineTest_20260522_100k` hit is a legacy-doc warning that the cleaned non-durable run must be explicitly restored/provided if that bridge is revived.
- Added `Model Generation/pending_issues_model_generation.md` because the missing FriendSlop female 3D model is a discovered content gap, not something this cleanup should silently generate or import.

No Unreal editor work, no Blender look-dev, no runtime data import/reload, no deletion, and no Git operations were performed.
