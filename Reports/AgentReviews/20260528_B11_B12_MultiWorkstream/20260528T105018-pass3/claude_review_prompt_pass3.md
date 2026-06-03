You are Claude reviewing a Codex implementation or answer plan for the T66 Unreal project.

Rules:
- Start your response immediately with the verdict line. Do not write any
  preface, summary, confirmation, Markdown rule, or other text before it.
- Do not edit files.
- Do not run commands.
- Do not implement the plan.
- Review only the packet below.
- Be strict about contradictions with repo instructions, missing verification, unsafe scope, and unclear goals.
- Treat Codex as the implementer and you as the reviewer.

The first non-empty line of your review must be exactly one of these three lines:
Verdict: APPROVE
Verdict: REVISE
Verdict: BLOCK

After that verdict line, return a concise Markdown review with exactly these headings:
Blockers
Major Issues
Minor Issues
Clarifying Questions
Required Verification
Rationale

Only use Verdict: APPROVE when the reviewed plan/output is safe for Codex to present as greenlit. If implementation still requires user go-ahead under AGENTS.md, APPROVE means safe to present at that go-ahead gate, not permission to skip the gate.

Review scope:
- Packet path: C:\UE\T66\Reports\AgentReviews\20260528_B11_B12_MultiWorkstream\plan_packet.md
- Output scope: review of the packet below only.

<review_packet>
# Lightweight Basic-Mob Divorce + VAT-State Ownership Multi-Workstream Plan Packet

Date: 2026-05-28

Output scope: review of this plan packet for implementation. Under the current root `AGENTS.md`, a valid Claude review with first non-empty line exactly `Verdict: APPROVE` authorizes implementation after Codex reports the review conclusion and caveats. If Claude is unavailable and the Codex fallback reviewer is used, manual user confirmation is still required before implementation.

## Working Goal

Run the coordinated multi-workstream pass under the new baseline direction: commit fully to lightweight basic mobs, deprecate rich basic-mob routing and related CVars, move remaining lightweight VAT state into manager-owned flat data, close independent proof/cleanup gaps, and validate the final binary with lightweight-only FPS health checks plus runtime proofs.

## Decision Status

No unresolved product decision is intentionally left in this packet.

The Stage 0a halt from the previous A/B plan is accepted as a correct process halt but no longer blocks the project direction. The failure was a rich basic-mob control-path problem: rich Ranged spawned and attempted to fire, but CMC movement intermittently failed to bring it into firing range, producing `NoProjectilesFired` rows. That path is being deprecated instead of negotiated with.

New direction:

- Basic mobs route lightweight unconditionally.
- The rich basic-mob routing branch and basic-mob routing CVars are deprecated, not deleted.
- Minibosses, specials, and bosses intentionally remain rich and are untouched by the divorce.
- There is no more rich-vs-lightweight A/B acceptance for basic mobs.
- Measurement is now a lightweight-only absolute FPS health check and a lightweight before/after neutrality check for this pass.
- B.10 closes on lightweight Ranged being functionally correct, reliable, and healthy in FPS captures; intermittent rich basic-mob delivery is recorded as a known limitation of the deprecated path.

## Applicable Instructions Read

- Root `C:\UE\T66\AGENTS.md` supplied in this conversation and verified live for the updated Claude approval rule: working goal, folder discovery, Claude review, pending issues, no destructive cleanup, report routing.
- `C:\UE\T66\PerformanceSystem\PERFORMANCE_SYSTEM_AGENTS.md`: performance/diagnostic work starts from PerformanceSystem contracts and records evidence.
- `C:\UE\T66\Reports\AGENTS.md`: review packets and durable report artifacts belong under `Reports/AgentReviews`.
- `C:\UE\T66\Gameplay\GAMEPLAY_AGENTS.md`: gameplay runtime changes require owning docs and compile/build/staged validation when standalone behavior changes.
- `C:\UE\T66\Gameplay\README.md`: gameplay area ownership; Mini/minigames remain isolated and out of scope unless named.
- `C:\UE\T66\Source\T66\Gameplay\pending_issues_Gameplay.md`: B.10 remains open; updates document rich delivery, placed miniboss, boss projectile manager, and the need for acceptance-finalization evidence.
- `C:\UE\T66\PerformanceSystem\pending_issues_PerformanceSystem.md`: PerformanceSystem overhead is mitigated; binary provenance and Git/LFS contamination remain capture hygiene concerns.
- Memory quick pass: confirms user preference for one combined Markdown packet and current T66 review-gated performance workflow, but live repo instructions remain authoritative.

