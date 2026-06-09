# Hero 1 Chad Physics-First Rig Report

## Source

- Raw GLB: `Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Outputs\Hero_1_Chad_Male.glb`
- Output root: `Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\PhysicsFirstHero`
- Process: fresh raw FriendSlop GLB import, fresh deformation armature, fresh single-pass physics-first weights, fresh pose-target clips.
- Explicitly not used: old spike rig, old FBX exports, Quaternius clips, Animated ToonStyle assets, Roll clip.

## Rig

- Required bones: 23
- Mesh objects: 1
- Vertex count: 163496
- Unweighted vertices: 0
- Max influences per vertex: 1
- Normalized target height: 180.0 cm
- Blender proof/front axis: `-Y`
- Expected Unreal forward after FBX axis conversion: `+X`

## Clips

- `Idle`: `Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\PhysicsFirstHero\AnimationSources\AM_Hero_1_Chad_PhysicsFirst_Idle.fbx` frames 1-60
- `Walk`: `Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\PhysicsFirstHero\AnimationSources\AM_Hero_1_Chad_PhysicsFirst_Walk.fbx` frames 1-30
- `Jump`: `Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\PhysicsFirstHero\AnimationSources\AM_Hero_1_Chad_PhysicsFirst_Jump.fbx` frames 1-34
- `Leap`: `Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\PhysicsFirstHero\AnimationSources\AM_Hero_1_Chad_PhysicsFirst_Leap.fbx` frames 1-42
- `RecoverStand`: `Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\PhysicsFirstHero\AnimationSources\AM_Hero_1_Chad_PhysicsFirst_RecoverStand.fbx` frames 1-45
- `GetUp_Back`: `Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\PhysicsFirstHero\AnimationSources\AM_Hero_1_Chad_PhysicsFirst_GetUp_Back.fbx` frames 1-50
- `GetUp_Front`: `Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\PhysicsFirstHero\AnimationSources\AM_Hero_1_Chad_PhysicsFirst_GetUp_Front.fbx` frames 1-50

## Proof Renders

- `front_blender_minus_y`: `Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\PhysicsFirstHero\ProofRenders\Hero_1_Chad_PhysicsFirst_front_blender_minus_y.png`
- `side_plus_x`: `Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\PhysicsFirstHero\ProofRenders\Hero_1_Chad_PhysicsFirst_side_plus_x.png`
- `iso`: `Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\PhysicsFirstHero\ProofRenders\Hero_1_Chad_PhysicsFirst_iso.png`

## QA Result

- Blender source generation: PASS
