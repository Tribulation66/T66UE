Verdict: REVISE

## Blockers
- None that hard-block, but see the legacy-mapping value risk under Major Issues; it should be resolved before this is treated as save-safe.

## Major Issues
- **Legacy `CritDamage` → `HeadshotChance` value semantics are unproven.** The parser maps the legacy backend `CritDamage` key into live `HeadshotChance`, and the smoke "proves legacy backend `CritDamage` key maps to live `HeadshotChance`." But `CritDamage` was a damage *multiplier* (historically ~1.5–2.0), while `HeadshotChance` is a 0–1 probability. Injecting an old multiplier value into a chance field could yield 150–200% headshot chance on legacy saves. The evidence proves the *key* routes; it does not prove the *value* is clamped, ignored, or sanity-bounded. This is the single weakest-pass risk: the implementation can look green while silently corrupting old-save stat ranges.
- **Boss stun vs. boss OHKO-rejection inconsistency.** Combat applies Headshot stun to "enemies/mobs/bosses," yet `item_taxonomy_smoke` deliberately proves Execute/Assassinate/Crush *reject* bosses. Whether bosses should be stun-immune (as they are instakill-immune) is a design call that the packet decided silently. If bosses are meant to be CC-resistant, this is a real gameplay regression hiding behind a passing smoke.

## Minor Issues
- **All heroes authored to `BaseHeadshotChance = 0.0`.** Reasonable given crit is now fixed at 2x and base crit-damage values are meaningless, but it means headshot comes only from items/levels/drugs. Confirm this is intended rather than a placeholder.
- **Stray `T66MinigamesScreen.cpp` worktree modification.** Correctly declared out of scope, but it is live in the worktree. If any commit follows, it risks being swept in. Flag for exclusion at commit time.
- **`GetSecondaryStatValue(CritDamage)` returns `2.0` as compatibility.** Confirm no surviving UI/tooltip path still reads this and renders a stale "Crit Damage 2.0" line that contradicts the Headshot migration.

## Clarifying Questions
- Should bosses be immune to Headshot stun (mirroring their OHKO immunity), or is stunning bosses intended?
- Is `BaseHeadshotChance = 0.0` for every hero the intended final authoring, or a temporary value pending per-hero tuning?
- For legacy saves carrying a `CritDamage` multiplier value: should that value be discarded/reset, clamped to a chance range, or is it acceptable to import it directly?

## Required Verification
- Add a legacy-save case that feeds a representative *old* `CritDamage` value (e.g., 1.5 or 2.0) through the parser and asserts the resulting live `HeadshotChance` is within `[0,1]` (or is reset/ignored per the design answer). Current smokes only assert key routing, not value bounds — this is the named gap.
- Verify and assert Headshot stun behavior against a boss target matches the intended design (stunned vs. immune). The existing stun proof (`Chance=0.500 StunRemaining=0.750`) is against a generic hit target only.
- Confirm enum reorder safety: assert that secondary-stat *serialization* keys are name/string-based (not ordinal), so appending `HeadshotChance` while reordering the Accuracy family does not shift any persisted/indexed values. The packet states string keys but provides no round-trip evidence.
- Confirm no UI surface still renders the `2.0` compatibility `CritDamage` value (grep/smoke over tooltip + stats panel render output, not just source edits).

## Rationale
The migration is broad and well-instrumented (data, runtime, UI, staged build, dual smokes), but the verification proves *plumbing* (keys route, items swap, 2x crit fixed, stun fires once) more than *semantics*. The assumption I most challenged is that mapping a legacy `CritDamage` multiplier into a 0–1 `HeadshotChance` field is value-safe — nothing in the evidence bounds that value, so a passing smoke could mask out-of-range chance on existing saves. The boss-stun/boss-OHKO asymmetry is the second silent design decision. Both are Codex-resolvable via tighter verification plus one user design confirmation, so REVISE rather than APPROVE or escalate.

