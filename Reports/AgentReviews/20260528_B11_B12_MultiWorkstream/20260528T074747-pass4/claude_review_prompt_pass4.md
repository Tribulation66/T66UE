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
# B.11/B.12 Multi-Workstream Consolidated Plan Packet

Date: 2026-05-28

Output scope: review of this plan packet only. No implementation, staging, capture, or source edit is authorized until this packet receives an external review greenlight and Pablo gives explicit go-ahead.

## Working Goal

Run a coordinated multi-workstream pass where the main Codex agent orchestrates one reviewed plan, performs the single-binary FPS measurement itself, delegates only disjoint implementation work, and returns one combined completion packet.

## Codex Live-Repo Position

The earlier B.11/B.12 premise has changed because live code already has manager-driven VAT advancement and disabled `AT66MobBase` actor tick:

- `C:\UE\T66\Source\T66\Gameplay\T66MobBase.cpp:79` sets `PrimaryActorTick.bCanEverTick = false`.
- `C:\UE\T66\Source\T66\Gameplay\T66MobBase.cpp:80` sets `PrimaryActorTick.bStartWithTickEnabled = false`.
- `C:\UE\T66\Source\T66\Gameplay\T66MobBase.cpp:897` calls `SetActorTickEnabled(false)` during reuse reset.
- `C:\UE\T66\Source\T66\Gameplay\T66MobManagerSubsystem.cpp:678` disables actor ticking for runtime active-state changes.
- `C:\UE\T66\Source\T66\Gameplay\T66MobManagerSubsystem.cpp:2149`, `2170`, `2185`, `2192`, `2199`, `2206`, `2218`, and `2228` call `Mob->TickMobVertexAnimationState(DeltaTime)` from the manager tick branch structure.

The real pending structural work is narrower and more precise: VAT state is still actor-resident in `C:\UE\T66\Source\T66\Gameplay\T66MobBase.h:268-276` (`ActiveMobVertexAnimationRow`, `ActiveMobVertexAnimationMID`, `ActiveMobVertexAnimationClip`, `MobVertexAnimationClipTime`, `MobVertexAnimationOverrideSecondsRemaining`, `bUsingMobVertexAnimation`). This pass moves that VAT state into manager-owned flat data laid out for B.13 custom-data consumption, without doing B.13 HISM mob rendering.

Codex's implementation opinion: do not re-enable actor tick or fabricate a historical A/B. Measure the current pre-change binary as Stage 0a, move the remaining VAT state ownership, merge independent cleanup/verification work, then measure the combined binary as Stage 0b.

## Applicable Instructions Read

- Root `C:\UE\T66\AGENTS.md` supplied in this conversation: working goal, folder instruction discovery, plan packet, Claude review, Pablo go-ahead, pending issue updates, no destructive cleanup.
- `C:\UE\T66\PerformanceSystem\PERFORMANCE_SYSTEM_AGENTS.md`: performance/diagnostic work starts from PerformanceSystem contracts and uses PerformanceSystem evidence.
- `C:\UE\T66\Gameplay\GAMEPLAY_AGENTS.md`: Gameplay runtime changes require `Gameplay/README.md`, owning docs, compile/build proof, and staged standalone validation when playable behavior changes.
- `C:\UE\T66\Gameplay\README.md`: Gameplay docs ownership; Mini/minigames are isolated and out of scope.
- `C:\UE\T66\Gameplay\World\WORLD_AGENTS.md` and `C:\UE\T66\Gameplay\World\T66_MAP_DESIGN_REFERENCE.md`: tower floors are `1` start, `2-4` combat, `5` boss; placed slime guardians gate floors `2`, `3`, `4`.
- `C:\UE\T66\Gameplay\Combat\MASTER_COMBAT.md`: basic mob and boss projectiles now flow through `UT66ProjectileManagerSubsystem`; boss targeting has separate registry paths.
- `C:\UE\T66\Gameplay\Traps\MASTER_TRAPS.md`: trap projectile owner is the wall-arrow trap path, not a generic `T66TrapProjectile` file.
- `C:\UE\T66\UI\UI_AGENTS.md`: UI changes must stay surgical; no fidelity-loop requirement applies because the Gambler item is gameplay ownership cleanup, not reference-image UI work.
- `C:\UE\T66\Reports\AGENTS.md`: review packets belong under `Reports/AgentReviews`.
- `C:\UE\T66\Source\T66\Gameplay\pending_issues_Gameplay.md`: B.10 remains open until a post-projectile-manager, post-placed-miniboss CVar-on acceptance passes and the staged SHA is recorded.
- `C:\UE\T66\PerformanceSystem\pending_issues_PerformanceSystem.md`: PerformanceSystem overhead is mitigated but binary provenance and Git/LFS contamination remain standing capture hygiene concerns.
- `C:\UE\T66\PerformanceSystem\Miniboss_Special_Boss_Spawn_and_Integration_Audit.md`: identifies the registry, Gambler, and trap-log cleanup gaps folded into this pass.

