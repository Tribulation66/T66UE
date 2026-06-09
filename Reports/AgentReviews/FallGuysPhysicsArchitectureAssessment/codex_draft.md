# Fall Guys-Style Physics Architecture Assessment - Codex Draft

## Task Contract

Working task:
Operator: Codex
Validator: Claude
Scope: Read-only assessment of current T66 physics/ragdoll/obstacle systems, why the current setup would not produce Fall Guys-like feel, and proposed physics folder/file organization.
Stop condition: Deliver repo-grounded assessment and reorganization recommendation; no implementation or file moves in this pass.

## Current Physics / Ragdoll / Obstacle Inventory

### Hero ragdoll and knockback

- `Source/T66/Gameplay/T66KnockbackComponent.{h,cpp}` is the central hero ragdoll path.
- `FT66KnockbackProfile` defaults to skeletal ragdoll, pure Chaos simulation, detached mesh, actor follow to simulated body center, floor guard, mass-scaled launch impulse, timed/settled recovery, and short component defaults:
  - `MinIncapacitationSeconds=0.15`
  - `MaxRagdollSeconds=0.40`
  - `SettleSpeed=165`
  - `SettleHoldSeconds=0.25`
  - `RecoveryBlendOutSeconds=0.10`
- Physical Animation Component support still exists in the component, but hero/detached ragdoll paths force PAC disabled. `pending_issues_Gameplay.md` records the policy: hero ragdoll/knockback is currently pure Chaos, and PAC should not be tuned for hero ragdoll unless hero physics architecture is explicitly reopened.
- The fallback if no usable skeletal mesh/PhysicsAsset exists is still `LaunchCharacter(..., true, true)`.

### TestRoom wipeout-arm prototype

- `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp` contains the only current Fall Guys-like obstacle prototype.
- It is enabled by default through `t66.TestRoom.EnableWipeoutArmTrap=1`.
- Current live TestRoom tuning differs from the component defaults:
  - `WipeoutArmLaunchXY=7800`
  - `WipeoutArmLaunchZ=750`
  - `WipeoutArmIncapSeconds=0.15`
  - `WipeoutArmRagdollMaxSeconds=2.35`
  - `WipeoutArmRagdollSettleHoldSeconds=0.12`
  - `WipeoutArmRagdollBlendOutSeconds=0.15`
- The arm is implemented inline in TestRoom game mode as timer-driven static mesh rotation plus a capsule-vs-cylinder hit check. On hit it computes radial plus tangential launch direction and calls `UT66KnockbackComponent::ApplyKnockbackLaunch`.
- It does not derive from `AT66TrapBase`, does not register with `UT66TrapSubsystem`, and is not yet a reusable obstacle actor.

### Production traps

- `Gameplay/Traps/MASTER_TRAPS.md`, `Source/T66/Gameplay/Traps/*`, and `Source/T66/Core/T66TrapSubsystem.*` define the production trap system.
- Current trap families are damage/trigger/cadence actors: wall arrows, floor flames, floor spikes, pressure plates, area control.
- `AT66TrapBase` owns activation mode, trigger target, damage, cadence, and safe-zone/floor filtering. It has no physics impulse/reaction profile.
- `T66TrapDamageUtils` routes trap damage to run-state or enemy damage. It does not apply obstacle-style physical reaction.
- `Config/DefaultT66TrapTuning.ini` stores visual/tower/damage/cadence data, not obstacle physics data.

### Other launch paths

- Movement and non-ragdoll hits still use direct `LaunchCharacter` paths:
  - roll/dash in `T66HeroMovementComponent`
  - enemy-touch bounce in `T66HeroBase`
  - shroom stage effects in `T66StageEffects`
- Stage launch objects are currently disabled (`T66AreStageLaunchObjectsEnabled()` returns false), so the shroom launcher is not an active runtime obstacle surface.

### PhysicsAsset tooling

- `Source/T66Editor/T66CreateTestRoomPhysicsAssetCommandlet.cpp` is a TestRoom-specific FriendSlop/Chad PhysicsAsset generator.
- It builds a controlled humanoid ragdoll asset with an allowed bone/body graph and writes a JSON report.
- Current proof artifacts show an 18-body / 17-constraint controlled physics asset for the TestRoom skeletal Chad path, but this is not yet generalized into a per-character production validation pipeline.

