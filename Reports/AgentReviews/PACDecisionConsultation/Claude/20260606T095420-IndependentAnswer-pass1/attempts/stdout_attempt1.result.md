Result: OK

## Independent Answer

**Recommendation: Keep pure ragdoll as the default now. Treat PAC as deferred-but-not-deleted — only reintroduce it once you move off the detach-mesh path (or build an attached-ragdoll variant), and only if you actually need a specific animated-blend look that pure ragdoll can't deliver.**

Why this is the right direction given the current code:

1. **The disable is intentional and correct, not a bug or a temporary hack.** In `T66KnockbackComponent.cpp:147-159`, PAC is force-disabled *only* when `bDetachMeshDuringRagdoll` is also on. That mutual exclusion is sound: a detached mesh has no live animation source pose to drive toward, and PAC's entire job is to push simulated bodies toward an animated target pose. Running PAC on a detached mesh would drive toward stale/zeroed transforms — wasted force and visual artifacts. So as long as detach-mesh is the chosen ragdoll strategy, PAC *should* stay off.

2. **The PAC infrastructure is fully retained, so re-enabling is a config/profile decision, not a rewrite.** `GetOrCreatePhysicalAnimationComponent`, `ApplyPhysicalAnimationDrive`, the drive modes (`PelvisOnly` / `AllBodiesBelowRoot`), strength multipliers, and the activation-delay pipeline (`TryActivatePhysicalAnimation`) are all still wired (lines ~1131-1276). Nothing about PAC has been amputated — it's gated behind the profile flags. This is a healthy "shelved, reversible" posture rather than a dead end.

**What stays pure ragdoll now:**
- Any knockback profile that uses `bDetachMeshDuringRagdoll` (the current path). Detach + pure ragdoll is the stable, shipping behavior.

**What must be true before PAC is reintroduced:**
- The mesh must remain **attached** during ragdoll (PAC requires `bDetachMeshDuringRagdoll = false`), OR you build a separate attached-ragdoll profile specifically for PAC use.
- A valid physics asset + populated pose buffers must exist — the code already guards this (`HasPhysicalAnimationPoseBuffers`, line 1075; `GetPhysicsAsset()` checks). Confirm the relevant skeletons satisfy these before flipping the flag.
- You have a concrete visual goal PAC uniquely solves (e.g., partial-body reactions, staggers, or "stay partly animated while hit" blends). If pure ragdoll already looks acceptable for the game's tone, **don't** reintroduce PAC — it adds tuning surface (orientation/angular/linear-force params) and per-character physics-asset work for marginal gain.

So: **not enabled now, and not on the current detach-mesh path — but architecturally kept alive for a future attached-ragdoll mode if a specific reaction look demands it.** The decision to actually invest in that path is yours.

## Evidence Checked
- `T66KnockbackComponent.cpp:147-159` — intentional PAC disable when detach-mesh is active (with explanatory log).
- `T66KnockbackComponent.cpp:296-299` — `DetachFromComponent` confirms the detach path.
- `T66KnockbackComponent.cpp:392-397, 1242-1276` — PAC activation is pending/delayed pipeline, still intact.
- `T66KnockbackComponent.cpp:1075-1101, 1131-1239` — pose-buffer guard + full drive infrastructure retained.
- `T66KnockbackComponent.cpp:96-99` — clean PAC component teardown.
- Task contract: consultation only, no code changes; Codex operator / Claude validator per AGENTS.md.

## Questions Or Blockers
None blocking the consultation. The final "do we ever turn it on" call is a design choice reserved to you, but a defensible recommendation can be given from the code as-is, so no user gate is required to answer.

## Caveats
- I did not enumerate which knockback profiles/data assets actually set `bDetachMeshDuringRagdoll` vs. leave it off — if some profiles already keep the mesh attached, PAC could in principle be enabled on *those* today. Worth a quick profile audit before any decision.
- Recommendation assumes pure ragdoll currently looks acceptable in-game; I verified the code path, not the on-screen result. If the visual is the actual pain point, that observation should drive the decision more than the code structure.
- "Eventually" is a roadmap question — the code shows PAC is *kept ready*, not *scheduled*. There is no in-repo evidence of a committed date or milestone to re-enable it.
