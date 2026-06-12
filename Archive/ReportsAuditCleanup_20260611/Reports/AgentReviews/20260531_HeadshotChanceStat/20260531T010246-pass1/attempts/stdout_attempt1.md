Verdict: REVISE

## Blockers
- None that require human decision or external prerequisites.

## Major Issues
- **Hardcoded `BonusPoints * 0.005` per-point conversion contradicts repo guidance.** `Gameplay/GAMEPLAY_AGENTS.md` says prefer data-authored tuning over hardcoded C++ defaults. The packet correctly data-drives the stun duration (`0.75`) but bakes the chance-per-point factor as a literal in the formula. Either author it alongside `HeadshotStunDurationSeconds` in `FT66PlayerExperienceDifficultyTuning`/CSV, or justify in the plan why this one constant is exempt.
- **Formula terms `M` and `HeroAccuracyMultiplier` are unsourced.** The clamp formula references a "selected-drug multiplier `M`" and an Accuracy primary multiplier, but the packet does not cite where these values come from (which getter, which data field) nor confirm they exist on the live path. Without anchors this cannot be verified as non-inventing behavior.

## Minor Issues
- **C++ seams are named but not anchored.** Item/CSV seams cite file paths, but the runtime/combat seams (`GetCritDamageMultiplier()`, `GetSecondaryStatValue`, auto-attack crit resolution, `ApplyStun` call sites) are described only by symbol name with no file path/location. Targeted review needs at least the owning file for each edited seam.
- **Hero data loss is asserted, not confirmed safe.** Converting all existing `BaseCritDamage` column values to `0.0` discards authored per-hero values. Stated as a design assumption, but the packet should confirm no hero currently relies on a non-default crit-damage base that designers expect to carry forward as headshot base.

## Clarifying Questions
- Does the drug-slot positional migration (CritDamage → HeadshotChance at same array index in `UT66BuffSubsystem`) match the intended player experience for already-owned Crit Damage drugs, or should owned slots be cleared/refunded instead? This is a balance/UX choice worth one confirmation.

## Required Verification
- The packet's verification plan is adequate in shape: data-table reloads (Items/Heroes/PlayerExperience), focused `Build.bat`, editor stat-pipeline smoke, staged standalone refresh + staged smoke, shortcut target check, and `ANTHROPIC_API_KEY` unset precondition.
- Add an explicit smoke assertion that the parser maps an old `CritDamage` save key to live `HeadshotChance` (save-compat path is claimed but not in the smoke list).
- Confirm the `VendorToken`-last-enum loop fix is covered by a build/smoke check, since an appended enum can silently break range loops.

## Rationale
Scope is coherent, correctly fenced (Mini excluded, compat paths preserved, existing Headshot passive/hit-zone untouched), and verification is appropriately staged for runtime gameplay changes. The plan is not yet approvable because the core chance formula carries an unsourced hardcoded factor that conflicts with the data-authored-tuning instruction, and two formula multipliers plus the C++ combat seams lack cited anchors to verify against live state. These are all Codex-resolvable by anchoring the seams, sourcing/data-driving the formula terms, and extending the save-compat smoke — so REVISE rather than BLOCK or NEEDS_HUMAN_DECISION.

