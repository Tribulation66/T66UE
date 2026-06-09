Verified the implementation against the draft's claims. It holds up well.

Result: OK

## Summary
Codex's draft accurately describes what's in the tree. I confirmed the keyed suppression model (`SuppressedPopupIds` TSet, schema v26), non-destructive migration of both legacy booleans, the consolidated `RunWillNotCount` popup with reason routing, and the Gameplay reset row. Critically, the backend-authority constraint is honored: `GI->bRunIneligibleForLeaderboard = true` is set at `T66HeroSelectionScreen.cpp:934` *before* and independent of the suppression check at `:938` — suppressing the popup never re-enables a run. This matches my independent answer; the draft can be finalized.

## Suggested Answer Patch
- In the final answer, explicitly state the suppression/eligibility separation with the line reference (`T66HeroSelectionScreen.cpp:934` sets ineligibility unconditionally; `:868`/`:938` only gate the visual popup). This is the highest-risk constraint and the draft's caveat states it only in prose — anchor it to evidence.
- Note the migration is non-destructive and one-directional: legacy `bShow*` booleans are read into the set at `:331-338` and left intact, so a future rollback still has the old state. Worth saying out loud.

## Issues To Fix
- **Reason-string ownership.** `T66FormatTeamRunWillNotCountReason` / `T66FormatLocalRestrictionRunWillNotCountReason` / the offline + ineligible strings live in the UI layer (`T66HeroSelectionScreen.cpp:16-90`), but suspension/ranked/Steam-auth wording is owned by `Backend/BACKEND_AGENTS.md`. Codex should either confirm these strings match backend-owned conventions or flag in the final answer that the wording needs a backend-owner sign-off. Not a blocker, but call it out rather than letting it pass silently.
- **Out-of-scope hardening.** The `RunStagedBuildReadinessGate.ps1` changes (CIM date parsing, build-process matching) are defensible as verification support but fall outside the stated task scope. Final answer should label them as incidental tooling fixes so they aren't mistaken for part of the popup feature.

## Question For User
None — Codex resolved the "consolidate with 5" ambiguity as the offline/unranked reason branch, which is a reasonable Operator call and is documented. No user-only decision remains.

## Evidence Or Verification Gaps
- Live two-client party session was not run (Codex caveat is honest). The team-suspended path is compiled, staged, and delegates through the generic API, but the actual host→client `ClientShowRunWillNotCountWarning` round-trip is unproven at runtime. Acceptable to ship with the caveat stated; flag it clearly.
- The staged popup proof used `t66.AccountStatus.Force 1` (local-suspended reason) only. Other reason branches (offline, backend-unavailable, team-suspended) are not individually screenshot-proven. Reasonable given they share one render path, but the final answer should say the proof covers one reason, not all.

## Notes
Migration, schema bump, generic API + compatibility wrappers, and reset control all match both the draft and my independent recommendation. The dirty-worktree caveat is real but pre-existing and correctly left untouched. This is a clean, well-scoped implementation — the remaining items are wording-ownership confirmation and honest scoping of the proof/verification in the final answer.
