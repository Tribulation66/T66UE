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
- Original prompt path: C:\UE\T66\Reports\AgentReviews\FallGuysPhysicsArchitectureAssessment\original_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\FallGuysPhysicsArchitectureAssessment\codex_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\FallGuysPhysicsArchitectureAssessment\Claude\20260606T104421-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
# Original User Request

Hey, so I've been working on implementing ragdoll physics in my game. The idea is I want it to feel just like Fall Guys. Like that's how I want the game to feel, just like Fall Guys and being driven by traps. Which traps, really what they're gonna be is basically the Fall Guys, you know, like the swinging arm, the bouncing things, stuff that tries to knock you out of the obstacles. That's the word. When I say traps, really what I'm referring to are obstacles. And I'm trying to get the exact feel of the exact same feel of Fall Guys. Okay, we don't have any of the same art, there's no copyright anything, don't worry about that. It's just about getting the feel of it, and then we'll tweak that to make, you know, the feel unique to our game. Now, so I started this yesterday, and I have, I think, some, you know, very basic systems in place. I had another agent that's working on this, but eventually I wanna move it to this agent because that agent's having some problems. So first I want you to go through, you and Claude, to go through what I have for physics in my game, okay, all the physics stuff. And two things, first, see exactly why it's not, why it doesn't feel, or it reads as if it wouldn't feel like Fall Guys, and what is the infrastructure solution for that. Okay, that's the first thing. The second thing is, look how our files are organized. I think we should create a physics folder and organize our files in a better way, having subfolders, archive folder, you know, different stuff, so that when future agents need to work with like a readme, so that when future agents need to work with our physics, they can get the information they need much faster. So it's these two things. First is an assessment of what we have, the systems we have in place, and why, because it currently does not feel like, it does not feel like Fall Guys at all, how to make it feel like Fall Guys. And the second is your and Claude's suggestion for file organization and restructure.

# Task Contract

Working task:
Operator: Codex
Validator: Claude
Scope: Read-only assessment of current T66 physics/ragdoll/obstacle systems, why they would not match a Fall Guys-style feel, and proposed physics folder/readme/archive organization. No code/content moves or implementation in this pass.
Stop condition: Deliver a repo-grounded assessment with Claude cross-check input, current evidence, caveats, and recommended next implementation phases.

# Repo Rules To Apply

- Start from live repo state.
- Follow AGENTS.md and OPERATOR_VALIDATOR_PROTOCOL.md.
- Current role state is Codex Operator / Claude Validator from `.t66/operator-state.json`.
- Treat this as read-only planning/architecture assessment.
- Do not mutate production files, assets, Git state, or Unreal assets.
- Report evidence paths and verification skipped.

# Relevant Current Evidence Already Found By Codex

- `Gameplay/GAMEPLAY_AGENTS.md` owns gameplay movement/traps.
- `Gameplay/README.md` lists Movement and Traps as separate docs; no dedicated Physics owner exists yet.
- `Source/T66/Gameplay/pending_issues_Gameplay.md` says current hero direction is pure Chaos ragdoll, PAC defaults off, TestRoom wipeout-arm PAC CVars default disabled, and subjective feel remains untuned.
- Key files found so far:
  - `Source/T66/Gameplay/T66KnockbackComponent.h`
  - `Source/T66/Gameplay/T66KnockbackComponent.cpp`
  - `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp`
  - `Source/T66Editor/T66CreateTestRoomPhysicsAssetCommandlet.cpp`
  - `Gameplay/Movement/MASTER_MOVEMENT.md`
  - `Gameplay/Traps/MASTER_TRAPS.md`
  - `Source/T66/Core/T66TrapSubsystem.*`
  - `Source/T66/Gameplay/Traps/*`

# Requested Validator Output

Please inspect the repo read-only and provide:

1. Your independent assessment of what physics/ragdoll/obstacle infrastructure currently exists.
2. Why it likely does not feel like Fall Guys yet.
3. The infrastructure solution, not just tuning values.
4. A proposed file/folder/doc organization for a dedicated physics area, including archive/readme guidance.
5. Any important caveats, missed files, or user decisions that would be required before implementation.

