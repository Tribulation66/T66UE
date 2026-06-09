# Fall Guys Full Physics Roadmap Review - Codex Draft

## Opinion On The Attached Proposal

The attached Claude proposal is the right architecture class for the target feel: capsule-backed, always-on active ragdoll for heroes.

The important distinction is that this is not the earlier "hybrid knockback" model. The current T66 model is:

- clean `CharacterMovement` locomotion most of the time;
- switch into detached Chaos ragdoll on hit;
- apply one major impulse;
- settle or time out;
- blend back to animation/control.

The attached proposal is a different runtime contract:

- the capsule remains the authoritative gameplay mover;
- the skeletal mesh is always simulated;
- a pose drive chases locomotion animation for wobble;
- a world-space hip/pelvis anchor constraint keeps the hero upright and dragged along by the capsule;
- impacts ramp down the anchor/drive instead of switching into a new ragdoll mode.

That method class better matches the requested "always bouncy, trying to keep balance, knocked down and gets up" feel.

## Corrections / Caveats

1. This is a deliberate pivot from the active repo policy.
   - `Source/T66/Gameplay/pending_issues_Gameplay.md` records the current hero direction as pure Chaos ragdoll with PAC off.
   - It explicitly says PAC should not be tuned for hero ragdoll unless the hero physics architecture is reopened.
   - The attached proposal is that reopening. It should be approved and documented as a new architecture direction before implementation.

2. The current `UT66KnockbackComponent` PAC plumbing is not the final framework.
   - `FT66KnockbackProfile` defaults PAC off and drive mode disabled.
   - `ApplyKnockbackLaunch` force-disables PAC for hero profiles and detached ragdoll profiles.
   - Existing `ApplyPhysicalAnimationDrive` sets `FPhysicalAnimationData::bIsLocalSimulation=false`, while the attached proposal depends on local simulation for pose wobble.
   - So the current component is useful reference code for body setup, impulses, floor guard, recovery logging, and proof scaffolding, but the always-on framework should be new or split out, not a reconfiguration of the old flow.

3. The "Fall Guys exact feel" cannot mean copying hidden/proprietary values.
   - Treat the target mechanisms as observed/inferred/tuned: persistent wobble, capsule-driven movement, physics-body perturbation, obstacle impulse, loosened control on hard hits, airborne repositioning, fast recovery, and repeatable proof captures.
   - Fall Guys' public update notes reinforce that ragdoll/recovery, jump/acceleration, mid-air control, launchers/flippers, and obstacle changes are coupled player-feel systems.

4. The performance claim needs proof.
   - The attached proposal says 1-4 hero active ragdolls should be fine. That is plausible, but unproven in T66.
   - T66 needs its own perf budget gate before this becomes production default.

## Recommended Architecture

Create a broad `Physics` layer, not a physics/obstacle-only layer:

- `UT66HeroPhysicsComponent`: always-on hero physical animation / active ragdoll runtime.
- `FT66HeroPhysicsProfile`: data-authored profile for pose drive, hip anchor, damping, restitution, sleep/CCD, recovery ramps, and input lock rules.
- `UT66PhysicsReactionComponent` or subsystem helpers: shared impulse/reaction contract used by heroes first, later monsters and obstacles.
- `FT66PhysicsReactionProfile`: hit/launch/contact reaction profile, with direction source, impulse strength, force-limit modifiers, anchor/drive loosen amount, recovery ramp, and air-control rules.
- `UT66PhysicsSubsystem`: registration, debug state, shared tuning, perf counters, and future support for mobs/obstacles.
- `Gameplay/Physics` docs as the owner layer.

Keep production traps separate at first. Traps can later call into Physics through reaction profiles, but the Physics layer should own the feel contract.

## Implementation Roadmap

### Phase 0 - Architecture Ownership And Decision Record

Goal: establish the physics direction before touching runtime behavior.

Work:

- Add `Gameplay/Physics/`.
- Add `PHYSICS_AGENTS.md`, `README.md`, `MASTER_PHYSICS.md`, `HeroPhysicsModel.md`, `PhysicsReactionProfiles.md`, `PhysicsAssetPipeline.md`, and `Archive/README.md`.
- Update `Gameplay/README.md` and `Gameplay/GAMEPLAY_AGENTS.md` to route Physics.
- Record that hero physics architecture is reopened and that the new target is capsule-backed always-on active ragdoll for heroes.
- Define coexistence: old `UT66KnockbackComponent` remains a fallback/prototype path until the new framework proves itself.

Proof:

- Documentation only; no gameplay proof required beyond review.

### Phase 1 - Single-Hero Always-On Active Ragdoll MVP

Goal: one hero can run around with constant wobble while still using capsule movement.

Work:

- Create a new hero physics component rather than mutating `UT66KnockbackComponent` in place.
- Keep `UCharacterMovementComponent` and capsule as the authoritative mover.
- Keep the skeletal mesh simulated below pelvis at blend weight 1.
- Add/use `UPhysicalAnimationComponent`.
- Use local pose drive (`bIsLocalSimulation=true`) with finite orientation/angular force.
- Add a kinematic hip/pelvis anchor component riding on the capsule at hip height.
- Add a `UPhysicsConstraintComponent` between the hip anchor and pelvis body with linear drive and weak angular stability.
- Drive animation pose/play rate from capsule speed, ideally through a minimal locomotion blendspace once the MVP proves the path.

Acceptance:

- In TestRoom, the hero visibly wobbles while idle/walking/running without needing to be hit.
- Capsule movement remains controllable.
- Mesh does not detach, freeze, sleep, tunnel, or collapse under normal walking.

### Phase 2 - Knockdown, Loosen, Get-Up