## Why The Current Setup Does Not Read As Fall Guys-Like

1. The baseline character is still a kinematic CharacterMovement capsule, not a continuously physically perturbed body.
   - The current model becomes physical only after a qualifying hit.
   - That produces "normal character, then hard ragdoll" rather than constant unstable, jelly-like obstacle contact.

2. Obstacle contact is event-driven, not contact-driven.
   - The wipeout arm currently detects overlap and injects one launch vector.
   - It does not own a persistent collision/impulse/contact contract: no sustained shove, no sliding along the arm, no per-body collision accumulation, no repeated contact semantics, no rebound profile.

3. Production traps are not obstacle physics actors.
   - The shipped trap framework is built around damage, activation, and spawn/progression data.
   - It cannot express spinner/flipper/bumper/conveyor/fan/seesaw behavior as first-class physical obstacle interactions.

4. Recovery is global/prototype-oriented instead of obstacle-authored.
   - Component defaults are short and snap-control-oriented.
   - TestRoom wipeout-arm tuning is different and longer.
   - Fall Guys-like feel needs per-obstacle/per-hit reaction profiles: tumble time, air control suppression, recovery blend, settle threshold, rebound, and repeated-hit grace should be data-authored by obstacle type.

5. The current PAC-off policy is coherent with the recent pure-Chaos direction but removes the active self-righting role that Fall Guys-like feel usually needs.
   - This does not mean "turn PAC back on blindly."
   - It means the architecture must explicitly choose how to provide controlled self-righting and springy recovery: a reopened PAC/active-ragdoll profile, or a pure-Chaos recovery layer that proves the same role without dragging the body to a fake target.

6. File ownership is fragmented.
   - Ragdoll lives in root gameplay source.
   - Obstacle prototype logic lives inside TestRoom game mode.
   - Production traps live under `Gameplay/Traps`.
   - PhysicsAsset authoring lives under `T66Editor`.
   - Historical proof/policy lives under `Reports/AgentReviews`.
   - `Gameplay/README.md` has Movement and Traps but no Physics owner.

## Infrastructure Recommendation

### Architecture

1. Create a dedicated physics/obstacle reaction model before more tuning.
   - Add a `FT66ObstacleImpulseProfile` or equivalent data row.
   - Include hit direction source, impulse mode, horizontal/vertical force, sustained contact vs one-shot, re-hit cooldown, air-control lockout, ragdoll/recovery profile, floor/wall behavior, and optional camera-follow behavior.

2. Extract the wipeout arm out of TestRoom game mode into a reusable obstacle actor.
   - Suggested path: `AT66PhysicsObstacleBase` plus `AT66RotatingArmObstacle`.
   - It can initially preserve the current wipeout-arm behavior, but the ownership should move from game-mode inline prototype to obstacle class plus data profile.

3. Extend or sibling the trap subsystem for physical obstacles.
   - Do not overload damage traps with physics until the contract is clear.
   - Either add a physics-obstacle registration path to `UT66TrapSubsystem`, or add a separate `UT66ObstaclePhysicsSubsystem` that `UT66TrapSubsystem` can spawn/register.

4. Split reaction state from hero-only knockback naming over time.
   - Keep `UT66KnockbackComponent` stable for now.
   - Later split types into a physics reaction module: profile types, ragdoll state machine, obstacle impulse utilities, recovery profile helpers.

5. Reopen the PAC/pure-Chaos decision only as an explicit architecture decision.
   - Current policy says heroes should not tune PAC for ragdoll.
   - If the target is "exact Fall Guys feel," the missing role is continuous self-righting/active recovery.
   - The implementation decision should be phrased as: "How will T66 provide active recovery without killing the free Chaos tumble?" not merely "enable PAC."

### Folder / Document Organization

Start with doc ownership before moving C++ files:

```text
Gameplay/
  Physics/
    PHYSICS_AGENTS.md
    README.md
    MASTER_PHYSICS.md
    ObstaclePhysicsModel.md
    RagdollRecoveryProfiles.md
    PhysicsAssetPipeline.md
    Archive/
      README.md
```

