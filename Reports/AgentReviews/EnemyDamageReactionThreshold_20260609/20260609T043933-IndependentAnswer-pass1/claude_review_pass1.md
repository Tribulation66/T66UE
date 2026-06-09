Result: OK

## Independent Answer

The request is implementable internally and maps cleanly onto the live damage path. No user-only decision blocks it, so Codex can proceed. The behavior change lives in `UT66RunStateSubsystem` combat code, not in a Blueprint.

**Live ownership (confirmed by reading the runtime):**
- Every accepted enemy hit flows through `UT66RunStateSubsystem::ApplyDamage` (`Source/T66/Core/RunState/T66RunStateSubsystem_Combat.cpp:911`). It already does two things on each hit: adds percent via `ApplyHeroDamagePercent` (`:1075`) and applies the physics reaction via `ApplyDamagePhysicsReaction` (`:1097`).
- `ApplyDamagePhysicsReaction` (`:402`) unconditionally calls `HeroPhysics->ApplyPhysicsReaction`, which routes to `EnterRagdoll` (`T66HeroPhysicsComponent.cpp:476/479`) — i.e. **today every hit throws and disables**. There is no 50% gate yet.
- Death at 100% already exists: `HeroDamageDeathPercent = 100.f` (`T66RunStateSubsystem.h:1717`), checked at `:1152`.
- Launch magnitude already scales with percent, but anchored at 0%: `GetHeroDamageLaunchScale` lerps `PercentBeforeDamage/100` from 1.0→`LaunchScaleAt100` (`:362`). Disable duration scales via `RagdollDurationDamagePercent` → `ComputeHealthScaledRagdollMaxSeconds()`.

**What the change requires:**
1. Keep percent-gain + knockback on every hit (already true for percent).
2. Gate the throw/disable: only call the ragdoll path (`ApplyPhysicsReaction`/`EnterRagdoll`) when percent is above 50%. Below 50%, apply a non-disabling knockback through the separate `LaunchCharacter` layer (the repo's stated non-ragdoll launch path) instead of ragdoll.
3. Re-anchor the throw-distance and disable-duration scaling so they ramp from 50% (not 0%) up through 99%, by changing the `Alpha` basis in `GetHeroDamageLaunchScale` and the duration scale to `(percent-50)/(100-50)`.

This is a runtime physics/gameplay change, so the stop-condition obligations apply: focused compile, staged-standalone refresh if playable behavior changes, and Unreal-owned capture/log proof (the `[CombatDamage]`/`[CombatDamagePhysics]` log lines already exist for verification).

## Evidence Checked
- `T66RunStateSubsystem_Combat.cpp` — full `ApplyDamage`, `ApplyDamagePhysicsReaction`, `GetHeroDamageLaunchScale`, `ApplyHeroDamagePercent`, lethal handling.
- `T66HeroPhysicsComponent.cpp:426–544` — `ApplyPhysicsReaction`/`EnterRagdoll`, confirming no threshold and that disable duration keys off `RagdollDurationDamagePercent`.
- `T66KnockbackComponent.cpp` — confirms the separate `LaunchCharacter` fallback layer exists for non-ragdoll knockback.
- `T66RunStateSubsystem.h:1717` — death percent = 100.
- `.t66/operator-state.json` — Codex Operator, Claude Validator (matches).

## Questions Or Blockers
None that require the user. The threshold and 50→99 scaling are specified in the prompt; remaining choices are tuning defaults Codex can pick.

## Caveats
- **Boundary semantics** ("once above 50%"): decide strict `>50` vs `>=50`, and whether the gate tests percent *before* or *after* this hit's gain — i.e. the hit that crosses 50% should it throw on that hit or the next. Pick one and state it in the log/proof; not user-blocking.
- **"Knockback only" below 50%** should route through the `LaunchCharacter`/`UT66KnockbackComponent` layer, not `EnterRagdoll`, otherwise the hero still gets disabled. Verify the below-threshold path does not suppress input or enter ragdoll.
- Existing CVars (`t66.HealthPercent.*`) currently assume 0%-anchored scaling; their descriptions/defaults should be revisited so the 50%-anchored ramp doesn't double-apply.
- `EnemyDamageMultiplier`, invuln window, and knockback-invuln bypass (`:1019–1028`) interact with hit cadence — confirm rapid sub-50% hits still accumulate percent toward the 50% gate as expected during capture.
- Confirm non-enemy sources (traps, lava, miasma) are intentionally in or out of scope; the prompt says "enemy" but `ApplyDamage` is shared across all damage sources.