## Existing Evidence From Halted A/B Attempt

The prior halted Stage 0a run produced valid lightweight rows on the pre-workstream staged binary after the missing `Hero_2_Chad/AnimatedToonStyle` content folder was added to the isolated source/content state and the standalone was restaged. These rows are useful as expected-range evidence, but the packet will rerun Phase 1 after Claude approval because no source manifest was captured before those rows under this new lightweight-only baseline contract.

Pre-workstream staged executable:

- Path: `C:\UE\T66_B11B12_Worktree\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
- SHA256: `86EDE7D6F2533614D9E0525305230BC06CD468F1547642A47C6A2B5C1613C9F5`
- Length: `311102976`

Valid lightweight rows already captured:

- `203.2903696577853 FPS`, overhead `811.8 us`, fired/hit `20/20`
- `190.6166717231366 FPS`, overhead `896.4 us`, fired/hit `21/21`
- `179.57190785943024 FPS`, overhead `1023.6 us`, fired/hit `19/19`
- Median: `190.6166717231366 FPS`

These rows are lightweight-only and independent of the rich CVar-off failure. They are not reused as the formal Phase 1 gate.

## Pre-Review Source Discovery

Narrow source search found the routing/touch CVar ownership before workstream launch:

- `T66.Mob.UseLightweight`: defined in `C:\UE\T66\Source\T66\Gameplay\T66EnemyDirector.cpp:52`.
- `T66.Mob.Diagnostics.RouteFlyingLightweight`: defined in `C:\UE\T66\Source\T66\Gameplay\T66EnemyDirector.cpp:74`.
- `T66.Mob.Diagnostics.RouteRangedLightweight`: defined in `C:\UE\T66\Source\T66\Gameplay\T66EnemyDirector.cpp:85`.
- `T66.Mob.Diagnostics.UseTouchDamageOverlap`: defined in `C:\UE\T66\Source\T66\Gameplay\T66MobManagerSubsystem.cpp:53`.
- Autocapture command-line setters/readback for these CVars live in `C:\UE\T66\Source\T66\Gameplay\T66PlayerController_Overlays.cpp:2324-2416`; that file is main-agent serialized hook scope if logging/readback needs to be updated.
- Gambler boss spawn duplication lives in `C:\UE\T66\Source\T66\Gameplay\T66PlayerController_Overlays.cpp` and `C:\UE\T66\Source\T66\UI\Gambler\T66CasinoGamblerTabWidget_Economy.cpp`: the widget calls `AT66PlayerController::TriggerCasinoBossIfAngry()` and then falls back to its own direct `SpawnActor<AT66GamblerBoss>`. Because the canonical gameplay-side path is currently the player-controller helper, Gambler cleanup is serialized to the main agent after parallel workstreams return.

This confirms DIVORCE and VAT-STATE are disjoint for CVar definitions: routing CVars belong to DIVORCE, touch-overlap belongs to VAT-STATE, and overlay readback is serialized by main if touched.

Deprecated CVar CLI policy:

- The command-line setters remain accepted for backward compatibility with existing capture profiles.
- After this pass they are explicitly logged/read back as deprecated/inert for basic-mob routing or touch-overlap behavior.
- Main-agent serialized scope updates `T66PlayerController_Overlays.cpp` so `T66MobUseLightweight`, `T66MobRouteFlyingLightweight`, `T66MobRouteRangedLightweight`, and `T66MobUseTouchDamageOverlap` no longer imply that they can change runtime behavior.
- The CVar definitions and affected branches use the deterministic marker shape `// DEPRECATED 2026-05-28: lightweight-only basic mobs; inert until cleanup pass removes this path.` For touch overlap, the marker is `// DEPRECATED 2026-05-28: lightweight touch damage path is fixed; inert until cleanup pass removes this diagnostic toggle.`

## User Constraints

