Original user request:
Ok now is time to import this model with the black outline. Youre also goign to crete a process doc for rigging friendslop models, writing down the process and best practice/things to avoid, discuss with claude what should go in this file, its supposed to be the central file which future agents without context will have to use to rig models Goal:
Create a clean skinned skeletal version of the FriendSlop male model that can generate a stable Unreal PhysicsAsset and support ragdoll/PAC. This rig is intended to replace the legacy SK_Hero_1_Chad TestRoom skeletal target after Unreal import.

The failure to avoid: current TestRoom PhysicsAsset has only 6 bodies / 2 constraints, no clean central pelvis chain, and PAC crashes or hangs.

Skeleton requirements:
- Single root hierarchy. No duplicate roots, mesh-parented bones, or exported leaf/end-bone junk.
- True central physics chain:
  root -> pelvis -> spine_01 -> spine_02 -> spine_03 -> neck_01 -> head
- Arms:
  clavicle_l/r -> upperarm_l/r -> lowerarm_l/r -> hand_l/r
- Legs:
  thigh_l/r -> calf_l/r -> foot_l/r -> ball_l/r
- UE5-Mannequin-like names are preferred unless you find an existing T66 convention that is clearly better.
- No zero-length or near-zero torso/limb bones.
- Rest pose should be neutral A-pose or T-pose.

Ragdoll/PhysicsAsset requirements:
- Skeleton must support separate Unreal physics bodies for pelvis, spine bodies, head, upper/lower arms, thighs, calves, and feet.
- Pelvis should be the central recovery/follow body, not an arbitrary floor root.
- Avoid exporting helper/control bones as physics candidates unless they are intentional deform bones.

Mesh and weights:
- Apply transforms before export: scale 1.0, clean rotation, feet on floor, placement documented.
- Preserve approx 180 cm height.
- Max 4 influences per vertex, normalized.
- No unweighted vertices.
- No large accidental root-only or pelvis-only weighted regions.
- Smooth symmetric deformation at shoulders, hips, elbows, knees, ankles, and neck.
- Do not blindly Decimate and call it production topology. If topology is too dense, keep it for this spike or document a retopo recommendation.

Facing/export:
- First do a four-axis facing proof in Blender (+X, -X, +Y, -Y) and document true visual front.
- Export UE-friendly FBX with Z-up and documented forward axis. Prefer UE +X forward if practical.
- Do not solve runtime yaw by guessing in Blender. Runtime yaw remains an Unreal/CharacterVisuals adapter.
- Export skeleton + skinned mesh only. No animation clips required.
- Keep/document material slots so Unreal can rebind the raw FriendSlop texture later. Do not run ToonStyle, Quad Retro, tint, outline, or stylization processing.

Deliverables:
- Suggested output folder, create if absent:
  C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\Rigging\
- Saved Blender working file.
- Exported skeletal FBX.
- Short rig report with:
  1. source file used
  2. final height/bounds
  3. forward-axis proof
  4. exported bone hierarchy
  5. vertex influence QA
  6. front/side proof of rest pose and simple limb bend
  7. known compromises and Unreal PhysicsAsset notes

Acceptance:
- Re-import exported FBX into Blender and confirm skeleton, mesh, weights, rest pose, and material slot survive.
- Real pelvis and 3-spine chain exist.
- No unweighted verts, no >4 influences, no zero-length torso/limb bones.
- Bend tests do not smear shoulders, hips, elbows, knees, or neck.
- The rig should give Unreal enough structure to build a meaningful PhysicsAsset, avoiding the current 6-body/2-constraint failure.

Do not edit Unreal assets, C++ code, CSV rows, or data tables. Blender rigging deliverable and report only.

Working task:
Operator: Codex
Validator: Claude
Scope: Blender-only rigging deliverable for the FriendSlop Hero 1 male Pixal3D model: create/save a clean skeletal Blender file, exported skeletal FBX, QA report, and a central FriendSlop rigging process doc. No Unreal asset edits, C++ edits, CSV/DataTable edits, or ToonStyle/Quad Retro/stylization processing.
Stop condition: Rigging folder contains the working `.blend`, skeletal FBX, proofs/QA report, and process doc, with Blender re-import validation completed or a concrete blocker reported.

Repo/process context:
- `AGENTS.md` requires Operator/Validator, PPF, current verification, and no native goal tools.
- `.t66/operator-state.json` says Codex operator / Claude validator.
- `Model Generation/MODEL_GENERATION_AGENTS.md` owns Pixal3D, Blender QA, rigging/retopo policy.
- `Model Generation/Instructions/04_BLENDER_PROCESSING_AND_RIGGING_INSTRUCTIONS.md` says do not promote Decimate as production topology; rigging work must bake mesh placement and preserve actions explicitly.
- `Model Generation/Instructions/11_FRIENDSLOP_RAW_PIXAL3D_IMPORT_GUIDELINES.md` says FriendSlop raw assets preserve the generated GLB texture/material and skip ToonStyle/Quad Retro/tint/outline processing unless explicitly approved.
- `Model Generation/Rigging and Animation/README.md` says old broad humanoid automation is retired and current FriendSlop work should not use the legacy Animated ToonStyle bridge. This request is an explicit new FriendSlop rigging process, so it needs its own central doc rather than reusing the retired bridge.

Source asset:
- `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Outputs\Hero_1_Chad_Male.glb`
- Previous black-outline look-dev proof exists, but export requirements say skeleton + skinned mesh only and no outline/stylization processing. Codex is treating the outline as reference/proof context, not an exported rig mesh/material.

Ask for Claude:
1. Independently review the task and identify key risks/must-have validation gates for the rigging pass.
2. Discuss what should go into the central FriendSlop rigging process doc for future agents without context.
3. Surface any scope conflicts or blocker before Codex implements.
