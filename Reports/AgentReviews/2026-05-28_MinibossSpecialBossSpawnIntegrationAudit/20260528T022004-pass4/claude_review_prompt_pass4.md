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
- Packet path: C:\UE\T66\Reports\AgentReviews\2026-05-28_MinibossSpecialBossSpawnIntegrationAudit\completion_packet.md
- Output scope: review of the packet below only.

<review_packet>
# Completion Review Packet: Miniboss Special Boss Spawn and Integration Audit

## Working Goal

Write the approved, documentation-only miniboss/special/boss spawn and integration audit under `PerformanceSystem/`, using live source/data evidence and no production behavior changes.

## User Constraints

- Documentation only.
- No code changes, no spawn-rule changes, no fixes.
- Audit current miniboss, special, and boss spawn logic.
- Inventory roster data and infrastructure integration with projectile firing, actor registry, HUD/minimap, damage attribution, pooling, and lightweight manager coexistence.
- Respect root `AGENTS.md`, `PerformanceSystem/PERFORMANCE_SYSTEM_AGENTS.md`, and `Reports/AGENTS.md`.

## Reviewed Plan Artifact

- Plan packet: `Reports/AgentReviews/2026-05-28_MinibossSpecialBossSpawnIntegrationAudit/plan_packet.md`
- Claude plan review: `Reports/AgentReviews/2026-05-28_MinibossSpecialBossSpawnIntegrationAudit/20260528T015949-pass1/claude_review_pass1.md`
- Plan review verdict: `Verdict: APPROVE`

## Output Produced

- Audit document: `PerformanceSystem/Miniboss_Special_Boss_Spawn_and_Integration_Audit.md`

## Scope Performed

- Inspected live director spawn logic, route-attribution reasons, miniboss promotion logic, special spawn logic, non-director tutorial/lab paths, boss flow, boss gate activation, casino boss trigger paths, projectile systems, actor registry, HUD/minimap, RunState damage attribution, pooling, and data table schemas.
- Inspected current data files:
  - `Content/Data/Enemies.csv`
  - `Content/Data/Stages.csv`
  - `Content/Data/Bosses.csv`
  - `Content/Data/BossEncounters.csv`
  - `Content/Data/BossEncounterMembers.csv`
- Wrote a consolidated audit with inline `path:line` citations.

## Main Findings Captured

- Miniboss promotion is family-neutral and runtime-only; the slot is selected before final `MobID` family resolution.
- Minibosses deliberately route rich through `ShouldRouteSpawnToLightweightMob` because `bIsMiniBoss` returns false for lightweight routing.
- `Feeling=MiniBossFeel` exists in `Enemies.csv` but is not enforced by promotion logic.
- Director specials are Goblin Thief only; other special-like systems are bespoke class paths.
- Normal rich Ranged and lightweight Ranged now use `UT66ProjectileManagerSubsystem`; boss and unique debuff projectiles still use actor projectile classes.
- Bosses register separately from enemies/mobs and drive boss HUD through RunState; minimap enemy marker caches do not include boss registry entries.
- Basic-mob performance acceptance should either disable/filter miniboss/special spawns or treat planned rich miniboss/special routes as expected non-basic routes.

## Verification Performed

- Document existence verified.
- Scope crosswalk:
  - Task 1 current miniboss spawn logic -> audit sections `Current Spawn Logic / Minibosses`, `Roster Inventory / Miniboss Inventory`, `Gaps and Risks`.
  - Task 2 current special spawn logic -> audit sections `Current Spawn Logic / Specials`, `Roster Inventory / Special Inventory`, `Infrastructure Integration Matrix`.
  - Task 3 current boss spawn logic -> audit sections `Current Spawn Logic / Bosses`, `Current Spawn Logic / Non-Director Spawn Paths`, `Roster Inventory / Boss Inventory`.
  - Task 4 roster inventory -> audit section `Roster Inventory`.
  - Task 5 infrastructure integration -> audit section `Infrastructure Integration Matrix`.
  - Task 6 seams and gaps -> audit section `Gaps and Risks`.
  - Task 7 audit document -> `PerformanceSystem/Miniboss_Special_Boss_Spawn_and_Integration_Audit.md`.
- Non-director spawn path coverage:
  - Tutorial mini-boss/rich enemy path -> `Current Spawn Logic / Non-Director Spawn Paths`.
  - Lab mob/special/boss path -> `Current Spawn Logic / Non-Director Spawn Paths`.
  - Stage boss flow -> `Current Spawn Logic / Bosses`.
  - Boss gate activation -> `Current Spawn Logic / Bosses`.
  - Casino/Gambler boss trigger -> `Current Spawn Logic / Bosses` and `Special Inventory`.
