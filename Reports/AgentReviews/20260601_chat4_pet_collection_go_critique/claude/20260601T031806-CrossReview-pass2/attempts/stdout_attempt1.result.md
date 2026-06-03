Confirmed: `IsEligibleForQuery` (T66MobLootSubsystem.cpp:651-653) does reject drops inside `ExclusionSpheres`, and the query path calls it (line 238). Codex's stronger claim here is correct and better-evidenced than the independent answer's cautious note.

Result: NEEDS_USER

## Summary
The Codex draft is accurate and aligns with my independent validation. The collector side (a, c, bond-movement-only, ExclusionSpheres) is clean and snaps onto the live reservation API. The one genuine blocker — boundary (b), the source-aware Economy credit path for Mob Loot — is absent from live source, and only the user can resolve whether it's unmerged or to narrow scope. NEEDS_USER is the correct routing. No file edits or mutating commands needed before the user decides.

## Suggested Answer Patch
- The draft is publishable as-is. One optional tightening: in the "Confirmed clean" ExclusionSpheres bullet, the draft can cite the exact evidence to preempt doubt: `IsEligibleForQuery` rejects drops inside spheres at T66MobLootSubsystem.cpp:651-653, reached from the query loop at line 238. This is correct and worth stating, since it's a stronger claim than the independent answer made.
- Minor wording: the draft notes `AddGold` "logs Source=Gambler" — phrase as "is generic/not source-aware (existing call sites use Source=Gambler)" so it doesn't read as if AddGold hardcodes Gambler.

## Issues To Fix
- None blocking. The draft correctly does not credit gold from the pet and correctly flags (b) as the stop point.

## Question For User
- Carry Codex's question forward verbatim: Where is Chat 3's source-aware Mob Loot gold-credit entry point in live code, or is it still unmerged? If absent, the user must choose: wait for Chat 3, or explicitly approve a narrower collection-only implementation with gold-credit proof deferred. This is the sole reason for NEEDS_USER.

## Evidence Or Verification Gaps
- ExclusionSpheres honoring: now verified (see above) — no longer a gap. The independent answer's caveat on this can be dropped.
- The "no Economy credit path" finding is a proven-negative search; both agents searched Core/RunState/`GoldValueCollected` consumers and found only the stress harness. Solid, but inherently bounded — if the Economy entry point lives under a non-obvious name, the user pointing to it would close (b).

## Notes
- Codex draft and independent answer converge on the same verdict and same blocker; confidence is high.
- Both correctly preserve the read-only/no-git/no-mutation constraints and defer all implementation to post-confirmation.
