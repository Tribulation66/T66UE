Original user request:
Ok go for it

Immediate context:
- The user first asked for consultation on making T66 feel like Fall Guys: bouncy, physics-y, traps/bosses knock heroes around, ragdoll/incapacitate so they cannot attack, low-end PC/Steam Deck constraints.
- We already built/changed a test-room wipeout arm trap and a skeletal Chad test override in prior turns.
- The user then corrected the trap to a center-rotating wipeout arm.
- The user observed the current implementation gives only a small impact/push and then the hero disappears/resets after a while; they want correct ragdoll physics where the hero keeps going until a wall/surface or drags/slows on ground.
- We researched active ragdoll sources; user supplied transcripts for UE physical animation, active ragdoll, fall/recover, and Fall Guys/Gang Beasts-like references.
- We inspected the current skeletal Chad mesh in Blender. The skeletal test FBX appears usable for a first active-ragdoll spike: armature exists, mesh is weighted, no unweighted verts, but the skeleton is Rigify-like with many control/deform bones and no clean central pelvis. The raw FriendSlop GLB remains static/no rig and is out of scope for now.

Working task:
Operator: Codex
Validator: Claude
Scope: implement the next test-room active-ragdoll step for skeletal Hero 1 Chad: Unreal-side PhysicsAsset/bone-map/physical-animation/capsule-recovery work, with Blender mesh changes only if inspection proves they are needed.
Stop condition: current code/assets updated, verified as far as practical with build/runtime evidence, and reported with remaining gaps.

Important repo rules:
- Codex is Operator; Claude is Validator.
- This is implementation now, but scoped to the test-room hero-only active ragdoll spike.
- Do not broaden into all heroes/enemies/bosses.
- Do not make unrelated cleanup.
- Use live repo state.
- This is a process-governed visual/animation/physics task: preserve the UE PhysicalAnimationComponent + PhysicsAsset + capsule-follow/recovery method class from the supplied references. If a load-bearing mechanism is missing, call it out.

Known current code state to verify/correct:
- `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp` has test-room CVars:
  - `t66.TestRoom.EnableWipeoutArmTrap`
  - `t66.TestRoom.UseSkeletalChadOneOverride`
- Same file applies skeletal Chad override with `SK_Hero_1_Chad`, hides static visuals, and logs the PhysicsAsset.
- Current impact code uses `LaunchCharacter`, enables full skeletal physics if a PhysicsAsset exists, calls `AddImpulseToAllBodiesBelow`, then uses a fixed timer to restore the mesh/collision. This likely causes the observed “impact then disappears/resets” failure.
- `Content/Characters/Heroes/Hero_1/Chad/AnimatedToonStyle/PA_Hero_1_Chad_TestRoom.uasset` exists.
- `Source/T66Editor/T66CreateTestRoomPhysicsAssetCommandlet.cpp` generated that test PhysicsAsset with `Bodies=6 Constraints=2`, likely too coarse but enough for first testing; it currently uses automatic creation settings.

Questions for validator:
1. What implementation shape should Codex use for this spike, grounded in the current T66 code? Keep it in `T66GameMode_TestRoom.cpp`, add a temporary component, or create a reusable gameplay component now?
2. What exact mechanisms must be changed so the hero keeps moving/bouncing/dragging instead of resetting from a fixed timer?
3. Is it acceptable for this phase to defer authored get-up animations and use a C++ settle/reset recovery, or is that too far from the reference method?
4. What risks/files should Codex inspect before editing?
5. What verification should be considered enough for this scoped pass?

Please provide an independent repo-grounded answer and end with a clear `Result: OK` or `Result: NEEDS_USER`.
