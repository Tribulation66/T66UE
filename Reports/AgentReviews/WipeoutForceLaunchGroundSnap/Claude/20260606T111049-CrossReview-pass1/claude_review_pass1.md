Result: OK

## Summary
Codex's draft is substantive and well-evidenced: it switches the launch to radial (matching my independent read of the tangential-launch root cause), raises launch distance to 10500, extends the ragdoll window to 3.10s for wall contact, adds damping/friction/restitution profile overrides for bounce feel, builds clean, and provides a staged standalone capture. The proof log directly refutes the two main symptoms — actor travels 850→14731 (no snap to origin), `ActorToTargetXY=0.0` throughout (no detach/drag-back), and `PACPending=0 DriveMode=0` (PAC stays off, confirming the user's "re-enabled" worry is unfounded). This is internally handleable; no user decision is blocking.

## Suggested Answer Patch
Add one explicit sentence to the user-facing answer reassuring them on PAC, since they raised it directly: "PAC was never re-enabled for heroes — the log shows `PACPending=0 DriveMode=0`; the symptoms were the tangential launch + short ragdoll window, not a PAC regression." Codex has this evidence but should surface it as a direct reply to the user's "somehow got re-enabled" concern.

## Issues To Fix
- **"Under the ground" is masked, not clearly root-caused.** My independent read found `ResolveRagdollFloorZ` traces once at the pre-impact location and caches via `bHasResolvedFloorZ` (T66KnockbackComponent.cpp:979-994). The draft says it "keeps the existing current-XY floor trace," which conflicts with that caching behavior. The TestRoom floor is flat, so the floor guard (`BodyMinZ=-14→Lift=18`, recovery `-20→Lift=24`) corrects it regardless — but the proof does **not** demonstrate the far-landing floor Z is re-traced. Codex should confirm whether the cache is actually invalidated at the new far XY, or explicitly downgrade "under the ground" to "masked on flat floor; will recur on uneven terrain" rather than "fixed."
- **Untracked source files.** Codex flagged that `T66KnockbackComponent.cpp/.h` are untracked in git. This is worth raising to the user but Codex can note it in the handoff; not a blocker.

## Question For User
None required to answer. (Whether to commit/track the untracked Knockback files is a routine follow-up Codex can surface, not a path decision.)

## Evidence Or Verification Gaps
- Bounce evidence is modest: actor returns only ~358 units (14731→14373). That supports "a bounce occurred" but not "bounces off the wall" as a satisfying feel. Fine as proof-of-mechanism; flag feel-tuning as open (Codex already does).
- 3.10s forced ragdoll window is long for final feel; correctly flagged as a tuning caveat with a contact-based-recovery follow-up.
- Floor-guard far-landing correctness not independently shown (see Issues).

## Notes
Stop condition is met: root cause patched, focused build + staged standalone proof run done, feel-tuning caveats (launch XY/Z, MaxRagdoll, camera obstruction, contact-based recovery) explicit. Recommend OK with the floor-Z caching clarification handled before final answer.
