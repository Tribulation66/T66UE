# Mike Chad Rig Prototype QA Notes - 2026-05-04

This folder contains the first Mike Pass02 rigging and weapon-attachment
prototype from the current rigging handoff model.

Source assembly:

- [Hero_3_Mike_Chad_RigPrototype_A03_LiftedNeckBridge.glb](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Assembly/HeadBody/Hero_3_Mike_Chad_RigPrototype_A03_LiftedNeckBridge.glb)

Rigging script:

- [rig_chad_mike_process_prototype.py](C:/UE/T66/Model%20Generation/Scripts/rig_chad_mike_process_prototype.py)

Outputs:

- [rig report](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Rigging/Mike_Chad_RigPrototype_A03_LiftedNeckBridge/rig_report.json)
- [rigged GLB with sword](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Rigging/Mike_Chad_RigPrototype_A03_LiftedNeckBridge/Exports/Hero_3_Mike_Chad_RigPrototype_A03_LiftedNeckBridge_RiggedWithSword.glb)
- [rigged FBX with sword](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Rigging/Mike_Chad_RigPrototype_A03_LiftedNeckBridge/Exports/Hero_3_Mike_Chad_RigPrototype_A03_LiftedNeckBridge_RiggedWithSword.fbx)
- [Blender scene](C:/UE/T66/Model%20Generation/Scenes/Chad_Pass02_Mike_Rigging/Mike_Chad_RigPrototype_A03_LiftedNeckBridge_Rigged.blend)
- [QA contact sheet](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Rigging/Mike_Chad_RigPrototype_A03_LiftedNeckBridge/Renders/Mike_Chad_RigPrototype_RigQA_ContactSheet.png)
- [sword scale/roll variant contact sheet](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Rigging/Mike_Chad_RigPrototype_A03_LiftedNeckBridge/Renders/SwordScaleRollTests/roll_variant_contact_sheet.png)
- [staged executable visual proof](C:/UE/T66/Saved/MikeRigPrototypeUnrealCheck/MikeRigPrototype_StagedHeroSelection_Textured_QA.png)

## Rig State

- One generated deform armature.
- `18` bones.
- Four skinned meshes: body, head, neck bridge, sword proxy.
- Two saved actions:
  - `Mike_Chad_Idle`
  - `Mike_Chad_WalkLegsOnly`
- Both actions have `use_fake_user = True`.
- Walk action keeps arms stable and only animates lower-body walk poses.
- Body disconnected-fragment stabilization ran and stabilized `3454` compact
  components in the current pass.
- FBX reimport passed: one armature, `18` bones, four skinned meshes, both
  actions present.
- GLB reimport passed: one armature, `18` bones, actions present.

## Unreal Handoff State

- Imported Mike-only process prototype under
  `/Game/Characters/Heroes/Hero_3/Chad/RigPrototype`.
- Rewired only `Hero_3_Chad` in `Content/Data/CharacterVisuals.csv`.
- Imported assets:
  - `SK_Hero_3_Mike_Chad_RigPrototype`
  - `AM_Hero_3_Mike_Chad_RigPrototype_Idle`
  - `AM_Hero_3_Mike_Chad_RigPrototype_WalkLegsOnly`
- Import verification passed: mesh and animations load, animation skeletons
  match, mesh bounds are roughly `200` cm tall, bottom Z is near `0`.
- First staged executable capture proved the row was wired correctly but showed
  a black silhouette because the FBX material import produced textureless
  material instances.
- Material repair extracted packed Blender base-color images, imported them as
  Unreal texture assets, created unlit material instances, and reassigned the
  four skeletal mesh material slots.
- Full stage/cook was rerun after the material repair.
- Staged executable capture after material repair shows Mike visible and
  textured in Hero Selection with the sword attached:
  [MikeRigPrototype_StagedHeroSelection_Textured_QA.png](C:/UE/T66/Saved/MikeRigPrototypeUnrealCheck/MikeRigPrototype_StagedHeroSelection_Textured_QA.png)

## Iteration Findings

Pass 1:

- Rig/export/reimport worked.
- Idle and walk actions existed.
- Neck bridge did not show obvious tearing.
- Sword was too low and read as standing on the ground beside the hand.

Adjustment:

- Changed weapon placement from the generated `hand_r` bone tail to a visible
  right-hand-volume estimate, because the stylized hand mesh sits higher than
  the procedural hand bone tail.
- Shortened the sword proxy slightly.

Pass 2:

- Sword moved into the hand area and followed the hand-region placement.
- Closeup still read slightly low, with the grip/crossguard hanging under the
  hand.

Adjustment:

- Raised the hand-volume target again and shortened the proxy a little more.

Pass 3, rejected after visual review:

- The sword was skinned to the right hand area, but it still did not read as a
  real held sword.
- The blade was effectively pointed down beside the body, and the handle/grip
  source point was wrong.
- This pass should not be used as the weapon-attachment reference.

Correction:

- Inspected the sword source render and mesh through Blender MCP.
- Found that the source sword is modeled blade-down: the blade tip is near mesh
  min Z and the handle is near mesh max Z.
- Changed the attachment source point from the lower sword section to the
  upper handle/grip region.
- Rotated the source blade direction from local `-Z` to an upright world-space
  direction.

Pass 4:

- Sword now reads as angled forward from the grip, not merely upright beside
  the body.
- The blade vector is now upward plus meaningfully toward character-front
  (`negative Y`), so the right-side view shows the sword pitching forward from
  the hand.
- Handle/crossguard is positioned at the right hand instead of the blade tip.
- The grip is still not production-quality because Mike's generated hand is open
  and does not wrap around the handle, but this pass correctly proves the
  attachment transform direction, forward blade angle, and handle-target
  procedure.
- Legs-only walk reads as a lower-body animation proof.
- Neck bridge remains visually stable in the rendered rest, idle, and walk
  checks.

Pass 5, 45-degree reference clarification:

- User reference clarified the visual target: it must look like a character
  organically holding a sword, with the blade projecting from the hip/hand line
  forward at roughly a `45` degree upward angle.
- Current attachment vector is `(0.035, -0.72, 0.693)`, where negative Y is
  character-front. In the right-side render this gives the intended forward/up
  blade read instead of a vertical hanging sword.
- Front view and walk side views show the weapon following the hand volume
  consistently.
- The remaining failure risk is not the blade angle anymore; it is the open
  generated hand. The handle is in the palm/fist volume, but the fingers do not
  close around it. A final organic weapon hold needs a right-hand/wrist pose,
  per-hero palm socket, or source hand shape that is generated for gripping.

Pass 6, sword scale and hilt roll:

- User clarified that the sword should be about twice the prior size and should
  be held sideways, with the hilt/guard parallel to the wrist rather than
  presented as a flat bar across the hand.
- Tested `-90`, `+90`, and `180` degree blade-axis roll variants at `2x` sword
  length. The `180` roll was rejected because the guard again read as a bar
  across the hand. The `-90` and `+90` variants were visually close because the
  proxy is mostly symmetrical; `-90` was baked into the script.
- Current values are sword target length `1.44` meters, blade vector
  `(0.035, -0.72, 0.693)`, and hilt roll `-90` degrees.
- Fresh contact sheet shows the larger sword reading as a real sword scale, with
  the side views still keeping the `45` degree forward/up blade line.
- The weapon now solves the size and roll issues well enough for the process
  proof. The remaining organic-hold limitation is still the open generated hand,
  which needs a hand pose, socket pass, or generated gripping hand for a final
  production result.

## Remaining Risks

- This is not a final animation set. It is a rigging and attachment process
  prototype.
- The sword proxy is Arthur's Excalibur-style test mesh, not Mike's final
  weapon.
- The hand mesh is not closed around the handle. A production weapon pass will
  need either a hand pose, a true weapon socket/offset authored per hero, or a
  weapon designed to fit the open hand silhouette.
- A sword attachment cannot be accepted as final unless the side view matches
  the `45` degree forward/up reference and the closeup reads like a real grip.
- Sword scale and hilt roll must be checked visually. A correctly attached sword
  can still fail if it reads too small or if the hilt is rotated like a flat
  bar across the hand.
- Weapon attachment must be judged visually from the handle and blade angle, not
  only from whether the weapon follows the `hand_r` bone.
- Mike's original model still has rear hair/scalp and neck-source limitations.
  Rigging did not solve those art-source issues.
- This has now passed staged Hero Selection visibility/material verification,
  but gameplay locomotion feel still needs a hands-on Unreal test before
  treating the animation pass as approved.

## Current Recommendation

Use this result as the Mike-only rigging and Unreal handoff process proof. It is
ready for in-game rig/animation testing from the staged build, with the known
limitation that the sword grip is still an open-hand process proof rather than a
production weapon hold. Do not batch-rig the roster from this candidate yet.
