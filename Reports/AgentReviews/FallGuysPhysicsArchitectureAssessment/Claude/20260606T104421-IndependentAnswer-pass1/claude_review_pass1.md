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