## User Constraints

- One consolidated plan packet, one combined completion packet.
- Plan packet must pass Claude review and then Pablo go-ahead before implementation.
- Main agent owns all FPS measurement on a single stable staged binary. Measurement is not delegated.
- Parallel implementation is allowed only on disjoint file sets. If a file is needed by two workstreams, merge or serialize that edit.
- `T66PlayerController_Overlays.cpp` must be assigned once. This packet assigns it to main-agent serialized/shared-hook scope, not to any parallel workstream.
- Classify the current dirty worktree before any staging and record an explicit per-path Pablo decision. Do not silently include unrelated runtime changes in Stage 0a.
- B.11/B.12 are not reimplemented as actor-tick disable work. Actor tick is already disabled; this pass moves VAT state ownership.
- No B.13 mob HISM rendering or per-instance custom data application.
- HeroHPOverride is `20000` for FPS captures.
- Aggregate counters only; no per-frame per-mob diagnostic logging.
- Stage 0a CVar-on median becomes the authoritative lightweight baseline and closes B.10 only if the gate passes under the recorded source/staged-binary provenance.

## PPF Check

Objective: Move lightweight-mob VAT state ownership into manager data while preserving the existing VAT animation method and runtime behavior.

Proven process: Existing T66 lightweight-mob VAT path: data rows from `Content\Data\MobVertexAnimations.csv`, static mesh plus VAT material applied by `UT66CharacterVisualSubsystem::ApplyMobVertexAnimationVisual`, material parameters `StartFrame`, `EndFrame`, and `Frame`, and clip selection based on lightweight mob motion/status/death state.

My planned implementation: Keep the same VAT assets, row schema, material, clips, frame math, and dynamic material parameter writes. Move only state ownership and advancement helpers from `AT66MobBase` into `UT66MobManagerSubsystem` manager-owned flat records.

Same method class: YES.

If NO, why: Not applicable.

User approval required before proceeding: YES, after Claude review.

Verification evidence: source audit, focused compile, staged standalone smoke, multi-frame VAT proof, runtime no-tick proof, pool-reuse reset proof, Stage 0a/0b PerformanceSystem captures with binary hashes.

## Artifact Parity Gate

Reference artifact/category: Existing lightweight-mob VAT visuals.

Role: Primary.

Required: YES.

Planned artifact/path: Existing data/assets referenced through `Content\Data\MobVertexAnimations.csv` and `UT66CharacterVisualSubsystem::ApplyMobVertexAnimationVisual`. No new visual assets.

Status: SAME.

Evidence: live source shows the current actor stores VAT row/MID/clip state but the visual application still comes from `UT66CharacterVisualSubsystem`; this pass does not change the asset/material contract.

## Mechanism Manifest

Reference/source: current `AT66MobBase` VAT helper plus manager tick branch structure.

Required mechanisms:

1. Mechanism: Clip selection.
   Required: YES.
   Planned implementation: Preserve idle/move selection from stored velocity and explicit attack/hit/death/status override clips; the manager state owns the current clip.
   Evidence needed: smoke proof for idle, move, attack, hit/status, and death samples across all four lightweight families.

2. Mechanism: Frame advancement.
   Required: YES.
   Planned implementation: Preserve `DeltaSeconds * SampleRate * PlayRate` frame math and clip-range wrapping inside a manager-owned tick helper.
   Evidence needed: at least three samples at least `0.10s` apart per sampled family/clip showing pose or `Frame` change for active non-death clips.

3. Mechanism: Material parameter application.
   Required: YES.
   Planned implementation: Continue to write `StartFrame`, `EndFrame`, and `Frame` to each mob's dynamic material instance from the manager loop. Do not switch to HISM custom data in this pass.
   Evidence needed: source proof and staged visual smoke after the refactor.

4. Mechanism: Pool reuse reset.
   Required: YES.
   Planned implementation: Register/reset manager VAT state on acquire/configure/reuse/release so reused mobs start from the correct clip/frame and do not carry stale overrides.
   Evidence needed: pooled mob reuse proof with reset frame/clip evidence.

