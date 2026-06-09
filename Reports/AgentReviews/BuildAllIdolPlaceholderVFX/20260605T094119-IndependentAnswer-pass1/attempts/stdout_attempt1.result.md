Result: OK

## Independent Answer

The work is implementable from current evidence without a user-only decision — the concept, element colors, and rarity ordering are all approved and the 20 idols plus the No Weapon single-target path already exist in code and data. Codex should proceed, but the draft must resolve one routing risk and one scope-fidelity risk that I'd otherwise expect to silently under-deliver.

**Likely owning files**
- `Source/T66/Gameplay/T66TemporaryProjectileSystem.cpp/.h` — the basic-shape lane (profiles → `Cone/Cylinder/Cube/Sphere`, colors, scales). `ProfileHeroSingleTarget` already returns a white sphere (`NoWeaponProjectileColor` ≈ white) — this is the natural home for the "white ball punching one enemy" No Weapon case (`SingleTarget` category already maps to single-target `PerformPierce` at `T66CombatComponent.cpp:3754/3767`).
- `Source/T66/Gameplay/T66CombatVFX.cpp` — `SpawnIdolImpactPlaceholderVFX` (category switch: Pierce→line, Bounce→chain, DOT/AOE→area), the `SpawnIdol{Pierce,AOE,Bounce,DOT}VFX` entry points, and `GetIdolNiagaraEffectPath`.
- `Source/T66/Core/T66IdolManagerSubsystem.cpp` — `GetIdolColor` (Wind = grey `(0.62,0.65,0.68)` at line 178; verify Fire/Ice/Electricity-purple/Nature/White all match the approved palette).
- `Source/T66/Gameplay/T66CombatShared.cpp` — rarity visual scale/quantity (verify Black→Red→Yellow→White is ascending intensity).
- `Content/Data/Idols.csv` — reference only; all 20 idols incl. Wind x4 are present.

**Top risks Codex must address in the draft**
1. **Niagara-fallback masking (highest).** `SpawnIdol*VFX` try `GetIdolNiagaraEffectPath` *first* and only fall through to the basic-shape placeholder when no path resolves. Fire/Ice/Electricity/Nature all have Niagara entries, so for 16 of the 20 idols the basic shapes will never render — only Wind (no Niagara entry) falls through. If the intent is basic-shape proof for *all 20*, Codex must add an explicit placeholder-mode gate (CVar/flag) that forces the basic-shape lane, or it has not actually built what was approved.
2. **Concept-fidelity vs current vocabulary.** The approved direction describes 20 *distinct* behaviors (tornado moving in a circle with smaller damage radius, lightning from above, frost nova, flame lance, root-on-ground, etc.). The existing placeholder lane only has 4 category reads (line/chain/area/area). Codex's draft should state plainly whether it is delivering per-element distinct primitive shapes or reusing the 4 generic category reads — the latter would be a thinner result than the user approved. This is an implementation judgment, not a user decision, but it must not be glossed over.

**Verification Codex should perform**
- C++ compile/build of the `T66` + `T66Editor` modules.
- Runtime spawn check in `T66GameMode_TestRoom`, equipping representative idols across all 5 elements + each category + No Weapon, watching for the `CombatVFXIdolImpactPlaceholderSpawned` verbose log lines to confirm the placeholder lane (not Niagara) actually fires.
- Report visual proof as skipped if headless — explicitly allowed by the stop condition.

## Evidence Checked
- `T66TemporaryProjectileSystem.cpp:54-205` — profile/shape/color/scale model; `ProfileHeroSingleTarget` white sphere.
- `T66CombatVFX.cpp:434-443, 1313-1447` — Niagara path table (no Wind entry), placeholder category switch, Niagara-first fallback ordering.
- `T66IdolManagerSubsystem.cpp:161-178` — all 20 idols incl. Wind; element colors.
- `Content/Data/Idols.csv:18-20` — Wind idols exist with Black/Red/Yellow/White rarity sprite tiers.
- `T66CombatComponent.cpp:3754/3767` — SingleTarget → single-target pierce.
- `AGENTS.md` §1–2 — process constraints; temp placeholder lane is the approved class, not final Niagara.

## Questions Or Blockers
None that are user-only. The Wind-idol existence question (a plausible blocker) is resolved: all 20 are in `Idols.csv` and the idol registry.

## Caveats
- I did not execute a build; "implementable" is a static read.
- I did not confirm every element color literal against the spec verbatim (Electricity is specified as *purple* — confirm `GetIdolColor` returns purple, not the legacy yellow/storm hue).
- Visual correctness of the primitive behaviors cannot be validated headlessly; treat any "looks right" claim as unverified until the user eyeballs it in-editor.
