Verified: component exists, hero exposes the API, TestRoom routes through `GetKnockbackComponent()->ApplyKnockbackLaunch(..., &Profile)` with a hero fallback, and the heavy `AddImpulse`/`SetAllBodiesBelow` machinery is gone from the GameMode. The draft matches the repo.

Result: OK

## Summary
Codex's draft is code-grounded and consistent with the repo and with the independent recommendation. A hero-owned `UT66KnockbackComponent` + `FT66KnockbackProfile` now owns the ragdoll/PAC machinery, `AT66HeroBase` exposes `GetKnockbackComponent`/`ApplyKnockbackLaunch`/`IsKnockbackActive`/`IsKnockbackIncapacitated`, and TestRoom is reduced to a trigger that passes CVar-derived profile overrides. Build/proof evidence is concrete (focused editor build success, gameplay video with log lines confirming the component path, staged standalone success). The infrastructure-vs-actor-scope constraint is honored (HordeFallback budget class stubbed, no non-hero actors implemented).

## Suggested Answer Patch
None required. One optional clarity edit for the draft's Verification section: state explicitly that the captured proof covers a *single* impact+recovery cycle, not the consecutive-hit restore case (see gaps below), so the reader doesn't over-read the evidence.

## Issues To Fix
- None blocking. The component lives at `Source/T66/Gameplay/T66KnockbackComponent.{h,cpp}` rather than a `Combat/` or `Movement/` subfolder as the independent answer suggested — cosmetic, not an error; leave as-is unless the user has a folder convention.

## Question For User
None. Scope is hero-only and decidable from repo evidence; the user owns only the later feel-tuning, which the draft correctly defers.

## Evidence Or Verification Gaps
- **Consecutive-hit suppression restore is unproven.** Both the independent answer and good practice call for confirming a *second* hit still knocks down and recovers (suppression flag reference-restored, not permanently disabling auto-attack). The captured proof shows one impact + recovery only. Recommend Codex either capture a two-hit run or confirm by code inspection that recovery restores movement/auto-attack via flag, not unconditional re-enable.
- **Stretch/smear fix is asserted by log, not visually confirmed.** Logs prove the skeletal+PAC path fires (RuntimeBodies=18, DriveMode=2, DrivenBodies=6), but the draft itself notes the arm can occlude the hero in proof frames. The core acceptance criterion ("no longer stretches/smears") is therefore not visually verified. Flag this honestly rather than treating the log as proof of correct visuals.
- **PAC/instance-component lifetime on EndPlay/recovery** — verify the component cleans up the PAC it creates so instance components don't leak across runs (raised in the independent answer; not addressed in the draft).

## Notes
The draft's `BelowBodiesImpulseFraction`/coherence tuning that the independent answer flagged as the likely smear cause is now profile-data; good. Numbers are explicitly left as user feel-tuning, which matches intent. No file mutations were made in this review.