5. Mechanism: Runtime tick absence.
   Required: YES.
   Planned implementation: Keep actor tick disabled and prove no lightweight actor/component tick is enabled while mobs are live.
   Evidence needed: `DumpTicks` or Pablo-approved one-shot hook evidence plus runtime `IsActorTickEnabled()` values.

Anti-lookalike discriminator: a single still image can hide a fixed VAT frame. The discriminator is multi-frame evidence plus source/runtime proof that the advancing `Frame` value is manager-owned while actor/component ticks stay disabled.

## Orchestration Model

1. Main Codex writes this single packet and submits it to Claude review.
2. Main Codex stops for Pablo go-ahead after review approval.
3. After go-ahead, main Codex classifies the dirty worktree before staging.
4. Main Codex stages and measures Stage 0a on the pre-workstream source state.
5. Main Codex launches parallel implementation agents only after Stage 0a passes and file ownership is confirmed.
6. Sub-agents are implementation-only. They do not run FPS acceptance sets.
7. Main Codex integrates the sub-agent outputs, resolves build issues, produces one combined staged binary, records SHA256, and runs Stage 0b plus smokes.
8. Main Codex updates docs/pending issues and produces one combined packet.

If no sub-agent/delegation tool is available at implementation time, main Codex must report that and ask whether Pablo wants the disjoint workstreams serialized. It must not silently replace the requested orchestration model.

## File Ownership Map

Documentation and measurement files are main-agent owned. Sub-agents may not edit plan docs, pending issues, capture runners, or shared final packet docs unless explicitly delegated by main after integration.

### Main-Agent-Only Files/Artifacts

- `C:\UE\T66\PerformanceSystem\2026-05-23_T66_LightweightActor_Plan.md`
- `C:\UE\T66\Source\T66\Gameplay\pending_issues_Gameplay.md`
- `C:\UE\T66\PerformanceSystem\pending_issues_PerformanceSystem.md`
- `C:\UE\T66\Reports\AgentReviews\20260528_B11_B12_MultiWorkstream\*`
- capture/stage runners and output folders under `C:\UE\T66\Saved\Codex\Performance\...`
- staged binary and hash/provenance artifacts

### Workstream 1: VAT State Into Manager-Owned Data

Exclusive source files:

- `C:\UE\T66\Source\T66\Gameplay\T66MobBase.h`
- `C:\UE\T66\Source\T66\Gameplay\T66MobBase.cpp`
- `C:\UE\T66\Source\T66\Gameplay\T66MobManagerSubsystem.h`
- `C:\UE\T66\Source\T66\Gameplay\T66MobManagerSubsystem.cpp`

No other workstream may edit these files.

### Workstream 2: Deferred Gameplay Verification

Exclusive source files:

- `C:\UE\T66\Source\T66\Gameplay\GameMode\T66GameMode_Tower.cpp`
- `C:\UE\T66\Source\T66\Gameplay\T66TowerDescentHole.h`
- `C:\UE\T66\Source\T66\Gameplay\T66TowerDescentHole.cpp`
- `C:\UE\T66\Source\T66\Gameplay\T66ProjectileManagerSubsystem.h`
- `C:\UE\T66\Source\T66\Gameplay\T66ProjectileManagerSubsystem.cpp`
- `C:\UE\T66\Source\T66\Gameplay\T66BossBase.cpp`

`T66PlayerController_Overlays.cpp` is not assigned here. Workstream 2 may inspect it and may rely on existing modes such as `T66BossProjectileSmokeKillMidFlight`, but may not edit it. If Workstream 2 proves that an overlay/autocapture hook edit is unavoidable, it must stop and return a request for a serialized main-agent hook change.

### Workstream 3: Minor Cleanup

Exclusive source files:

- `C:\UE\T66\Source\T66\Core\T66ActorRegistrySubsystem.h`
- `C:\UE\T66\Source\T66\Core\T66ActorRegistrySubsystem.cpp`
- `C:\UE\T66\Source\T66\UI\Gambler\T66CasinoGamblerTabWidget_Economy.cpp`
- `C:\UE\T66\Source\T66\Gameplay\Traps\T66TrapArrowProjectile.h`
- `C:\UE\T66\Source\T66\Gameplay\Traps\T66TrapArrowProjectile.cpp`

The user prompt named `T66TrapProjectile.h/.cpp`, but the live trap projectile owner is `T66TrapArrowProjectile.h/.cpp`; this packet uses the live file names.

### Conflict Policy

