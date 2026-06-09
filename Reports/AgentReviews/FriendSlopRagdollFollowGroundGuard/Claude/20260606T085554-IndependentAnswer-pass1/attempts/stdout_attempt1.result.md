Result: OK

## Independent Answer

Both bugs are in shared runtime, not TestRoom-only symptoms, so they belong in `UT66KnockbackComponent`. The TestRoom code (`T66GameMode_TestRoom.cpp`) only computes the launch direction and forwards to `ApplyKnockbackLaunch`; the follow/settle behavior all lives in the component.

**1. Probable root cause(s)**

*Outer-arm hit → actor/camera snaps near spawn while mesh ends at the arm:*
- The visible mesh is the detached, free-simulating ragdoll (`bDetachMeshDuringRagdoll`, `T66KnockbackComponent.cpp:258-262`), which flies out to the arm. The actor root (and the spring-arm/camera, which is attached to the capsule/actor root) only tracks the mesh when the per-tick follow block runs: `UpdateActiveKnockback` lines 414-440. If that follow path is effectively not running for this hit, the actor stays put while the mesh leaves — exactly the reported "hero at arm, camera at spawn" split.
- Two concrete suspects:
  - `ActiveProfile.bFollowActorToRagdoll` is gated at line 414 but is **never set** in `MakeWipeoutArmKnockbackProfile` (lines 438-467), so it inherits `DefaultProfile`. If that default is false (or differs from the centering CVar intent), the actor never follows. This is the first thing Codex should confirm.
  - The follow target is `GetFollowLocation + ActorToFollowBoneOffset`, where the offset is captured **once** at impact as `actorLoc - followLoc` when `bUsePreImpactActorToFollowBoneOffset` is true (the default, since `CenterActorOnRagdoll == 0`, line 458). For an outer-side hit the body tumbles hard; a stale rigid additive offset against a tumbling pelvis can place the actor target far from the visible mesh. The Z is clamped (lines 420-424) but there is **no horizontal clamp**, so this is the more likely driver of large horizontal divergence than a pure Z issue.
- The "only outer side" specificity needs runtime confirmation rather than a guess — it is most likely that outer hits produce the largest tangential throw, making an always-present follow gap visible. Confirm with a per-tick log before committing to a mechanism.

*Body going halfway under the ground:*
- There is **no hard floor guard anywhere.** Penetration is left entirely to the PhysX/Chaos solver. The mesh has CCD (line 287) and blocks `WorldStatic`/`WorldDynamic` (lines 282-283), but a fast launch (LaunchXY/LaunchZ) plus thin floor or per-body tunneling will still sink limbs below the floor. A hard gate must be added explicitly.

**2. Recommended code-level fix location(s)**

- Bug 1: In `MakeWipeoutArmKnockbackProfile`, explicitly set `bFollowActorToRagdoll` (don't rely on default). Then in `UpdateActiveKnockback` (lines 414-440), reconsider the rigid `ActorToFollowBoneOffset` for detached ragdolls — for a center-on-follow feel, drive the actor straight to the follow-bone XY (decay the captured offset toward zero), keeping the existing Z clamp. Keep the fix in the component so it covers all knockback callers, not just the arm.
- Bug 2: Add a new `EnforceFloorPenetrationGuard()` called at the end of `UpdateActiveKnockback`. Resolve floor Z once per tick (downward trace under the actor against `WorldStatic`, or a configured floor-Z profile field), and for the lowest simulated bodies clamp any body whose `worldZ - extent` is below floor: lift it back to the floor and zero only the **downward** velocity component. Expose a profile toggle + skin-depth tolerance so it's tunable and disableable.

**3. Verification hooks/logs**

- Reuse `LogT66Knockback`. Add a throttled per-tick line logging: actor location, follow-bone world location, mesh component location, and lowest-body Z vs resolved floor Z. This directly proves/disproves the follow-divergence hypothesis and the penetration depth.
- Log a one-line correction event whenever the floor guard lifts a body (bone name, penetration depth corrected).
- The existing recovery log (lines 470-479) already prints actor location at recovery — compare it against mesh location to quantify the split.
- Staged proof: standalone TestRoom with `t66.TestRoom.EnableWipeoutArmTrap 1`, take an outer-side hit, confirm camera/actor track the thrown hero and that no limb clips below the floor.

**4. Risks / things to avoid**

- Don't fix Bug 1 by teleporting the actor only at recovery — that masks the in-flight camera detachment the user is complaining about. Fix the per-tick follow.
- Floor guard via per-tick `SetBodyTransform` fights the solver and causes jitter/constraint explosions. Prefer: correct only when penetration exceeds a tolerance, lift minimally, and zero only the downward velocity rather than full teleport. Avoid per-tick downward traces for *every* body (expensive) — trace once for the floor under the actor or use a known floor-Z.
- Keep the guard and follow logic in `UT66KnockbackComponent` (shared runtime), per repo rule, not in the TestRoom timer lambda.
- Gameplay runtime change → requires focused compile/build + staged standalone validation before the stop condition is met.

## Evidence Checked
- `Source/T66/Gameplay/T66KnockbackComponent.cpp` (full): follow logic 414-440, detach 258-262, CCD/collision 279-287, recovery restore 526-562, profile clamps.
- `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp`: impact/launch routing 470-519, profile builder 438-467 (note: `bFollowActorToRagdoll` not set), arm hit geometry and side-sign direction 666-693.
- Original prompt and task framing.

## Questions Or Blockers
None requiring the user. The exact "outer-side only" trigger needs a runtime log to confirm, but that is Codex's instrumentation/build step, not a user decision.

## Caveats
- I did not read `T66KnockbackComponent.h` or the camera/spring-arm attachment in `T66HeroBase`; my claim that the camera rides the actor root (not the mesh) is inferred from the symptom (camera stays at spawn while detached mesh flies). Codex should confirm the spring-arm parent before settling the Bug 1 mechanism.
- `DefaultProfile`'s value for `bFollowActorToRagdoll` is in the header/defaults I didn't open — verify it directly.
- Floor-Z resolution strategy depends on whether the real game floors are flat/known; the TestRoom floor is, but a shared guard should trace rather than assume.
