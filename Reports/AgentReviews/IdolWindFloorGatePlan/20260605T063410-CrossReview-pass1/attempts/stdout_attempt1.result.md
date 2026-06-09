Result: OK

## Summary
Codex's draft is accurate, on-scope (planning-only), and resolves the earlier blockers. It confirms Pierce retention, Wind as a 5th element (20 idols + No Idol), the 4-floor layout, and gate-adjacent idol unlocks. It correctly keeps `MaxEquippedIdolSlots = 4` and lists the right implementation risks and a verification plan. It aligns closely with my independent answer; the math (5×4=20, 4 stages × 3 gates = 12) checks out.

## Suggested Answer Patch
Add one precision question Codex omitted (my independent Q3). Codex's proposed layout silently assumes the Floor-1 altar is one of the three gate idols; that should be stated as an assumption or asked, since the user's wording ("idol altars will always show up in the first floor" + "above the floor gates") could read as a first-floor altar *plus* gate altars (which would break the count of 12). Suggest appending to "Remaining precision questions":

> 4. Confirm the first-floor idol altar **is** the first descent gate (not an extra interaction on top of the gates) — this is what keeps it at exactly 3 idol interactions per stage and 12 total. The draft assumes yes.

## Issues To Fix
- The 12-interaction total depends on the Floor-1 altar being one of the 3 gates. Codex assumes this without surfacing it; make it an explicit assumption or question so the count isn't silently wrong.

## Question For User
None blocking — the open items are design refinements Codex can carry into its confirmation as questions. No prerequisite, tool, or scope decision requires the user before Codex can answer.

## Evidence Or Verification Gaps
- Codex's draft and my answer both rely on Codex's live-inspection repo facts (terrain hard-codes, traveler pool, altar widget counts) that I did not independently re-read. They are internally consistent with the files I did verify (`T66DataTypes.h`, `T66IdolManagerSubsystem.h`).
- Verification plan is adequate; ensure the save round-trip explicitly uses a **pre-Wind legacy save** through `T66MigrateLegacyIdolID`, not just new Wind saves. Codex says "old 4-element saves" — that covers it.

## Notes
Codex's added Q3 (should No Idol also unlock the gate — recommend yes to avoid soft-block) is a good catch not in my answer. Both drafts agree on the parity-sweep risk for hard-coded 4-element loops. No mutating actions taken; review only.
