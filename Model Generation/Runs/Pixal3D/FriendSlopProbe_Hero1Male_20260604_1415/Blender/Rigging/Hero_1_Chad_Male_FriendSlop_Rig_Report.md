# Hero 1 Chad Male FriendSlop Rig Report

## Source

- Source GLB: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Outputs\Hero_1_Chad_Male.glb`
- Source true visual front from four-axis proof: `+Y`.
- Final rig visual front: `+X` after a documented `-90` degree Z rotation.
- Black-outline look-dev is reference only. The skeletal FBX does not export outline/stylization geometry or materials.
- Raw GLB material slot and UV data are preserved for later Unreal texture rebind; the exported FBX keeps `Material_0`.

## Output

- Blender rig source: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\Rigging\Hero_1_Chad_Male_FriendSlop_Rig.blend`
- Skeletal FBX: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\Rigging\Hero_1_Chad_Male_FriendSlop_Skeletal.fbx`
- Re-import validation blend: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\Rigging\Hero_1_Chad_Male_FriendSlop_Skeletal_reimport_validation.blend`
- QA JSON: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\Rigging\Hero_1_Chad_Male_FriendSlop_Rig_QA.json`

## Height And Bounds

- Final height: `1.8000 m`.
- Bounds min: `[-0.26218053698539734, -0.4510159492492676, 0.0]`.
- Bounds max: `[0.2621805667877197, 0.4510159194469452, 1.8000000715255737]`.
- Mesh transforms are identity in the exported rig source; feet are on floor at `Z=0`.

## Forward-Axis Proof

- Source facing contact sheet: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\Rigging\Proofs\Hero_1_Chad_Male_facing_contact_sheet.png`
- Final `+X` front proof: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\Rigging\Proofs\Hero_1_Chad_Male_final_facing_plus_x_camera_front.png`
- Final `-X` back proof: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\Rigging\Proofs\Hero_1_Chad_Male_final_facing_minus_x_camera_back.png`
- Final `+Y` side proof: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\Rigging\Proofs\Hero_1_Chad_Male_final_facing_plus_y_camera_side.png`
- Final `-Y` side proof: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\Rigging\Proofs\Hero_1_Chad_Male_final_facing_minus_y_camera_side.png`

## Exported Bone Hierarchy

```text
root parent=None length=0.1400
pelvis parent=root length=0.1800
spine_01 parent=pelvis length=0.1700
spine_02 parent=spine_01 length=0.1700
spine_03 parent=spine_02 length=0.1400
neck_01 parent=spine_03 length=0.1005
head parent=neck_01 length=0.1910
clavicle_l parent=spine_03 length=0.2184
upperarm_l parent=clavicle_l length=0.4604
lowerarm_l parent=upperarm_l length=0.3534
hand_l parent=lowerarm_l length=0.1738
clavicle_r parent=spine_03 length=0.2184
upperarm_r parent=clavicle_r length=0.4604
lowerarm_r parent=upperarm_r length=0.3534
hand_r parent=lowerarm_r length=0.1738
thigh_l parent=pelvis length=0.3801
calf_l parent=thigh_l length=0.3100
foot_l parent=calf_l length=0.1991
ball_l parent=foot_l length=0.1300
thigh_r parent=pelvis length=0.3801
calf_r parent=thigh_r length=0.3100
foot_r parent=calf_r length=0.1991
ball_r parent=foot_r length=0.1300
```

- Bone count: `23`.
- Count reconciliation: `7` central/root bones + `8` arm bones + `8` leg bones = `23` deform bones; no helper/control/leaf bones are exported as physics candidates.
- Missing required bones: `[]`.
- Wrong required parents: `[]`.
- Short required bones under `0.035` m: `[]`.

## Vertex Influence QA

- Vertex count: `163496`.
- Unweighted vertices: `0`.
- Vertices over 4 influences: `0`.
- Max influences: `3`.
- Max normalization error: `0.00000004`.
- Root-only vertices: `0`.
- Pelvis-only vertices: `0`.

## Rest And Bend Proof

- Rest front: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\Rigging\Proofs\Hero_1_Chad_Male_rest_front.png`
- Rest side: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\Rigging\Proofs\Hero_1_Chad_Male_rest_side.png`
- Bend front: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\Rigging\Proofs\Hero_1_Chad_Male_bend_front.png`
- Bend side: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\Rigging\Proofs\Hero_1_Chad_Male_bend_side.png`
- Visual inspection: rest pose preserves the source relaxed low-arm stance; simple bend proofs show separate limb motion without root-only or pelvis-only smearing. Costume-panel and dense-topology deformation remains a documented spike compromise.

## Re-import Validation

- Re-import passed: `True`.
- Re-import armature count: `1`.
- Re-import mesh count: `1`.
- Re-import material slots: `['Material_0']`.
- Re-import height: `1.8000 m`.

## Known Compromises And PhysicsAsset Notes

- This is a deterministic spike rig for PhysicsAsset/PAC structure, not a hand-polished production animation rig.
- The mesh remains dense Pixal3D topology; no Decimate or retopo was run.
- Coordinate-region weights meet the hard influence gates, but shoulder/hip polish should be reviewed before final authored animation use.
- Rest pose keeps the source relaxed low-arm stance because forcing a broad A-pose on this fused coat mesh distorts the coat panels.
- The relaxed low-arm stance is still suitable for the immediate PhysicsAsset/PAC structure spike because the deform hierarchy exposes pelvis, spine, head, arm, and leg chains. A true authored A/T-pose remains recommended before production animation/retargeting.
- The real pelvis and three-spine central chain should give Unreal enough structure to build a meaningful PhysicsAsset instead of collapsing to the legacy 6-body/2-constraint failure.
- PhysicsAsset handoff: pelvis should be the central recovery/follow body, with expected bodies on pelvis, three spine segments, head, upper/lower arms, thighs, calves, and feet.
- Pelvis should be used as the later recovery/follow body in Unreal, not the floor `root`.
- Central process doc `Model Generation/Instructions/13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md` was discussed with Claude through the independent-answer and cross-review loop.
