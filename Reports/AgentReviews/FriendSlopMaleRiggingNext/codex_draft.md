# Codex Draft

Recommendation: proceed to Unreal now; do not send the rig back to Blender for A/T-pose or retopo before the ragdoll/PAC spike.

Reasoning:
- The immediate blocker was not production animation quality. It was the degenerate TestRoom PhysicsAsset/PAC setup: 6 bodies, 2 constraints, no central pelvis chain, PAC crash/hang.
- The new rig directly addresses that blocker: 23 deform bones, real pelvis, three spine bones, symmetric limbs, no unweighted verts, max 3 influences, FBX reimport passed.
- The relaxed low-arm stance is a real compromise, but it is acceptable for the next physics-structure test. It is not acceptable as the final animation/retargeting rig without later cleanup.

Next sequence:
1. Import `Hero_1_Chad_Male_FriendSlop_Skeletal.fbx` into Unreal as an isolated experimental skeletal mesh/skeleton, not as a replacement for normal `Hero_1_Chad` gameplay rows yet.
2. Preserve the existing static FriendSlop `CharacterVisuals.csv` rows until the physics proof is good. The normal row should not flip to skeletal until we know the mesh imports, simulates, recovers, and captures correctly.
3. Bind/rebind the raw FriendSlop material enough that the mesh is recognizable, but do not make material polish the gate. The gate is physics and PAC stability.
4. Generate or hand-author a PhysicsAsset on the new skeleton. Required bodies: pelvis, spine_01, spine_02/spine_03, head, upper/lower arms, thighs, calves, feet. Pelvis should be the recovery/follow body, not root.
5. If auto-generation still collapses bodies/constraints, do not go back to Blender first. Hand-author/tune the PhysicsAsset central chain in Unreal before blaming the rig.
6. Update the TestRoom-only skeletal override to use the new FriendSlop skeletal mesh/PhysicsAsset instead of legacy `SK_Hero_1_Chad`.
7. Re-enable `t66.TestRoom.WipeoutArmEnablePhysicalAnimation=1` only after confirming valid pose buffers and a sane PhysicsAsset body/constraint count.
8. Run the wipeout-arm capture proving: impact, body travel, wall/floor collision, PAC drive, incapacitation, settle/recovery, restored control, no crash/hang.
9. Fix or add a dedicated TestRoom ragdoll review camera before using the video as visual proof, because the previous capture framing was weak.

Defer:
- A/T-pose conversion.
- Retopo.
- production animation/retargeting.
- replacing the normal `Hero_1_Chad` runtime row.
- horde/enemy ragdoll expansion.

Stop criterion for the next phase:
- A new FriendSlop skeletal mesh imports into Unreal.
- A PhysicsAsset exists with meaningful torso/limb bodies and constraints, not 6/2.
- TestRoom ragdoll with PAC enabled runs without crash/hang.
- Capture/log evidence proves the ragdoll path and recovery.

Suggested next task wording:
"Implement the Unreal-side FriendSlop Hero 1 skeletal import and TestRoom ragdoll/PAC spike using `Hero_1_Chad_Male_FriendSlop_Skeletal.fbx`. Keep normal CharacterVisuals static rows untouched. Import as an isolated experimental skeletal asset, create/tune a PhysicsAsset with pelvis/spine/limb bodies, point only the TestRoom skeletal override at it, re-enable PAC if stable, and capture wipeout-arm impact/recovery proof."
