# Hero 1 Temporary Combat Shape Implementation Plan

## Task Contract

Working task:
Operator: Codex
Validator: Claude
Scope: Planning-only for temporary Hero 1 AOE weapon shapes and idol visual variants, including purple Electricity, ice-blue Ice, canonical idol ID normalization, and removal of legacy-ID dependence from temporary visual lookup.
Stop condition: Deliver an implementation plan. No runtime/content implementation yet.

## Live Repo Facts Used

- `Content/Data/Weapons.csv` confirms all four Hero 1 weapon rows are `AOE`: `Hero_1_black_aoe`, `Hero_1_red_aoe`, `Hero_1_yellow_aoe`, and `Hero_1_white_aoe`.
- `Content/Data/Idols.csv` confirms the current 16 canonical idol rows: four elements by four attack categories.
- `UT66IdolManagerSubsystem::NormalizeLegacyIdolID` maps older idol IDs such as `Idol_Water`, `Idol_Storm`, and `Idol_Electric` to canonical IDs.
- `UT66IdolManagerSubsystem::GetAllIdolIDs` is already canonical-only.
- `UT66IdolManagerSubsystem::IdolTierValueToRarity` maps idol tier values `1..4` to existing `ET66ItemRarity` values: `Black`, `Red`, `Yellow`, and `White`.
- `UT66IdolManagerSubsystem::GetIdolColor` and `T66CombatComponent` still return yellow-ish Electricity colors.
- `FT66TemporaryProjectileSystem` currently hard-forces hero and idol overlay temporary profiles to blue, and enemy/trap profiles to red.
- `OutgoingTravelerVisualProfilesAuthoring.md` defines 16 element/delivery traveler slots, not a rarity axis.
- `MASTER_COMBAT.md` still documents the older blue hero/idol temporary visual contract, so docs must be updated when the new element-color temp convention is implemented.

## PPF Gate For The Later Implementation

PPF CHECK
Objective: Build temporary shape/color infrastructure for Hero 1 AOE weapons and 16 idol x 4 rarity visual variants using simple primitive/pattern placeholders.
Proven process: `Gameplay/Combat/MASTER_COMBAT.md`, `Source/T66/Gameplay/T66TemporaryProjectileSystem.*`, and `Gameplay/Combat/OutgoingTravelerVisualProfilesAuthoring.md`.
My planned implementation: Extend the existing temporary projectile and outgoing traveler presentation layer. Do not author final Niagara/material VFX in this pass.
Same method class: YES for temporary placeholder infrastructure; NO for final production VFX, which remains deferred to `Gameplay/Combat/CombatVFXAuthoringProcedure.md`.
If NO, why: Only final Niagara/material visual production is out of scope here.
User approval required before proceeding: NO for the temporary infrastructure plan; YES before treating any temporary shape as final production VFX.
Verification evidence: data validation, focused compile, DataTable reload if a new table is added, Unreal-owned proof captures for selected shapes, and staged standalone after runtime visual changes are implemented.

ARTIFACT PARITY GATE
Reference artifact/category: Existing temporary projectile primitive shapes and outgoing traveler visual profiles.
Role: Primary for this temporary pass.
Required: YES.
Planned artifact/path: data-driven temporary profile rows plus resolver changes in `FT66TemporaryProjectileSystem` / combat integration.
Status: SAME.
Evidence: uses current basic shape meshes, profile IDs, colors, scale, and traveler mesh index selection.

Reference artifact/category: Final Niagara/material projectile, trail, slash, aura, or elemental VFX.
Role: Primary for final VFX only.
Required: NO in this temporary infrastructure pass.
Planned artifact/path: deferred.
Status: DEFERRED.
Evidence: final VFX remains governed by `Gameplay/Combat/CombatVFXAuthoringProcedure.md`.

MECHANISM MANIFEST
Reference/source: Temporary projectile system and outgoing traveler visual profile docs.
Required mechanisms:
  1. Mechanism: canonical source resolution
     Required: YES
     Planned implementation: normalize every idol visual lookup to canonical IDs before profile resolution.
     Evidence needed: validator rejects legacy IDs as temporary visual keys.
  2. Mechanism: color policy resolution
     Required: YES
     Planned implementation: centralize Hero 1 weapon temporary attack color as black per user direction, and idol element colors as Fire red, Ice ice-blue, Electricity purple, Nature green.
     Evidence needed: automated color-row validation plus visual proof for sample cases.
  3. Mechanism: shape/pattern selection
     Required: YES
     Planned implementation: resolve a temporary profile by source type, canonical source ID, category, and rarity, then apply primitive shape, scale, offset, and pattern metadata.
     Evidence needed: each implemented shape reports its resolved profile and appears in an Unreal-owned capture.
  4. Mechanism: pooled traveler compatibility
     Required: YES
     Planned implementation: keep the 16 element/delivery traveler slot model as the first mesh-slot layer, and layer 64 profile keys through runtime color/scale/pattern data unless a selected shape truly needs a new slot.
     Evidence needed: outgoing traveler distinctness gate still shows valid slot use and no pool regressions.

