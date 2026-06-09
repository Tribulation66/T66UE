Result: NEEDS_USER

## Summary
Codex did real work and reached a genuine, user-only fork. It built the reference-first extraction + per-component gate the user asked for, ran it on six built-in imagegen candidates, and all six FAILed the auto gate (IoU 0.143–0.656, material delta up to 72.4, all manual REVIEW). Nothing was wired to runtime. The draft now asks the user to choose among three options, two of which are explicit policy changes (ship cropped/masked reference plates; or enable CLI/API mask inpaint) and one of which relaxes the "exactly like reference" goal. This matches exactly the *mandatory stop condition* my independent answer predicted, and it has now actually triggered. Only the user can pick the path.

## Suggested Answer Patch
The draft is substantially correct and well-evidenced; minor tightening for the final answer to the user:
- In "Work Performed," qualify the first bullet: "Reverted the rejected freeform sheet by restoring the pass13 plates **as the prior baseline only — none re-wired as accepted art**." As written, "Restored the pass13 runtime/source plates" reads like a runtime change and invites the same objection the user raised.
- In the Decision block, surface the **transient-error rule** explicitly: a built-in `TooManyRequests`/session error is a fork-and-restart, not a trigger to fall back to CLI/API. This keeps option 2 honestly framed as a true policy change rather than an error-handling escape hatch.
- State the gate tolerances that define PASS (the IoU/material/remnant thresholds) so the user understands what "exact" means quantitatively when weighing option 3.

## Issues To Fix
- "Restored the pass13 runtime/source plates" wording risks re-triggering the user's exact complaint. Confirm pass13 plates are not themselves a previously-rejected generic lookalike, and clarify they are baseline-only. (Codex can fix in text.)
- Decision block option 1 says "remove only masked live content zones locally or with approved edits" — "locally" edge-touches the crop-as-runtime-plate boundary. Codex should make clear this is *precisely* the rule change being requested, not something already permitted.

## Question For User
Forwarded by Codex correctly. The user must choose: (1) allow direct reference-derived runtime plates with strict gates, (2) authorize a true mask-inpaint path (requires lifting the no-CLI/no-`OPENAI_API_KEY` rule), or (3) accept approximate built-in-only regeneration. No further extraction/gating work is blocked behind this — only runtime wiring is.

## Evidence Or Verification Gaps
- Verified all four named artifacts plus the manifest/metrics/contract addendum exist on disk with plausible timestamps (02:40–02:54, 2026-06-06). Gate report confirms FAIL on all six families with REVIEW manual status — consistent with the draft.
- Not independently verified: the visual fidelity of the contact sheets, and whether the restored pass13 plates differ from the rejected freeform sheet. Codex/user should eyeball the contact sheet before accepting any option.

## Notes
My independent answer rated this OK-with-contingent-user-decision because at answer-time the gate hadn't run. The gate has now run and the contingency fired (all candidates failed), so the correct routing is NEEDS_USER. The three options are real and mutually exclusive policy choices; Codex cannot pick for the user.
