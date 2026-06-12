Verdict: APPROVE

## Blockers

None. The live-code correction is well-evidenced (concrete file:line citations for constructor tick-disable, pool reset, runtime SetActorTickEnabled, and the manager-driven TickMobVertexAnimationState call sites), and the plan refuses to fabricate a staged FPS isolation that the current source cannot honestly produce.

## Major Issues

- **B.10 closure binary provenance is under-specified for the "post-miniboss-placement" leg.** The plan asserts Stage 0 closes B.10 acceptance "after projectile manager and placed miniboss changes," and it correctly hashes source files and the staged `T66.exe`. But it does not name a concrete artifact (commit SHA, file:line, or symbol) that proves the rebuilt binary contains the miniboss-placement change — only the projectile-manager side is implicitly covered by SHA256 of `T66MobManagerSubsystem.cpp`. Recommend adding an explicit provenance row that names the miniboss-placement source path(s) + their hashes alongside the projectile-manager evidence, so the closure artifact is independently auditable.
- **Manager-side VAT call-site fan-out is acknowledged but not classified.** The finding lists eight `TickMobVertexAnimationState` call sites (2149, 2170, 2185, 2192, 2199, 2206, 2218, 2228) without saying which are lightweight-family paths vs. status/death/override branches. The Task 0 audit should enumerate what each call site represents and confirm none double-advance a clip for the same mob in a single manager tick — otherwise "manager-driven" still hides per-frame redundancy that B.11 was originally meant to clean up.

## Minor Issues

- The 95% acceptance gate is described twice with slightly different phrasings ("at least 95% of current CVar-off median" vs. "not more than 5% below CVar-off"). Lock to one phrasing in the doc append to avoid drift.
- "Bounded sampled `Frame` scalar report" is named but the sampling mechanism isn't (console command, automation hook, etc.). Default to screenshot sequence and only invoke a scalar-sampling path if Pablo authorizes a source hook — same discipline as the DumpTicks fallback.
- The worktree contamination preflight should explicitly call out the currently-modified runtime files visible at session start (e.g. `T66RunStateSubsystem_Combat.cpp`, `Weapons.csv`, `DT_Weapons.uasset`) so classification is concrete, not abstract.
- Rollback section says "do not revert unrelated user or generated changes" — good — but should also forbid `git clean`/discarding LFS-untracked artifacts as part of "clean environment" hygiene.

## Clarifying Questions

1. Reviewer Q2 (B.11 intent): the packet's resolution that "manager-owned per-frame execution" satisfies B.11 without physical field relocation is defensible from the quoted plan wording, but worth a one-line confirmation from Pablo before the packet treats it as settled.
2. Is the placed Slime guardian/miniboss expected on every Stage 0 route, or only a subset? The capture table column is described, but the expected count per run is not — without that, "excluded from route-validity rejection" can mask a route that lost the guardian entirely.
3. If Stage 0 CVar-on lands inside the 2x-stdev escalation band and the 10-capture set is *also* borderline, does Pablo want the pass to halt-and-report, or auto-extend? The packet halts; confirm that's what he wants.

## Required Verification

- Stage 0: 3 (or escalated 10) accepted CVar-off + CVar-on `enemywaveperf` rows with `PerformanceSystemOverheadMaxUs ≤ 10000`, zero HeroDeath rejects, stable staged-binary SHA256 across pass start / per-capture pre+post / pass end.
- Source provenance row: SHA256 + mtime for `T66MobBase.{h,cpp}`, `T66MobManagerSubsystem.{h,cpp}`, `T66CharacterVisualSubsystem.{h,cpp}`, `MobVertexAnimations.csv`, **plus** the miniboss-placement source paths.
- Component-tick audit table for all four lightweight families (Melee/Rush/Flying/Ranged) covering actor tick + `VisualMesh` + `CapsuleComponent` + `BodyHitZone` + `HeadHitZone` + `LockIndicatorWidget` + any movement/timeline component, with class-map confirmation if any family uses a subclass.
- Runtime tick proof via `DumpTicks` parsed log under `Saved/StandaloneLogs/T66_B11_B12_TickRemoval_Smoke.log` — or explicit halt-and-ask if `DumpTicks` cannot be invoked through existing automation.
- Multi-frame VAT animation evidence for each lightweight family: ≥3 samples per active non-death clip, ≥0.10s apart, demonstrating observed pose or `Frame` scalar change. Pool-reuse smoke showing reset-to-idle/walk while actor tick stays disabled.
- Doc append to `2026-05-23_T66_LightweightActor_Plan.md` and pending-issue updates only after captures + smoke pass.

## Rationale

The packet's central move — refusing to manufacture a B.11→B.12 isolation that the live branch can't honestly produce, and instead repurposing the pass as (a) Stage 0 B.10 closure and (b) verification-first B.11/B.12 with optional cleanup — is the right call given the cited evidence at `T66MobBase.cpp:79-80,897` and the manager call sites in `T66MobManagerSubsystem.cpp`. PPF and Artifact Parity are satisfied (same VAT method, same assets, same MID parameters). Hygiene gates mirror the established B.10.1D pattern (overhead ≤10000µs, HeroDeath halt, binary hash chain, 3-to-10 escalation, clean environment). The worktree contamination preflight is the right addition given the visibly dirty tree at session start. Out-of-scope fencing (B.13 HISM, per-instance custom data, rich enemies, projectiles, minigames) is tight. The Major Issues above are tightening asks, not directional changes — safe to present at the AGENTS.md go-ahead gate with those clarifications folded in.