## Plan

### 1. Normalize IDs First

1. Define one canonical temporary-visual ID list:
   - `Idol_Fire_DOT`, `Idol_Fire_AOE`, `Idol_Fire_Pierce`, `Idol_Fire_Bounce`
   - `Idol_Ice_DOT`, `Idol_Ice_AOE`, `Idol_Ice_Pierce`, `Idol_Ice_Bounce`
   - `Idol_Electricity_DOT`, `Idol_Electricity_AOE`, `Idol_Electricity_Pierce`, `Idol_Electricity_Bounce`
   - `Idol_Nature_DOT`, `Idol_Nature_AOE`, `Idol_Nature_Pierce`, `Idol_Nature_Bounce`
2. Add or expose a small canonical helper, likely near `UT66IdolManagerSubsystem::NormalizeLegacyIdolID`, that returns canonical IDs for visual lookup and reports whether the input was legacy.
3. Remove legacy IDs from all new temp visual profile keys and proof arguments.
4. Keep `NormalizeLegacyIdolID` as a compatibility boundary until a save/proof/content audit proves no legacy IDs are still needed. The first implementation should remove legacy IDs from authored visual data and runtime lookup, not delete compatibility mappings blindly.
5. Add validation that fails if a new temporary visual row uses `Idol_Water`, `Idol_Storm`, `Idol_Electric`, or any other legacy alias as a key.

### 2. Establish The New Temporary Color Contract

1. Update the central temporary visual color policy:
   - Hero 1 weapon temporary attacks: black, regardless of the weapon row's rarity label.
   - Fire idols: red.
   - Ice idols: ice-blue.
   - Electricity idols: purple.
   - Nature idols: green.
2. Audit current color consumers before changing shared helpers:
   - `UT66IdolManagerSubsystem::GetIdolColor`
   - `T66CombatComponent::GetT66IdolElementTravelerColor`
   - `FT66TemporaryProjectileSystem::HeroProjectileColor`
   - UI/HUD uses of idol colors, if any.
3. Avoid accidentally turning non-combat UI Electricity purple unless that is desired. If UI should stay separate, introduce a combat temporary color helper instead of overloading the UI/general idol color helper.
4. Update `Gameplay/Combat/MASTER_COMBAT.md` and related combat docs so the new temporary visual contract no longer contradicts the implementation.

### 3. Add A Data-Driven Temporary Visual Matrix

1. Add a dedicated temporary combat visual profile data source rather than hardcoding all shapes in `T66CombatComponent`.
2. Preferred shape:
   - `Content/Data/TemporaryCombatVisualProfiles.csv`
   - new row struct such as `FT66TemporaryCombatVisualProfileData`
   - setup/reload script similar to `Scripts/SetupIdolsDataTable.py`
3. Suggested columns:
   - `ProfileID`
   - `SourceType` (`WeaponBase` or `IdolModifier`)
   - `SourceID`
   - `AttackCategory`
   - `Rarity`
   - `Element`
   - `ColorPolicy`
   - `ColorOverride`
   - `TemporaryShape`
   - `PatternID`
   - `PrimitiveCount`
   - `ScaleMultiplier`
   - `RadiusPolicy`
   - `SpawnOffsetZ`
   - `TravelerVisualProfileID`
   - `bUseOutgoingTraveler`
   - `Notes`
4. Seed exactly 68 initial rows:
   - 4 Hero 1 AOE weapon rows with `SourceType=WeaponBase`.
   - 64 idol rows with `SourceType=IdolModifier`: 16 canonical idols x 4 `ET66ItemRarity` values.
5. For rows whose exact shape has not been chosen yet, use a clear placeholder profile such as `TBD_PrimitiveDefault` and mark `Notes=ShapePendingUserSelection`. The infrastructure is complete, but shape approval remains one-by-one.

### 4. Resolve Profiles At Runtime

1. Add a resolver context struct that includes:
   - `SourceType`
   - canonical `SourceID`
   - `AttackCategory`
   - `Rarity`
   - `Element`
   - optional parent weapon/source context.
