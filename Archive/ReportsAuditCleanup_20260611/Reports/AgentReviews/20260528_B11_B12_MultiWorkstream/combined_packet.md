# B.11/B.12 Multi-Workstream Combined Packet

Date: 2026-05-28

## Working Goal

Run the reviewed lightweight-only multi-workstream pass: close B.10 on an absolute lightweight health baseline, divorce basic mobs from the rich routing path, move VAT state ownership into the manager, close deferred proof gaps, complete minor audit cleanup, and verify the final binary with one stable staged executable.

## Review Gate

- Plan packet: `Reports/AgentReviews/20260528_B11_B12_MultiWorkstream/plan_packet.md`
- Claude review artifact: `Reports/AgentReviews/20260528_B11_B12_MultiWorkstream/20260528T105634-pass4/claude_review_pass4.md`
- Verdict: `Verdict: APPROVE`
- Process note: `AGENTS.md` now routes a valid Claude `Verdict: APPROVE` to implementation after reporting the review conclusion and caveats. Manual Pablo approval is no longer required for routine Claude-approved implementation unless the packet or user explicitly holds it. Codex fallback approvals still require manual confirmation.

Accepted Claude caveats:

1. Confirm the real boss projectile-manager API before authoring the kill-mid-flight hook.
2. Confirm whether the Gambler widget has a status surface before relying on warning-only feedback.
3. Keep the 10 ms PerformanceSystem overhead rejection rule.

All three were satisfied:

- Boss proof used `UT66ProjectileManagerSubsystem::FireBossProjectile(const FT66ManagedProjectileFireParams& Params)`.
- Gambler UI has `SetStatus(...)`; the removed UI-owned spawn fallback now reports through that surface when no owning player controller exists.
- The final captures used `PerformanceSystemOverheadMaxUs > 10000` as the rejection threshold.

## Source-State Classification

The live tree contained unrelated runtime/content work, so measuring directly from the whole dirty tree would have mixed this pass with unrelated source changes. The pass used an isolated measurement tree:

- Live repo: `C:\UE\T66`
- Isolated source tree: `C:\UE\T66_B11B12_Worktree`
- Source-state classification: `Reports/AgentReviews/20260528_B11_B12_MultiWorkstream/source_state_classification.md`
- Phase 1 source manifest: `Reports/AgentReviews/20260528_B11_B12_MultiWorkstream/phase1_source_manifest.csv`
- Live integration manifest: `Reports/AgentReviews/20260528_B11_B12_MultiWorkstream/isolated_to_live_copy_manifest.csv`

Before live integration, every copied live file either matched the isolated pre-change manifest or was clean in Git. No unrelated live dirty paths were overwritten.

The Stage 0a halt diagnosis changed the measurement contract:

- Rich basic-mob Ranged remains intermittent because the deprecated rich CMC path can fail to reach firing distance.
- Basic mobs are now committed to lightweight unconditionally.
- Rich-vs-lightweight A/B is retired for basic mobs.
- Measurement is now a lightweight-only absolute FPS health check plus before/after neutrality for this pass.
- Minibosses, specials, guardians, and bosses remain rich intentionally.

## File Ownership

| Workstream | Owner | Files |
| --- | --- | --- |
| Rich basic-mob divorce | Dirac | `Source/T66/Gameplay/T66EnemyDirector.cpp` |
| VAT state ownership | Main | `Source/T66/Gameplay/T66MobBase.h/.cpp`, `Source/T66/Gameplay/T66MobManagerSubsystem.h/.cpp` |
| Deferred proof hooks | Faraday + Main exit patch | `Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp`, `Source/T66/Gameplay/T66TowerDescentHole.h/.cpp`, `Source/T66/Gameplay/T66ProjectileManagerSubsystem.h/.cpp` |
| Minor cleanup | Herschel + Main Gambler edit | `Source/T66/Core/T66ActorRegistrySubsystem.h/.cpp`, `Source/T66/Gameplay/Traps/T66TrapArrowProjectile.h/.cpp`, `Source/T66/UI/Gambler/T66CasinoGamblerTabWidget_Economy.cpp` |

`T66PlayerController_Overlays.cpp` was intentionally not edited because the live file had drift from the isolated baseline. The Gambler cleanup was kept in the widget file instead.

## Phase 1 Lightweight Baseline

Staged executable:

- SHA256: `86EDE7D6F2533614D9E0525305230BC06CD468F1547642A47C6A2B5C1613C9F5`
- Report: `Reports/AgentReviews/20260528_B11_B12_MultiWorkstream/phase1_lightweight_baseline.md`