- One consolidated plan packet and one combined completion packet.
- Claude review approval permits implementation under the updated root workflow. Codex fallback approval still requires manual user confirmation.
- Main agent owns FPS measurement on a single stable staged binary; no delegated FPS acceptance sets.
- Parallel implementation only on disjoint file sets. If a file is needed by two workstreams, merge or serialize that edit.
- Dirty worktree must be classified before staging. Non-destructive isolated measurement/implementation state is allowed when it leaves the user's dirty worktree untouched and records source provenance.
- Do not revert, stash, reset, clean, or discard user-owned changes.
- `git add -A` or related Git work from another agent should not be killed. Capture hygiene may wait or record overlap, but do not interfere with that agent's requested Git work.
- Ensure staged content completeness for capture, including the previously missing `Hero_2_Chad/AnimatedToonStyle` folder. No new automated content gate is requested.
- B.11/B.12 actor tick disable is already done; do not re-enable tick or manufacture an A/B.
- No B.13 mob HISM rendering or HISM custom data application.
- HeroHPOverride is `20000` for autocapture survival.
- Aggregate counters only; no per-frame per-mob diagnostic logging.
- Build/stage configuration for all Phase 1 and Phase 3 FPS captures is packaged Win64 Development standalone via `Scripts\StageStandaloneBuild.ps1 -ClientConfig Development -SkipShortcutRefresh`.
- Escalate capture count only if the result is a borderline fail, not a borderline pass.

## PPF Check

Objective: Move lightweight-mob VAT state ownership into manager data while preserving the existing VAT animation method and runtime behavior.

Proven process: Existing T66 lightweight-mob VAT path: data rows from `Content\Data\MobVertexAnimations.csv`, static mesh plus VAT material applied by `UT66CharacterVisualSubsystem::ApplyMobVertexAnimationVisual`, material parameters `StartFrame`, `EndFrame`, and `Frame`, and clip selection based on lightweight mob motion/status/death state.

My planned implementation: Keep the same VAT assets, row schema, material, clips, frame math, and dynamic material parameter writes. Move state ownership and advancement from `AT66MobBase` into `UT66MobManagerSubsystem` manager-owned flat records laid out for B.13 per-instance custom data.

Same method class: YES.

If NO, why: Not applicable.

User approval required before proceeding: NO after Claude review approval. YES only if Codex fallback review is used, the user marks the work planning-only, or the reviewer identifies an unresolved user-only decision.

Verification evidence: source audit, focused compile, staged standalone smoke, multi-frame VAT proof, runtime no-tick proof, pool-reuse reset proof, Phase 1/Phase 3 PerformanceSystem captures with binary hashes.

## Artifact Parity Gate

Reference artifact/category: Existing lightweight-mob VAT visuals.

Role: Primary.

Required: YES.

Planned artifact/path: Existing data/assets referenced through `Content\Data\MobVertexAnimations.csv` and `UT66CharacterVisualSubsystem::ApplyMobVertexAnimationVisual`. No new visual assets.

Status: SAME.

Evidence: live source already uses the current VAT visual subsystem and material parameter contract; this pass changes ownership of runtime VAT state, not the visual assets or animation method.

## Mechanism Manifest

Reference/source: current `AT66MobBase` VAT helper plus manager tick branch structure.

Required mechanisms:

1. Mechanism: Clip selection.
   Required: YES.
   Planned implementation: Preserve idle/move selection from stored velocity and explicit attack/hit/death/status override clips; manager state owns current clip.
   Evidence needed: smoke proof for idle, move, attack, hit/status, and death samples across all four lightweight families.

2. Mechanism: Frame advancement.
   Required: YES.
   Planned implementation: Preserve `DeltaSeconds * SampleRate * PlayRate` frame math and clip-range wrapping inside a manager-owned tick helper.
   Evidence needed: at least three samples at least `0.10s` apart per sampled family/clip showing pose or `Frame` change for active non-death clips.

3. Mechanism: Material parameter application.
   Required: YES.
   Planned implementation: Continue writing `StartFrame`, `EndFrame`, and `Frame` to each mob's dynamic material instance from the manager loop. Do not switch to HISM custom data in this pass.
   Evidence needed: source proof and staged visual smoke after refactor.

4. Mechanism: Pool reuse reset.
   Required: YES.
   Planned implementation: Register/reset manager VAT state on acquire/configure/reuse/release so reused mobs start from the correct clip/frame and do not carry stale overrides.
   Evidence needed: non-shipping proof output with slot/mob id, pre-release clip+frame, same-manager-tick post-acquire clip+frame before any advancement, and reset result. The same-tick post-acquire frame must be at clip start within `0.5` frames unless the acquired state explicitly starts in a timed override.