2. Extend `FT66TemporaryProjectileSystem` from profile-only shape lookup to context/profile lookup.
3. Keep existing hostile/enemy/trap behavior stable.
4. Change Hero 1 weapon visual calls so the four AOE rows resolve by exact weapon ID and rarity, not just `ET66AttackCategory::AOE`.
5. Change idol visual calls so each equipped slot uses:
   - normalized idol ID
   - `FIdolData.Category`
   - `FIdolData.Element`
   - equipped idol rarity from `GetEquippedIdolRarityInSlot`
6. Reuse the same resolver for traveler spawns, impact placeholders, and any temporary mesh overlay so the 64 idol rows cannot drift apart between paths.

### 5. Treat 64 Variants As Profile Keys, Not 64 Traveler Slots Yet

1. Do not expand the outgoing traveler pool to 64 or 68 Niagara mesh slots in the first implementation.
2. Keep the current 16 element/delivery slots as the first pooled-traveler mesh layer.
3. Let the 64 idol rows choose color, scale, primitive fallback, simple pattern, and profile metadata.
4. If a later one-by-one shape needs a distinct mesh/material slot that cannot be represented by the current primitive/profile vocabulary, add that slot in the narrow shape pass with its own proof.
5. This keeps performance risk contained and matches the existing single pooled Niagara selector contract.

### 6. Implement Shapes One By One

Order for the later shape passes:

1. Hero 1 black AOE.
2. Hero 1 red AOE.
3. Hero 1 yellow AOE.
4. Hero 1 white AOE.
5. Idol rows by element and category, one row or small coherent group at a time:
   - Fire DOT/AOE/Pierce/Bounce across `ET66ItemRarity` Black/Red/Yellow/White.
   - Ice DOT/AOE/Pierce/Bounce across `ET66ItemRarity` Black/Red/Yellow/White.
   - Electricity DOT/AOE/Pierce/Bounce across `ET66ItemRarity` Black/Red/Yellow/White.
   - Nature DOT/AOE/Pierce/Bounce across `ET66ItemRarity` Black/Red/Yellow/White.

For each shape pass:

1. Pick the target `ProfileID`.
2. Choose primitive shape/pattern/scale from the approved simple-shape vocabulary.
3. Implement only that row or group.
4. Run the temp visual validator.
5. Capture the selected shape with an Unreal-owned capture path.
6. Log accepted/rejected and move to the next shape.

### 7. Validation Required When Implementation Starts

Required automated checks:

1. Validator confirms exactly 4 Hero 1 weapon temp rows.
2. Validator confirms exactly 64 idol temp rows.
3. Validator confirms all idol temp rows use canonical IDs only.
4. Validator confirms all 64 idol row colors match the new element policy.
5. Validator confirms Hero 1 rows use black as the temporary weapon-attack color, independent of weapon rarity.
6. Validator confirms no Mini/minigame paths are touched.
7. Validator confirms every non-`TBD` row resolves to a legal temporary shape/pattern.

Required Unreal/data checks:

1. Run the new setup script if a DataTable is added.
2. Run focused compile after C++ resolver changes.
3. Run the outgoing traveler visual profile gate after changing traveler path behavior.
4. Run per-shape Unreal-owned capture for each accepted shape.
5. Refresh staged standalone after runtime visual implementation because it affects playable combat presentation.

## Out Of Scope For This Plan

- Final Niagara/material VFX authoring.
- Texture/material polish.
- Mini/minigame systems.
- Deleting legacy save compatibility before a separate save/content audit.
- Bulk approving all 68 shapes without per-shape review.

## Main Risks

- Deleting legacy aliases too early could break saved runs, automation proof modes, or older docs/scripts that still pass legacy IDs.
- Reusing shared idol color helpers could unintentionally change non-combat UI color semantics.
- Treating 64 variants as 64 traveler mesh slots immediately would increase authoring and performance risk before we know the actual shapes.
- `MASTER_COMBAT.md` currently contradicts the requested new element-color temp contract, so docs and implementation must change together.

## Rollback Strategy

- Keep the new temporary profile matrix isolated in one data source and one resolver.
- Preserve existing hostile/enemy/trap temporary profiles.
- Preserve legacy-ID normalization compatibility until explicit deletion is verified safe.
- If the new table path causes runtime issues, revert integration to the existing profile-only `FT66TemporaryProjectileSystem` while keeping the plan and validator artifacts for a cleaner second pass.
