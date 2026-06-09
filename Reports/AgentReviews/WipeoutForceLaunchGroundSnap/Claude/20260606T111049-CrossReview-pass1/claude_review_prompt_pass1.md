You are Claude cross-reviewing a Codex draft for the T66 Unreal project.

Rules:
- Include a clear Result: OK or Result: NEEDS_USER line near the top.
- Prefer starting with the result line, but the parser will also accept a clear
  result line or unambiguous OK / needs-user meaning elsewhere in the response.
- Do not edit files.
- Do not run mutating commands.
- Treat Codex as the Operator/final router and you as the Validator.
- Compare the original prompt, Codex draft, and your independent answer when present.
- Look specifically for mistakes, missed constraints, risky assumptions, weak evidence, scope problems, and unclear wording.
- Patch the answer text when the fix is straightforward.
- Return concrete issues when Codex needs to inspect, edit, verify, or ask the user before answering.
- Ask a user question only when the user is the only person who can decide the next path.
- Keep the review concise and practical. Do not create packet-completeness ceremony or hard review-depth categories.

Your result should be one of these two lines:
Result: OK
Result: NEEDS_USER

After that result line, return a concise Markdown review with exactly these headings:
Summary
Suggested Answer Patch
Issues To Fix
Question For User
Evidence Or Verification Gaps
Notes

Result meanings:
- OK: the models can handle the prompt internally. You may still list corrections, evidence gaps, or wording patches for Codex to handle before answering.
- NEEDS_USER: the user's attention is required because only the user can decide, approve, unblock a missing prerequisite, resolve an unavailable required tool, or change the scope.

Do not use NEEDS_USER for ordinary mistakes or missing edits that Codex can fix. List those inside the review body and keep the result OK.

Review scope:
- Original prompt path: C:\UE\T66\Reports\AgentReviews\WipeoutForceLaunchGroundSnap\original_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\WipeoutForceLaunchGroundSnap\codex_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\WipeoutForceLaunchGroundSnap\Claude\20260606T103622-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
User request:

Again were having the problem where im under neath the ground, also it seems like im dragged back to the original location. So look into this, I've had somehow got re-enabled or something, but I'm both getting under the ground and I'm having the issue of going back to the location, and the... The way the physics is working right now, when I get hit by the tool, it should push me out of the radius of the tool. Like you should, you know, we're focusing a bit too much on the ragdoll, like rigging and skeletal aspect of it. It should also be more force driven and really like throw me out until I hit a wall, and then I should bounce off the wall. That's what I want next because right now I spawn in and I just get stuck in this little loop. So we need to fix going underneath the ground, we need to fix going back to the location, and then we need to make sure that the distance I'm thrown is much more than it is now.

Task contract:

Working task:
Operator: Codex
Validator: Claude
Scope: fix the current TestRoom wipeout-arm behavior where the hero can sink below the floor, snap/return near the original location, and fail to be launched far enough out of the arm radius.
Stop condition: root cause is identified and patched in runtime code, focused build and staged standalone proof run, and any remaining manual feel-tuning caveats are explicit.

Relevant repo/process rules:
- C:\UE\T66\AGENTS.md requires Codex operator / Claude validator loop for T66 prompts.
- Current operator state is Codex operator, Claude validator.
- Runtime gameplay changes need focused compile verification and staged standalone validation when they affect the playable standalone.
- Existing T66 pure-Chaos hero ragdoll direction should be preserved. Do not re-enable PAC for heroes.

Live-code observations before edit:
- `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp` currently builds wipeout launch from `MotionDirection` only; that is tangential arm motion, not a radial ejection away from the hub.
- TestRoom wipeout uses a short active/recovery profile (`Incap=0.15`, `MaxRagdoll=0.40`, `BlendOut=0.10`), so repeated hits can happen quickly if the hero remains inside the arm radius.
- `Source/T66/Gameplay/T66KnockbackComponent.cpp` restores control at `RestoreFromKnockback()` and reattaches the mesh after detached ragdoll.

</original_prompt>

<validator_independent_answer>
Result: OK

## Independent Answer

The task is well-scoped and the three symptoms map to identifiable causes in the code I read. Codex can patch this without a user decision. My independent read:

**1. "Thrown back to original location" / stuck in a loop — root cause confirmed.**
The launch direction is purely tangential. In `ScheduleWipeoutArmTrap` the impact direction is `MotionDirection(-ArmAxis.Y*SideSign, ArmAxis.X*SideSign, 0)` (T66GameMode_TestRoom.cpp:700-701) — that is perpendicular to the arm, i.e. the sweep tangent, **not** radial away from the hub. `ApplyWipeoutArmHeroImpact` then builds `LaunchVelocity` from that direction only (lines 498-512). So the hero is shoved sideways along the sweep, stays inside `WipeoutArmRadiusUU`, and the still-rotating arm re-hits after the 1.0s cooldown (`WipeoutArmImpactCooldownSeconds`). With the short profile (Incap 0.15 / MaxRagdoll 0.40 / BlendOut 0.10) recovery snaps the actor to the body center near where it started — reading as "dragged back." **Fix:** derive the launch from the radial vector `(HeroLocation - ArmLocation)` flattened to XY (optionally add a fraction of the tangent for flavor), and raise `WipeoutArmLaunchXY` substantially so the hero clears the radius in one hit.

**2. Distance too small.** `WipeoutArmLaunchXY=1850` combined with the 0.40s max ragdoll window caps travel. Once direction is radial, increase launch XY (and likely lengthen `MaxRagdollSeconds`/`SettleHoldSeconds`) so the hero travels until it hits a wall. True wall-bounce is not in the current code path — the ragdoll bodies block `WorldStatic` (T66KnockbackComponent.cpp:335) so they will physically collide, but the actor-follow + fast forced recovery will cut the bounce short. Treat "bounce off the wall" as needing a longer ragdoll window, not just a bigger impulse.

**3. Under the ground.** The floor guard (`EnforceFloorPenetrationGuard`) and anchor (`ResolveActorFloorAnchorZ`) both rely on `ResolveRagdollFloorZ`, which traces **once** at the pre-impact location and then caches via `bHasResolvedFloorZ` (T66KnockbackComponent.cpp:979-994). The TestRoom floor is flat so this is usually fine, but once the hero is launched far (the desired behavior), the cached `PreImpactFloorZ` no longer corresponds to where the ragdoll lands, so the anchor/guard can seat the capsule below the actual surface. **Recommend** re-tracing floor Z at the ragdoll's current XY (invalidate the cache when the body center moves beyond a threshold) so a far landing anchors correctly.

**On the user's PAC worry ("somehow got re-enabled"):** the code does **not** re-enable PAC for heroes. It is hard-disabled in two places — `MakeWipeoutArmKnockbackProfile` (lines 466-471) and `ApplyKnockbackLaunch` (lines 150-163). So the symptoms are not a PAC regression; the pure-Chaos direction is intact. Worth telling the user explicitly so they stop chasing that thread.

## Evidence Checked
- `original_prompt.md` (full).
- `T66GameMode_TestRoom.cpp` lines 1-906 — wipeout cvars, `MakeWipeoutArmKnockbackProfile`, `ApplyWipeoutArmHeroImpact`, `ScheduleWipeoutArmTrap` (the full trap timer + hit detection, 639-711). PAC hard-off for heroes confirmed.
- `T66KnockbackComponent.cpp` (full, 1-1384) — launch, ragdoll, floor guard, floor-Z caching, recovery/restore, PAC disable logic.

## Questions Or Blockers
None requiring the user. All fixes are runtime-code changes Codex can make and verify.

## Caveats
- I did not read `T66GameMode_TestRoom.cpp` lines 907-1794, but the entire wipeout-arm path lives in 104-711, which I read in full.
- The "under the ground" root cause is the strongest hypothesis from static reading; on a flat floor it may only manifest once launch distance increases. Confirm against the `T66Knockback floor guard` / `actor follow` log lines during the staged standalone run before declaring it fixed.
- "Bounce off the wall" likely needs a longer ragdoll window, not just impulse — flag remaining feel-tuning (launch XY/Z, MaxRagdoll, SettleHold, cooldown) as explicit manual caveats per the stop condition.
- Stop condition still requires focused compile + staged standalone proof run; this validator answer is read-only and does not satisfy that.

</validator_independent_answer>

<codex_draft>
# Codex Draft: Wipeout Force Launch / Ground Snap

## Scope

Fix the current TestRoom wipeout-arm behavior where the hero could sink below the floor, appear to return near the original location, and fail to be launched far enough out of the trap radius. Keep the path pure C++ / pure Chaos for heroes; PAC must remain disabled for hero ragdoll.

## Changes

- `Source/T66/Gameplay/T66KnockbackComponent.h`
  - Added reusable `FT66KnockbackProfile` physics-response fields:
    - `RagdollLinearDampingOverride`
    - `RagdollAngularDampingOverride`
    - `RagdollFrictionOverride`
    - `RagdollRestitutionOverride`
  - Added runtime state for caching/restoring body damping and physical-material overrides during ragdoll.