End with `Result: OK` if Codex can answer internally, or `Result: NEEDS_USER` only if the user must decide before even delivering this assessment.

</original_prompt>

<validator_independent_answer>
Result: OK

## Independent Answer

**1. What physics/ragdoll/obstacle infrastructure exists today**

- **Knockback/ragdoll core**: `Source/T66/Gameplay/T66KnockbackComponent.{h,cpp}` is the only real ragdoll system. It is a discrete, three-phase state machine (`Inactive → Active → Recovering`) driven by `ApplyKnockbackLaunch(LaunchVelocity)`. On impact it switches the skeletal mesh to Chaos ragdoll, applies a one-shot mass-scaled impulse, runs a settle/floor-guard loop, then blends back to the animated pose. It is **hero-only** by design (`ET66KnockbackBudgetClass` exists for Boss/Elite/Horde but the path is the first hero pass).
- **Physical Animation (PAC)**: fully plumbed in the same component (`FT66KnockbackProfile` PAC fields, drive modes, `GetOrCreatePhysicalAnimationComponent`) but **defaulted OFF** and runtime-force-disabled for hero/detached ragdoll per the 2026-06-06 policy decision (`pending_issues_Gameplay.md`).
- **Obstacles ("traps")**: `Source/T66/Gameplay/Traps/*` (`T66TrapBase`, FloorFlame, FloorSpikePatch, WallArrow, PressurePlate, ArrowProjectile) plus `Source/T66/Core/T66TrapSubsystem.*`. These are **damage-volume / trigger actors**, not physical bodies. `T66TrapBase` has activation modes and damage flags but no momentum/collision-impulse contract.
- **The only "Fall Guys-like" surface**: the TestRoom center-pivot **wipeout arm**, defined entirely as `t66.TestRoom.WipeoutArm*` CVars in `T66GameMode_TestRoom.cpp`, defaulted disabled. It calls into the hero knockback component. There is also an editor commandlet `T66CreateTestRoomPhysicsAssetCommandlet` to author the Chad PhysicsAsset.
- **PhysicsAssets** exist for Companion idle/walk and demo mannequins, plus FriendSlop ragdoll import artifacts under `Reports/AgentReviews/`.
- Locomotion is standard `UCharacterMovementComponent` (capsule), not physics-driven.

**2. Why it will not feel like Fall Guys**

Fall Guys feel comes from a **continuously physically-present body**: a capsule whose mesh is always driven by physical animation toward a target pose, so every contact perturbs a wobbly, momentum-carrying body, and obstacles impart force through *persistent collision*, not scripted events. T66's model is the opposite on three axes:

- **Ragdoll is an exceptional state, not the default.** The hero is a kinematic CMC capsule that *switches into* ragdoll on a hit and blends back out. Between hits there is zero physical squishiness. Fall Guys is "always slightly ragdoll."
- **PAC is off.** Pure Chaos ragdoll with PAC disabled gives a floppy dead-body flop, not the spring-loaded, self-righting, comedic struggle that reads as Fall Guys. The active-ragdoll "trying to stand" layer is exactly what's disabled.
- **Obstacles push via one-shot impulse, not continuous contact.** A Fall Guys swinging arm shoves a physics body through sustained collision; here it fires a single `LaunchVelocity` into a state machine. No persistent contact, no body-to-body shoving between players/enemies, no rolling/sliding momentum.
- Plus: hero-only, TestRoom-only, CVar-gated, explicitly untuned.

**3. Infrastructure solution (not tuning)**

The fix is an architecture shift, not new launch numbers:

- **Adopt a persistent active-ragdoll / physical-animation locomotion model** as the standing state: capsule + always-on `PhysicalAnimationComponent` driving the mesh toward the animated pose, so the body is physically present every frame and collisions continuously perturb it. This means **reopening the 2026-06-06 "pure Chaos, PAC off" hero decision** — that decision and Fall Guys feel are mutually exclusive.
- **Make obstacles physics impulse sources, not damage volumes.** Give `T66TrapBase` (or a new physics-trap contract) a real collision/impulse interface so swinging/bouncing geometry imparts momentum through contact rather than firing a scripted launch.
- **Unify into one shared physics-actor state** (hero, companions, enemies) instead of the hero-only knockback component, so body-to-body shoving and consistent recovery behavior exist across actors. The existing `ET66KnockbackBudgetClass` enum already anticipates this.
- Keep the discrete hard-knockback path as a *layer on top* of the continuous model (big hits still launch), but the baseline must be continuously physical.

