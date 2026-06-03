Result: OK

## Summary
Codex's draft is coherent and largely matches my independent audit: it correctly distinguishes deletable idol-ID aliases from live sprite asset names in `Idols.csv`, keeps `NormalizeLegacyIdolID`/migration as the back-compat net (validated by finding legacy `Idol_Light` in `Saved/StageBackups`), and preserves proof/diagnostic vocabulary. It claims a clean build and a passing save round-trip proof with concrete artifact paths and exit code 0. The conclusions are sound and the scope discipline (no sprite rename, no doc/proof-label cleanup, Mini excluded) is right.

## Suggested Answer Patch
- In the "Files changed" list, the line "Changed canonical electricity idol color to purple. Ice remains light blue." should be removed from this packet or explicitly justified, because a color change is not part of the alias save-audit/deletion scope. If it must stay, add one sentence tying it to the accepted Hero1 temp-visual plan; otherwise split it into a separate change.
- Add an explicit statement that idol display text (names/tooltips) was verified non-empty after removing the legacy-keyed localization branches — currently the draft only asserts the branches were removed, not that the canonical path returns equivalent text.

## Issues To Fix
- **Out-of-scope color change.** The electricity→purple / ice color edit is unrelated to alias save-proof deletion. Either justify it against the accepted Hero1TempShapes plan or move it out of this packet.
- **Localization re-key not proven.** My independent audit found names/tooltips were *fully* keyed on legacy IDs (`T66LocalizationSubsystem.cpp:1705-1775`). Codex removed those branches and routes through normalize + "canonical category suffix text." Codex must confirm the canonical path yields the same (or intended) display strings, not blank/placeholder text — a silent text-loss regression is the top risk here.
- **VFX proof not run after VFX edits.** Codex removed legacy imported-VFX fallback branches and the `Idol_Storm` BP fallback in `T66CombatVFX.cpp`, then explicitly skipped the Hero1 VFX impact proof scripts. Removing VFX routing branches without running the VFX proof leaves the runtime VFX path unverified. Run `RunHero1Axe*IdolImpactProof.ps1` or justify why the removed branches were provably dead.

## Question For User
None required — the user authorized the audit-and-scoped-change work ("good go"). The color change is borderline but can be resolved by Codex scoping/justifying it rather than a user decision.

## Evidence Or Verification Gaps
- Save round-trip proof is reported with manifest `ok=true` and migration of `Idol_Light/Water/Storm/Poison` → canonical IDs; that directly satisfies the user's save-proof ask. Good.
- Gap: no Hero1 VFX impact proof despite VFX branch removals (see Issues).
- Gap: localization equivalence is asserted, not evidenced.
- `Idols.csv` was not touched, so no DataTable import commandlet was required — consistent with repo rules.

## Notes
The build mutex (`ConflictingInstance`) recovery and process audit are reasonable; second attempt's `BUILD SUCCESSFUL` covers the runtime-compile requirement. Core audit logic agrees with my independent pass — the only real follow-ups are the scope-creep color edit and the two unverified runtime surfaces (localization text, VFX routing).
