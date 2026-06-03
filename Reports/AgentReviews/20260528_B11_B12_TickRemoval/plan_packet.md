# B.11+B.12 VAT Into Manager And Tick Disable Plan Packet

Date: 2026-05-28

Output scope: review of this plan packet only. No implementation is authorized by this packet.

## Working Goal

Produce a reviewed execution plan for the combined B.11+B.12 lightweight-mob VAT/tick-removal pass, including the Stage 0 post-projectile-manager CVar-on baseline that closes the open B.10 acceptance number.

## Codex Live-Repo Finding

The original pass premise says `AT66MobBase` per-actor tick currently owns VAT animation and B.12 will disable the actor tick after B.11. The live source does not match that premise.

Evidence:

- `C:\UE\T66\Source\T66\Gameplay\T66MobBase.cpp:79` sets `PrimaryActorTick.bCanEverTick = false`.
- `C:\UE\T66\Source\T66\Gameplay\T66MobBase.cpp:80` sets `PrimaryActorTick.bStartWithTickEnabled = false`.
- `C:\UE\T66\Source\T66\Gameplay\T66MobBase.cpp:897` calls `SetActorTickEnabled(false)` during pooled reuse reset/configuration.
- `C:\UE\T66\Source\T66\Gameplay\T66MobManagerSubsystem.cpp:678` calls `Mob->SetActorTickEnabled(false)` when runtime active state changes.
- `C:\UE\T66\Source\T66\Gameplay\T66MobManagerSubsystem.cpp:2149`, `2170`, `2185`, `2192`, `2199`, `2206`, `2218`, and `2228` call `Mob->TickMobVertexAnimationState(DeltaTime)` from the manager tick loop.
- There is no `AT66MobBase::Tick` override in `C:\UE\T66\Source\T66\Gameplay\T66MobBase.cpp`.

Current VAT state is not fully manager-owned data yet: `AT66MobBase` still stores `ActiveMobVertexAnimationRow`, `ActiveMobVertexAnimationMID`, `ActiveMobVertexAnimationClip`, `MobVertexAnimationClipTime`, `MobVertexAnimationOverrideSecondsRemaining`, and `bUsingMobVertexAnimation` in `C:\UE\T66\Source\T66\Gameplay\T66MobBase.h:268-276`, and the helper `AT66MobBase::TickMobVertexAnimationState` still mutates those fields. But the per-frame VAT advancement is already manager-driven and the actor tick is already disabled.

That means the requested stage-by-stage FPS isolation cannot be produced honestly from the current live branch. There is no current "pre-B.11 actor-tick VAT" binary to measure, and reintroducing actor ticking just to manufacture an A/B comparison would be measurement contamination.

## Applicable Instructions Read

- Root `C:\UE\T66\AGENTS.md` pasted into the conversation: plan packet, external review, Pablo go-ahead before implementation.
- `C:\UE\T66\PerformanceSystem\PERFORMANCE_SYSTEM_AGENTS.md`: performance/optimization work starts in `PerformanceSystem`; playable runtime changes require staged standalone proof.
- `C:\UE\T66\Gameplay\GAMEPLAY_AGENTS.md`: Gameplay source work requires reading `Gameplay/README.md` and compile/staged verification for runtime changes.
- `C:\UE\T66\Gameplay\README.md`: confirms Gameplay docs ownership; Mini/minigames are separate and out of scope.
- `C:\UE\T66\Reports\AGENTS.md`: review packets belong under `Reports/AgentReviews`.
- `C:\UE\T66\PerformanceSystem\pending_issues_PerformanceSystem.md`: overhead spike is mitigated but capture hygiene, binary provenance, and Git/LFS isolation remain standing requirements.
- `C:\UE\T66\Source\T66\Gameplay\pending_issues_Gameplay.md`: B.10 acceptance remains open until post-projectile-manager/post-miniboss-placement CVar-on acceptance is captured.

## PPF Check

