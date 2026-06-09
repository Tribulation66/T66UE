All Codex claims verify against the actual files. Edits match, decision is sound, caveats are honest.

Result: OK

## Summary
Codex made the correct architecture call (NO — heroes do not use PAC) on the user's own stated principle (let chaos run, no return-to-pose). The claimed edits all check out: `T66KnockbackComponent.h:145,148` now default `bEnablePhysicalAnimation=false` / `DriveMode=Disabled`; `T66KnockbackComponent.cpp:150-162` adds a hero-specific runtime guard alongside the existing detached-path guard (171-173); `pending_issues_Gameplay.md:6-8` updates the stale note with honest caveats; the TestRoom wipeout-arm CVars exist. This matches my independent answer, and Codex correctly kept the PAC infrastructure in place (disable, not removal).

## Suggested Answer Patch
None required. The draft is accurate and ready to send. Optional tightening: state explicitly that the hero guard keys off `BudgetClass == Hero || IsA<AT66HeroBase>()` (cpp:150-152), so reviewers know the disable is scoped by hero identity, not global.

## Issues To Fix
- None blocking. Codex went slightly beyond the minimal change my independent answer suggested (a dedicated hero guard branch in addition to flipped defaults + detached guard). This is reasonable defense-in-depth, not a problem — no action needed.

## Question For User
None. The user supplied the deciding principle; this was an internal architecture call.

## Evidence Or Verification Gaps
- The "focused build passed" and "StageStandaloneBuild.ps1 passed / T66.exe produced" claims are not independently verifiable from here (no build log in scope). They are plausible and the source compiles logically, but if the loop wants hard proof, Codex should retain the build output line/log path.
- I did not confirm the default *value* of `CVarT66TestRoomEnableWipeoutArmTrap` is off — only that the CVars exist. Codex should confirm the "default off/disabled" wording matches the actual registered default.

## Notes
- The user's "even bosses... don't come back to a specific place" remark is a related but separate behavior (boss anchoring/targeting), not PAC. Codex correctly did not fold it into this task; flag it as a future scope item rather than silently expanding.
