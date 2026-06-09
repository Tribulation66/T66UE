Verified the repo against Codex's claims. All Stage 1 changes landed and the folder-boundary tension I flagged is handled cleanly.

Result: OK

## Summary
Codex's draft is accurate and complete against the prompt. I verified the live repo: the `Gameplay/Physics/` layer exists with router + policy + Stage 2 plan + archive index; `Gameplay/README.md` and `GAMEPLAY_AGENTS.md` now carry the broad Physics area and trigger words; `pending_issues_Gameplay.md` has the dated 2026-06-06 supersede note with the heading flipped to "Superseded". The Stage 2 plan correctly frames rigging as the manual doc-13 process and explicitly excludes the retired bakeoff (HeroPhysicsModel.md:39), which was my main caveat. Scope (docs/routing only, no source/asset mutation) is respected. This is internally resolvable — no user decision is gated.

## Suggested Answer Patch
Codex's draft can ship largely as written. Two small wording tightenings for the final answer:
- In the "Files Changed" entry for `pending_issues_Gameplay.md`, note that the *original Resolution text was left intact* and the supersede is layered as an additive dated note (this is what actually happened and is the correct ledger discipline) — the current bullet implies a heavier rewrite than occurred.
- In the Stage 2 summary, add one line stating the forward-leap/roll-replacement is the user's stated locomotion *target/direction*, not approved build scope for Stage 2 — matching the prompt's "eventual" framing.

## Issues To Fix
- None blocking. The two wording patches above are optional polish for Codex to fold in before sending.

## Question For User
None. Architecture reopening is already user-approved; everything in this pass is docs/planning.

## Evidence Or Verification Gaps
- Confirmed on disk: `Gameplay/Physics/` (7 docs + Archive), README Physics area line, GAMEPLAY_AGENTS trigger words, pending_issues supersede note, doc-13 manual-rigging framing. Codex's "no source/asset mutation" claim is consistent with the file set touched.
- I did not diff every one of the seven new Physics docs line-by-line for internal consistency (e.g., PhysicsReactionProfiles vs MASTER_PHYSICS field overlap). Low risk for a planning pass; Codex should skim for duplicate/contradictory contracts before finalizing.

## Notes
- Codex resolved the rigging folder-boundary cleanly (HeroPhysicsModel.md:39 + :175 require user approval before any bakeoff/AccuRig/ToonStyle substitution), so the contradiction risk I raised independently is closed.
- PPF/artifact-parity gate and a `Gameplay/Physics/pending_issues_*.md` entry are correctly deferred to the future Stage 2 implementation pass.
