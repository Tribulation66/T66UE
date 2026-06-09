# Codex Draft

The current docs say FriendSlop raw Pixal3D is a static-mesh path and manual humanoid rigging is outside the automated Rigging and Animation folder. This rigging task is therefore a new manual character-rig pass, not an existing FriendSlop rigging pipeline. The old Animated ToonStyle bridge is useful only as historical context and should not be revived as the active source path.

Current source facts:
- Primary source GLB: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Outputs\Hero_1_Chad_Male.glb`
- Textured FBX fallback: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\RawTexturedFBX\Hero_1_Chad_Male\Hero_1_Chad_Male_Textured.fbx`
- Texture: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\RawTexturedFBX\Hero_1_Chad_Male\Textures\Hero_1_Chad_Male_00_Image_0.png`
- Runtime rows: `Hero_1_Chad` and `Hero_1_Chad_DemoSkin`
- Current runtime row is static mesh only, yaw 90, target height 180 cm.
- Textured FBX export report says approx 90.2 x 52.4 x 180 cm, 163,496 verts.

Ragdoll/PAC target:
- The TestRoom prototype currently ragdolls a legacy `SK_Hero_1_Chad` mesh and follows a simulated spine body in C++.
- The new FriendSlop rig is intended to supersede that legacy TestRoom skeletal mesh for PAC/ragdoll testing once imported.
- PAC is default OFF because the generated PhysicsAsset is too coarse: proof logs show 6 bodies and 2 constraints, no clean central pelvis chain, and PAC crashed/hung.
- The rigging deliverable should make Unreal able to build or hand-author a clean PhysicsAsset with pelvis/spine/head/limb bodies and meaningful constraints.

Copy-ready prompt:

```text
You are working in the T66 repo with Blender already open. Rig the FriendSlop Hero 1 male model so it can become a UE5 skeletal hero that supports ragdoll and PhysicalAnimationComponent testing.

This is a manual rigging pass. Do not use or revive the old automated Animated ToonStyle / AccuRig bridge as the active FriendSlop source path. Preserve the raw FriendSlop Pixal3D visual identity and texture.

Source files:
- Primary mesh: C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Outputs\Hero_1_Chad_Male.glb
- Textured FBX fallback if GLB import is unusable: C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\RawTexturedFBX\Hero_1_Chad_Male\Hero_1_Chad_Male_Textured.fbx
- Raw FriendSlop BaseColor texture to preserve/reference: C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\RawTexturedFBX\Hero_1_Chad_Male\Textures\ (verify and use the correct base-color PNG in that folder)

Known source facts:
- AssetID: Hero_1_Chad_Male
- Target runtime rows later: Hero_1_Chad and Hero_1_Chad_DemoSkin
- Current static row uses target height 180 cm and yaw 90 in CharacterVisuals.csv. Do not edit CharacterVisuals.csv in this Blender pass.
- The textured FBX export report measured approx 90.2 x 52.4 x 180 cm and 163,496 verts.

Goal:
Create a clean skinned skeletal version of the FriendSlop male model that can generate a stable UE PhysicsAsset and support ragdoll/PAC. This rig is intended to replace the legacy `SK_Hero_1_Chad` TestRoom skeletal target for PAC once the Unreal import/PhysicsAsset follow-up happens. The main failure we are trying to avoid is the current TestRoom physics asset: 6 bodies / 2 constraints, no clean central pelvis chain, PAC crashes or hangs.

Skeleton requirements:
- Single root hierarchy. No duplicate roots, no mesh-parented bones, no leaf/end-bone junk.
- True central physics chain:
  root -> pelvis -> spine_01 -> spine_02 -> spine_03 -> neck_01 -> head
- Symmetric arms:
  clavicle_l/r -> upperarm_l/r -> lowerarm_l/r -> hand_l/r
- Symmetric legs:
  thigh_l/r -> calf_l/r -> foot_l/r -> ball_l/r
- Bone names should be UE-friendly and stable. UE5-Mannequin-like names are preferred unless you find an existing T66 convention that is clearly better.
- No zero-length or near-zero torso/limb bones.
- Rest pose should be a neutral A-pose or T-pose suitable for skinning and physics asset generation.

Ragdoll/PhysicsAsset-friendly requirements:
- The skeleton must support separate Unreal physics bodies for at least: pelvis, spine_01, spine_02/spine_03, head, upperarms, lowerarms, thighs, calves, feet.
- Keep enough anatomical separation in the hierarchy that Unreal auto-generation does not collapse the torso into one body or skip limb bodies.
- The pelvis should be the central body for recovery/follow, not an arbitrary root at the floor.
- Avoid helper/control/deformer bones being exported as physics candidates unless they are intentionally part of the deform skeleton.

Mesh and weights:
- Apply transforms before export: scale 1.0, rotation clean, feet on floor, origin/placement documented.
- Preserve approximately 180 cm height.
- Max 4 bone influences per vertex, normalized.
- No unweighted vertices.
- No large regions weighted only to root or pelvis unless anatomically correct.
- Smooth, symmetric deformation on shoulders, hips, knees, elbows, ankles, and neck.
- If the source topology is too dense for good deformation, do not blindly Decimate and call it production topology. Either keep the mesh for this spike or make a clearly labeled retopo recommendation. Decimate is diagnostic/prototype only for deformation-critical characters.

Facing and export:
- First do a four-axis facing proof in Blender (+X, -X, +Y, -Y) and document which direction is the model's true visual front.
- Export a UE-friendly FBX with Z-up and forward axis documented. Prefer UE +X forward in the exported skeletal asset if practical.
- Do not solve runtime yaw in Blender by guessing. Runtime yaw remains an Unreal/CharacterVisuals adapter. Just document the exported forward axis clearly.
- Export skeleton + skinned mesh only. No animation clips are required in this pass, but include the clean rest pose.
- Keep or document texture/material slots so Unreal can rebind the raw FriendSlop texture later. Do not run ToonStyle, Quad Retro, tint, outline, or material stylization processing.

Deliverables:
- A saved Blender working file in this suggested output folder; create it if absent:
  C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\Rigging\
- An exported skeletal FBX from that same folder.
- A short rig report markdown/json containing:
  1. Source file used.
  2. Final mesh height and bounds.
  3. Forward axis/facing proof result.
  4. Full exported bone hierarchy.
  5. Vertex influence QA: max influences, unweighted vertex count, root-only/pelvis-only anomaly count if available.
  6. Screenshots or rendered proof from front/side showing rest pose and a simple limb bend/deformation test.
  7. Any known compromises or recommended Unreal PhysicsAsset notes.

Acceptance criteria:
- Re-open/re-import the exported FBX in Blender and confirm the skeleton, mesh, weights, rest pose, and material slot survive.
- Bone hierarchy has a real pelvis and 3-spine chain.
- No unweighted verts; no >4 influences; no zero-length torso/limb bones.
- Simple bend tests do not visibly smear the shoulders, hips, elbows, knees, or neck.
- The exported rig should give Unreal enough structure to create a PhysicsAsset with meaningful torso and limb bodies, avoiding the current 6-body/2-constraint failure.

Do not edit Unreal assets, C++ code, CSV rows, or data tables in this pass. Your job is the Blender rigging deliverable and report only.
```

Caveat:
The PhysicsAsset itself is still an Unreal-side follow-up. The Blender rig can make a good PhysicsAsset possible; it cannot prove PAC stability until Unreal imports the skeletal FBX, creates/tunes a PhysicsAsset, re-enables `t66.TestRoom.WipeoutArmEnablePhysicalAnimation=1`, and reruns the TestRoom wipeout-arm capture.
