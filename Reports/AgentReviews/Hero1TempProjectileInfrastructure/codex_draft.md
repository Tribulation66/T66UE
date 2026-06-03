Draft answer:

Your high-level model mostly matches the live game, with two important corrections.

Confirmed:
- Hero 1 currently has 4 weapon rows: `Hero_1_black_aoe`, `Hero_1_red_aoe`, `Hero_1_yellow_aoe`, and `Hero_1_white_aoe`.
- Those 4 Hero 1 weapon rows are all `Branch=AOE`; they are rarity variants of Hero 1's locked AOE weapon family, not four different attack categories for Hero 1.
- `Heroes.csv` sets `Hero_1` `PrimaryCategory=AOE`; `UT66WeaponManagerSubsystem::BuildWeaponOffers` uses that primary category when building the rarity offer.
- `Idols.csv` has 16 rows in the intended 4 element x 4 category grid: Fire/Ice/Electricity/Nature x DOT/AOE/Pierce/Bounce.
- Each idol row has four rarity icon columns: `BlackIcon`, `RedIcon`, `YellowIcon`, `WhiteIcon`.
- Therefore "16 idols x 4 rarity variants = 64 idol temporary visual variants" is correct as a visual expansion. There are still only 16 idol data rows. Including the 4 Hero 1 weapon rarity variants, the first pass is 68 temporary visual variants.
- The live code name is `Electricity`, not `Lightning`; use `Electricity` in IDs/profile keys even if the user-facing copy says lightning.

Mismatches:
- Your proposed colors are partly new authoring intent. Current `UT66IdolManagerSubsystem::GetIdolColor` is Fire red/orange, Ice blue, Electricity yellow, Nature green. The current outgoing-traveler helper also uses yellow for Electricity. If you want purple lightning, the temp visual layer needs a new color convention.
- The existing `FT66TemporaryProjectileSystem` is not sufficient for the requested matrix. It has four hero attack profiles plus one idol overlay profile, and it forces hero/idol overlay profiles to the hardcoded hero blue color. That does not support black weapons, element-colored idols, or rarity-specific idol shapes.
- `UT66OutgoingTravelerPoolSubsystem` already has 16 `TravelerVisual.<Element>.<Category>` mesh slots, matching the 16 idol rows, but it has no rarity dimension. The 64 variant plan needs either profile IDs that include rarity or a separate rarity-to-shape selector.
- `CombatVFXBindings.csv` currently has 20 `WeaponBase` rows and no `IdolModifier` rows. The Hero 1 AOE rarity rows are bound to production Niagara and suppress temp weapon projectiles by default. Idol overlays are explicitly deferred in binding notes.

Recommended infrastructure:
- Keep this as a temporary-shape infrastructure pass, not a Niagara/material authoring pass.
- Add a small data-driven temporary visual registry keyed by source kind + source ID/category/rarity, instead of growing hardcoded `if ProfileID == ...` branches.
- For weapons, key by `WeaponID` or by `HeroID + Branch + Rarity`. For this pass, Hero 1 maps to four black primitive shapes.
- For idols, key by `IdolID + Rarity`, with `Element` providing color and `Category + Rarity` or explicit row data providing shape/profile. This produces the 64 idol visual variants without creating 64 idol data rows.
- Feed the registry into both live presentation seams: moving travelers through `FT66OutgoingTravelerFireParams::TravelerVisualProfileID` / color, and idol-owned impact placeholders through the impact-context VFX path. Do not collapse idol visuals into weapon visuals, because production VFX proof later requires idol-owned source identity, parent source identity, impact context, and damage attribution.
- Keep production bindings and final Niagara assets separate. Temporary primitives can be acceptable for infrastructure proof only; final VFX still need the existing PPF, artifact parity, mechanism manifest, visual/damage alignment, impact-context proof, Unreal-owned capture, and staged validation when runtime/playable behavior changes.

Suggested first implementation decision:
- Confirm whether the temporary idol shapes should be authored in a new CSV/table, in `CombatVFXBindings.csv` with extended fields/profile IDs, or in C++ defaults. I recommend a small data table/CSV for temporary visual profiles so the 68 variants are auditable and later replaceable without code edits.

Verification performed:
- Read root/operator/combat/report instructions and pending issues.
- Ran Claude independent answer with `Result: OK`.
- Imported and counted `Weapons.csv`, `Idols.csv`, `Heroes.csv`, and `CombatVFXBindings.csv`.
- Read relevant C++ seams: weapon offer routing, temporary projectile helper, outgoing traveler pool visual profile slots, idol manager colors/stock, combat component traveler/impact paths, and idol placeholder spawning.
- No code/content implementation, compile, Unreal run, capture, or staged build was performed because this was read-only exploration.
