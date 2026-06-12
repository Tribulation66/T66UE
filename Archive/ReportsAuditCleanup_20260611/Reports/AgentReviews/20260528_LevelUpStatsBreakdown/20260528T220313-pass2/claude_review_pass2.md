Verdict: APPROVE

## Blockers
- None. All four human decisions are resolved, applicable instructions are cited, and the edit scope maps cleanly to the stated live state.

## Major Issues
- **Level-up wave XP reentrancy.** The OHKO level-up wave can kill mobs/rich enemies that themselves call `AddHeroXP`, which loops `ApplyOneHeroLevelUp` and can trigger another wave — unbounded chaining or recursive level-ups. The packet flags this and proposes a guarded XP queue / no-chain wave source, but leaves it conditional ("if review sees a reentrancy risk"). The risk is real: make a no-chain/guarded wave source mandatory, not conditional, and assert it in the smoke (kill count from a single level-up must not re-grant XP into another level-up).
- **`GetTotalStatBonus()` semantics underspecified.** "Visible unlocked fill steps plus random/overflow bonus" is the one place the plan is vague. Before implementing, confirm against the actual diploma fill-step/overflow data model so the returned tenths match what `ApplyPrimaryGainToSecondaryBonuses(...)` expects (units, scaling). This is Codex-owned inspection, not a user decision.

## Minor Issues
- Starting defaults `100` (XP threshold) and `20` (per-mob XP) are placeholders the packet itself hedges ("unless code review finds a better default"). Acceptable since data-driven and tunable later, but the smoke should not assert exact stat magnitudes that depend on these.
- Lightweight-vs-rich XP/score/loot parity is explicitly deferred. Fine for this pass, but note it in the docs so the asymmetry is intentional, not a latent bug.

## Clarifying Questions
- None blocking. Diploma fill-step semantics (above) is resolvable by reading `UT66BuffSubsystem`, not by asking the user.

## Required Verification
- All six verification steps as written. Specifically require:
  - Data-table reload (`SetupPlayerExperienceDataTable.py`, `SetupCombatRosterDataTables.py`) **before** compile, since schema fields are new.
  - `Build.bat T66Editor` clean compile.
  - Non-shipping smoke proving every claim in the packet's smoke list, **plus** an explicit no-XP-chain assertion for the level-up wave.
  - Staged standalone refresh (`StageStandaloneBuild.ps1`) and staged proof, per `GAMEPLAY_AGENTS.md` (playable runtime affected).
  - JSON proof saved under `Reports/AgentReviews/20260528_LevelUpStatsBreakdown/`.

## Rationale
Scope is unambiguous, save/UI compatibility is preserved by keeping inert fields, the data-driven approach matches root `AGENTS.md`, and the deprecated `MASTER_STATS.md` / `MASTER_PLAYER_EXPERIENCE.md` doc updates are included. No Mini/minigame scope creep. The plan reuses the existing `ApplyPrimaryGainToSecondaryBonuses(...)` helper rather than inventing new propagation. The only substantive risk — wave-driven XP chaining — is identified by the packet and is solvable with Codex-owned implementation; making that mitigation mandatory keeps this an APPROVE rather than a REVISE. No product-direction decision remains open.

