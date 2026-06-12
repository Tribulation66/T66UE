# Claude Review Packet: Miniboss / Special / Boss Spawn and Integration Audit

## Working Goal

Audit and document the current spawn logic and infrastructure integration for minibosses, specials, and bosses, so a later reviewed pass can restructure their spawning deliberately. Documentation only; no code changes, fixes, spawn-rule changes, or enemywaveperf changes.

## User Constraints

- Audit only. Do not modify production code, data, scripts, CVars, capture harnesses, or spawn behavior.
- Produce one consolidated Markdown audit document titled `Miniboss Special Boss Spawn and Integration Audit` under `PerformanceSystem/`.
- Cover current miniboss, special, and boss spawn logic with code references.
- Inventory all miniboss/special/boss roster data from `Content/Data/*.csv` and related runtime types.
- Audit infrastructure integration for projectiles, actor registry, HUD/minimap, damage attribution, pooling, and coexistence with the lightweight mob manager.
- Explicitly flag family-neutral miniboss promotion, the deprecated rich projectile actor path's impact on ranged specials/minibosses/bosses, and any other integration gaps found.
- No Mini/minigame scope unless a matching symbol/path is only a false-positive from search; do not include Mini assets or minigame systems in the audit.

## Applicable Repo Instructions

- Root `AGENTS.md`: derive working goal; inspect live repo state; use folder instructions; write a plan before doc/code changes; use Claude review by default; wait for user go-ahead after review; report exact verification.
- Root `AGENTS.md` Performance registry: performance/profiling/diagnostics work starts with `PerformanceSystem/PERFORMANCE_SYSTEM_AGENTS.md`.
- Root `AGENTS.md` report routing: agent-authored reports/review packets belong under `Reports/`; ordinary durable performance contracts/reports belong under `PerformanceSystem/`.
- `PerformanceSystem/PERFORMANCE_SYSTEM_AGENTS.md`: keep human-readable performance/diagnostic contracts and reports under `PerformanceSystem/`; keep runtime code under `Source/T66/PerformanceSystem/`; do not add optimizer fixes while working on diagnostics unless explicitly requested.
- `Reports/AGENTS.md`: review packets and reviewer outputs go under `Reports/AgentReviews`.
- Pending issues read:
  - `PerformanceSystem/pending_issues_PerformanceSystem.md`
  - `Source/T66/Gameplay/pending_issues_Gameplay.md`

## Current Live-Repo Evidence Before Planning

Narrow source/data mapping identified these audit seams:

- Spawn/director:
  - `Source/T66/Gameplay/T66EnemyDirector.cpp`
  - `Source/T66/Gameplay/T66EnemyDirector.h`
  - `Source/T66/Gameplay/T66TutorialManager.cpp`
  - `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`
  - `Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp`
  - `Source/T66/Core/RunState/T66RunStateSubsystem_TimersBoss.cpp`
- Enemy tier data/runtime types:
  - `Content/Data/Enemies.csv`
  - `Content/Data/UniqueEnemies.csv`
  - `Content/Data/Bosses.csv`
  - `Content/Data/BossEncounters.csv`
  - `Content/Data/BossEncounterMembers.csv`
  - `Content/Data/Stages.csv`
  - `Config/DefaultT66StageProgression.ini`
  - `Source/T66/Data/T66DataTypes.h`
- Projectile paths:
  - `Source/T66/Gameplay/T66ProjectileManagerSubsystem.cpp`
  - `Source/T66/Gameplay/T66MobBase.cpp`
  - `Source/T66/Gameplay/Enemies/T66RangedEnemy.cpp`
  - `Source/T66/Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.*`
  - `Source/T66/Gameplay/T66UniqueDebuffEnemy.*`
  - `Source/T66/Gameplay/T66UniqueDebuffProjectile.*`
  - `Source/T66/Gameplay/T66BossBase.*`
  - `Source/T66/Gameplay/T66BossProjectile.*`
- Registry/HUD/minimap/damage/pooling:
  - `Source/T66/Core/T66ActorRegistrySubsystem.*`
  - `Source/T66/UI/HUD/T66GameplayHUDWidget_Map.cpp`
  - `Source/T66/UI/HUD/T66GameplayHUDWidget_Refresh.cpp`
  - `Source/T66/Gameplay/T66PlayerController_Combat.cpp`
  - `Source/T66/Core/RunState/*`
  - `Source/T66/Core/T66EnemyPoolSubsystem.*`
  - `Source/T66/Gameplay/T66MobManagerSubsystem.*`

The existing pending issue `Ranged Autocapture Acceptance Remains Blocked After Projectile Manager` already records that B.10.1D Resume5 found source evidence for family-neutral mini-boss promotion: `ShouldRouteSpawnToLightweightMob` returns false for `bIsMiniBoss`, and runtime waves choose the mini-boss slot before rolling final `MobID`. The audit should verify and expand that evidence rather than assuming it is complete.