Objective: Preserve the existing VAT animation method while verifying and, only if needed, completing lightweight-mob tick removal.

Proven process: Existing T66 lightweight-mob VAT path: static mesh + VAT material from `MobVertexAnimations.csv`, material scalar parameters `StartFrame`, `EndFrame`, and `Frame`, with clip selection from mob movement/status/death state.

My planned implementation: Do not author new animation assets or switch rendering method. Keep existing VAT assets/material parameter semantics. First verify the live state. If a source change is still justified, limit it to moving VAT state ownership/accounting into `UT66MobManagerSubsystem` while preserving the same static mesh, dynamic material instance, clip names, frame math, and scalar parameter writes.

Same method class: YES for verification and any targeted ownership refactor. NO for any B.13-style per-instance custom data or HISM mob rendering, which is explicitly out of scope.

If NO, why: Not applicable for this packet's planned scope.

User approval required before proceeding: YES, after Claude review.

Verification evidence: focused build, staged standalone smoke, animation visual proof for all four lightweight families, pool-reuse animation reset proof, and enemywaveperf capture rows with binary hash provenance.

## Artifact Parity Gate

Reference artifact/category: Existing lightweight-mob VAT visuals.

Role: Primary.

Required: YES.

Planned artifact/path: Existing assets referenced by `C:\UE\T66\Content\Data\MobVertexAnimations.csv`, including rows such as `Slime`, `CaveBat`, `RatPack`, and `HexSlinger`.

Status: SAME.

Evidence: `UT66CharacterVisualSubsystem::ApplyMobVertexAnimationVisual` applies the same static mesh, VAT material, position/normal textures, and scalar parameters at `C:\UE\T66\Source\T66\Core\T66CharacterVisualSubsystem.cpp:1029-1139`.

## Mechanism Manifest

Reference/source: Current `AT66MobBase` VAT helper and manager tick path.

Required mechanisms:

1. Mechanism: VAT clip selection.
   Required: YES.
   Planned implementation: Preserve current move-vs-idle selection from velocity and explicit death/hit-react overrides. Do not change clip ranges or names.
   Evidence needed: smoke log/screenshot proof that moving mobs animate, idle mobs idle, and death/hit-react clips still trigger.

2. Mechanism: VAT frame advancement.
   Required: YES.
   Planned implementation: Preserve current frame math based on `DeltaSeconds`, `SampleRate`, play rate, and clip frame range.
   Evidence needed: multi-frame screenshot sequence or log proof showing non-frozen frames for all four migrated families.

3. Mechanism: Material parameter application.
   Required: YES.
   Planned implementation: Preserve interim MID writes to `Frame`, `StartFrame`, and `EndFrame`; do not move to per-instance custom data until B.13.
   Evidence needed: source inspection plus visual smoke that VAT assets animate after the change or verification pass.

4. Mechanism: Pool reuse reset.
   Required: YES.
   Planned implementation: Preserve `TryApplyMobVertexAnimationVisual`/configure reset semantics so reused mobs start from the expected clip/frame and remain actor-tick disabled.
   Evidence needed: pool-reuse smoke with an acquired/reused mob animating correctly.

Anti-lookalike discriminator: A static mesh with a fixed VAT frame can look like a valid mob in a single screenshot. The discriminator is multi-frame or runtime evidence that the `Frame` parameter advances over time while the actor tick remains disabled.

## User Constraints

- Scope is lightweight `AT66MobBase` mobs only.
- Rich `AT66EnemyBase`, placed minibosses, bosses, and specials are untouched.
- Bosses and minibosses stay rich.
- HeroHPOverride is `20000` for autocapture survival.
- Aggregate counters only. No per-frame per-mob diagnostic log emission.
- Stage 0 establishes the clean current CVar-on baseline and closes the B.10 acceptance number if gates pass.
- 3-capture default, escalate to 10 if within 2x stdev of a gate boundary.
- Reject captures with `PerformanceSystemOverheadMaxUs > 10000`.
- Halt set on 2+ rejected captures, and halt on first HeroDeath per the current harness rule.
- Staged `T66.exe` binary hash must remain stable across the pass.
- Pre-capture clean environment: no `RunUAT`, `UnrealEditor-Cmd`, staged `T66.exe`, or Git/LFS contamination during FPS acceptance rows.
- B.13 mob HISM rendering and per-instance custom data are out of scope.

