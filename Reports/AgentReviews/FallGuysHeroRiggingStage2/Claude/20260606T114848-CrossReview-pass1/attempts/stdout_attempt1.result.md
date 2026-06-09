Result: OK

## Summary
The Codex draft and my independent answer agree on substance: this is a planning-only turn the models can complete, the roadmap reshuffle (rig/anim standard → Stage 2, active-ragdoll → Stage 3) is correct, and the existing FriendSlop spike rig + Quaternius Idle/Walk/Jump/Roll clips are reusable as skeleton/QA evidence but not as the production standard. The draft is well-grounded and answers all five questions. It has one notable omission and a couple of unsupported specifics that Codex should tighten before sending.

## Suggested Answer Patch
Add a short "Naming / confirmation needed at go-ahead" note to the Codex answer, since the draft never reconciles the user's words with the repo:

> Two items to confirm with you at go-ahead (they do not block this plan):
> 1. **"Francois folder."** The repo has no folder by that name. The instruction file I plan to rewrite is `Model Generation/Instructions/13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md`. Please confirm that is the "rigging and animation file" you meant before I replace its contents.
> 2. **Leap vs Roll in data/code.** I recommend renaming `RollAnimation`/`RollForward()`/`TryRollForward()` → Leap rather than hiding Leap in the Roll column. This sizes the later runtime/data pass; confirm at approval.

Also soften the Stage 3 path claim (see Issues).

## Issues To Fix
- **Missed constraint — "Francois folder."** The original prompt explicitly says replace the files "in the Francois folder." The draft silently substitutes the FriendSlop instruction doc without flagging that "Francois" appears nowhere in the repo. My independent answer caught this; Codex must surface it rather than assume.
- **Unsupported import path.** The draft asserts the new asset lands at `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw` "or a versioned successor." That path is presented as known but isn't evidenced in the context. Mark it as a proposal to confirm, not a fact.
- **Schema change understated.** The draft says CSV "gains/uses `LeapAnimation`." My answer notes this is a cross-cutting change (`T66DataTypes.h` struct + every consuming row + ~10 runtime files referencing Roll). Codex should state the blast radius so the user understands the later pass isn't a one-line edit.

## Question For User
None required to complete this planning turn. The Francois-folder naming and Leap-vs-Roll decisions are confirm-at-go-ahead items, not blockers — keep them in the answer body, not as a stop.

## Evidence Or Verification Gaps
- Neither model opened Blender/FBX; rig-quality claims rest on the rig report's own "deterministic spike" admission (line 95), which is explicit enough for planning. Fine to proceed; just don't overstate weight/topology specifics.
- CSV was read partially but the Hero_1_Chad row and header (the only relevant rows) were seen. Acceptable.

## Notes
The draft's PPF/ARTIFACT/MECHANISM gate scaffolding is heavier than this planning turn needs but isn't wrong; it doesn't introduce errors. Both answers correctly keep all mutation deferred to post-approval. Result stays OK — every gap here is something Codex can fix in the answer text.