Goal: hard impacts become continuous changes in drive strength, not state switches.

Work:

- Add hit reaction that applies body impulse at hit location.
- Add anchor loosen profile: reduce hip-anchor stiffness/force limit and PAC strength for hard hits.
- Add recovery profile: when velocity/angle/contact state qualifies, ramp anchor and pose drive back over a tunable window.
- Move capsule under the settled pelvis only during recovery handoff if needed, not as a normal snap.
- Define input/attack suppression by physics state: balanced, staggered, knocked down, recovering.

Acceptance:

- One sweep-arm hit knocks the hero down or off balance without a mesh/capsule pop.
- Recovery is visible as physical pull-up, not instant animation reset.
- Multi-frame capture proves tumble, rebound, recovery ramp, and restored input.

### Phase 3 - Physics Reaction Profiles

Goal: make "how things hit bodies" data-authored and shared.

Work:

- Add `FT66PhysicsReactionProfile`.
- Include contact source, impulse type, force magnitude, radial/tangent blend, anchor loosen amount, pose-drive loosen amount, air-control suppression, recovery profile, repeat-hit cooldown, and camera behavior.
- Convert TestRoom wipeout arm to call the new reaction API.
- Preserve current TestRoom CVar tuning as seed profile values only, not as final source of truth.

Acceptance:

- Same wipeout arm can produce at least two authored reactions: stagger and full knockdown.
- Logs/captures show profile ID and resulting state.

### Phase 4 - Solver / Asset / Stability Pass

Goal: make always-on physics stable enough to tune.

Work:

- Generalize the current TestRoom PhysicsAsset commandlet into a hero physics asset pipeline.
- Validate pelvis/spine chain, body count, constraint count, mass distribution, joint limits, damping, restitution, CCD, and sleep behavior.
- Add project/profile defaults for Chaos settings needed by always-on hero physics.
- Add Chaos Visual Debugger or equivalent debug hooks where useful.

Acceptance:

- Validator report for the active hero PhysicsAsset.
- No obvious joint inversion, stretch, buzzing, or sleep-freeze in TestRoom.
- Focused build and staged standalone smoke once runtime code changes land.

### Phase 5 - Obstacle And Environment Integration

Goal: obstacles become physics feel content, not just damage traps.

Work:

- Extract wipeout arm out of TestRoom game mode into reusable physics obstacle actor code.
- Add obstacle actors for rotating arms, bumpers, flippers, launch pads, fans, conveyors, seesaws.
- Let traps/obstacles reference physics reaction profiles.
- Decide whether `UT66TrapSubsystem` spawns them directly or delegates to `UT66PhysicsSubsystem`.

Acceptance:

- At least one rotating arm, one bumper/flipper, and one launcher use the same hero reaction framework.
- Captures prove sustained contact or force over time where required, not only one-shot launch.

### Phase 6 - Feel Proof Matrix

Goal: prove the framework against the target mechanisms.

Mechanisms to prove:

- always-on wobble while moving;
- player can steer while unstable;
- obstacle hit perturbs the body continuously;
- hard hit can knock down;
- wall/floor rebound reads as bouncy;
- airborne control exists but is limited;
- recovery is fast and physical;
- repeated hits do not create broken state;
- camera follows the playable body without snapping.

Anti-lookalike:

- A stronger one-shot launch is not enough.
- A passive ragdoll that only activates on impact is not enough.
- A PAC drive with no world anchor is not enough.
- A rigid capsule with animated wobble is not enough.

Acceptance:

- Unreal-owned video/capture proof, frame evidence, and logs for each mechanism.

### Phase 7 - Monsters / Mobs Variant

Goal: extend chaos selectively without paying hero-level cost on hordes.

Work:

- Keep the hero framework high fidelity.
- For bosses/elites, consider limited physics reaction or partial active ragdoll.
- For horde mobs, use cheaper fake wobble, impulse animation, or lightweight physics proxies.
- Make all three tiers use the same reaction vocabulary, even if implementation differs.

Acceptance:

- Monster physics does not compromise horde performance.
- Hero feel remains the gold standard.

## Recommended First Implementation Slice

The first implementation pass should not try to build the whole roadmap. It should do exactly this:

1. Create `Gameplay/Physics` docs and decision record.
2. Add the new hero active-ragdoll component skeleton and profile types behind a CVar/off-by-default gate.
3. Prove one hero in TestRoom with:
   - always simulated mesh;
   - PAC local pose drive;
   - hip-anchor physics constraint;
   - capsule movement still authoritative;
   - one sweep-arm hit that loosens anchor/drive and recovers.
4. Capture TestRoom video and logs.

Do not migrate production traps, monsters, or all heroes in the first slice.

## Final Recommendation

Approve the attached proposal's method class, with the wording changed from "keep using the existing ragdoll" to "build a new always-on hero physics framework."

The current TestRoom ragdoll work is useful as evidence and scaffolding, but it is not the destination. The destination is broad `Physics` ownership plus a capsule-backed active-ragdoll hero runtime.

Implementation should not begin until the next prompt explicitly approves reopening hero physics architecture away from pure-Chaos/PAC-off policy.

## Verification Performed For This Planning Pass

- Read attached proposal.
- Read live T66 root and gameplay instructions.
- Read live operator/validator state.
- Read live current hero PAC/pure-Chaos pending issue.
- Read live `T66KnockbackComponent` header and implementation excerpts.
- Read live TestRoom wipeout arm source.
- Read live trap subsystem/base contracts.
- Read live TestRoom PhysicsAsset commandlet and generated report.
- Ran Claude independent answer pass before this draft.
- No build, staged run, or capture was run because the user requested analysis/planning only.
