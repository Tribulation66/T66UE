# Findings And Limitations

This file records repo-proven findings that matter to the current mob/VAT animation workflow. Historical humanoid rigging notes were removed from the active path on 2026-05-21 because heroes and humanoid companions are now handled manually outside this folder.

## Blender Setup

- Blender 5.1.1 is installed at `C:\Program Files\Blender Foundation\Blender 5.1\blender.exe`.
- Native Blender MP4 rendering works for mob previews.
- Use unlit/emissive preview materials for mob QA so dark faces are not mistaken for texture or orientation problems.

## 2026-05-14 Enemy VAT Pipeline Exploration

What was inspected:

- `Content/Data/Enemies.csv`
- `Content/Data/Stages.csv`
- `Content/Data/CharacterVisuals.csv`
- `Source/T66/Gameplay/T66EnemyBase.h`
- `Source/T66/Gameplay/T66EnemyBase.cpp`
- `Source/T66/Gameplay/Enemies/T66EnemyFamilyResolver.cpp`
- `Source/T66/Gameplay/Enemies/T66FlyingEnemy.cpp`
- `Source/T66/Gameplay/Enemies/T66RushEnemy.cpp`
- `Source/T66/Data/T66DataTypes.h`
- `Source/T66/Gameplay/Enemies/pending_issues_Enemies.md`
- `Model Generation/Production/Roster_v1/AgentA/Report.md`
- `Model Generation/Production/Roster_v1/AgentB/Report.md`
- UE 5.7 local `AnimToTexture` plugin headers and source references

Current Easy mob set:

- `Slime`
- `BoneWalker`
- `RatPack`
- `CaveBat`
- `HexSlinger`
- `TombSpider`
- `StoneSentinel`
- `MimicLure`
- `BoneConjurer`
- `CryptWraith`

Runtime visual findings:

- Easy mob `CharacterVisuals.csv` rows were static-only before the VAT QA path.
- Live static meshes resolve under `/Game/Characters/Mobs/<EnemyID>/SM_<EnemyID>`.
- `AT66EnemyBase::ConfigureAsMob(...)` applies the row through `UT66CharacterVisualSubsystem::ApplyCharacterVisual(...)`.
- The VAT implementation uses a dedicated mob vertex animation table instead of overloading skeletal animation slots.

Source findings:

- Source GLBs for all ten Easy mobs exist under `Model Generation/Production/Roster_v1`.
- Agent A owns `Slime`, `RatPack`, `HexSlinger`, `StoneSentinel`, and `BoneConjurer`.
- Agent B owns `BoneWalker`, `CaveBat`, `TombSpider`, `MimicLure`, and `CryptWraith`.
- Promote Pixal3D GLB outputs only after visual and runtime acceptance.

Runtime behavior finding:

- The implemented enemy families currently resolve to `Melee`, `Rush`, `Ranged`, and `Flying`.
- Data archetypes such as `Exploder`, `Turret`, `Necromancer`, and `Stutterer` exist in the roster data, but matching runtime subclasses are currently missing.
- VAT animation can provide visual cues for those archetypes, but it does not implement missing gameplay behavior.

## UE 5.7 AnimToTexture Notes

- UE 5.7 has an Experimental `AnimToTexture` plugin at `C:\Program Files\Epic Games\UE_5.7\Engine\Plugins\Experimental\AnimToTexture`.
- The plugin exposes `UAnimToTextureBPLibrary::AnimationToTexture(UAnimToTextureDataAsset*)`.
- Use VAT UV channel `2`. Channel `1` conflicts with generated lightmap UVs on the converted static mesh.
- First-time bakes may warn that UV channel `2` is out of range before the plugin writes the channel. Treat this as non-fatal only if texture/material/runtime playback verification passes.
- In UE 5.7 Python, direct reads of `UAnimToTextureDataAsset` `FVector3f` bounds can return zero even when the bake is valid.
- After `UpdateMaterialInstanceFromDataAsset`, read `MinBBox` and `SizeBBox` from the material instance and write those values to `MobVertexAnimations.csv`.
- Generated VAT material custom nodes that call `TransformLocalVectorToWorld` must pass the material `Parameters` argument: `TransformLocalVectorToWorld(Parameters, local_delta)`.
- After changing generated custom-node HLSL, force-recreate or explicitly rebuild the generated material master before reparenting material instances.
- Full cook/stage is required after generated material or content-bake changes. `-SkipCook` restage is only acceptable for a later code-only verification pass after content cookability has already passed.
- Staged smoke must fail if the log contains VAT material compile failures, invalid shader maps, uncooked shader-map IDs, default-material fallback, fatal errors, or assertion failures.

## Current Easy VAT Runtime Path

- Blender source: `Model Generation/Rigging and Animation/Runs/Easy_Mob_VAT_20260514/Easy_Mob_VAT_Source.blend`
- Runtime CSV: `Content/Data/MobVertexAnimations.csv`
- Runtime data table: `/Game/Data/DT_MobVertexAnimations.DT_MobVertexAnimations`
- Unreal VAT root: `/Game/Characters/MobsVAT/<EnemyID>/`
- Runtime row type: `FT66MobVertexAnimationRow`
- Runtime loader/application: `UT66CharacterVisualSubsystem::TryGetMobVertexAnimationRow(...)` and `ApplyMobVertexAnimationVisual(...)`
- Playback owner: `AT66EnemyBase`
- Fallback: original `CharacterVisuals.csv` static visual path when no enabled VAT row exists

Current accepted warnings:

- Verification treats material instance `MinBBox`/`SizeBBox` as authoritative because UE Python did not reliably read the data asset vector bounds in this plugin path.
- Commandlet Python UV inspection can report zero channels for generated VAT static meshes. Run `Tools/verify_easy_mob_vat_in_unreal.py` through the full editor wrapper for authoritative UV2 verification; a passing full-editor report should show LOD `0` with `3` UV channels.
- The current Easy VAT rows are QA/runtime assets, not final release art until visual and crowd performance acceptance is complete.

## 2026-05-21 Slime Movement Preview

Accepted preview:

```text
Model Generation/Rigging and Animation/Runs/Slime_MoveTowardBouncyStutterPreview_V2_20260521/Slime_MoveTowardCamera_preview.mp4
```

What changed:

- The Slime front-facing direction was corrected to `+Y`.
- Preview camera was placed on the front-facing axis.
- Preview materials were switched to unlit/emissive to remove false face shadow.
- The move loop was keyed every frame with constant interpolation.
- The preview render uses a longer 72-frame native MP4 at 15 fps.
- Travel/root hop were added to the preview video so the user can judge actual movement without opening Unreal.

Current style finding:

- The user prefers enemy movement where every authored frame visibly changes the silhouette or contact state.
- The desired read is intentionally crunchy and slightly stuttered, like a lower-frame animation, not a smooth physically interpolated glide.
- For Slime specifically, the motion should bounce more than the first pass while still reading as ground-dragging blob motion, not invisible legs.

## Performance Status

VAT is the correct direction for many mobs because it avoids per-enemy skeletal pose evaluation for normal playback. The current implementation is not yet proven for hundreds of enemies with no lag.

Remaining proof needed:

- crowd test at 50 / 100 / 200 / 300 active enemies
- CPU comparison against skeletal or static fallback paths
- GPU/draw-call/material update measurement
- phase randomization check so mobs do not pulse in sync
- evaluation of instancing or per-instance custom data if per-enemy dynamic material instances become the bottleneck