| Metric | Value |
| --- | ---: |
| Accepted captures | 3/3 |
| Median FPS | 192.80 |
| Mean FPS | 193.71 |
| Stdev FPS | 5.80 |
| Max PerformanceSystem overhead | 1011.4 us |
| Hero deaths / rejects / overhead rejects | 0 / 0 / 0 |

| Run | Avg FPS | Overhead us | Fired | Hit hero | Hero HP | Peak lightweight | Peak rich |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | 199.91 | 837.0 | 45 | 45 | 19560 | 90 | 0 |
| 2 | 192.80 | 1011.4 | 26 | 26 | 19580 | 90 | 0 |
| 3 | 188.41 | 920.2 | 63 | 63 | 19400 | 90 | 0 |

This closes B.10 on the lightweight-only contract. Basic Ranged is reliable through the lightweight/projectile-manager path; the deprecated rich basic-mob path is no longer a measurement comparator.

## Implementation Summary

### Rich Basic-Mob Divorce

- Basic Melee/Rush/Flying/Ranged now route to `AT66MobBase` unconditionally.
- `T66.Mob.UseLightweight`, `T66.Mob.Diagnostics.RouteFlyingLightweight`, and `T66.Mob.Diagnostics.RouteRangedLightweight` no longer affect basic-mob routing.
- The rich basic-mob routing branch is marked deprecated and preserved for later cleanup.
- Minibosses, specials, guardians, and bosses still route rich.
- Lightweight acquire failure no longer falls back to rich for basic mobs; the spawn is skipped because basic-rich fallback is retired.

### VAT State Ownership

- Removed actor-owned VAT runtime fields from `AT66MobBase`.
- Manager now owns `FT66MobVertexAnimationRuntimeState` as flat per-mob state:
  - `ClipIndex`
  - `ClipTime`
  - `PlayRate`
  - `OverrideSecondsRemaining`
  - `bUsingVertexAnimation`
  - `CustomDataFrame`
  - `CustomDataStartFrame`
  - `CustomDataEndFrame`
  - `CustomDataClipIndex`
  - `CustomDataPlayRate`
  - `CustomDataFlags`
- Manager tick advances VAT once per active mob per tick.
- Dynamic material instance application remains for this pass; B.13 owns HISM custom-data application.
- `T66.Mob.Diagnostics.UseTouchDamageOverlap` is neutralized and marked deprecated.
- Lightweight mob component ticks are explicitly disabled on construction and pool reuse for the capsule, visual mesh, lock indicator widget, body hit zone, and head hit zone.

### Deferred Proof Hooks

- Added non-shipping miniboss traversal proof:
  - `-T66GameplayAutoCapture=MinibossTraversalProof`
- Added non-shipping boss projectile kill-mid-flight proof:
  - `-T66GameplayAutoCapture=BossProjectileKillMidFlightProof`
- Both hooks now request process exit after terminal summary so automation does not hang.

### Minor Cleanup

- Bosses now participate in actor-registry damageable-target iteration.
- Boss registration/unregistration broadcasts `EnemiesChanged`.
- Gambler boss spawn ownership is centralized through `AT66PlayerController::TriggerCasinoBossIfAngry`.
- Routine trap arrow fire/impact logs are demoted to `VeryVerbose`.

## Final Binary Verification

Final staged executable:

- Path: `C:\UE\T66_B11B12_Worktree\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
- SHA256: `BD1F3BFB6AE27000684F0980FB5EA4FB356D2B94C3941278F57CAA43826B6E33`
- Last write UTC: `2026-05-28T15:00:48.2823967Z`
- Size: `311122944` bytes

### Runtime Tick and VAT Proof

Log:

- `C:\UE\T66_B11B12_Worktree\Saved\StandaloneLogs\T66_B11B12_MobTickVatRuntimeProof_final_after_component_fix.log`

Summary:

```text
[MobTickVatRuntimeProofSummary] Terminal=1 ActiveMobs=16 FamiliesPresent=4 ActorTickEnabled=0 ComponentTickEnabled=0 ComponentTickRegistered=0 ClipSamples=16 ClipSamplesWithFrameChange=16 PoolReuseResetChecks=4 PoolReuseResetPasses=4 ClassMap=Melee/Rush/Flying/Ranged:AT66MobBase DataSource=Enemies.csv+T66EnemyDirectorBasicRouting Pass=1
```

Result: pass.

### Miniboss Traversal Proof

Log:

- `C:\UE\T66_B11B12_Worktree\Saved\StandaloneLogs\T66_B11B12_MinibossTraversalProof_final_after_component_fix.log`

Summary:

```text
[MinibossTraversalProofSummary] Terminal=1 Floors=2->3->4 Floor2GuardianSpawned=1 Floor2BlockedWhileAlive=1 Floor2UnblockedAfterDeath=1 Floor2InteractAfterDeath=1 Floor3GuardianSpawned=1 Floor3BlockedWhileAlive=1 Floor3UnblockedAfterDeath=1 Floor3InteractAfterDeath=1 Floor4GuardianSpawned=1 Floor4BlockedWhileAlive=1 Floor4UnblockedAfterDeath=1 Floor4InteractAfterDeath=1 Pass=1
```

Result: pass.

### Boss Projectile Kill-Mid-Flight Proof

Log:

- `C:\UE\T66_B11B12_Worktree\Saved\StandaloneLogs\T66_B11B12_BossProjectileKillMidFlightProof_final_after_component_fix.log`

Summary:

```text
[BossProjectileKillMidFlightProofSummary] Terminal=1 FireBossProjectile=1 SourceDestroyed=1 ActiveAfterFire=1 FixedTicks=4 DroppedInvalidSource=1 DroppedInvalidSourceDelta=1 ProjectilesHitHeroBefore=0 ProjectilesHitHeroAfter=0 PostDeathHitDelta=0 HeroHPBefore=100.0 HeroHPAfter=100.0 PostDeathDamage=0 Pass=1
```

Result: pass.

### Phase 3 Lightweight Health Check

Results:

- `C:\UE\T66_B11B12_Worktree\Saved\Codex\Performance\LightweightActorB10_1D\capture_results.json`
- `C:\UE\T66_B11B12_Worktree\Saved\Codex\Performance\LightweightActorB10_1D\capture_rows.jsonl`

| Metric | Value |
| --- | ---: |
| Accepted captures | 3/3 |
| Median FPS | 200.99 |
| Mean FPS | 192.04 |
| Stdev FPS | 16.89 |
| Max PerformanceSystem overhead | 911.8 us |
| Total projectiles fired / hit hero | 22 / 22 |
| Hero deaths / rejects / overhead rejects | 0 / 0 / 0 |

| Run | Avg FPS | Overhead us | Fired | Hit hero | Hero HP | Peak lightweight | Peak rich | Route validity |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| 1 | 172.56 | 716.7 | 11 | 11 | 19800 | 88 | 2 | Pass |
| 2 | 202.58 | 805.9 | 9 | 9 | 19820 | 88 | 2 | Pass |
| 3 | 200.99 | 911.8 | 2 | 2 | 19960 | 90 | 0 | Pass |

Phase 3 median is `+8.19 FPS` over Phase 1 (`200.99` vs `192.80`) and does not regress. The wider Phase 3 stdev is noted but did not create a borderline failure.

`RouteLeaksObserved=3` in the runner summary is the expected placed guardian/non-director rich actor bucket. Each row kept `RouteValid=true`, `CounterMismatch=0`, `LightweightAcquireFailed=0`, and all Ranged basic mobs routed lightweight.

## Live Integration

Verified source edits were copied from the isolated tree into live `C:\UE\T66` after targeted manifest checks.

Live editor build:

```text
C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex -NoHotReload
Result: Succeeded
```

Existing unrelated warning:

- `Source\T66\Gameplay\T66Hero1AxeAOEVFXLabActor.cpp`: deprecated Niagara `FNiagaraEmitterInstance::IsReadyToRun`.

Existing cook/stage noise observed in the isolated staging arc:

- Missing `/Game/World/Tower/Textures/T_TowerDescentGate_Closed`.

## Documentation Updates

Updated:

- `PerformanceSystem/2026-05-23_T66_LightweightActor_Plan.md`
- `Source/T66/Gameplay/pending_issues_Gameplay.md`
- `PerformanceSystem/pending_issues_PerformanceSystem.md`
- `AGENTS.md`

Closed or updated:

- B.10 Ranged/basic-mob acceptance: closed on lightweight-only baseline with staged SHA recorded.
- Random rich-basic route comparator: retired; rich basic path deprecated.
- Floors 3/4 miniboss runtime proof gap: closed.
- Kill-mid-flight boss projectile source invalidation proof gap: closed.
- Trap projectile routine log issue: closed.
- Boss registry damageable-target/broadcast minor gap: closed by implementation.
- Gambler boss dual spawn path: closed by centralizing spawn ownership.

## Remaining Scope

- B.13: mob HISM rendering and per-instance custom data application.
- Cleanup pass: delete deprecated rich-basic routing branches/CVars and deprecated projectile actor code after final rendering validation.
- Unique Debuff, hero projectiles, trap projectile manager work, and human roster review remain separate future passes.