- No sub-agent may touch a file outside its assigned set without stopping.
- `C:\UE\T66\Source\T66\Gameplay\T66PlayerController_Overlays.cpp` is main-agent serialized/shared-hook scope, not a parallel sub-agent file. Workstream 2 may request an automation-hook change there; Workstream 3 may request a Gambler controller-path change there; main applies any approved overlay edit after the disjoint workstreams complete.
- If Workstream 3 discovers that boss damageable-target changes require `T66CombatComponent.cpp` or HUD/minimap files, it must document that as out of scope or request serialization. It must not expand its file set silently.

## Phase 0: Review And Go-Ahead Gate

Run Claude review through `Scripts\Invoke-ClaudePlanReview.ps1` after verifying `ANTHROPIC_API_KEY` is absent from Process/User/Machine scopes. Revise until the first non-empty reviewer line is exactly `Verdict: APPROVE` and no Blocker/Major issues remain. Then report the packet and review artifact paths to Pablo and wait for explicit go-ahead.

## Phase 1: Worktree Classification And Stage 0a Baseline

### 1.1 Dirty Worktree Classification

After go-ahead and before any staging:

- Avoid broad Git/LFS-heavy scans over `Content`, `SourceAssets`, and staged outputs.
- Use narrow status/diff checks for the files relevant to this pass and for the dirty categories already flagged by prior review: weapon data, RunState combat, deleted Content assets, and runtime Config.
- Record a table with each modified/deleted/untracked path that could affect the Stage 0a binary.
- Store that table as a durable artifact under `C:\UE\T66\Reports\AgentReviews\20260528_B11_B12_MultiWorkstream\worktree_classification.md` before staging so Pablo's per-path decisions live next to this reviewed packet.
- The classification table must include deleted `Content/...` paths returned by the narrow status checks. Do not omit them just because they are likely non-source; classify them as runtime-affecting, non-runtime, generated, contamination, or unknown with evidence.
- The classification table must explicitly include any dirty `C:\UE\T66\Source\T66\Core\RunState\T66RunStateSubsystem_Combat.cpp`, `C:\UE\T66\Content\Data\Weapons.csv`, `C:\UE\T66\Content\Data\DT_Weapons.uasset`, and runtime `Config\...` files if present.
- Classify each path as:
  - Intended for this pass
  - Pre-existing but intentionally included in the measurement source state
  - Contamination / must not be included
  - Report/doc only
  - Generated artifact / ignored for source measurement
  - Unknown / requires Pablo decision
- Halt before staging if any runtime-affecting or unknown path lacks a Pablo decision.
- Do not revert, stash, clean, reset, or discard user changes unless Pablo explicitly approves that exact path action.

The Stage 0a baseline must be documented as a source-provenance set, not as an abstract "current main" claim.

### 1.2 Stage Current Pre-Change Binary

After worktree classification is accepted:

- Build/stage the current source before workstream changes.
- Record staged `T66.exe` SHA256, mtime, and length.
- Record source file SHA/mtime for the in-scope systems, including projectile manager and placed-miniboss files.
- Verify clean environment before each capture: no `RunUAT`, `UnrealEditor-Cmd`, staged `T66.exe`, or Git/LFS scan overlap during the Unreal process.

### 1.3 Stage 0a Captures

Run:

- CVar-off: 3 accepted captures, `T66.Mob.UseLightweight=0`, `T66.AutoCapture.HeroHPOverride=20000`, standard `enemywaveperf`.
- CVar-on: 3 accepted captures, `T66.Mob.UseLightweight=1`, `T66.AutoCapture.HeroHPOverride=20000`, standard `enemywaveperf`.

Capture policy:

- `PerformanceSystemOverheadMaxUs > 10000` rejects a capture.
- first `HeroDeath` halts the set.
- halt set on 2+ rejects.
- binary hash recorded before/after each capture and at pass end.
- no per-frame diagnostics.
- placed slime guardian evidence (`BossOrGuardianObserved=0 or 1`) is expected depending on autocapture floor route and is not a route leak.
- any random miniboss promotion bucket, unexpected `RichSpawns`, or `BossOrGuardianObserved > 1` is a failure.

Acceptance:

