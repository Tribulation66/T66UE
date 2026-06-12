# Operator Completion — Enemy Roster Restructure Plan

Task: Investigate current T66 code/data state for Pablo's locked enemy-roster restructure decisions and produce ONE consolidated implementation plan for review.
Operator: Claude (T66 FullOperator), under Codex approval `codex_operator_approval.md` (APPROVE).
Date: 2026-05-29
Outcome: COMPLETE — plan delivered, no implementation performed.

## Deliverable

- `C:\UE\T66\Reports\RosterReview\enemy_roster_restructure_implementation_plan.md`
  - 8 required sections present.
  - All 11 numbered items covered (each with Current state / Proposed approach / Files-data touched / Verification needed / Risks-open questions).
  - Explicit "Unclear / Missing Foundation" section (§6) and "Out-of-Scope / Non-Actions" section (§7).

## Stop-condition check

| Requirement | Met | Note |
|---|---|---|
| Plan exists at approved path | Yes | `Reports\RosterReview\...implementation_plan.md` |
| Covers every numbered item 1-11 | Yes | §3 |
| Loan Shark reported found/not-found with evidence | Yes | FOUND complete + wired; anchors in §2/§8 |
| Backrooms/Stalker claim supported | Yes | FOUND built & reachable; corrects stale prior-report claim |
| Shop steal trigger claim supported | Yes | `T66CasinoVendorTabWidget.cpp:2453,:2481` |
| Gambler/Vendor claim supported | Yes | `T66GamblerBoss.*`; `Overlays:5983,:6052` |
| Placed guardian / mega-mob claim supported | Yes | `T66GameMode_Tower.cpp:146` etc. |
| Proposed affected files/data plausible | Yes | §4 map |
| No implementation occurred | Yes | §7; only the two approved report files written |
| Changed files inside approved report paths | Yes | see below |

## Files written (all within approved scope)

1. `C:\UE\T66\Reports\RosterReview\enemy_roster_restructure_implementation_plan.md` (main deliverable)
2. `C:\UE\T66\Reports\AgentReviews\20260529_EnemyRosterRestructurePlan\operator_completion.md` (this file)

No source/config/content/save/CSV/uasset/DataTable/Git changes. No cleanup/deletion. No Mini/minigame inspection. No broad Git/LFS scans.

## Key findings flagged for Pablo

1. **Loan Shark is fully implemented** (prior report omitted it) — item 1 is verification, not build.
2. **Backrooms Stalker room encounter is built & reachable** (8%/tower stage) — prior report's "unimplemented/disabled" claim is stale; item 2 is largely verification.
3. **Tower floor structure already matches target** (1/2-4/5; 3 gates/stage) — item 10 is a terminology rename.
4. **Mega-mob mechanism exists** (`ConfigureAsMob` + `ApplyMiniBossMultipliers`) — item 6 needs a per-(stage,gate) MobID assignment replacing hardcoded "Slime".
5. **Gambler boss is a complete entity** ready to repurpose into Vendor; steal-trigger plumbing exists.
6. **RISK:** working-tree deletions of `QuickReviveIcon.uasset` + QuickReviveVending assets may break the Backrooms Quick Revive reward — pre-existing, out of scope, flagged.
7. **No archetype classes exist** — item 8 tag removal matches current runtime reality (honest cleanup).

## Open decisions for Pablo (from §6)

- Mega-mob MobID coverage + gate-index source (item 6).
- Theme→stages mapping for 12-per-theme vs 10 stage slots (item 7).
- Match named placeholders to real EnemyIDs (item 7).
- GamblersToken keep-or-rename; CasinoAnger usage post-change (item 5).
- Whether a specials/hidden-boss registry exists to enforce "exactly two specials + one hidden boss."

## Note

This artifact is an Operator work product, not a greenlight. Implementation requires a fresh Codex approval per the documented pass split (§5: Verification → Removal → Repurpose → Terminology → Additive).
