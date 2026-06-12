Verdict: REVISE

Blockers
- Boss `Attacker` pointer safety is unspecified. The manager will hold a raw `AT66BossBase*` to pass into `RunState->ApplyDamage` for projectiles with up to 6s lifetime. If a boss is destroyed or torn down mid-flight (death, level transition, subsystem teardown, MultiBoss partial wipe), every in-flight projectile dereferences a stale pointer. Required: weak-pointer storage plus an explicit "if attacker invalid, drop or attribute to last-known `BossID` only" rule, and a stated test for it in the smoke.
- HISM visual-key bucket cardinality is unbounded. Profiles × quantized-byte-color (primary or secondary) can grow large, and the plan doesn't cap component count, doesn't say what happens on bucket-creation failure, and doesn't say whether existing buckets are reclaimed. Specify max bucket count, eviction rule, and overflow fallback before implementation.
- Manager-owned Niagara trail component lifecycle on slot reuse is not defined. Slots are reused; trail components attached to slot indices must be reset between owners, not carried over. Required: explicit "on slot deactivate, detach+destroy trail before reuse" rule, plus subsystem-teardown cleanup.
- MultiBoss smoke procedure says it "directly spawns or initializes the four Stage 17 boss IDs." This is too vague and risks validating a fake worst case (e.g., bypassing spawn rules, awakening conditions, encounter wiring) rather than the production Four Horsemen path. Specify the exact entry point used (existing Stage 17 encounter trigger vs. new spawn helper) and confirm it exercises the same `Awaken`/`FireAtPlayer` path as production.

Major Issues
- Movement parity isn't fully enumerated. Old actor: zero gravity, `bRotationFollowsVelocity=true`, `InitialSpeed=MaxSpeed`, `InitialLifeSpan=6.0`. The plan covers speed and lifetime but does not state how HISM instance rotation will be updated each tick to match velocity, nor verifies the manager's existing tick is doing this for the enemy-spit type already. Add an explicit parity checklist for Task 1/Task 2 and a screenshot/log check for orientation.
- Capacity is chosen by back-of-envelope, then measured in the same smoke. Either size it generously enough that 512 is provably safe (state the worst sustained concurrent slot count, not just peak burst) or instrument first and tune after. Also missing: behavior when capacity is exceeded (drop vs. evict oldest), which matters for boss patterns where dropping mid-fan produces an obvious visual gap.
- Collision-semantics change is documented as intentional, but the second-order effect of boss shots no longer self-destroying on enemy/peer bodies in narrow arenas (potential stuck/overshoot through crowd into hero) is not analyzed. Add a smoke check that boss projectiles travel through mob crowd without unintended hero punch-throughs.
- "Force-awaken" automation can mask real awakening conditions and pattern timing. State whether `Awaken()` is called via the same hook production uses, and whether `FireIntervalSeconds` data path is loaded identically.
- BossPartProfile is noted in data but Task scope says lane-blocker/multipart redesign is out of scope. Confirm explicitly that Sewer Slime King's lobe/mouth/base routing is rerouted into manager fires for lobe/mouth (in scope) and left untouched for base lane blockers (out of scope), with a per-call-site mapping table.

Minor Issues
- Deprecation is only a comment. Add `UE_DEPRECATED` macro on the class/constructor so any stray reference triggers a compile warning, not just a doc note.
- The two pending_issues file edits (Gameplay vs Combat) aren't differentiated by what content goes where. Specify before editing.
- Static search command should also catch Blueprint/asset references to `AT66BossProjectile` (e.g., references via `T66BossProjectile_*` BP class names in cooked data) — not just C++ `SpawnActor`.
- `FT66ManagedProjectileFireParams` is presented as an "or", not a decision. Pick one before implementing to avoid mid-pass API churn.

Clarifying Questions
- Is `T66GameplayAutoCapture` an existing pluggable mode list, and is `bossprojectilemanager` a new mode to be registered there? Where is the registry?
- Is there an existing Stage 17 Four Horsemen spawn/encounter entry point we should drive, or does this pass introduce a new test-only spawner?
- Should boss fire audio remain on the boss actor (likely yes, since pattern logic stays there) or move to manager projectile activation? The packet doesn't say.
- Are there Blueprint assets referencing `AT66BossProjectile` (boss BPs, data tables, particle bindings) that must be checked before deprecation, even without deletion?
- For Stage 17 color distinguishability, what is the acceptance threshold (any two distinguishable, all four distinguishable)?

Required Verification
- Focused `Build.bat T66 Win64 Development` succeeds; staged exe exists at `Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe` and shortcut still points to it.
- `T66GameplayAutoCapture=bossprojectilemanager` smoke: floors 2/3/4 placed minibosses traversed, boss floor reached, managed projectiles visible, `[CombatDamage]` log shows `SourceID=<BossID>` and `Delivery=BossProjectile`, `ProjectileManagerSummary` quoted in completion packet with fired/hit/ActivePeak/DroppedFires, and trail+impact VFX visibly present.
- `T66BossProjectileSmoke=FourHorsemen` smoke: four bosses awakened via the agreed-upon path, ActivePeak captured, DroppedFires==0, screenshot evidence showing at least two coexisting distinct colors.
- Boss-death-during-flight check: induce boss death mid-pattern and confirm in-flight projectiles do not crash or misattribute damage.
- Static search: `rg -n "SpawnActor<AT66BossProjectile>|AT66BossProjectile::StaticClass\(" Source/T66/Gameplay` returns no production fire sites; Blueprint reference scan run separately.
- Per AGENTS.md: Claude review (this pass) precedes implementation; final combined packet lands under `Reports/AgentReviews/20260528_BossProjectileManager/`.

Rationale
The plan correctly identifies the manager's single-type and visual gaps, locks down good scope guardrails (patterns stay on the boss actor, capacity raise documented, deprecation not deletion, narrow `#if !UE_BUILD_SHIPPING` automation), and aligns with AGENTS.md verification posture. The reason this is REVISE rather than APPROVE is that several real lifetime/ownership hazards (stale boss pointer, unbounded HISM buckets, trail component slot reuse) are unspecified and would otherwise be discovered during smoke or — worse — silently shipped, and the MultiBoss smoke method is too loose to count as production-parity evidence. Once those are pinned down, this is implementable as one pass.