## Plan Revision Required By Live Code

### Original Requested Plan

1. Stage 0: capture clean CVar-off and CVar-on baseline.
2. Stage 1 / B.11: move VAT animation state into manager.
3. Stage 1 measurement: CVar-on, expected neutral.
4. Stage 2 / B.12: disable `AT66MobBase` actor tick.
5. Stage 2 measurement: CVar-on, expected measurable gain.

### Repo-Grounded Correction

The live branch already has manager-driven VAT advancement and actor tick disabled. Therefore:

- Stage 0 remains valid and should run first.
- Stage 1 should become "B.11 ownership verification and optional small state-owner cleanup", not a guaranteed behavior-moving implementation.
- Stage 2 should become "B.12 tick-disable verification and residual component-tick audit", not a guaranteed actor-tick-disabling implementation.
- The pass cannot claim a new B.12 FPS gain over Stage 1 unless a real residual per-frame actor/component tick is found and removed.
- If no residual tick work is found, the correct completion claim is: B.11/B.12 are already structurally satisfied in current source; this pass documents that fact, closes B.10 acceptance through Stage 0 capture, proves visual correctness, and updates the plan baseline table.
- If the audit finds a residual in-scope lightweight component tick, Pablo go-ahead to this packet authorizes a small Stage 2 source change to disable or move that residual tick only within the listed in-scope files/components. If the fix touches rich enemies, bosses, minibosses, projectiles, rendering strategy, assets, data tables, or any file outside this packet's source scope, halt and write a follow-up packet.

## Proposed Execution Plan

### Task 0: Final Pre-Implementation Audit

Read and document:

- `C:\UE\T66\Source\T66\Gameplay\T66MobBase.h`
- `C:\UE\T66\Source\T66\Gameplay\T66MobBase.cpp`
- `C:\UE\T66\Source\T66\Gameplay\T66MobManagerSubsystem.h`
- `C:\UE\T66\Source\T66\Gameplay\T66MobManagerSubsystem.cpp`
- `C:\UE\T66\Source\T66\Core\T66CharacterVisualSubsystem.h`
- `C:\UE\T66\Source\T66\Core\T66CharacterVisualSubsystem.cpp`
- `C:\UE\T66\Content\Data\MobVertexAnimations.csv`
- current B.10.1D validation runner under `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1D\`

Audit outputs:

- Worktree contamination preflight before staging:
  - Enumerate modified/deleted paths relevant to runtime source, runtime data, runtime config, and staged/cooked outputs without broad LFS-heavy scans where possible.
  - Classify each changed path as runtime-affecting for Stage 0, documentation-only, report-only, generated artifact, or unrelated/unknown.
  - Explicitly include currently visible runtime-risk examples in the classification if present: `C:\UE\T66\Source\T66\Core\RunState\T66RunStateSubsystem_Combat.cpp`, `C:\UE\T66\Content\Data\Weapons.csv`, `C:\UE\T66\Content\Data\DT_Weapons.uasset`, deleted `Content` assets, and runtime `Config` files.
  - Do not revert, stash, or discard user changes without explicit user approval.
  - If any unrelated runtime-affecting or unknown change would be included in the Stage 0 binary, halt before staging and ask Pablo whether to include it, defer it, or resolve it. This halt is mandatory, not advisory.
  - The Stage 0 baseline artifact must not silently include unrelated WIP such as weapon data, run-state combat changes, deleted content assets, or other runtime data without per-path classification.
  - Clean-environment hygiene must not use `git clean`, broad discard, or LFS-untracked deletion. Treat cleanup as process management, not destructive worktree mutation.
- Every per-frame responsibility currently owned by `UT66MobManagerSubsystem::Tick`.
- Manager VAT call-site classification:
  - Classify the current call sites at `C:\UE\T66\Source\T66\Gameplay\T66MobManagerSubsystem.cpp:2149`, `:2170`, `:2185`, `:2192`, `:2199`, `:2206`, `:2218`, and `:2228`.
  - Identify each as death, knockback, status-blocked, Rush, Flying, Ranged, arrival/idle, or default Melee movement.
  - Prove each active mob reaches at most one VAT advancement call per manager tick through the branch/continue structure before claiming B.11 has no redundant per-frame advancement.
- Whether any `AT66MobBase` method is invoked by actor tick.
- A concrete component-tick table for `AT66MobBase` and any subclass used by the four lightweight families. The audit must first list the lightweight family class map:
  - Melee: expected `AT66MobBase`.
  - Rush: expected `AT66MobBase`.
  - Flying: expected `AT66MobBase`.
  - Ranged: expected `AT66MobBase`.
  - If source audit finds a subclass or alternate class for any family, name it and include its component list before making a B.12 claim.
  The table must list, at minimum:
  - Actor tick: `PrimaryActorTick.bCanEverTick`, `PrimaryActorTick.bStartWithTickEnabled`, and runtime `IsActorTickEnabled()`.
  - `VisualMesh` / VAT-driving `UStaticMeshComponent`: `PrimaryComponentTick.bCanEverTick` and runtime tick-enabled state.
  - `CapsuleComponent`: tick capability and runtime tick-enabled state.
  - `BodyHitZone` and `HeadHitZone`: tick capability and runtime tick-enabled state.
  - `LockIndicatorWidget`: tick capability and runtime tick-enabled state.
  - Any `UMovementComponent`-derived component, including `UCharacterMovementComponent` if present, plus any custom movement component, timeline, curve helper, or latent per-frame helper attached to lightweight mobs. If absent, state absent with evidence.
- Whether current VAT material setup relies on MID scalar updates rather than component tick.
- Whether B.10 has a clean post-miniboss-placement CVar-on acceptance number. Current docs say no.
- A narrow documentation sweep for existing docs that assert VAT or actor-tick ownership:
  - `C:\UE\T66\PerformanceSystem\2026-05-23_T66_LightweightActor_Plan.md`
  - `C:\UE\T66\PerformanceSystem\2026-05-23_T66_LightweightActor_Evaluation.md`
  - `C:\UE\T66\Gameplay\Combat\MASTER_COMBAT.md`
  - `C:\UE\T66\Gameplay\Combat\MOB_SYSTEM_MODEL_PIPELINE_REPORT_2026-05-15.md`
  - `C:\UE\T66\PerformanceSystem\2026-05-23_T66_Mass_Migration_Plan.md`
  - `C:\UE\T66\PerformanceSystem\PERFORMANCE_SYSTEM_AGENTS.md`
  Update only docs that still make stale ownership assertions relevant to this pass.
- B.11 intent resolution:
  - Quote the migration plan wording from `C:\UE\T66\PerformanceSystem\2026-05-23_T66_LightweightActor_Plan.md:43` and `:57`, which defines B.11 as moving VAT frame/update ownership and frame ticking out of per-actor tick and into the manager.
  - Treat that wording as manager-owned per-frame execution, not mandatory physical relocation of every VAT state field off `AT66MobBase`, unless Pablo explicitly expands scope.
  - Therefore physical VAT state-record relocation is out of the default B.11 closure path for this pass.
- Miniboss-placement provenance row for Stage 0:
  - Include source hashes/mtimes for `C:\UE\T66\Source\T66\Gameplay\GameMode\T66GameMode_Tower.cpp`, `C:\UE\T66\Source\T66\Gameplay\T66GameMode.h`, `C:\UE\T66\Source\T66\Gameplay\T66TowerDescentHole.cpp`, `C:\UE\T66\Source\T66\Gameplay\T66TowerDescentHole.h`, and `C:\UE\T66\Source\T66\Gameplay\T66EnemyBase.cpp`.
  - The relevant placed-guardian seams include `T66PlacedTowerMinibossMobID`, `IsPlacedTowerMinibossFloor`, `EnsurePlacedTowerMinibossForFloor`, `SetGuardianEnemy`, `bRequiresGuardianDefeated`, and `HandleTowerGateGuardianDefeated`.
  - This row is required so the B.10 closure artifact proves both projectile-manager and placed-miniboss source are included.
- Confirm before Stage 0 whether the existing B.10.1D runner is reused. Default to reusing `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1D\run_b101d_projectile_manager_validation.ps1`; fork/create a B.11+B.12 runner only if a concrete field or mode is missing.
- Confirm before Stage 0 whether `DumpTicks` can be invoked after lightweight mobs are present using existing staged standalone command/automation paths. If not, use the fallback decision path in Smoke Validation before adding any source hook.

### Stage 0: Establish Current Clean Baseline And Close B.10 Acceptance

Default to a fresh build/stage from the current worktree before Stage 0 so the baseline definitely contains the post-projectile-manager and post-miniboss-placement source. Record:

- Current source provenance for the in-scope files by file path, last-write time, and SHA256.
- The staged `T66.exe` SHA256, last-write time, and file length after staging.
- The stage log path.

Reuse an existing staged binary only if a positive provenance artifact already ties that exact staged `T66.exe` SHA256 to the current worktree state for the in-scope source files. If that proof is absent, rebuild/stage. The recorded staged SHA is the B.10 closure artifact if Stage 0 passes.

- CVar-off: 3 captures, `T66.Mob.UseLightweight=0`, `T66.AutoCapture.HeroHPOverride=20000`, `enemywaveperf`.
- CVar-on: 3 captures, `T66.Mob.UseLightweight=1`, `T66.AutoCapture.HeroHPOverride=20000`, `enemywaveperf`.

Use the B.10.1D validation-runner hygiene shape:

- `PerformanceSystemOverheadMaxUs <= 10000`.
- first HeroDeath halt.
- binary hash at pass start, before/after every capture, and pass end. The existing B.10.1D runner at `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1D\run_b101d_projectile_manager_validation.ps1` records executable fingerprint data in progress/result artifacts; if a new runner is created, it must preserve equivalent per-capture pre/post hash fields.
- clean environment before each set and capture.
- no per-frame ranged diagnostic lines.
- expected placed Slime guardian/miniboss route attribution must be documented as a concrete field or note in the Stage 0 capture summary table, using `BossOrGuardianObserved` / route-attribution counts where available, and explicitly excluded from route-validity rejection. Expected count per run is `0` or `1` depending on whether the autocapture route initializes a guardian-gated combat floor; `BossOrGuardianObserved > 1`, any `MiniBossPromotionSlots > 0`, or any `RoutedRichMiniBossPromotion > 0` remains a failure.

Acceptance for Stage 0:

- 3 accepted CVar-off and 3 accepted CVar-on rows, or escalation to 10 if the CVar-on median is within 2x stdev of the 95% gate against CVar-off.
- CVar-on median must be at least 95% of current CVar-off median. Use this exact phrasing in documentation. This is the canonical B.10.1D acceptance shape: `C:\UE\T66\PerformanceSystem\B10_1D_ProjectileManager_HISM_ConsolidatedPacket.md:23` states the CVar-on median must be at least 95% of the new CVar-off median, and `C:\UE\T66\PerformanceSystem\2026-05-23_T66_LightweightActor_Plan.md:762` records the same 95% CVar-on/CVar-off acceptance pattern.
- Zero HeroDeath rejects.
- Zero accepted rows above the `PerformanceSystemOverheadMaxUs > 10000` rejection threshold; the set still follows the standing halt rule for 2+ rejected captures.
- Stable binary hash.

If Stage 0 passes, document the CVar-on median as the current authoritative lightweight baseline and the closed B.10 acceptance number. The closure wording must be specific: "B.10 acceptance closed under the recorded source provenance set." Do not write a generic "B.10 closed" claim that could be inherited after later projectile-manager, placed-miniboss, or runtime-source changes.

If Stage 0 fails:

- Halt the pass.
- Do not close B.10.
- Do not proceed to B.11/B.12 verification claims.
- Preserve the staged binary, rows, logs, progress files, and screenshots as diagnostic evidence under the pass artifact directory; do not prune them during the pass.
- Update the plan document and relevant pending issue with the failure mode: FPS gate failure, route-validity failure, HeroDeath, overhead, binary drift, Git/LFS contamination, missing summaries, or another concrete reason.
- If CVar-on is within the 2x stdev escalation band, escalate to 10 before calling the gate failed or passed. If the 10-capture set remains below the 95% gate, mark B.10 inconclusive/failed and halt.
- Escalation operand: escalate when `abs(CVarOnMedian - (0.95 * CVarOffMedian)) <= 2 * CVarOnStdev`. Use the same operand for post-change CVar-on comparisons against their current gate/baseline.

### Stage 1 / B.11: Manager VAT Ownership Verification, Optional Cleanup Only If Justified

Default expected path:

- No source code change for B.11 behavior if the audit confirms current manager-driven VAT advancement is complete.
- Document that the manager tick already advances VAT via `Mob->TickMobVertexAnimationState(DeltaTime)`.
- Document that remaining VAT state is actor-resident data but not actor-tick-owned.

Source-change trigger:

- Default for this pass is no B.11 source change.
- A B.11 source change is allowed only if the audit finds concrete residual per-frame VAT work outside the manager tick, or Pablo explicitly expands the go-ahead to require physical VAT state records to move out of `AT66MobBase`.
- Actor-resident VAT state alone is not treated as a blocker for B.11/B.12 verification in this packet because per-frame execution is already manager-owned and B.13 will replace the per-mob MID application path.

If explicitly authorized by the trigger above, the source-change shape is:

- Add a manager-owned lightweight VAT state record keyed by active `AT66MobBase` identity, with `Clip`, `ClipTime`, `OverrideSecondsRemaining`, and cached row/MID references.
- Convert `AT66MobBase` VAT helpers into non-ticking application helpers or manager-accessible setup methods.
- Keep per-frame `SetScalarParameterValue` in the manager loop.
- Do not introduce per-instance custom data.
- Keep actor tick disabled throughout.

Stage 1 measurement:

- If no source change is made, do not run a redundant "Stage 1 FPS" set as though it were a new binary. Instead, use Stage 0 CVar-on as the B.11 verified-current baseline and document why no between-stage measurement exists.
- If a B.11 cleanup change is made, run 3 CVar-on captures and compare to Stage 0 CVar-on. The same 3-to-10 escalation rule applies if the result is within 2x stdev of the neutrality/gate boundary. Expected neutral within noise. Investigate before proceeding if it regresses beyond noise.

### Stage 2 / B.12: Actor Tick Disable Verification And Residual Tick Audit

Default expected path:

- No source code change if the audit confirms `AT66MobBase` actor tick is disabled in constructor, reset/reuse, and runtime activation.
- Document the exact source lines and runner/smoke evidence.
- The component-tick table from Task 0 is required before any B.12 verified claim.

Optional source change path, only if a residual tick source is found:

- Disable or move only the residual lightweight-mob actor/component tick source.
- Preserve collision, hit zones, lock indicator behavior, visual mesh visibility, and pool reuse.
- Do not touch rich enemies, minibosses, bosses, specials, hero/trap projectiles, or B.13 rendering.

Stage 2 measurement:

- If no source change is made, do not claim a new B.12 FPS gain. Document B.12 as already satisfied by existing code and Stage 0 baseline.
- If a residual tick is removed, run 3 CVar-on captures and compare to the previous CVar-on set. The same 3-to-10 escalation rule applies if the result is within 2x stdev of the neutrality/gain boundary. Document gain or neutrality.

### Smoke Validation

Run staged standalone smoke after any source change, or as verification-only if no source change is made:

- Spawn/observe all four lightweight families: Melee, Rush, Flying, Ranged.
- Confirm walk/idle animation advances.
- Confirm attack/death/hit-react/status animation behavior is not frozen.
- Confirm hit-react/status overrides return to idle/walk after the override or status ends. Death can remain in death behavior until release, but pooled reuse must reset to active idle/walk.
- Confirm pool reuse resets animation state and actor tick remains disabled.
- Runtime actor-tick-disabled proof must use a settled mechanism:
  - Preferred non-source mechanism: run a bounded staged smoke with Unreal's built-in `DumpTicks` console command after lightweight mobs are present, then parse the smoke log to prove no `AT66MobBase` actor tick function and no lightweight-family component tick function is registered/enabled. Record the raw command line, raw `DumpTicks` log excerpt/path, and the exact parsing rule used so actor-vs-component tick interpretation is auditable.
  - If `DumpTicks` cannot be run after mobs are present through existing command/automation paths, halt before adding any verification hook and ask Pablo whether to approve a small non-shipping one-shot verification command. That hook is a source change, requires rebuild/stage, and invalidates any prior Stage 0 binary for final closure unless Stage 0 is rerun on the new binary.
- Multi-frame animation proof must be concrete. Default to an Unreal-owned screenshot sequence showing changed VAT poses over at least 3 samples per sampled family/clip. A bounded sampled `Frame` scalar report is allowed only if an existing non-source automation path can sample it, or Pablo approves a one-shot non-shipping verification hook. Samples must be separated by at least `0.10s` and must show an observed pose or `Frame` value change for active non-death clips. A single still image or three same-frame samples are not enough.
- Save evidence under `C:\UE\T66\Saved\Codex\Performance\B11_B12_TickRemoval\`.
- Save log to `C:\UE\T66\Saved\StandaloneLogs\T66_B11_B12_TickRemoval_Smoke.log`.

### Documentation

Append `Pass B.11+B.12 VAT-into-Manager and Tick Disable` to:

- `C:\UE\T66\PerformanceSystem\2026-05-23_T66_LightweightActor_Plan.md`

Include:

- Live-code correction: B.11/B.12 behavior already manager-driven/tick-disabled before this pass.
- Stage 0 CVar-off/CVar-on table and B.10 closure status.
- Any B.11 cleanup change, or explicit no-code verification rationale.
- Any B.12 tick/component change, or explicit no-code verification rationale.
- Component-tick audit table and source/runtime evidence.
- Smoke evidence and artifact paths.
- Binary hash start/end and per-capture stability.
- Updated baseline table entry: current authoritative lightweight baseline.
- Results of the narrow docs sweep, with doc updates applied only where an existing doc asserts stale VAT/tick ownership.
- Placed guardian handling as a concrete summary column or note in the Stage 0 capture table: `BossOrGuardianObserved` / route-attribution counts are expected only for placed guardian behavior and excluded from route-validity rejection. Expected count per run is `0` or `1` depending on the autocapture floor route; any random miniboss promotion remains a failure.

Update pending issues:

- `C:\UE\T66\Source\T66\Gameplay\pending_issues_Gameplay.md`: close or update B.10 acceptance blocker if Stage 0 passes.
- `C:\UE\T66\PerformanceSystem\pending_issues_PerformanceSystem.md`: add any capture-hygiene observations if encountered.

## Files In Scope

Potential source edit scope only if the reviewed plan, component audit, and user go-ahead require it:

- `C:\UE\T66\Source\T66\Gameplay\T66MobBase.h`
- `C:\UE\T66\Source\T66\Gameplay\T66MobBase.cpp`
- `C:\UE\T66\Source\T66\Gameplay\T66MobManagerSubsystem.h`
- `C:\UE\T66\Source\T66\Gameplay\T66MobManagerSubsystem.cpp`

Documentation scope:

- `C:\UE\T66\PerformanceSystem\2026-05-23_T66_LightweightActor_Plan.md`
- `C:\UE\T66\Source\T66\Gameplay\pending_issues_Gameplay.md`
- `C:\UE\T66\PerformanceSystem\pending_issues_PerformanceSystem.md`

Validation artifacts:

- `C:\UE\T66\Saved\Codex\Performance\B11_B12_TickRemoval\`
- `C:\UE\T66\Saved\StandaloneLogs\T66_B11_B12_TickRemoval_Smoke.log`

## Out Of Scope

- B.13 mob HISM rendering.
- VAT per-instance custom data.
- Rich `AT66EnemyBase` changes.
- Miniboss, boss, special changes.
- Hero, companion, trap, or unique-debuff projectile systems.
- Mini/minigame systems.
- Reintroducing actor tick to fabricate an isolation measurement.
- New per-frame diagnostic logging.
- Ranged cadence or combat balance tuning.

## Risks And Mitigations

- Risk: The original B.11/B.12 expected FPS isolation is not possible because the code is already in the post-B.12 state.
  Mitigation: Do not manufacture a comparison. Document the live-state correction and use Stage 0 as the authoritative current lightweight baseline.

- Risk: Moving VAT state from actor fields into manager-owned state could create churn without measurable value before B.13.
  Mitigation: Default it out of this pass. Implement it only if a concrete residual per-frame VAT responsibility is found outside the manager tick, or Pablo explicitly expands scope to require physical manager-owned state records now.

- Risk: Visual smoke could miss frozen animation if it relies on a single screenshot.
  Mitigation: require an Unreal-owned screenshot sequence or bounded sampled `Frame` scalar report with at least three samples separated by `0.10s` and observed pose/frame change for active non-death clips.

- Risk: Git/LFS or staged-binary drift contaminates captures.
  Mitigation: reuse current B.10.1D hygiene gates: clean environment, binary hashes, and Git/LFS isolation for FPS acceptance rows.

- Risk: unrelated dirty worktree changes contaminate the Stage 0 baseline binary.
  Mitigation: classify every runtime-relevant modified/deleted path before staging; halt before staging if unrelated runtime-affecting or unknown changes would enter the closure binary without explicit approval.

## Rollback Considerations

- If no source change is made, rollback is documentation/artifact-only.
- If optional VAT state cleanup or a verification hook is made and visual smoke fails, revert only that cleanup/hook before rerunning Stage 0/verification.
- Do not revert unrelated user or generated changes in the worktree.
- Do not delete existing B.10/B.10.1D runner artifacts.

## Reviewer Questions

1. Given the live evidence that actor tick is already disabled and VAT is already advanced from `UT66MobManagerSubsystem::Tick`, is this verification-first revision safer than implementing the original staged plan literally?
2. Is the packet's B.11 intent resolution sound: the plan wording requires manager-owned VAT frame/update execution, so physical VAT field relocation is out of default scope unless Pablo explicitly expands it?
3. Are the Stage 0 acceptance gates sufficient to close the open B.10 lightweight baseline after projectile manager and placed miniboss changes?
4. Is any additional component-tick proof required beyond source audit plus staged visual smoke?

## Required Reviewer Output

The first non-empty line must be exactly one of:

`Verdict: APPROVE`

`Verdict: REVISE`

`Verdict: BLOCK`

Please review for flawed assumptions, unsafe scope, missing files, inadequate verification, contradictions with repo instructions, and whether the live-code correction is sufficiently supported.