5. Mechanism: Runtime tick absence.
   Required: YES.
   Planned implementation: Keep actor tick disabled and prove no lightweight actor/component tick is enabled while mobs are live.
   Evidence needed: `DumpTicks` or a one-shot proof hook plus runtime `IsActorTickEnabled()` values.

Anti-lookalike discriminator: a still image can hide a frozen VAT frame. The discriminator is multi-frame evidence plus source/runtime proof that advancing `Frame` is manager-owned while actor/component ticks remain disabled.

VAT proof numeric floor:

- At least one live sampled mob per lightweight family for each available proof clip: idle, move, attack, and death. If a clip is not naturally reachable, the proof hook must force a safe non-shipping sample or document why that family lacks the clip.
- For non-paused clips, three samples at least `0.10s` apart must show total `Frame` parameter delta of at least `1.0` frame between first and last sample.
- For freeze/status proof, expected behavior is explicit non-advancement while frozen/rooted; status proof must label the state and not count as active-animation advancement.
- At-most-one advancement proof uses a non-shipping per-manager-frame guard counter. The recorded proof must include `DuplicateVatAdvanceCount=0` and the sampled active mob count for the proof window.

## File Ownership Map

Main agent owns documentation, review artifacts, measurement scripts/output, staging, integration, and final packet.

### Main-Agent-Only Artifacts

- `C:\UE\T66\Reports\AgentReviews\20260528_B11_B12_MultiWorkstream\*`
- `C:\UE\T66\PerformanceSystem\2026-05-23_T66_LightweightActor_Plan.md`
- `C:\UE\T66\Source\T66\Gameplay\pending_issues_Gameplay.md`
- `C:\UE\T66\PerformanceSystem\pending_issues_PerformanceSystem.md`
- `C:\UE\T66\Source\T66\Gameplay\T66PlayerController_Overlays.cpp` for serialized CVar readback/deprecated CLI logging, autocapture dispatch hooks, and canonical Gambler boss spawn ownership
- `C:\UE\T66\Source\T66\UI\Gambler\T66CasinoGamblerTabWidget_Economy.cpp` for serialized removal of the UI fallback direct `AT66GamblerBoss` spawn after the player-controller canonical helper is updated
- capture/stage output under `C:\UE\T66_B11B12_Worktree\Saved\Codex\Performance\...`
- staged binary hash/provenance artifacts

### Workstream DIVORCE: Commit To Lightweight Basic Mobs

Exclusive files:

- `C:\UE\T66\Source\T66\Gameplay\T66EnemyDirector.cpp`

Responsibilities:

- Basic Melee/Rush/Flying/Ranged route to `AT66MobBase` unconditionally.
- Preserve intentional rich routing for minibosses, specials, bosses, and other explicitly non-basic actors.
- Neutralize basic-mob routing CVars so they no longer affect basic-mob routing.
- Mark routing CVars and rich-basic-mob routing branch with `// DEPRECATED 2026-05-28: lightweight-only basic mobs; inert until cleanup pass removes this path.` Deletion is deferred to cleanup.
- Do not delete rich basic-mob code in this pass.

Conflict note: `UseTouchDamageOverlap` lives with manager touch-damage code and belongs to VAT-STATE, not DIVORCE.

### Workstream VAT-STATE: Manager-Owned VAT State

Exclusive files:

- `C:\UE\T66\Source\T66\Gameplay\T66MobBase.h`
- `C:\UE\T66\Source\T66\Gameplay\T66MobBase.cpp`
- `C:\UE\T66\Source\T66\Gameplay\T66MobManagerSubsystem.h`
- `C:\UE\T66\Source\T66\Gameplay\T66MobManagerSubsystem.cpp`

Responsibilities:

- Move actor-resident VAT fields into manager-owned per-mob data: clip index, clip time, play rate, override seconds, using-VAT flag, and B.13-ready custom-data layout values.
- Keep interim MID parameter application from the manager loop.
- Remove dead actor-resident VAT fields and actor-side mutation.
- Confirm at most one VAT advancement per active mob per manager tick.
- Neutralize `T66.Mob.Diagnostics.UseTouchDamageOverlap`; mark it with `// DEPRECATED 2026-05-28: lightweight touch damage path is fixed; inert until cleanup pass removes this diagnostic toggle.` Delete later.
- Preserve visual behavior and reset on pool reuse.

### Workstream VERIFICATION: Deferred Proof Gaps

Exclusive files:

- `C:\UE\T66\Source\T66\Gameplay\GameMode\T66GameMode_Tower.cpp`
- `C:\UE\T66\Source\T66\Gameplay\T66TowerDescentHole.h`
- `C:\UE\T66\Source\T66\Gameplay\T66TowerDescentHole.cpp`
- `C:\UE\T66\Source\T66\Gameplay\T66ProjectileManagerSubsystem.h`
- `C:\UE\T66\Source\T66\Gameplay\T66ProjectileManagerSubsystem.cpp`
- `C:\UE\T66\Source\T66\Gameplay\T66BossBase.cpp`

Responsibilities:

- Add or reuse a non-shipping traversal smoke invoked as `-T66GameplayAutoCapture=MinibossTraversalProof` that walks floors `2 -> 3 -> 4`, kills each placed Slime guardian, and emits a terminal `MinibossTraversalProofSummary` line with per-floor `BlockedWhileAlive=1` and `UnblockedAfterDeath=1`.
- Positively exercise kill-mid-flight source invalidation through `-T66GameplayAutoCapture=BossProjectileKillMidFlightProof`, forcing a boss projectile in flight when source boss dies and emitting `BossProjectileKillMidFlightProofSummary DroppedInvalidSource=1 PostDeathDamage=0` or an equivalent explicit zero-post-death-damage result.
- The kill-mid-flight proof is deterministic: a non-shipping proof hook creates or reuses a boss actor as projectile source, calls `UT66ProjectileManagerSubsystem::FireBossProjectile(...)` with a fixed origin/direction/speed/lifetime so the projectile is in flight but not intersecting the hero, records hero HP/damage counters, kills or unregisters the boss source before the next projectile-manager tick, advances the manager for fixed ticks, then asserts `DroppedInvalidSource >= 1`, `ProjectilesHitHero` did not increase after source death, and hero HP did not decrease after source death. It must not rely on natural boss fire timing.
- If a real bug surfaces in source invalidation, document and fix it within this file set.

Constraint: Do not edit `T66PlayerController_Overlays.cpp`; request serialized main-agent hook change if unavoidable.

### Workstream MINOR-CLEANUP: Independent Audit Gaps

Exclusive files:

- `C:\UE\T66\Source\T66\Core\T66ActorRegistrySubsystem.h`
- `C:\UE\T66\Source\T66\Core\T66ActorRegistrySubsystem.cpp`
- `C:\UE\T66\Source\T66\Gameplay\Traps\T66TrapArrowProjectile.h`
- `C:\UE\T66\Source\T66\Gameplay\Traps\T66TrapArrowProjectile.cpp`

Responsibilities:

- Bosses in damageable-target queries: implement inclusion or document deliberate separation with a boss-target API note.
- EnemiesChanged broadcast: add boss-registration broadcast or document deliberate separation and update consumers as needed.
- Trap projectile fire/impact logs: demote routine telemetry to `VeryVerbose`, preserve warnings/errors.

Constraint: Gambler boss dual spawn path cleanup is main-agent serialized scope, not MINOR-CLEANUP parallel scope, because it touches `T66PlayerController_Overlays.cpp` and `T66CasinoGamblerTabWidget_Economy.cpp`.

### Main-Agent Serialized Cleanup: Gambler And Deprecated Autocapture CLI

Files:

- `C:\UE\T66\Source\T66\Gameplay\T66PlayerController_Overlays.cpp`
- `C:\UE\T66\Source\T66\UI\Gambler\T66CasinoGamblerTabWidget_Economy.cpp`

Responsibilities:

- Make `AT66PlayerController::TriggerCasinoBossIfAngry()` the canonical gameplay-owned Gambler boss spawn path for this pass.
- Remove the widget's fallback direct `SpawnActor<AT66GamblerBoss>` path; the widget delegates to the player-controller helper and reports failure without creating a second ownership path.
- Update autocapture command-line setter/readback logs for neutralized CVars so existing CLI flags are back-compatible but explicitly deprecated/inert.

## Orchestration Model