- CVar-on median must be at least 95% of the current Stage 0a CVar-off median.
- Escalate to 10 if `abs(CVarOnMedian - (0.95 * CVarOffMedian)) <= 2 * max(CVarOnStdev, CVarOffStdev)`.
- If Stage 0a passes, mark B.10 as closure-eligible and update the plan baseline table after the later proof gates pass. Defer the actual `pending_issues_Gameplay.md` close edit until runtime tick proof and any hook-related rerun decision are complete.
- Record both CVar-off and CVar-on stdev in the Stage 0a output because Stage 0b uses the Stage 0a CVar-on stdev for the neutrality comparison.
- If Stage 0a fails, stop. Do not run sub-agent implementation and do not close B.10.

## Phase 2: Parallel Implementation Workstreams

### Workstream 1: VAT State Into Manager-Owned Data

Goal: move remaining VAT state off `AT66MobBase` and into `UT66MobManagerSubsystem` flat manager records while preserving current visuals and behavior.

Pre-implementation caller and boundary audit:

- Confirm actor-side `AT66MobBase` VAT APIs have no unplanned external edit surface before launching Workstream 1.
- Current source search shows `AT66MobBase::TickMobVertexAnimationState` call sites only in `T66MobManagerSubsystem.cpp` at the eight manager branches.
- Current source search shows `AT66MobBase::SetMobVertexAnimationClip`, `TryApplyMobVertexAnimationVisual`, and `GetMobVertexAnimationClipRange` are private actor helpers called only inside `T66MobBase.cpp`.
- Current source search shows external `ForceMobVertexAnimationClipForAutomation` call text in `T66MobManagerSubsystem.cpp` and `T66PlayerController_Overlays.cpp`, while similarly named rich `AT66EnemyBase` APIs appear in rich-enemy/test paths. Workstream 1 must preserve the `AT66MobBase::ForceMobVertexAnimationClipForAutomation` signature as a manager-delegating wrapper so these existing automation callers do not require edits outside W1/main-owned overlay scope.
- Workstream 1 must not edit `AT66EnemyBase.*`; rich enemy VAT helpers are a separate actor path and are out of scope.
- `UT66CharacterVisualSubsystem::ApplyMobVertexAnimationVisual` currently returns `OutMID` and `OutRow`; it does not write `AT66MobBase` actor fields. No `UT66CharacterVisualSubsystem.*` edit is planned. If implementation proves a visual-subsystem interface change is necessary, Workstream 1 must stop and request main-agent serialization or an expanded reviewed file set.

Implementation shape:

1. Add a manager-owned VAT state struct, for example `FT66ManagedMobVertexAnimationState`, in `T66MobManagerSubsystem.h`.
2. Store the values B.13 will need in a contiguous manager-owned array:
   - `TWeakObjectPtr<AT66MobBase> Mob`
   - `FT66MobVertexAnimationRow Row`
   - `TObjectPtr<UMaterialInstanceDynamic> MID`
   - `FName CurrentClip`
   - `int32 CurrentClipIndex`
   - `float ClipTime`
   - `float OverrideSecondsRemaining`
   - `float CurrentFrame`
   - `float StartFrame`
   - `float EndFrame`
   - `float PlayRate`
   - `float RowsPerFrame`
   - `bool bUsingVAT`
3. Keep a lookup from active mob to VAT state index. Compact or rebuild this lookup on register/unregister/release so the state array stays manager-owned and B.13-friendly.
4. Convert actor VAT setup into a registration/reset handshake:
   - `AT66MobBase::ApplyConfiguredVisual` may still call the visual subsystem and obtain a row/MID.
   - The row/MID/initial clip state must be handed to the manager.
   - `AT66MobBase` must not retain the old row/MID/clip/time/override/bUsingVAT fields.
5. Move clip-range lookup, clip set, override decrement, frame advancement, and `Frame`/`StartFrame`/`EndFrame` MID writes into manager helpers.
6. Replace manager calls to `Mob->TickMobVertexAnimationState(DeltaTime)` with manager-owned advancement calls.
7. Replace actor-side `SetMobVertexAnimationClip` uses for death/hit/status/automation with manager API calls, e.g. `ForceMobVertexAnimationClip(Mob, ClipName, OverrideSeconds)`.
8. Keep `ForceMobVertexAnimationClipForAutomation` as an actor-facing non-shipping convenience only if it delegates to the manager and stores zero state on `AT66MobBase`. The source audit must explicitly verify this symbol after the refactor.
9. If `ForceMobVertexAnimationClipForAutomation` cannot delegate without storing transient actor state, stop and report the design conflict; do not leave actor-resident VAT state as an implementation compromise.
10. Confirm the eight current manager tick branches still advance VAT at most once per active mob per tick.
11. Preserve `AT66MobBase` actor tick disabled and do not introduce any component tick.

