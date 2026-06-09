# Operator Draft: Hero 1 Male FriendSlop Rigging

## Task Contract

- Operator: Codex.
- Validator: Claude.
- Scope: Blender-only FriendSlop Hero 1 male rigging deliverable plus central FriendSlop rigging process doc. No Unreal asset, C++, CSV, DataTable, ToonStyle, Quad Retro, tint, or outline export work.
- Stop condition: Rigging folder contains a saved working `.blend`, exported skeletal FBX, re-import validation blend, proof renders, QA/report files, and the process doc is discoverable from Model Generation and Rigging docs.

## Source And Outputs

- Source GLB: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Outputs\Hero_1_Chad_Male.glb`
- Rigging folder: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\Rigging\`
- Blender working file: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\Rigging\Hero_1_Chad_Male_FriendSlop_Rig.blend`
- Skeletal FBX: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\Rigging\Hero_1_Chad_Male_FriendSlop_Skeletal.fbx`
- Re-import validation blend: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\Rigging\Hero_1_Chad_Male_FriendSlop_Skeletal_reimport_validation.blend`
- QA JSON: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\Rigging\Hero_1_Chad_Male_FriendSlop_Rig_QA.json`
- Rig report: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\Rigging\Hero_1_Chad_Male_FriendSlop_Rig_Report.md`

## Process Doc Changes

- Added central doc: `C:\UE\T66\Model Generation\Instructions\13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md`
- Updated routing/index docs:
  - `C:\UE\T66\Model Generation\Instructions\README.md`
  - `C:\UE\T66\Model Generation\Instructions\00_MODEL_GENERATION_ROUTING_INSTRUCTIONS.md`
  - `C:\UE\T66\Model Generation\Rigging and Animation\README.md`
- The doc explicitly says raw FriendSlop rigs should not use archived AccuRig/Animated ToonStyle sources, should not export look-dev outlines/stylization by default, and should re-import the FBX for validation.
- Added a specific warning that fused coat/costume meshes should not be forced into an A/T-pose by broad coordinate-space deformation. If a true A-pose is required, segment/retopo/hand-clean the mesh.

## Rig Details

- Final height: 1.8000000715 m.
- Bounds: min `[-0.26218053698539734, -0.4510159492492676, 0.0]`, max `[0.2621805667877197, 0.4510159194469452, 1.8000000715255737]`.
- Source true front from four-axis proof: `+Y`.
- Final rig visual front: `+X` after documented `-90` degree Z rotation.
- Material slots preserved: `Material_0`.
- Raw GLB UV data and `Material_0` are preserved for later Unreal texture rebind.
- Exported deform skeleton has 23 bones, one root, no leaf/end-bone junk.
- Count reconciliation: 7 central/root bones + 8 arm bones + 8 leg bones = 23 deform bones, with no helper/control bones exported as physics candidates.
- Required hierarchy exists:
  - `root -> pelvis -> spine_01 -> spine_02 -> spine_03 -> neck_01 -> head`
  - `clavicle_l/r -> upperarm_l/r -> lowerarm_l/r -> hand_l/r`
  - `thigh_l/r -> calf_l/r -> foot_l/r -> ball_l/r`

## QA Evidence

- Blender 5.1.1 command run:
  - `& "C:\Program Files\Blender Foundation\Blender 5.1\blender.exe" --background --factory-startup --python "C:\UE\T66\Reports\AgentReviews\20260605_Hero1MaleFriendSlopRigging\build_friendslop_hero1_male_rig.py"`
- Script produced summary:
  - `reimport_passed: true`
  - `unweighted: 0`
  - `over4: 0`
  - `short_bones: []`
- Vertex count: 163,496.
- Max influences: 3.
- Root-only vertices: 0.
- Pelvis-only vertices: 0.
- Re-import validation: 1 armature, 1 mesh, `Material_0` survived, hierarchy survived, height 1.8000000735 m.
- Proof renders:
  - four-axis source proof and final `+X` front proof under `...\Blender\Rigging\Proofs\`
  - rest front/side proof
  - simple bend front/side proof
- Visual inspection: rest proof preserves the source relaxed low-arm stance; simple bend proofs show separate limb motion without root-only or pelvis-only smearing. Costume-panel/dense-topology deformation remains documented as a spike compromise.

## Known Compromise

- Rest pose keeps the source relaxed low-arm stance instead of forcing a broad A-pose. I tried broad coordinate-space A-pose conversion and it visibly pulled the fused coat panels outward, so the deliverable documents that a true authored A-pose requires segmentation, retopo, or hand cleanup.
- Weighting is deterministic coordinate-region weighting for a PhysicsAsset/PAC structure spike. It passes hard influence/re-import gates but is not claimed as hand-polished production animation weighting.
- Mesh remains dense Pixal3D topology. No Decimate or retopo was run.
- PhysicsAsset handoff: pelvis should be the central recovery/follow body, with expected bodies on pelvis, three spine segments, head, upper/lower arms, thighs, calves, and feet.
- The process doc contents were reviewed through Claude's independent-answer and cross-review loop.

## PPF Close Draft

- Process used: `Model Generation/Instructions/13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md` plus Blender scripted rigging/export/re-import validation.
- Matches declared process: YES for Blender-only clean skeletal rig/FBX/report/doc; PARTIAL against the optional ideal A/T-pose preference because the fused coat mesh made a forced A-pose destructive, and that compromise is documented.
- Evidence: saved `.blend`, skeletal FBX, re-import validation blend, proof renders, QA JSON/report, and updated routing docs.
