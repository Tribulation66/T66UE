# Next Steps

This is the current recommended work order as of `2026-05-04`.

## Priority Order

1. Test the Mike rig prototype in Unreal through the staged executable. Focus on
   idle, legs-only walk, neck/head/body stability, grounding, material read, and
   right-hand sword attachment.
2. If Mike fails in motion, fix the Mike-only rig/material/source problem before
   touching the rest of the roster.
3. If Mike passes in motion, document the accepted rig/import/material handoff
   as the next repeatable character process.
4. Only after the Mike process is accepted, decide whether to batch-apply the
   rigging/import procedure or return to source-model art-quality rerolls.
5. Resume the CoherentThemeKit01 environment review after the current character
   rigging handoff is accepted or explicitly paused.

## Active Environment-Kit Experiment

Active source folder:

- [CoherentThemeKit01](C:/UE/T66/Model%20Generation/Runs/Environment/CoherentThemeKit01)

Current raw TRELLIS set:

- Dungeon / Easy: four wall modules and four floor modules
- Forest / Medium: four wall modules and four floor modules
- Ocean / Hard: four wall modules and four floor modules
- Martian / VeryHard: four wall modules and four floor modules
- Hell / Impossible: four wall modules and four floor modules

Runtime principle:

- use generated meshes as visual modules first
- keep current procedural tower collision stable
- do not replace the whole room or the whole tower generator in the first pass

## Immediate Experiment Plan

For the next model-generation pass:

- do not regenerate the CoherentThemeKit01 wall/floor set until the current raw GLBs have been reviewed
- use the Blender comparison scene as the first visual gate
- normalize scale/pivot in Blender only for accepted raw modules
- save raw and normalized QA renders
- reject any module with unwanted background platform geometry

## Character Batch Status

Type A masculine standard plus beach goer rigging and Unreal wiring are now complete for the 12-hero active roster in [TypeA_Masculine_Batch01](C:/UE/T66/Model%20Generation/Runs/Heroes/TypeA_Masculine_Batch01/batch_plan.md). Merlin, Zeus, Dog, and Forsen are out of the active gameplay/model-generation set.

Current completed outputs:

- `24/24` standard plus beach goer assembled GLBs were rigged with `Model Generation/Scripts/rig_typea_batch01.py`.
- Each output has an `18`-bone armature, procedural skin weights, and a placeholder idle arm-sway animation.
- Unreal imports were created under `/Game/Characters/Heroes/Hero_X/TypeA` and `/Game/Characters/Heroes/Hero_X/TypeA/Beachgoer`.
- `Content/Data/CharacterVisuals.csv` and `DT_CharacterVisuals` now wire every `Hero_X_TypeA` and `Hero_X_TypeA_Beachgoer` row to the imported skeletal mesh and `RigIdleV2` animation.
- Verification passed in [TypeABatch01HeroVisualVerification.json](C:/UE/T66/Saved/TypeABatch01HeroVisualVerification.json): `24/24` rows checked, `0` errors, matching mesh/animation skeletons, all `48` material slots bound to imported textures, `M_Character_Unlit` using texture-alpha opacity masking, imported mesh heights from `199.99997` to `200.00003` cm, and bottom Z within `-0.000002` to `0.000024` cm.
- In-game visual captures were taken from the real gameplay renderer: [standard lineup](C:/UE/T66/Saved/TypeABatch01VisualCheck/TypeABatch01_Gameplay_QA_standard_Yaw180.png) and [beach goer lineup](C:/UE/T66/Saved/TypeABatch01VisualCheck/TypeABatch01_Gameplay_QA_beachgoer_Yaw180.png).

Immediate character follow-up:

- Read [Model Processing.md](C:/UE/T66/Model%20Generation/Model%20Processing.md) before changing character assembly, rigging, import, material repair, DataTable wiring, or staged visual verification.
- Mike Pass02 model generation is paused on the Mike rig prototype:
  [Hero_3_Mike_Chad_RigPrototype_A03_LiftedNeckBridge.glb](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Assembly/HeadBody/Hero_3_Mike_Chad_RigPrototype_A03_LiftedNeckBridge.glb).
- The Mike-only rigging process proof now exists in
  [Rigging/Mike_Chad_RigPrototype_A03_LiftedNeckBridge](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Rigging/Mike_Chad_RigPrototype_A03_LiftedNeckBridge).
  It has idle, legs-only walk, and a right-hand sword proxy attachment.
- Mike is now imported and wired for staged in-game testing through `Hero_3_Chad`
  at `/Game/Characters/Heroes/Hero_3/Chad/RigPrototype`.
- The staged executable was rebuilt after material repair, and the visual proof
  is [MikeRigPrototype_StagedHeroSelection_Textured_QA.png](C:/UE/T66/Saved/MikeRigPrototypeUnrealCheck/MikeRigPrototype_StagedHeroSelection_Textured_QA.png).
- The next character step is hands-on Unreal testing of Mike's idle/walk rig and
  sword attachment behavior in the actual game viewport. Do not start roster
  batch work until this Mike-only process is accepted.
- If the Mike rig prototype tears, deforms the separate neck bridge badly, fails head rigidity, or the weapon attachment is unacceptable during the idle/walk test, stop and return to model/retopo/attachment cleanup before import.
- Review the imported Batch01 Type A heroes in-editor for art-quality issues that scripts cannot judge only after the Mike prototype rigging path is understood.
- Reroll any fallback-derived source that still produced poster panels, background cards, or bad body/head joins after the Mike rigging/process test has informed the source rules.
- Keep using separate body-only, head-only, and weapon-only TRELLIS inputs; do not return to combined body-plus-head or weapon-on-body prompts.
- Add a weapon socket or attachment rule after the character mesh review is accepted.
- Replace the placeholder arm-sway idle with production locomotion after the mesh scale and skeleton compatibility remain stable.

## Weapon Attachment Status

The current Mike sword proxy solves the process-level scale, forward/up blade
angle, and hilt roll issues well enough for in-game rig testing. It is not final
production weapon art. Future weapon work should still generate separate weapon
meshes, attach them through Blender or sockets, and check front, side, oblique,
close grip, and user-like staged views before approval.

## Current Proven Settings

- Hero raw generation:
  - input: [Arthur_HeroReference_Full_White.png](C:/UE/T66/Model%20Generation/Archive/PreviousCharacterScreenshots_2026-05-02/Runs/Arthur/Inputs/Arthur_HeroReference_Full_White.png)
  - seed: `1337`
  - `X-Texture-Size: 2048`
  - `X-Decimation: 80000`
- Hero low-poly follow-up:
  - preferred: `40k` tris
  - alternate: `20k` tris with slight softening
- Easy enemy raw generation:
  - seed: `1337`
  - `X-Texture-Size: 2048`
  - `X-Decimation: 200000`
- Easy enemy retopo targets:
  - bipeds: `24k`
  - slime: `10k`
  - flying: `14k`

## Stop Conditions

Stop and reassess if:

- the next agent starts treating the sword as already solved
- the environment drifts away from [ENVIRONMENT_LOCK.md](C:/UE/T66/Model%20Generation/ENVIRONMENT_LOCK.md)
- a new sword pass is approved from only one or two screenshots
- repeated transform-only sword attempts keep failing and nobody escalates to a hand-pose or rig step

## Defer Until Later

- production rerolls for any fallback-derived Type A hero assets that fail Blender lineup QA
- production locomotion polish beyond the current placeholder idle arm-sway animation
- new full hero TRELLIS seed sweeps
- new wall/floor TRELLIS batches for CoherentThemeKit01 until the current 40 raw GLBs have been reviewed