- `Source/T66/Gameplay/T66KnockbackComponent.cpp`
  - Clamps the new profile overrides when a knockback starts.
  - Caches each skeletal physics body's pre-impact damping and physical material override.
  - Applies optional runtime damping and a transient `UPhysicalMaterial` override to all simulated ragdoll bodies.
  - Restores each body's previous damping/material settings on recovery.
  - Keeps the existing current-XY floor trace/floor guard and carry-velocity restore behavior.
  - Keeps hero PAC disabled; packaged proof logs show `PACPending=0 DriveMode=0`.

- `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp`
  - Increased wipeout-arm radial launch from the earlier stronger pass to `10500`.
  - Increased `WipeoutArmRagdollMaxSeconds` to `3.10` for a wall-contact window.
  - Added TestRoom cvars for ragdoll damping/friction/restitution:
    - `t66.TestRoom.WipeoutArmLinearDamping=0.01`
    - `t66.TestRoom.WipeoutArmAngularDamping=0.02`
    - `t66.TestRoom.WipeoutArmFriction=0.04`
    - `t66.TestRoom.WipeoutArmRestitution=0.72`
  - Wipeout impact log now reports damping/friction/restitution.

- `Source/T66/T66.Build.cs`
  - Added public `PhysicsCore` dependency because `UPhysicalMaterial` is now used in a public component header.

## Git/Tracking Caveat

`Source/T66/Gameplay/T66KnockbackComponent.cpp` and `.h` are currently untracked in this working tree. The focused build and staged standalone did compile them from disk, but they are not tracked by Git at the moment. I did not stage or change tracking state.

## Verification

- Focused build:
  - Command: `C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex`
  - Result: succeeded after adding `PhysicsCore`.

- Staged standalone refresh:
  - Command: `C:\UE\T66\Scripts\StageStandaloneBuild.ps1`
  - Result: `BUILD SUCCESSFUL`
  - Staged executable: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
  - Shortcut target verified: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`

- Packaged TestRoom capture:
  - Log: `C:\UE\T66\Saved\AgentReviews\WipeoutForceLaunchGroundSnap\testragdoll_force_launch_staged_v3.log`
  - Frames: `C:\UE\T66\Saved\AgentReviews\WipeoutForceLaunchGroundSnap\frames_staged_v3`
  - Frame count: 220 PNGs.
  - Launch log:
    - `Velocity=V(X=10280.41, Y=2136.15, Z=750.00)`
    - `MaxRagdoll=3.10s`
    - `LinearDamping=0.01 AngularDamping=0.02 Friction=0.04 Restitution=0.72`
    - `PACPending=0 DriveMode=0`
  - Impact log:
    - `Launch=V(X=10280.41, Y=2136.15, Z=750.00)`
    - `Radial=V(X=1.00) Tangent=V(X=0.18, Y=0.98)`
    - `PAC=0 DriveMode=0`
  - Actor follow samples:
    - Start: `Actor=V(X=850.11, Y=-0.13, Z=100.00)`
    - Mid launch: `Actor=V(X=7131.97, Y=1296.18, Z=100.00)`
    - Far wall/bounce area: `Actor=V(X=14731.09, Y=2881.40, Z=100.00)`
    - Post-contact/recovery area: `Actor=V(X=14373.04, Y=3367.68, Z=100.00)`
    - Every sampled line reports `ActorToTargetXY=0.0`.
  - Bounce evidence:
    - Actor moves out from `X=850.11` to `X=14731.09`, then returns to `X=14373.04`.
    - Restore carry velocity is negative on X/Y: `CarryVelocity=V(X=-625.44, Y=-701.46)`.
  - Floor guard evidence:
    - Initial floor correction: `BodyMinZ=-14.0 AllowedMinZ=4.0 Lift=18.0`.
    - Recovery floor correction: `BodyMinZ=-20.0 AllowedMinZ=4.0 Lift=24.0`.
    - Actor remains restored at `Z=100.00`.
  - Log check:
    - No `Fatal`, `Critical error`, `Assertion`, or `ensure` lines matched in the proof parse.

## Remaining Caveats

- The mechanics now launch far and bounce back, but the capture shows the wipeout arm can fill/obstruct the camera during the high-speed launch. That is a camera-obstruction/feel tuning problem, not the original actor-detach snap-back bug.
- The current forced ragdoll window is `3.10s` to prove wall contact and bounce. If the desired final play feel returns control faster, the next tuning pass should decide whether recovery should happen immediately after a confirmed wall/ground contact instead of on a fixed max timer.

## PPF Close

Process used: existing T66 C++ knockback/TestRoom trap path with pure Chaos hero ragdoll, profile-level physics response tuning, focused build, staged standalone refresh, and Unreal-owned packaged capture/log proof.

Matches declared process: YES

Evidence: build/stage/capture paths above; proof log shows PAC disabled, radial launch, zero actor-follow divergence, floor-guard corrections, and bounce-back carry velocity.

</codex_draft>
