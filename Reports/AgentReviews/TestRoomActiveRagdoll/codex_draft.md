Task contract:
- Operator: Codex
- Validator: Claude
- Scope: implement the next test-room active-ragdoll step for skeletal Hero 1 Chad: Unreal-side PhysicsAsset/bone-map/physical-animation/capsule-recovery work, with Blender mesh changes only if inspection proves they are needed.
- Stop condition: current code/assets updated, verified as far as practical with build/runtime evidence, and reported with remaining gaps.

Codex draft:
Implemented the Hero 1 TestRoom wipeout-arm active-ragdoll spike in `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp`.

What changed:
- TestRoom now applies the existing skeletal Hero 1 Chad mesh/PhysicsAsset only for the TestRoom Hero_1 path, leaving the normal static Chad path alone.
- Wipeout-arm impact now prefers skeletal ragdoll when a valid skeletal mesh and PhysicsAsset exist: disables movement/combat, disables capsule collision, enables skeletal mesh QueryAndPhysics, enables gravity/CCD, simulates all bodies, applies impulse, and enters a ragdoll state.
- The ragdoll state keeps the actor/capsule following a chosen simulated spine body while preserving mesh world transform. This removes the original fixed-timer reset as the disappearance cause; recovery is driven by settle detection or forced max timeout.
- Restores movement, capsule collision, body simulation, mesh collision, and combat input on recovery.
- Added CVar tuning for launch, incapacitation, max ragdoll, settle, blend-out, and PhysicalAnimationComponent enablement.
- PhysicalAnimationComponent support exists behind `t66.TestRoom.WipeoutArmEnablePhysicalAnimation`, but defaults OFF because the current generated PhysicsAsset/PAC path is not stable.
- Added a pending issue in `Source/T66/Gameplay/pending_issues_Gameplay.md` documenting the PAC crash/hang and the required next fix.

Verification:
- `git diff --check -- Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp Source/T66/Gameplay/pending_issues_Gameplay.md`: clean except Git line-ending warnings.
- Focused T66Editor build passed after the code changes.
- Hero 1 TestRoom capture completed:
  - `Saved/AgentReviews/TestRoomActiveRagdoll/testragdoll_hero1_ccd.mp4`
  - ffprobe: 1280x720, 12 fps, 10.0s, 120 frames.
  - Log proves Hero_1 direct entry, skeletal Chad override applied, ragdoll impact, recovery start, and restored control.
  - Impact log: `PhysicalAnimation=0 PhysicsBodies=6 PhysicsConstraints=2`.
  - Recovery log: `Speed=344.9 Settled=0 Forced=1`, then restored at Z=92.
- `Scripts/StageStandaloneBuild.ps1` succeeded and updated both standalone shortcuts to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Explicit shortcut check confirmed both `C:\UE\T66\T66 Standalone.lnk` and the taskbar pinned shortcut target the staged exe.

Known gaps/caveats:
- This is partial against the full active-ragdoll vision. Default path is skeletal ragdoll and capsule-follow recovery, not PAC-driven active ragdoll.
- PAC was tried and rejected for default use because Hero_1 capture with PAC enabled first asserted with `Array index out of bounds: 5 into an array of size 0`, then a guarded/world-space follow-up hung after the active-ragdoll impact log.
- The current generated PhysicsAsset is very coarse: 6 bodies and 2 constraints. It is enough for the first sim/capsule-follow spike, but not enough for the intended bouncy Fall Guys read.
- The verified recovery was forced by max timeout, not clean settle (`Settled=0 Forced=1`), so settle-based recovery is implemented but not yet proven at runtime. The next tuning pass must improve body collision/friction/drag and/or PhysicsAsset quality.
- Existing capture framing is poor; the MP4/logs prove the path ran, but a dedicated TestRoom ragdoll review camera is still needed for visual judgment.

PPF close:
- Process used: UE skeletal PhysicsAsset ragdoll, C++ ragdoll state, capsule-follow recovery, and attempted PhysicalAnimationComponent drive.
- Matches declared process: PARTIAL.
- Evidence: runtime Hero_1 capture and logs prove skeletal ragdoll impact, incapacitation, body-follow recovery, and restore; PAC remains off by default pending PhysicsAsset/PAC stability work.

Mechanism close:
- Impact impulse to skeletal bodies: PRESENT.
- World collision/body travel: PARTIAL. CCD and room tuning are enabled; proof shows return to valid floor height, but capture framing does not visually prove wall/floor reads.
- Physical animation drive: DEFERRED. Code path is guarded by CVar, default off due crash/hang.
- Capsule follows ragdoll body: PRESENT.
- Incapacitation/cannot attack: PRESENT by movement/combat disable around ragdoll state.
- Recovery: PRESENT, but current proof used forced max timeout rather than settled recovery.
