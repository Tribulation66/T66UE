Verdict: REVISE

## Blockers
None.

## Major Issues
- **HISM color strategy is unresolved.** Plan offers "profile/tint-aware component buckets OR per-instance custom data" but does not commit. Stage 17 Four Horsemen is explicitly in scope and four bosses with different `AttackPrimaryColor`/`AttackSecondaryColor` sharing a profile bucket will visibly collapse to a single color. Pick one approach (per-instance custom data is the only correct one if any two same-profile bosses coexist with different tints) and enumerate the buckets before Codex implements.
- **Trail/impact VFX disposition is left dangling.** Working goal says "preserving... visual language", but Risks says "explicitly document if trail/impact VFX is deferred." These contradict. Acceptance criterion "no obvious stuck/flicker artifacts" does not cover missing trails. Decide: migrate trail/impact via manager-side Niagara budget, OR explicitly mark deferred and accept reduced visual parity in acceptance criteria. Also state what happens to `T66.VFX.BossProjectileMaxPerFrame` and `T66.VFX.BossProjectileUseEffectsScalability` cvars — if trails are dropped, do these become orphaned or follow into the manager.
- **Damage causer parameter is vague.** Plan Task 5 reads `ApplyDamage(DamageHP, BossActor, "BossProjectile", BossActor or equivalent manager source context)`. Source-ID resolution in `UT66RunStateSubsystem_Combat.cpp` keys on `AT66BossBase`, so the SourceMob argument must be the boss actor pointer specifically. "Equivalent manager source context" should be removed — commit to passing the live `AT66BossBase*` through the fire spec.
- **Four Horsemen capacity bump is unverified.** 256→512 is justified entirely by arithmetic (`~340` worst-case), but the verification plan downgrades Stage 17 to "if feasible... otherwise document as analysis + follow-up." The capacity raise is the only mitigation against the highest-risk scenario; punting validation makes the smoke pass insufficient to clear acceptance criterion "documented against single-boss plus Four Horsemen load." Either include a Stage 17 standalone smoke (even with debug spawn-all-four cheat) or drop the capacity bump from this pass.
- **Per-projectile collision radius support not confirmed in current manager.** Live audit shows boss actor scales radius `24 * max(1, VisualScaleMultiplier)` but does not confirm `FT66ManagedProjectile` already carries a per-instance radius field or that the hero capsule-segment/world-sphere sweep uses it. Task 1 implies adding it; Task 3 assumes it works. Make Task 1 explicit: extend the struct field set and the sweep path to consume per-projectile radius.

## Minor Issues
- Behavior change in peer collision (boss shots no longer destroyed on mob overlap) is buried in Risks. Promote to explicit acceptance/doc note — this can alter perceived combat readability and is a meaningful regression vector versus the actor-based path.
- "Linear scanning acceptable" at 512 is asserted without a measurement reference. Cheap to add a Stat or `ProjectileManagerSummary` peak active count assertion to the smoke log.
- Audio fire event: live audit lists it but tasks do not explicitly state the boss-side helper keeps owning it. Add one sentence to Task 4 confirming audio stays actor-side.
- Lifetime `6.0s` should be per-projectile state on the fire spec, not a manager constant, to remain future-proof for hero/Unique-debuff projectiles. Worth a one-line clarification in Task 1.
- `SewerSlimeKing` lobe-volley uses delayed scaled spawns. Confirm pattern-side timers continue to call the new manager API per shot rather than batching, since the manager is shot-granular.
- Plan lists `pending_issues_Gameplay.md` twice in Task 8 file list — likely a typo for one Gameplay and one Combat pending file (Applicable Instructions already notes Combat pending stays separate).

## Clarifying Questions
- Will per-instance custom data be wired into the boss projectile master material this pass, or will Codex add a new material? If new, that is an art-side artifact that should be named.
- For Four Horsemen smoke: is there a debug cheat or test map that can spawn all four simultaneously in a standalone run, or must this be deferred?
- Does AGENTS.md require explicit user go-ahead before Codex starts implementation after Claude review? If yes, this APPROVE/REVISE gate is the plan-review gate, not an implementation gate.

## Required Verification
The plan's verification section is mostly sound but must add:
- Per-instance color check: in Stage 17 (or simulated multi-boss), confirm two coexisting bosses with different primary tints render as distinct colors in flight.
- Active-peak measurement: extract peak active projectile count from `ProjectileManagerSummary` log and quote it in the completion packet, especially under multi-boss load.
- Damage attribution check: grep the standalone log for `SourceID=<actual BossID string>` and `Delivery=BossProjectile` — current plan only describes this, completion packet should quote the lines.
- Static search expectation: `rg` command escaping in the packet (`StaticClass\\(`) — confirm Codex runs the un-escaped form on the actual shell.
- Trail/impact decision verification: if deferred, screenshot diff should be acknowledged in completion packet, not silently shipped.

## Rationale
The plan demonstrates solid live-audit grounding and respects scope (boss actor stays rich, patterns stay actor-side, manager owns instances). The 8-task decomposition is coherent and reversible. However, three of the highest-risk design decisions — HISM color strategy, trail/impact VFX fate, and Four Horsemen capacity validation — are left as "either/or" branches. These are exactly the decisions that should be locked before Codex begins, because each has a substantially different implementation cost and acceptance footprint. The damage-causer ambiguity and unconfirmed per-projectile radius support are smaller but concrete correctness risks. Once these are pinned down, the packet should be ready for implementation greenlight.