1. Main agent writes this packet and submits it to Claude review.
2. If Claude approves, main agent reports the review artifact and proceeds without a separate manual go-ahead.
3. If Codex fallback reviewer is used, main agent pauses for manual user confirmation.
4. Main agent classifies the dirty worktree and either uses the existing isolated worktree/source provenance or refreshes it non-destructively.
5. Main agent reruns three lightweight-only Phase 1 captures before implementation because the previous rows were captured before this reviewed contract had a source manifest.
6. Main agent discovers delegation tools. If available, launch sub-agents with the file ownership map above. If unavailable, serialize workstreams without changing the file ownership map and report the tooling limitation in the final packet.
7. Main agent applies serialized cleanup for `T66PlayerController_Overlays.cpp` and `T66CasinoGamblerTabWidget_Economy.cpp` after sub-agent outputs return, then builds one combined staged binary, records SHA256, and owns Phase 3 measurement/proofs.
8. Main agent updates docs/pending issues and returns one combined packet.

## Phase 1: Lightweight Baseline

Run three lightweight-only `enemywaveperf` captures on a content-complete staged Win64 Development standalone binary before implementation. Before staging, write a source manifest for all routing, VAT, projectile manager, autocapture HP, and capture-profile files that affect this pass. Reuse of the previous lightweight rows is explicitly not part of the formal Phase 1 gate.

Capture profile:

- Basic mobs lightweight.
- `T66.AutoCapture.HeroHPOverride=20000`.
- `T66.Ranged.DiagnosticLogging=1` aggregate counters only.
- No rich CVar-off row.
- Reject `PerformanceSystemOverheadMaxUs > 10000`.
- First `HeroDeath` halts.
- `NoProjectilesFired` applies only to the lightweight path now.
- Binary hash recorded before/after each capture and pass end.
- Clean environment before captures: no `RunUAT`, `UnrealEditor-Cmd`, staged `T66.exe`, or `git-lfs` scan overlapping the Unreal runtime window. Git status/add work from another agent is not killed; plain `git.exe` overlap is recorded and accepted only if no `git-lfs.exe` appears during the Unreal process lifetime and the row is not rejected by PerformanceSystem overhead. If `git-lfs.exe` overlaps any accepted row's Unreal start/end window, that row is discarded.

Acceptance:

- Three accepted rows.
- Absolute FPS health floor: Phase 1 median must be at least `170.0 FPS`. This is below the existing observed lightweight median (`190.6166717231366 FPS`) but high enough to reject a materially unhealthy saturated run.
- Dispersion guard: every accepted Phase 1 row must be at least `160.0 FPS` and the three-row stdev must be `<= 20.0 FPS`. If median passes but either dispersion guard fails, run seven additional rows and use the ten-row median/stdev; if the ten-row median is still at least `170.0 FPS` and no more than two accepted rows are below `160.0 FPS`, Phase 1 passes with the wider distribution documented.
- Per-row projectile sanity floor: each accepted lightweight row must record `ProjectileManagerFired >= 10` and `ProjectilesHitHero >= 10`. A row below either floor rejects as insufficient ranged exercise.
- Zero non-zero exits, zero overhead rejects, zero HeroDeath.
- Lightweight projectiles fire/hit in aggregate counters.
- Staged build content-complete enough to avoid missing-asset log spam. Before captures, run a narrow confirmation for `Hero_2_Chad/AnimatedToonStyle`: verify the source content exists in the measurement source state and scan the stage/cook log for no `Hero_2_Chad` or `AnimatedToonStyle` missing-load warnings. This is a documented preflight, not a new automated gate.
- DIVORCE behavior/parity gate: this Phase 1 lightweight-only projectile sanity floor replaces the retired rich A/B gate. Basic Ranged must fire and hit reliably in lightweight mode before the divorce can proceed.

This Phase 1 median becomes the authoritative lightweight baseline and B.10 closure evidence, subject to final documentation and runtime proofs.

## Phase 2: Implementation

Run the four workstreams described in the file ownership map. Main agent handles any shared-file hook serialization after sub-agent results return.

Implementation guardrails:

- No B.13 HISM mob rendering.
- No deletion of deprecated code.
- No rich miniboss/special/boss behavior changes except the serialized Gambler spawn-path ownership cleanup described above.
- No hero/trap/Unique-Debuff projectile manager migration.
- No Mini/minigame scope.
- No per-frame diagnostic logs.
- Use `apply_patch` for manual source edits.

## Phase 3: Combined Binary Measurement And Proofs

After integration:

1. Build one combined binary from the implementation source state.
2. Stage it content-complete; repeat the same narrow `Hero_2_Chad/AnimatedToonStyle` source presence and missing-warning preflight used in Phase 1.
3. Record staged exe SHA256, mtime, length.
4. Run three lightweight-only `enemywaveperf` captures with the same profile as Phase 1.
5. Confirm Phase 3 median is no more than `5%` below the Phase 1 median. A median above Phase 1 is accepted. If Phase 3 fails but is within `2 * max(Phase1Stdev, Phase3Stdev)` of the `95%` threshold, escalate Phase 3 to `10` captures; passing rows do not escalate just because they are close. If Phase 3 remains below threshold, investigate VAT-STATE before reporting completion.
6. Phase 3 per-row projectile sanity floor remains `ProjectileManagerFired >= 10` and `ProjectilesHitHero >= 10`; below-floor rows reject.
7. Phase 3 dispersion guard mirrors Phase 1: every accepted three-row set row must be at least `160.0 FPS` and stdev must be `<= 20.0 FPS`, unless the set escalates to ten rows and passes the documented ten-row distribution rule.
8. Run runtime tick proof: no `AT66MobBase` actor tick and no lightweight component tick while mobs are live, using actual runtime `IsActorTickEnabled()` and component tick state, not header defaults.
9. Run multi-frame VAT proof: at least three samples at least `0.10s` apart per family showing frame/pose change for idle/move/attack/death where applicable, plus pool-reuse reset evidence.
10. Run Workstream VERIFICATION smokes for floors `3/4` minibosses and kill-mid-flight source invalidation.

## Phase 4: Documentation And Pending Issues

Append/update one combined section in `C:\UE\T66\PerformanceSystem\2026-05-23_T66_LightweightActor_Plan.md` with:

- orchestration summary
- worktree/source classification
- file ownership map
- Phase 1 lightweight baseline and staged SHA
- rich basic-mob divorce decision and deprecated CVar/code notes
- each workstream implementation and verification
- Phase 3 lightweight neutrality result and staged SHA
- runtime tick proof
- multi-frame VAT proof
- deferred-smoke results
- caveats

Update `C:\UE\T66\Source\T66\Gameplay\pending_issues_Gameplay.md`:

- close B.10 with Phase 1 staged SHA inline
- document rich basic-mob path deprecation and intermittent rich delivery limitation
- close floors `3/4` verification gap if proof passes
- close kill-mid-flight gap if proof passes or document bug/fix
- close or update audit minor gaps handled by cleanup workstream

Update `C:\UE\T66\PerformanceSystem\pending_issues_PerformanceSystem.md` if capture hygiene findings changed.

## Acceptance Criteria

- Claude approved this packet and implementation proceeded under the updated workflow.
- Dirty worktree/source state classified before staging; no destructive path action performed.
- Basic mobs route lightweight unconditionally.
- Rich basic-mob routing and basic-mob routing CVars deprecated/neutralized, not deleted.
- Minibosses, specials, and bosses remain rich.
- `UseTouchDamageOverlap` neutralized/deprecated.
- VAT state manager-owned, laid out for B.13 custom data; dead actor VAT fields removed.
- At most one VAT advancement per active mob per manager tick.
- Runtime proof: no `AT66MobBase` actor tick and no lightweight component tick while mobs live.
- Multi-frame VAT proof shows correct animation per family/clip and pool-reuse reset.
- Phase 1 lightweight baseline rerun under this packet with source manifest and staged SHA; median `>=170.0 FPS`; every accepted row has `ProjectileManagerFired >= 10` and `ProjectilesHitHero >= 10`; B.10 closed with staged SHA in pending issues.
- Phase 1 and Phase 3 pass the `160.0 FPS` per-row / `20.0 FPS` stdev dispersion guards or the documented ten-row fallback.
- Phase 3 lightweight median is at least `95%` of Phase 1 median after escalation rules if needed; zero non-zero exits, zero overhead rejects, zero HeroDeath.
- Floors `3/4` and kill-mid-flight verified or bug found/fixed/documented.
- Audit minor gaps and trap log issue closed or explicitly documented.
- One combined final packet delivered.

## Out Of Scope

- B.13 mob HISM rendering and per-instance custom data application.
- Deleting deprecated rich basic-mob path, routing CVars, projectile actor classes, or neutralized code.
- Moving non-VAT per-mob behavioral state into manager arrays.
- Rich miniboss/special/boss behavior changes beyond the scoped Gambler spawn ownership cleanup.
- Hero projectile, trap projectile manager migration, or Unique-Debuff projectile migration.
- Human roster review.
- Mini/minigame systems.

</review_packet>
