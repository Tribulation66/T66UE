Verdict: REVISE

## Blockers

None. The strategic direction (HISM-only for body, manager-owned trails, weak-source rule, peer-filter alignment, capacity bump with drop-not-evict, deprecate-don't-delete) is sound and consistent with `AGENTS.md` and `Gameplay/GAMEPLAY_AGENTS.md`. The remaining items are corrections and clarifications, not stop-the-presses problems.

## Major Issues

1. **Stage 17 automated setup is underspecified.** Task 8 says the MultiBoss smoke "sets up a validation-only run state for Stage 17 and calls the existing production `AT66GameMode::SpawnBossForCurrentStage()`." Concretely, how is `CurrentStage` advanced to 17 inside a `#if !UE_BUILD_SHIPPING` automation hook *without* bypassing production progression (which would itself be a fake setup the packet explicitly forbids elsewhere)? Either: (a) name the existing dev console/cheat that already legitimately sets stage, (b) state that the automation drives through real stages, or (c) call this out as a scope risk. As written, "validation-only run state" reads like the kind of fake setup the packet rules out in the autocapture section.

2. **Boss-death-during-flight check has no mechanism.** Verification requires "induce boss death mid-pattern and confirm in-flight projectiles deactivate/drop safely." Task 8 doesn't define how the smoke induces this. Without a concrete trigger (cheat kill, scheduled damage hook, etc.), this becomes a manual step that won't run in automation. Either define the hook or relabel this as a separate manual verification step.

3. **Per-projectile lifetime change risks basic-enemy regression.** Task 1 moves lifetime from "manager-wide constant" to per-projectile state. The packet does not state the existing enemy-spit lifetime or guarantee it is preserved unchanged. Add an explicit statement that enemy-spit projectiles retain whatever lifetime they have today (quoted from the current code), and only boss projectiles use 6.0s.

4. **Task 3 "wrapper only if clearer" is non-committal.** A plan should commit to a shape. Either you add `FireBossProjectile(Params)` or you don't. Decide before review approval so reviewers know what API surface to evaluate.

## Minor Issues

1. **Bucket cap rationale is asserted, not derived.** "Max 32 exact boss visual buckets" — where does 32 come from? Profiles (5) × plausible distinct tint keys observed in `Bosses.csv`? Show the count or note it as a soft cap with documented overflow behavior. Stage 17 failing the smoke for exact-bucket overflow needs a defensible cap.

2. **HISM rotation update every tick for up to 512 projectiles** is asserted as parity for `bRotationFollowsVelocity=true`. No perf cost note. For Stage 17 with ~340 sustained, this is per-tick HISM transform updates × 340; worth at least a one-line "expected cost negligible vs current actor tick" justification or a smoke counter.

3. **`UCLASS(..., Deprecated)` on `AT66BossProjectile`** can break loading of any asset that still references the class. Packet says compatibility references may remain (e.g., `T66GameMode_Backrooms.cpp`). Confirm `Deprecated` UCLASS specifier is safe given those references, or fall back to the comment + `UE_DEPRECATED` on members only.

4. **Bucket release semantics during world transitions.** "Released during subsystem deinitialization" — what about between encounters within one session (e.g., re-runs)? Confirm buckets either persist safely or are reset cleanly without leaking HISM components across runs.

5. **Doc update list omits `MASTER_PLAYER_EXPERIENCE.md`** or any UX-facing note. Probably fine, but call it out as intentionally untouched if there's no player-visible behavior change worth noting.

6. **The static search command `rg -n "SpawnActor<AT66BossProjectile>|AT66BossProjectile::StaticClass\("`** is good, but the expected-result line "no production boss firing path still spawns `AT66BossProjectile`" should also explicitly say `T66BossProjectile_Spawn` style helpers (if any) and confirm `T66GameMode_Backrooms.cpp` cleanup filter behavior is documented as acceptable in the completion packet.

## Clarifying Questions

1. How is `CurrentStage = 17` established inside the `bossprojectilemanager` autocapture mode without bypassing production stage progression?
2. What concrete mechanism induces boss death mid-flight in the automated smoke, and is it inside `#if !UE_BUILD_SHIPPING`?
3. Are you committing to adding `FireBossProjectile(Params)` or routing both paths through a single function on the manager?
4. Does any currently-existing Blueprint or data asset reference `AT66BossProjectile` such that `UCLASS(Deprecated)` would break load? (A read of `Content/Data` and any BP boss class is required before deciding.)
5. What is the current enemy-spit lifetime in `T66ProjectileManagerSubsystem`, and is the per-projectile lifetime change guaranteed to preserve it bit-for-bit?

## Required Verification

- Build with the documented `Build.bat` command and stage via `Scripts\StageStandaloneBuild.ps1`.
- Run `T66GameplayAutoCapture=bossprojectilemanager` smoke through floors 2/3/4/5; collect screenshots + `T66_BossProjectileManager_Smoke.log`.
- Run `T66BossProjectileSmoke=FourHorsemen`; require `DroppedFires=0`, no overflow-bucket warnings, `ActivePeak` logged, ≥2 coexisting projectile colors evidenced.
- Confirm `[CombatDamage]` log shows `SourceID=<BossID>` and `Delivery=BossProjectile` for boss hits.
- Confirm fire audio still emits from `AT66BossBase`, not the manager.
- Confirm HISM bodies rotate to velocity for cone/cylinder profiles in screenshot evidence.
- Crowded-scene peer-filter check: projectiles visibly cross peer bodies without disappearing, hero damage still gated to capsule/hurtbox.
- Boss-death-during-flight check: in-flight projectiles deactivate cleanly, no crash, no stale-source damage (mechanism per Major #2).
- Static search results pasted into the completion packet.
- Doc updates landed in `MASTER_COMBAT.md`, `Miniboss_Special_Boss_Spawn_and_Integration_Audit.md`, and the relevant `pending_issues_*.md`.

## Rationale

The packet correctly inventories live actor behavior, identifies real manager gaps (type ignored, single visual, peer filtering, damage attribution), and proposes a migration that preserves the actor-side pattern logic — which matches the locked decision. Capacity math and overflow policy are explicit. Source invalidation is handled with weak pointers and a documented drop rule. Trail/impact preservation is required, not waved away, with a stop-and-revise gate if engine limits bite. Peer-filter behavior change is acknowledged as intentional rather than hidden. Deprecation is conservative (no deletes this pass).

What pushes this to REVISE rather than APPROVE: the two smoke-automation gaps (Stage 17 setup and boss-death-during-flight) and the per-projectile lifetime risk to enemy-spit parity are non-trivial and would either degrade verification quality or risk regressing existing behavior if implementation proceeds as written. These are tightenings, not redesigns — once addressed in a packet revision, this is ready for the standard implementation go-ahead gate.