Recommended responsibilities:

- `PHYSICS_AGENTS.md`: router for physics/ragdoll/obstacle-reaction work; points to Gameplay rules, staged verification rules, and physics proof requirements.
- `README.md`: fast map for future agents: current source files, CVars, proof artifacts, known policy decisions, and "where to start."
- `MASTER_PHYSICS.md`: current architecture and target architecture, including pure-Chaos/PAC policy and known caveats.
- `ObstaclePhysicsModel.md`: obstacle taxonomy and contact semantics for spinner, bumper, flipper, launcher, conveyor, fan, seesaw, etc.
- `RagdollRecoveryProfiles.md`: profile fields, timing semantics, air-control behavior, control restore rules.
- `PhysicsAssetPipeline.md`: commandlet path, body/constraint requirements, validation reports, and per-character acceptance gates.
- `Archive/README.md`: index of older reports/proofs with current/superseded status. Prefer linking reports first; physically moving historical report folders should be a separate cleanup pass.

Later source layout, after an explicit refactor pass:

```text
Source/T66/Gameplay/Physics/
  T66PhysicsReactionTypes.h
  T66KnockbackComponent.h/.cpp          (or renamed only after references are stable)
  T66ObstacleImpulseTypes.h
  T66ObstaclePhysicsComponent.h/.cpp
  T66RagdollRecoveryTypes.h

Source/T66/Gameplay/Obstacles/
  T66PhysicsObstacleBase.h/.cpp
  T66RotatingArmObstacle.h/.cpp
  T66BumperObstacle.h/.cpp

Source/T66Editor/Physics/
  T66CreateHumanoidPhysicsAssetCommandlet.h/.cpp
  T66ValidateHumanoidPhysicsAssetCommandlet.h/.cpp
```

Do not bulk-move C++ in the first organization pass. Unreal include paths, generated headers, module dependencies, and concurrent agent work make that risky. First pass should establish docs, ownership, and indexes; second pass should extract the TestRoom obstacle into reusable code; third pass can reorganize source once behavior is stable.

## Proposed Implementation Phases

1. Documentation/ownership pass only.
   - Add `Gameplay/Physics` docs and route `Gameplay/README.md` / `GAMEPLAY_AGENTS.md`.
   - Index current source files and proof reports.

2. Wipeout-arm extraction pass.
   - Move behavior from `T66GameMode_TestRoom.cpp` into a reusable obstacle actor while preserving behavior.
   - Keep TestRoom as a spawner/configurator.

3. Obstacle reaction profile pass.
   - Introduce data-authored physical reaction profiles.
   - Route TestRoom arm through the same profile path future obstacles will use.

4. Production obstacle integration pass.
   - Register physics obstacles with the trap/progression system or a sibling subsystem.
   - Add spinner/bumper/flipper archetypes as data-authored obstacle types.

5. Character physics asset validation pass.
   - Generalize the current TestRoom commandlet/report into a per-character validator.
   - Require body/constraint count, pelvis/spine chain, damping/mass settings, and capture proof before declaring production-ready.

6. Feel-proof pass.
   - Define Fall Guys-like reference mechanisms as observed/inferred/tuned, not copied values.
   - Use Unreal-owned multi-frame capture to verify sustained contact, launch, tumble, rebound, recovery, and repeated-hit behavior.

## Anti-Lookalike Gate

Cheapest wrong result: a bigger one-shot `LaunchCharacter` or `ApplyKnockbackLaunch` value that throws the hero farther and looks dramatic in one frame.

Discriminator: multi-frame proof that the obstacle physically pushes or perturbs the character over contact time, the body tumbles/rebounds from world contact, recovery timing is obstacle-authored, and player control returns without a visible snap. A still frame or single impact log is not enough.

## Verification Performed

- Read-only source and doc inspection only.
- No compile, staged build, or capture was run because this pass makes no gameplay/code changes.
- Claude independent-answer validator pass ran successfully and largely agreed with the architecture diagnosis. One stale claim from Claude was corrected here: live source has `t66.TestRoom.EnableWipeoutArmTrap=1`, not disabled.
