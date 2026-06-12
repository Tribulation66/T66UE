Verdict: NEEDS_HUMAN_DECISION

## Blockers
- None that Codex can resolve alone. The implementation cannot proceed because four inputs are not inferable from code and are genuine product/tuning decisions: XP threshold/curve, level-up wave radius, lightweight-mob XP scope, and whether diplomas/drugs are re-enabled in this same pass. These are correctly flagged in "Human Decisions Needed."

## Major Issues
- **Diplomas/drugs scope is ambiguous, not just a tuning value.** The user request lists items + level-up explicitly and asks for diplomas/drugs *breakdown + wiring status*, but never says "re-enable" them. The packet's proposed implementation (steps 3 and 4) would re-enable purchases and runtime effects, which is scope expansion beyond the request. The breakdown/report portion is clearly in-scope and safe; the re-enabling is not. Codex must not bundle diploma/drug re-enabling into this pass without an explicit user yes.
- **Primary→secondary propagation for level-up needs a storage decision.** The plan says to call `ApplyPrimaryGainToSecondaryBonuses()` "into persistent secondary bonus storage" but currently that helper feeds item-derived recompute. Since items are being made secondary-only (and `RecomputeItemDerivedStats` is being stripped of that call), the destination bucket for level-up-driven secondary bonuses must be confirmed to survive `RecomputeItemDerivedStats()` re-runs without being cleared. This is a real wiring risk, not just tuning — verify before implementing.
- **Item `PrimaryStatType` retention is a half-state.** Keeping `PrimaryStatType` as a UI grouping field while it no longer applies a bonus is reasonable, but the card-text change must guarantee no primary *value* leaks into display. The plan says "suppress primary line" — confirm the grouping use doesn't reintroduce a numeric primary line.

## Minor Issues
- Boss exclusion for the kill wave is asserted but the boss-identification path isn't named. The packet should cite the actual boss flag/class check rather than "existing damage/OHKO helper paths."
- "heal to full" — confirm whether this means current max health including any in-run modifiers, and whether it should also refill any shield/armor-pool concept if one exists.

## Clarifying Questions
1. XP curve: flat 100/level, 100+25·level, or other?
2. Wave radius: 900 UU, 1200 UU, current attack range, or other?
3. Do lightweight `AT66MobBase` kills grant XP this pass, or only rich `AT66EnemyBase`?
4. Are diplomas and drugs to be re-enabled in this implementation pass, or is this pass report-only for those two plus items + level-up?

## Required Verification
- Focused compile via the stated `Build.bat T66Editor` command — packet includes it. Good.
- Non-shipping smoke hook list is appropriate and covers secondary-only items, XP award, level-up heal, wave targeting (in/out of range, boss exclusion), and primary→secondary propagation.
- Staged standalone (`StageStandaloneBuild.ps1`) required since runtime playable behavior changes — packet includes it. Good.
- Add explicit verification that level-up secondary bonuses are NOT wiped by a subsequent `RecomputeItemDerivedStats()` call (the storage-bucket concern above).

## Rationale
The inspection/breakdown work is thorough, cites concrete files, and correctly concludes the current code is not in the target state. Verification and repo-instruction coverage (AGENTS, GAMEPLAY_AGENTS, Reports artifact placement) are present. However, the plan cannot safely advance to implementation: it depends on four user-owned decisions (XP curve, wave radius, mob-XP scope, and the diplomas/drugs activation scope question), and the diplomas/drugs portion risks expanding beyond the literal request. The correct path is to deliver the breakdown, save the decision block, ask once, and stop — hence NEEDS_HUMAN_DECISION rather than REVISE.