- Citation spot-checks performed against live files:
  - `Source/T66/Gameplay/T66EnemyDirector.h:60` confirms `MiniBossChancePerWave = 0.10f`.
  - `Source/T66/Gameplay/T66EnemyDirector.cpp:1175` confirms mini-boss selects from mob spawn slots, not specials.
  - `Source/T66/Gameplay/T66EnemyDirector.cpp:1494` confirms final `MobID` is resolved after the mini-boss slot decision.
  - `Source/T66/Gameplay/T66EnemyDirector.cpp:724` confirms `bIsMiniBoss` blocks lightweight routing.
  - `Source/T66/Gameplay/T66EnemyDirector.cpp:1121` confirms Goblin Thief special wave logic.
  - `Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp:728` and `:799` confirm stage boss encounter resolution and boss actor spawn.
  - `Source/T66/Gameplay/Enemies/T66RangedEnemy.cpp:223` and `Source/T66/Gameplay/T66MobBase.cpp:802` confirm rich/lightweight normal Ranged use `UT66ProjectileManagerSubsystem`.
  - `Source/T66/Gameplay/T66BossBase.cpp:955` and `Source/T66/Gameplay/T66UniqueDebuffEnemy.cpp:139` confirm boss and unique debuff projectiles remain actor-based.
  - `Source/T66/Core/T66ActorRegistrySubsystem.cpp:95` and `:147` confirm live enemy counts exclude bosses while bosses register separately.
  - `Source/T66/UI/HUD/T66GameplayHUDWidget_Map.cpp:394` confirms minimap caches rich enemies and lightweight mobs, not bosses.
- No-fix language pass:
  - The audit was revised to avoid presenting remediation as implemented behavior.
  - Remaining `should` matches are only inside the code identifier `ShouldRouteSpawnToLightweightMob`.
  - Follow-up options are framed under `Deferred Decision Areas`, not as changes made in this pass.
- Additional clarification pass after Pass 3 minor notes:
  - Changed "now fire through" to "currently fire through" to avoid implying this audit introduced projectile-manager routing.
  - Added explicit statement that no standard director/stage progression production spawn path for `AT66UniqueDebuffEnemy` was found in inspected source.
  - Added explicit boss-pool search note: no `BossPool*`, `AcquireBoss*`, `TryAcquireBoss*`, or `ReleaseBoss*` path was found in current source searches.
  - Added explicit elite-tier search note: no implemented elite promotion/routing tier was found in Gameplay/Core/Data searches.
  - Added explicit `EnemyFamily=Special` search note: only Goblin Thief and Unique Debuff Enemy define themselves as `Special` classes in inspected gameplay source.
- Git/path checks:
  - `git status --short --untracked-files=all -- PerformanceSystem Reports/AgentReviews/2026-05-28_MinibossSpecialBossSpawnIntegrationAudit` shows the new audit file plus review artifacts and several pre-existing untracked PerformanceSystem docs. It does not show production source/code/data changes from this pass.
  - `git diff --stat -- PerformanceSystem/Miniboss_Special_Boss_Spawn_and_Integration_Audit.md Reports/AgentReviews/2026-05-28_MinibossSpecialBossSpawnIntegrationAudit/completion_packet.md` produced no output because these are untracked docs, not tracked modifications.
  - `git status --short -- Content/Data/Enemies.csv Content/Data/Stages.csv Content/Data/Bosses.csv Content/Data/BossEncounters.csv Content/Data/BossEncounterMembers.csv` produced no output; the cited data tables were not modified by this audit.
- No build, cook, staged standalone, or runtime capture was run because this was documentation-only and made no production behavior changes.
- No broad git/LFS status scan was run.

## Reviewer Request

Please review the produced audit document for:

1. Whether it satisfies the approved audit scope.
2. Whether the findings are supported by live source/data citations.
3. Whether any required miniboss/special/boss spawn path, roster item, or integration surface appears missing.
4. Whether the document accidentally proposes or claims implemented fixes.
5. Whether any Blocker or Major issue should prevent reporting this audit as complete.

Return exactly one of:

- `Verdict: APPROVE`
- `Verdict: REVISE`
- `Verdict: BLOCK`

Then list findings by severity.

</review_packet>
