Result: OK

## Summary
The Codex draft is solid and converges with the independent answer on the central diagnosis: no single motion authority, a kinematic pelvis defeating the documented world-anchor constraint, angular-only PAC, a non-physical timer-based obstacle, the 10500→1700 clamp, and a CharacterMovement override that competes with body physics. The architecture recommendation (simulated pelvis + kinematic hip target + constrained drive + real collision) matches. It is publishable after the evidence-sourcing notes below are addressed.

## Suggested Answer Patch
Minor wording/precision tweaks Codex can fold in:
- Cause #1: make the mechanism explicit — "the hip constraint targets the pelvis body, but because the pelvis is excluded from simulation the constraint drive is inert (you cannot drive a kinematic body through a constraint)." The current phrasing states the facts but not why the anchor does nothing.
- Cause #6/#7: cite the line evidence as the independent answer does (clamp at component cpp:190-192 / header `MaxReactionVelocityChange=1700`; override at cpp:204-210) so the synthesis is repo-grounded, not asserted.
- "Unreal Reference Patterns": the docs are paraphrased without source names/links, which the prompt explicitly requested ("with source names/links if known"). Name them: UE "Physics-Driven Animation / Physical Animation Component" docs and the "Collision Response / Collision Filtering" docs. If exact URLs aren't certain, say so rather than implying citations exist.

## Issues To Fix
- Causes #8 (movement tuning numbers: Speed*840, MaxAccel 9000, etc.) and #9 (`AT66HeroBase::Tick()` still calling `GetMesh()->PlayAnimation(...)`) are NOT corroborated by the independent answer, which explicitly did not read `T66HeroBase.cpp` or `MASTER_MOVEMENT.md`. These appear to come from Codex's own reads — fine — but Codex must confirm the exact source/line for each number before publishing, since they are concrete claims a reader will trust.
- The draft omits the `ApplyBalanceWobble` continuous-impulse-into-kinematic-pelvis cause (independent #8) and the overlapping `ResyncPelvisBodyToActor`/`FollowActorToPelvisBody`/`MaxPelvisCapsuleDistance` "five authorities" point (independent #10/#9 region). #3 partially covers follow, but the every-0.14s wobble injection during normal movement is a strong contributor to the "spazzing while moving" symptom and is worth adding.

## Question For User
None. This is a read-only diagnostic fully derivable from repo state; no user decision, scope change, or unavailable tool blocks it.

## Evidence Or Verification Gaps
- Proof-artifact coverage: both drafts assert the prior proof only captured the flattening case, not movement+hit+recovery. Neither actually opened `physicsfirst_asset_final_reportonly.json`. Codex should confirm frame coverage before stating it as fact, or soften to "appears to."
- `PhysicsReactionProfiles.md` and `MASTER_MOVEMENT.md` were not confirmed read in the independent answer; if a profile value diverges from the header defaults (`MaxReactionVelocityChange=1700`), the clamp argument changes. Codex should verify the live profile value, not just the header default.
- Linchpin caveat (independent answer): if any code path flips the pelvis to simulated after init, several downstream conclusions soften. Read evidence shows kinematic at init (cpp:392-417, 523); Codex should confirm no later override.

## Notes
Runtime "feel" claims in both drafts are inferred from configuration, not a live capture — Codex should frame them as high-confidence hypotheses to confirm with the proposed movement+hit+recovery proof, which the draft's diagnostic-gates section already sets up well.
