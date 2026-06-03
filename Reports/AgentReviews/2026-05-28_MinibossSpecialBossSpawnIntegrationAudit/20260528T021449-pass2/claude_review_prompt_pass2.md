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