## Codex Plan

1. Perform read-only source inspection of the director, non-director spawn sites, boss flow, tutorial spawns, and any runtime utility that can create miniboss/special/boss actors.
2. Perform read-only data inspection of enemy, unique enemy, boss, boss encounter/member, stage, and stage-progression CSV/INI files. Use structured CSV parsing where practical for roster tables, and cite source columns/row IDs in the audit.
3. Build a spawn-logic section for each tier:
   - Minibosses: trigger, selection order, MiniBossIndex semantics, relation to final MobID family roll, promotion frequency, data/hardcode boundary.
   - Specials: trigger, classification, stage/floor/condition relation, rich-route reason, and non-director paths.
   - Bosses: floor/encounter trigger, boss data and encounter member resolution, boss-only finale behavior, casino/gambler or overlay-triggered boss paths if live gameplay code supports them.
4. Build a roster inventory:
   - Include enemies classified as miniboss, special, or boss.
   - Record family/archetype, ranged component/projectile behavior, visual path or VAT/skeletal/placeholder state if visible from data/code, and stage/theme appearance.
   - Do not include Mini/minigame-only assets.
5. Build an integration matrix for minibosses, specials, and bosses:
   - Projectile firing: manager, deprecated `AT66EnemyProjectileBase`, `AT66BossProjectile`, `AT66UniqueDebuffProjectile`, or other path.
   - Actor registry: registration and query coverage.
   - HUD/minimap: live count, enemy markers, boss bars, and any tier-specific UI behavior.
   - Damage attribution: source IDs and delivery names for damage to/from these tiers.
   - Pooling: pooled vs spawned/destroyed, and whether pooling only applies to basic rich enemies.
   - Lightweight manager coexistence: whether tier stays rich, whether route attribution accounts for it, and whether collisions/projectiles interact with lightweight mobs.
6. Write a prioritized gap list:
   - Family-neutral miniboss promotion.
   - Any ranged special/miniboss/boss still using a deprecated actor projectile path or otherwise not using the projectile manager where design expects straight enemy projectiles.
   - Any missing registry/HUD/minimap/damage/pooling integration found.
   - Any ambiguous data classification or spawn path found.
7. Write exactly one durable audit document under `PerformanceSystem/Miniboss_Special_Boss_Spawn_Integration_Audit.md`.
8. Update pending issue docs only if the audit finds a concrete, out-of-scope issue that is not already tracked and would otherwise be lost. If all findings are covered by the audit and existing pending issues, do not duplicate them.

## Intended Edit Scope

Allowed:

- Add `PerformanceSystem/Miniboss_Special_Boss_Spawn_Integration_Audit.md`.
- Optionally append/update pending issue docs if new concrete out-of-scope problems are found:
  - `Source/T66/Gameplay/pending_issues_Gameplay.md`
  - `PerformanceSystem/pending_issues_PerformanceSystem.md`

Not allowed:

- Any `.cpp`, `.h`, `.csv`, `.ini`, asset, build, staged executable, capture runner, or harness change.
- Any runtime validation capture or build/stage unless a later user request changes scope.
- Any Mini/minigame audit expansion.

## Risks And Mitigations

- Risk: Broad source searches could include Mini/minigame files. Mitigation: exclude Mini/minigame-only paths from findings unless they are only search false-positives; explicitly keep the durable audit scoped to gameplay tiers.
- Risk: Boss/special spawn logic may have multiple non-director entry points. Mitigation: search source for `SpawnActor` plus boss/enemy class names and classify every live path found.
- Risk: Data classification may be split across CSVs and runtime defaults. Mitigation: inspect both data files and `T66DataTypes.h` / game-instance loaders, then label uncertainty rather than inferring.
- Risk: The audit could accidentally become a design proposal. Mitigation: keep recommendations limited to "proposed next-pass fix scope" and clearly separate facts from later design decisions.

## Verification Evidence

Before reporting completion, provide:

- Path to the consolidated audit document.
- A concise list of source/data files inspected.
- Confirmation that no production source/data/script/build files were modified.
- Confirmation that no build, stage, or gameplay capture was run because the pass is documentation-only.
- If pending issue docs changed, identify the exact updates; otherwise state that the audit document captured the gaps without extra pending-issue edits.

## Reviewer Request

Please review this plan as a read-only cross-check. Focus on:

- Missing spawn paths or data files that would make the audit incomplete.
- Contradictions with root `AGENTS.md`, `PerformanceSystem/PERFORMANCE_SYSTEM_AGENTS.md`, or report routing.
- Any accidental scope creep into fixes, captures, optimization work, or Mini/minigame systems.
- Whether the verification gates are sufficient for a documentation-only audit.

Return a verdict as the first non-empty line exactly in one of these forms:

`Verdict: APPROVE`
`Verdict: REVISE`
`Verdict: BLOCK`

Then provide findings ordered by severity.
