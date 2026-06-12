Verdict: APPROVE

## Blockers
- None.

## Major Issues
- None that block. The fixed-`2.0` retention of `GetCritDamageMultiplier()` is the right call: it preserves crit-damage math at every existing call site (auto-attack, idol procs, abilities) without requiring the plan to enumerate them all, which neutralizes the otherwise-real risk of missing a crit-damage consumer.

## Minor Issues
- Headshot stun is wired only into the auto-attack resolve path. If any non-auto-attack damage source (abilities, idol procs) is expected to also benefit from the new secondary, that is unstated. Plan scope (auto-attack hits roll `GetHeadshotChance01()`) is internally consistent, so this is a scope clarity note, not a defect.
- The live formula multiplies a probability by `M` (drug/single-use multipliers) and `HeroAccuracyMultiplier`. This mirrors the existing secondary path and is clamped to `[0,1]`, so it is consistent, but designers should be aware base-chance tuning interacts multiplicatively with accuracy — worth a one-line note in `MASTER_STATS.md`.

## Clarifying Questions
- Positional drug-slot migration (old Crit Damage single-use slots → Headshot Chance at the same array index) is a product interpretation of "replace." The packet flags it explicitly and it is defensible from the user's "replace Crit Damage" wording. Proceeding is reasonable; if the user later wanted refund/clear instead, that would be a separate change. No stop required.

## Required Verification
- The plan's verification is adequate and matches AGENTS/GAMEPLAY_AGENTS requirements: data-table reloads (Items/Heroes/PlayerExperience), focused editor build, editor smoke, staged standalone refresh + staged smoke, shortcut-target check.
- Ensure the extended `-T66StatPipelineSmoke` actually asserts all eight enumerated cases — in particular old `CritDamage` backend/save key → live `HeadshotChance`, and the appended-enum inclusion past `VendorToken` (the two highest-regression-risk seams).
- Confirm `DT_Heroes` reload succeeds after the `BaseCritDamage` → `BaseHeadshotChance` header rename before relying on compile validation, since a header/struct mismatch will surface there first.

## Rationale
Pass-1 corrections are all addressed: chance-per-point is data-driven in `FT66PlayerExperienceDifficultyTuning`, formula terms and C++ seams are anchored to owning files, old-key migration has explicit smoke coverage, and the old-drug positional-migration assumption is stated. Anchors are specific and live-repo-facing, deprecated compatibility paths are preserved per the out-of-scope rules, the dirty `T66MinigamesScreen.cpp` is explicitly fenced off, and verification covers runtime + staged build paths. No instruction contradictions, no unsafe scope expansion, and the one product judgment (drug migration) is surfaced rather than hidden. Safe for Codex to implement under the reviewed scope.

