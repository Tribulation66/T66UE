Result: OK

## Summary
Codex's draft is substantively correct and aligns tightly with the independent answer: there is no magic parameter, flattening is a structural PhysicsAsset/rig failure, and the fix order is volume bodies → joint limits → self-collision/mass/solver → drive feel last. It correctly separates what the Stage 2 seed proves from what it does not, and it does not overclaim. It satisfies the stop condition. Ship after the minor patches below.

## Suggested Answer Patch
- In "Practical Guarantee," soften the absolute "the skeletal mesh cannot physically pancake." Even with the invariant met, extreme impulses or solver substep limits can still produce transient collapse. Suggest: "…the mesh will not collapse into a flat plane under the expected TestRoom impulse while passing the validation gate." This keeps the "not guessing" framing honest without promising physical impossibility.
- Add one line making explicit (as the independent answer does) that the runtime hip anchor only governs the pelvis↔capsule relationship and does nothing for inter-body folding — this preempts the user assuming the existing anchor already covers it.
- Optional: name the specific tuneables Codex is deferring (`PoseOrientationStrength`, `BalancedPoseStrengthMultiplier`, anchor strengths) so step 5 is concrete rather than abstract.

## Issues To Fix
- The "Claude Alignment" section is accurate — keep it, no overstatement.
- "solver iteration overrides/projection/inertia" is correctly stated as a requirement to verify, not a confirmed defect. Ensure that framing survives any edit; do not let it read as a known misconfiguration, since neither model inspected the actual Chaos iteration values.

## Question For User
None. Both models can answer this internally. The only verification step (opening the PhysicsAsset to read body radii and per-constraint limits) is a next-pass implementation task, not a user decision.

## Evidence Or Verification Gaps
- Seed JSON does not record shape radii, angular limit modes, self-collision, or solver iterations — both draft and independent answer correctly flag this. Items 2–4 are requirements-to-verify, not confirmed defects.
- Neither model confirmed actual Chaos solver iteration settings; keep that as "verify," consistent with the draft.

## Notes
Draft and independent answer are in genuine agreement; the alignment claim is warranted. Patches above are wording-level honesty guards, all within Codex's scope to apply before answering.
