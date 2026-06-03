Result: OK

## Independent Answer

I confirmed the user's stated structure against live repo data. The counts, grid, and rarity columns are correct; a few naming/scoping nuances need calling out before any build.

**Confirmations (live data):**
- **Hero 1 weapons = 4.** `Weapons.csv` rows 2–5: `Hero_1_black_aoe`, `_red_aoe`, `_yellow_aoe`, `_white_aoe`. All are `Branch=AOE`; the 4 are **rarity tiers** (Black/Red/Yellow/White) of one AOE axe — *not* 4 distinct weapon categories. So "4 weapons → 4 shapes" means 4 rarity-shape variants of the AOE axe.
- **Idols = 16 rows** in a clean **4 element × 4 category** grid: elements Fire/Ice/Electricity/Nature × categories DOT/AOE/Pierce/Bounce (`Idols.csv` rows 2–17). Confirmed.
- **Each idol row has 4 rarity columns:** `BlackIcon`, `RedIcon`, `YellowIcon`, `WhiteIcon`. Confirmed.
- **"64 projectile types" = 16 idol rows × 4 rarity visual variants = 64.** Correct. There are only **16** idol data rows, not 64; the 64 is the visual-variant expansion. The 4 Hero 1 weapon shapes are **separate** from (and in addition to) the 64 idol shapes (68 temp shapes total for this first pass).

**Corrections / nuances:**
- The user said "lightning"; the data element is named **Electricity**. Same concept, different label — use `Electricity` when keying anything.
- The proposed colors (weapons=black; fire=red, ice=light blue, lightning/electricity=purple, nature=green) are a **new temp-marker convention** — they do **not** exist in the data today. Idols carry sprite icon paths and hero rows carry a `PlaceholderColor`, but there is no element→color field. Fine to adopt, but it's authoring intent, not current state.
- Note weapons get **black + shape-per-rarity** (so rarity is read by shape only), while idols get **element-color + shape-per-rarity**. Consistent with the user's framing; worth confirming that's intended.

**Safest infrastructure lane for temporary shapes:**
- An explicit temp-vs-production seam **already exists**: `CombatVFXBindings.csv` has `bSuppressTemporaryProjectile` and `bDevelopmentFallbackAllowed`, plus `VFXProfile`, and `Heroes.csv` has an `AutoAttackProjectileMesh` static-mesh slot. The cheapest correct lane is to drive simple primitives (sphere/etc.) through the **existing static-mesh projectile + development-fallback binding path**, keeping `bDevelopmentFallbackAllowed=True` and clearly tagging rows as placeholder — **do not author Niagara** for these.
- **Open infra gap to flag:** idols are currently modeled as **weapon overlays/modifiers**, not as first-class projectile owners — every binding row is `SourceType=WeaponBase`, and notes say "idol overlays deferred." There is no per-idol temporary-shape binding slot today. The 64 idol shapes need a decision on *where they attach*: a new idol-keyed binding (`SourceType=Idol...`) vs. overlaying the weapon projectile. This is a design decision, not derivable from current data.

## Evidence Checked
- `Content/Data/Weapons.csv` — Hero_1 has 4 AOE rarity rows; confirmed 4-weapon claim and rarity dimension.
- `Content/Data/Idols.csv` — 16 rows, 4×4 element/category grid, 4 rarity icon columns.
- `Content/Data/Heroes.csv` — Hero_1 = "Founding Chad", `PrimaryCategory=AOE`, has `AutoAttackProjectileMesh` static-mesh slot and `PlaceholderColor`.
- `Content/Data/CombatVFXBindings.csv` — temp/production seam columns (`bSuppressTemporaryProjectile`, `bDevelopmentFallbackAllowed`, `VFXProfile`); all rows `SourceType=WeaponBase`; "idol overlays deferred."
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md` — staged acceptance gates, visual/damage alignment (3.4), impact-context (3.5), flipbook/bake rule, Unreal-owned capture.

## Questions Or Blockers
None block the read-only deliverable. Decisions to surface to the user (recommendations, not blockers): (1) confirm idols attach as weapon overlays vs. get their own `SourceType=Idol` projectile/temp-shape binding; (2) confirm the weapons-black / idols-element-color split is intended; (3) confirm temporary primitive shapes are explicitly exempt from the full Niagara acceptance gate while flagged as placeholders.

## Caveats
- Temporary simple shapes do **not** satisfy production acceptance. Per `AGENTS.md` + the authoring procedure, promotion still requires PPF, artifact parity, mechanism manifest, **idol-owned impact-context proof** (3.5 — cannot be inherited from the weapon context), **visual/damage alignment** (3.4 — a compact marker is not a full AOE area read), anti-lookalike, Unreal-owned capture, and staged verification when runtime/playable content changes.
- Colors are unowned authoring intent today; if adopted, they need a real data/material home, not just prose.
- Scope is read-only exploration; nothing here authorizes content/code changes, and mini/minigame scope stays excluded.