**4. Proposed file/folder/doc organization**

There is currently **no Physics owner** — `Gameplay/README.md` lists Movement and Traps separately, and ragdoll logic is orphaned inside `Gameplay/`. Recommend a dedicated doc area mirroring the existing pattern (`Gameplay/Movement/MASTER_MOVEMENT.md`, `Gameplay/Traps/MASTER_TRAPS.md`):

- `Gameplay/Physics/`
  - `MASTER_PHYSICS.md` — owner doc: the active-ragdoll model, the hero PAC policy and its history, the knockback state machine, obstacle-impulse contract, CVar index, budget classes.
  - `README.md` — fast-orientation map for future agents: "where is X" pointers into `Source/T66/Gameplay/T66KnockbackComponent.*`, `Source/T66/Gameplay/Traps/*`, `Source/T66/Core/T66TrapSubsystem.*`, the TestRoom wipeout-arm CVars, and the PhysicsAsset commandlet.
  - `Archive/` — superseded experiments (PAC-stabilization proofs, FriendSlop import reassessments now scattered under `Reports/AgentReviews/`) moved or linked here with a one-line "superseded by" note each.
- Add a `- [Physics]` line to `Gameplay/README.md` Areas list, and add a Physics owner section to `Gameplay/GAMEPLAY_AGENTS.md` so ownership is explicit.
- **Leave C++ source paths where they are** for this pass (moving `.cpp`/`.h` touches includes/UHT/generated headers and is a code move, which is out of scope). The doc area is what makes future agents fast; the code can be reorganized in a later explicit refactor pass.

**5. Caveats / user decisions required before implementation**

- This assessment is deliverable now read-only; nothing blocks delivery.
- **The one real user decision is whether to reopen the hero physics architecture.** `pending_issues_Gameplay.md` records an explicit, recent (2026-06-06) decision that hero ragdoll is pure Chaos with PAC off, and says "PAC should not be tuned for hero ragdoll unless the hero physics architecture is explicitly reopened." Fall Guys feel *requires* reopening it. Codex should surface this trade-off but the user owns the call.

## Evidence Checked

- `.t66/operator-state.json` (Codex Operator / Claude Validator confirmed)
- `Source/T66/Gameplay/T66KnockbackComponent.h` (full profile + state machine read)
- `Source/T66/Gameplay/pending_issues_Gameplay.md` (hero pure-Chaos/PAC-off policy, wipeout-arm capture state)
- `Source/T66/Gameplay/Traps/T66TrapBase.h` and `Traps/*` listing
- `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp` (wipeout-arm CVar surface)
- `Gameplay/README.md`, `Gameplay/Movement/`, `Gameplay/Traps/` doc layout
- Physics asset / commandlet inventory across `Content/`, `Source/T66Editor/`, `Reports/AgentReviews/`

## Questions Or Blockers

None that block delivering the assessment. One decision to flag inside the deliverable (not a NEEDS_USER gate): does the user want to reopen the hero pure-Chaos/PAC-off architecture decision, which is the prerequisite for any real Fall Guys feel?

## Caveats

- I did not read `T66KnockbackComponent.cpp` body or `T66TrapSubsystem.cpp` in full — header/contract level only — so impulse-math specifics are inferred from the header and CVar surface, not line-verified.
- A prior Codex draft exists at `Saved/AgentReviews/fall_guys_physics_consultation_codex_draft.md`; I did not read it to keep this cross-check independent.
- Recommendation that source files stay put is deliberate: the prompt is read-only/no-moves, so folder restructure should ship as doc scaffolding now and a separate code-move pass later.
- No build/compile verification was run (read-only pass); architecture claims are from source inspection, not runtime proof.

</validator_independent_answer>

<codex_draft>
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

</codex_draft>
