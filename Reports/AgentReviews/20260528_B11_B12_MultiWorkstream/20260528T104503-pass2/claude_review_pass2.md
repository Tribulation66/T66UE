Verdict: REVISE

## Blockers

None. The packet's direction (commit to lightweight, deprecate rich routing, move VAT state to manager) is internally consistent and the verification gates are explicit. The remaining concerns are scoped enough to address before implementation rather than at gate.

## Major Issues

1. **Gambler centralized spawn path file ownership is underspecified.** MINOR-CLEANUP states "centralize spawn ownership in the gameplay-owned spawn path; UI delegates to that path", but the only gameplay-side files it owns are `T66ActorRegistrySubsystem.*` and `T66TrapArrowProjectile.*`. The actual gameplay-owned spawn path almost certainly lives in `T66EnemyDirector.cpp` (DIVORCE-owned), `T66GameMode_Tower.cpp` (VERIFICATION-owned), or `T66BossBase.cpp` (VERIFICATION-owned). This creates a hidden cross-workstream file edit not covered by the disjoint ownership map. Either (a) name the canonical spawn-path file and assign it explicitly, or (b) serialize this cleanup to main-agent after sub-agent integration.

2. **Boss projectile kill-mid-flight proof lacks a determinism strategy.** The packet says `-T66GameplayAutoCapture=BossProjectileKillMidFlightProof` will "force a boss projectile in flight when source boss dies", but does not specify how that interleaving is arranged deterministically (e.g., spawn projectile, freeze, kill boss, unfreeze; or scripted time gate; or instrumentation-driven). Without a concrete mechanism, a passing run could be lucky and a failing run could be a sequencing artifact rather than a real source-invalidation bug.

3. **Overlays.cpp autocapture CLI setters for deprecated CVars are not addressed.** The discovery section calls out `T66PlayerController_Overlays.cpp:2324-2416` as the autocapture setter/readback site for the routing CVars being neutralized. After neutralization, the CLI setters become no-ops or misleading log lines. The packet defers Overlays edits to main-agent serialized scope "if needed", but does not commit to either (a) leaving CLI setters intact and silently inert, or (b) updating readback/log strings to match deprecation. A capture profile that relies on these flags would silently change semantics.

## Minor Issues

1. **170 FPS Phase 1 floor is justified narratively but not statistically.** No stdev floor or per-row dispersion guard for Phase 1. A median of 170 with a wide spread could mask a regression at the Phase 3 95% threshold. Consider a per-row minimum or a stdev cap.

2. **PPF check covers VAT-STATE only.** DIVORCE deprecates a behavioral routing branch; no PPF/parity statement is provided for "lightweight Basic Ranged firing reliably in CMC range". The packet treats reliable lightweight Ranged delivery as observed evidence in the prior halted rows, but those rows are explicitly excluded from the formal gate. The Phase 1 per-row projectile sanity floor (≥10 fired/≥10 hit) is the substitute — call this out as the DIVORCE parity gate to make the intent explicit.

3. **`Hero_2_Chad/AnimatedToonStyle` presence is required for Phase 1/3 but the enforcement mechanism is informal** ("specifically confirm … is present/cooked or no longer referenced"). The user explicitly declined a new automated content gate, so a one-line staged manifest grep or referenced-asset check before each capture would suffice.

4. **"DEPRECATED" marker contract is not defined.** Packet says "Mark routing CVars and rich-basic-mob routing branch `// DEPRECATED`". Specify the comment shape (e.g., `// DEPRECATED 2026-05-28: lightweight-only; remove after B.13`) so the later deletion pass can grep deterministically.

5. **Pool-reuse reset numeric criterion** ("clip start within 0.5 frames unless … timed override") is reasonable but does not state the sampling source (post-acquire same tick vs. one tick later). Tighten to remove ambiguity.

## Clarifying Questions

1. Which file owns the canonical gameplay-side boss spawn path that Gambler UI should delegate to? If it overlaps DIVORCE or VERIFICATION ownership, will that cleanup serialize to main-agent post-integration?

2. How will `BossProjectileKillMidFlightProof` deterministically arrange a live boss projectile at the moment of boss death? Is there a planned non-shipping hook (force-spawn, time-dilation, scripted kill) or does the autocapture rely on natural timing?

3. Are the routing/touch CVars expected to remain CLI-settable as inert flags for back-compat with existing capture profiles, or should the Overlays.cpp setters be updated as part of this pass?

4. Is the previously observed lightweight median (190.6 FPS) intended as an informal sanity anchor against drift between the halted run and the new Phase 1, or should it be ignored entirely?

## Required Verification

- Phase 1 three-row lightweight capture on a content-complete staged Win64 Development binary with source manifest written **before** staging and SHA256 recorded.
- Phase 3 three-row neutrality capture on the post-implementation binary with the same profile and SHA256, with escalation to ten rows per the borderline-fail rule.
- Runtime tick proof using `IsActorTickEnabled()` and component tick state on live mobs (not header defaults). `DumpTicks` output retained.
- Multi-frame VAT proof: ≥3 samples ≥0.10s apart per lightweight family for idle/move/attack/death, plus `DuplicateVatAdvanceCount=0` evidence and pool-reuse reset capture.
- `MinibossTraversalProofSummary` with `BlockedWhileAlive=1` / `UnblockedAfterDeath=1` per floor across `2 → 3 → 4`.
- `BossProjectileKillMidFlightProofSummary DroppedInvalidSource=1 PostDeathDamage=0` (or equivalent zero-post-death-damage signal) with the determinism mechanism documented in the final packet.
- Compile, focused staged smoke, and Phase 4 documentation updates to the named owning docs and pending-issues files.
- Capture-window hygiene: rejection rule for any row whose Unreal lifetime overlaps `git-lfs.exe`; `git.exe` overlap recorded but not auto-rejecting.

## Rationale

The packet correctly reads the Stage 0a halt as a rich-path control problem, not a measurement problem, and pivots to a lightweight-only direction that is supported by valid prior lightweight rows and by repo-level constraints. The PPF check, mechanism manifest with anti-lookalike discriminator, and per-workstream file ownership map are unusually rigorous and align with `PerformanceSystem` and `Gameplay` AGENTS contracts. The Phase 1/3 capture profile, projectile sanity floor, and escalation rule are reasonable and respect the user's "escalate only borderline fails" constraint. Out-of-scope guardrails (no B.13 HISM, no rich miniboss/special/boss behavior change, no deletions, no Mini/minigame work) match instructions.

The verdict is REVISE rather than APPROVE because the Gambler spawn-path cleanup almost certainly touches a file owned by another workstream, the kill-mid-flight proof lacks a determinism mechanism, and the Overlays CLI setters for deprecated CVars are left dangling. These are all addressable by tightening the packet without changing direction. Once those three points are resolved, this is APPROVE-shaped.