Verification for Workstream 1 before integration:

- Focused compile succeeds.
- Source audit shows old actor-resident VAT fields removed.
- Source audit shows manager owns the VAT data and helper functions.
- Source audit classifies all VAT advancement call sites and proves at-most-one advancement.
- Source audit records the external VAT API caller enumeration and confirms no unexpected W1 file-set expansion occurred.
- Smoke evidence shows all four families animate through idle/move/attack-or-hit/death/status where practical.
- Pool reuse reset evidence shows manager VAT state resets for reused mobs.

Out of Workstream 1:

- B.13 HISM rendering.
- MID-to-per-instance-custom-data switch.
- Moving rush/flying/ranged/status behavioral state into arrays.
- Any rich enemy, boss, miniboss, special, UI, trap, or projectile edit.

### Workstream 2: Deferred Verification

Goal: close two deferred proof gaps using existing systems or bounded non-shipping automation in its assigned files.

Proof artifacts for Workstream 2 must land under `C:\UE\T66\Reports\AgentReviews\20260528_B11_B12_MultiWorkstream\proofs\workstream2\` unless the main agent assigns a more specific final packet proof folder.

Task A: Floors 3/4 miniboss runtime verification.

- Add or reuse a non-shipping traversal smoke that walks the normal tower floor chain through floors `2`, `3`, and `4`.
- Verify the floor-2 placed slime guardian exists on the floor-2 exit hole and blocks descent while alive.
- Verify the floor-3 placed slime guardian exists on the floor-3 exit hole and blocks descent while alive.
- Verify the floor-4 placed slime guardian exists on the floor-4 exit hole and blocks boss-floor descent while alive.
- Assert the `AT66TowerDescentHole` is blocked while the guardian is alive.
- Kill each guardian through the normal damage/death path or a tightly scoped automation-only kill helper.
- Assert the descent hole becomes open after death.
- Confirm floor `4` guardian gates the boss-floor entrance.
- Save log/screenshot evidence under `C:\UE\T66\Reports\AgentReviews\20260528_B11_B12_MultiWorkstream\proofs\workstream2\`.
- If a new traversal invocation hook is required, request a serialized main-agent edit to `T66PlayerController_Overlays.cpp`; Workstream 2 may not create or edit unlisted automation files.

Task B: Boss projectile kill-mid-flight source invalidation.

- Force or schedule a boss projectile so it is active when its source boss dies or becomes invalid.
- Prefer the existing `T66BossProjectileSmokeKillMidFlight` overlay smoke path without editing `T66PlayerController_Overlays.cpp`.
- The existing manager code already increments `DroppedInvalidSource` and deactivates a boss projectile when `SourceMob` is no longer a live `AT66BossBase`; the smoke must positively exercise that branch.
- Assert the projectile is dropped and no post-death hero damage is applied.
- If the positive exercise reveals a real bug, fix it within `T66ProjectileManagerSubsystem.*`/`T66BossBase.cpp` only and document the bug plus fix.

Constraints:

- No FPS captures.
- No `T66PlayerController_Overlays.cpp` edits.
- No new harness/test files unless this packet is revised or the main agent serializes the hook in the shared overlay file after a Pablo decision.
- No per-frame logging.
- No boss behavior redesign beyond test/fix for source invalidation.

### Workstream 3: Minor Cleanup

Goal: close minor audit gaps and reduce hot-path trap log noise within the assigned files.

Task A: Actor registry boss target and broadcast decision.

- Audit callers of `ForEachDamageableTarget`, `GetAllDamageableTargets`, `GetBosses`, and `OnEnemiesChanged`.
- Current source already has several boss-specific caller loops in combat/controller/game mode paths, so blindly adding bosses to `ForEachDamageableTarget` may duplicate boss target evaluation.
- Default outcome: document deliberate boss separation in `T66ActorRegistrySubsystem.h/.cpp` comments/API naming and leave bosses out of general damageable target iteration.
- Include bosses in the general damageable iteration only if the caller audit proves no duplicate target behavior, no behavior regression, and no extra file edits are needed.
- Add a boss-registration broadcast if it preserves separation cleanly, preferably a `BossesChanged` delegate. If no consumer should react today, document that in comments and the final packet.

Task B: Gambler boss spawn ownership.

- Centralize Gambler boss spawn ownership in the player-controller gameplay path.
- Make `UT66CasinoGamblerTabWidget::TriggerGamblerBossIfAngry` call the player-controller path only; remove the widget's direct fallback `SpawnActor<AT66GamblerBoss>` path.
- If the player-controller path needs the UI's casino NPC location behavior to avoid behavior regression, Workstream 3 must request a serialized main-agent change to `T66PlayerController_Overlays.cpp` rather than editing it directly.
- Removing the widget fallback must not create a silent no-op in any angry-Gambler state. If the controller path is unavailable from a menu/widget state that can currently trigger the boss, Workstream 3 must either extend the controller-owned path to support that state or stop and report the unresolved ownership decision.
- Do not create a third spawn owner.

Task C: Trap projectile log demotion.

- Demote routine `LogT66TrapProjectile` `[ProjectileFired]` and normal `[ProjectileImpact]` telemetry from `Log` to `VeryVerbose` in `T66TrapArrowProjectile.cpp`.
- Preserve warnings/errors and any truly exceptional logs.
- Ensure combat damage provenance still emits through the shared damage path when HP is actually removed.

Constraints:

- No Mini/minigame files.
- No broad UI redesign.
- No changes outside the assigned source files without stopping.

## Phase 3: Combined Binary Build, Stage 0b Measurement, And Verification

After all sub-agents complete:

1. Main Codex integrates and resolves conflicts.
2. Build one combined binary.
3. Stage it once.
4. Record staged `T66.exe` SHA256, mtime, length, and source provenance.
5. Run Stage 0b CVar-on only:
   - 3 accepted captures, `T66.Mob.UseLightweight=1`, `T66.AutoCapture.HeroHPOverride=20000`, standard `enemywaveperf`.
   - Same overhead, HeroDeath, reject, clean-environment, and binary-hash rules.
6. Compare Stage 0b CVar-on against Stage 0a CVar-on:
   - Expected neutral within noise.
   - Use `abs(Stage0bMedian - Stage0aCVarOnMedian) <= 2 * max(Stage0aOnStdev, Stage0bStdev)` as the neutrality band.
   - A small improvement is acceptable, especially from trap log demotion or cleaner capture logs.
   - A regression outside noise must be investigated before reporting completion. Workstream 1 is the only expected perf-affecting runtime change.

Runtime verification on the combined binary:

- Floors 3/4 traversal smoke from Workstream 2.
- Boss projectile kill-mid-flight positive smoke from Workstream 2.
- Runtime tick proof:
  - Prefer `DumpTicks` after lightweight mobs are live.
  - Attempt the `DumpTicks` route first. If `DumpTicks` cannot be invoked with existing automation, use a Pablo-approved one-shot non-shipping hook; adding such a hook after Stage 0a invalidates the closure binary and requires the relevant measurement to be rerun on the new combined binary.
  - If a one-shot hook is added after Stage 0a, the B.10 closure SHA must be replaced with the post-hook staged SHA after rerunning the closure measurement. Do not leave `pending_issues_Gameplay.md` pointing at a pre-hook closure binary.
  - Evidence must prove no `AT66MobBase` actor tick and no lightweight component tick while mobs are live.
- Multi-frame VAT proof:
  - At least three samples at least `0.10s` apart.
  - Sample all four families.
  - Minimum clip set per family: move plus either attack/hit/death where the family can be driven deterministically. At least one family must show death, and pool reuse must show reset.
  - Evidence can be Unreal-owned screenshot sequence and/or bounded `Frame` scalar dump from an existing or approved one-shot hook.

## Phase 4: Documentation And Combined Packet

Main Codex owns all final documentation:

- Append `Pass B.11+B.12 Multi-Workstream VAT Ownership and Verification` to `C:\UE\T66\PerformanceSystem\2026-05-23_T66_LightweightActor_Plan.md`.
- Update the plan baseline table with Stage 0a CVar-on as the current authoritative lightweight baseline if Stage 0a passed.
- Close B.10 acceptance in `C:\UE\T66\Source\T66\Gameplay\pending_issues_Gameplay.md` only after Stage 0a passes and the runtime tick proof/hook decision is complete. The inline SHA must be the final closure binary SHA, replacing Stage 0a if a later hook required rerunning closure measurement.
- Close or update floors 3/4 placed-miniboss proof gap.
- Close or update kill-mid-flight source-invalidation proof gap.
- Close or update registry boss query/broadcast gaps.
- Close or update Gambler dual-spawn gap.
- Close trap projectile hot-path log issue.
- Update `C:\UE\T66\PerformanceSystem\pending_issues_PerformanceSystem.md` with any capture hygiene observations, especially Git/LFS or binary drift.
- Produce one combined packet with:
  - orchestration summary
  - worktree classification table and Pablo decisions
  - file ownership map
  - Stage 0a baseline tables and B.10 closure
  - each workstream's implementation summary and verification
  - Stage 0b comparison
  - runtime tick proof
  - multi-frame VAT proof
  - floors 3/4 proof
  - kill-mid-flight proof
  - binary hashes and source provenance
  - unresolved caveats or deferred items

## Verification Gates

- Claude review greenlight before implementation.
- Pablo go-ahead after the reviewed packet.
- Dirty worktree classified and accepted before staging.
- Stage 0a passes before implementation starts.
- Focused compile after each workstream by default. If a per-workstream compile is genuinely impractical, record the exception and run the combined focused compile immediately after integration.
- Per-workstream focused compile is the default. If delegated tooling makes that impossible, the main agent must record an explicit exception before merging and then run the combined focused compile immediately after integration.
- One combined build/stage after integration.
- Stage 0b CVar-on neutral within noise against Stage 0a CVar-on.
- Runtime no-tick proof.
- Multi-frame VAT proof and pool-reuse proof.
- Floors 3/4 placed guardian proof.
- Kill-mid-flight source invalidation proof.
- Zero accepted captures with overhead over 10 ms.
- Zero HeroDeath rejects.
- Stable binary hash within each measurement set.

## Risks And Mitigations

- Risk: dirty runtime work contaminates Stage 0a.
  Mitigation: mandatory per-path classification before staging, halt on unknown runtime-affecting paths.

- Risk: Workstream 1 accidentally changes animation behavior while moving state.
  Mitigation: preserve VAT assets/material/frame math, require multi-frame proof and pool-reuse reset proof.

- Risk: disjoint workstreams need the same file.
  Mitigation: `T66PlayerController_Overlays.cpp` is assigned only to main-agent serialized/shared-hook scope; sub-agents request overlay changes instead of editing it.

- Risk: adding bosses to general target queries duplicates existing boss-specific combat loops.
  Mitigation: caller audit first; document separation unless inclusion is proven safe within the assigned files.

- Risk: Stage 0b regression is misattributed.
  Mitigation: cleanup work is non-perf by design; isolate Workstream 1 if Stage 0b regresses outside the noise band.

- Risk: Git/LFS scans or staged binary drift corrupt capture validity.
  Mitigation: standing clean-environment and per-capture hash gates remain hard requirements.

## Rollback Considerations

- Do not revert user changes or unrelated dirty files.
- If Workstream 1 visual smoke fails, revert only the VAT ownership refactor and keep diagnostic evidence.
- If Workstream 2 hook/fix breaks traversal or projectiles, revert only that workstream's files and document the proof gap still open.
- If Workstream 3 boss registry inclusion duplicates boss targeting, prefer reverting that inclusion and documenting explicit boss-query separation.
- Documentation updates are made only after verification and can be reverted independently if the pass halts before completion.

## Out Of Scope

- B.13 mob HISM rendering and per-instance custom data application.
- Retiring `UseLightweight` or per-family diagnostic CVars.
- Moving non-VAT behavioral state into manager arrays.
- Rich `AT66EnemyBase`, miniboss, boss, or special behavior changes except Gambler spawn ownership centralization and kill-mid-flight verification/fix.
- Unique Debuff projectiles into the manager.
- Hero projectile or trap projectile manager migration.
- Deleting deprecated `AT66EnemyProjectileBase` / `AT66BossProjectile` files.
- Mini/minigame systems.
- New per-frame diagnostic logging.

## Reviewer Questions

1. Is the file ownership map sufficiently disjoint, especially the explicit `T66PlayerController_Overlays.cpp` assignment to serialized main-agent scope?
2. Does the Stage 0a-before-implementation measurement correctly make B.10 closure-eligible, with the actual pending-issue close deferred until runtime tick proof/hook decisions are complete?
3. Is Workstream 1's VAT state move sufficiently constrained to manager-owned data without drifting into B.13 rendering?
4. Are the Workstream 2 and Workstream 3 scopes safe to parallelize given the stop/serialize rules?
5. Are the Stage 0b neutrality and runtime proof gates strong enough to catch regressions?

## Required Reviewer Output

The first non-empty line must be exactly one of:

`Verdict: APPROVE`

`Verdict: REVISE`

`Verdict: BLOCK`

Please review for flawed assumptions, missing files, unsafe parallelism, inadequate verification, contradictions with repo instructions, and whether this packet can be safely presented to Pablo for go-ahead.

</review_packet>
