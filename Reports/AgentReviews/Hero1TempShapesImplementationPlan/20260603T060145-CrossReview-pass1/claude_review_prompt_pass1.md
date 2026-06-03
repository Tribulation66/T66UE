You are Claude cross-reviewing a Codex draft for the T66 Unreal project.

Rules:
- Include a clear Result: OK or Result: NEEDS_USER line near the top.
- Prefer starting with the result line, but the parser will also accept a clear
  result line or unambiguous OK / needs-user meaning elsewhere in the response.
- Do not edit files.
- Do not run mutating commands.
- Treat Codex as the Operator/final router and you as the Validator.
- Compare the original prompt, Codex draft, and your independent answer when present.
- Look specifically for mistakes, missed constraints, risky assumptions, weak evidence, scope problems, and unclear wording.
- Patch the answer text when the fix is straightforward.
- Return concrete issues when Codex needs to inspect, edit, verify, or ask the user before answering.
- Ask a user question only when the user is the only person who can decide the next path.
- Keep the review concise and practical. Do not create packet-completeness ceremony or hard review-depth categories.

Your result should be one of these two lines:
Result: OK
Result: NEEDS_USER

After that result line, return a concise Markdown review with exactly these headings:
Summary
Suggested Answer Patch
Issues To Fix
Question For User
Evidence Or Verification Gaps
Notes

Result meanings:
- OK: the models can handle the prompt internally. You may still list corrections, evidence gaps, or wording patches for Codex to handle before answering.
- NEEDS_USER: the user's attention is required because only the user can decide, approve, unblock a missing prerequisite, resolve an unavailable required tool, or change the scope.

Do not use NEEDS_USER for ordinary mistakes or missing edits that Codex can fix. List those inside the review body and keep the result OK.

Review scope:
- Original prompt path: C:\UE\T66\Reports\AgentReviews\Hero1TempShapesImplementationPlan\original_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\Hero1TempShapesImplementationPlan\codex_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\Hero1TempShapesImplementationPlan\20260603T055619-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
Original user request:

Yes correct all the hero 1 weapons are AOE, but they should have different AOE, shapes and patters. I do want to change the electricity to purple, and make sure we use ice blue for ice. Also we should get rid of the legacy ids and normalize ids first for sure. I agree with your other suggestions create an implementation plan first. And then we will go one by one on the shapes.

Task contract:

Working task:
Operator: Codex
Validator: Claude
Scope: planning-only for temporary Hero 1 AOE weapon shapes and idol visual variants, including purple Electricity, ice-blue Ice, canonical idol ID normalization, and removal of legacy-ID dependence.
Stop condition: deliver an implementation plan; no code/content changes beyond required review artifacts.

Relevant repo rules:

- Root router is C:\UE\T66\AGENTS.md.
- Current operator state is Codex operator, Claude validator.
- Mini/minigame scope is excluded.
- This is planning-only; do not implement.
- Combat work is owned by Gameplay and must respect Gameplay/GAMEPLAY_AGENTS.md and existing combat VFX contracts.
- Temporary simple shapes are infrastructure placeholders only. Final Niagara/material VFX remains governed by PPF, artifact parity, mechanism manifest, impact-context proof, visual/damage alignment, Unreal-owned capture, and staged verification when runtime/playable content changes.

Live facts from prior repo read to preserve:

- Hero 1 has four weapon rows: Hero_1_black_aoe, Hero_1_red_aoe, Hero_1_yellow_aoe, Hero_1_white_aoe. All are Branch=AOE.
- Idols have 16 canonical rows: Fire/Ice/Electricity/Nature x DOT/AOE/Pierce/Bounce, with four rarity variants per row conceptually needed for temporary visual shapes.
- Current outgoing traveler visual slots cover 16 element/category profiles but no rarity dimension.
- Current temporary projectile helper has four hero profiles and one idol overlay profile, not enough for the requested matrix.
- Current color helpers use yellow for Electricity; user wants purple. User wants ice blue for Ice.
- Legacy idol aliases should not be allowed as separate temp-visual keys; normalize IDs before lookup.

Requested Validator output:

- Review the implementation plan for scope, missing steps, risky assumptions, and repo-process conflicts.
- Keep output concise and end with Result: OK or Result: NEEDS_USER.

</original_prompt>

<validator_independent_answer>
Result: OK

## Independent Answer

No Codex draft plan exists in the artifact folder yet (only `original_prompt.md` and the review prompt), so this is my independent baseline for what the plan must contain. The prompt is fully actionable as planning-only — every consequential decision (purple Electricity, ice-blue Ice, normalize-IDs-first, drop legacy-ID dependence, shapes chosen later "one by one") is already settled by the user. No user gate is required to *write the plan*; the per-shape choices come after and are explicitly sequenced by the user. Hence OK.

A correct implementation plan should include, in this order:

1. **ID normalization first (true prerequisite).** Define the canonical idol ID set (Fire/Ice/Electricity/Nature × DOT/AOE/Pierce/Bounce), a single normalization function applied before any temp-visual lookup, and removal of legacy aliases as lookup keys. Sequence this before any visual matrix work so downstream slots key off canonical IDs only.
2. **Color helper changes.** Electricity yellow→purple, Ice→ice-blue, in the existing color helper. Confirm no other system (UI tints, damage-number colors) consumes the same Electricity color and would regress — call out the audit step rather than assuming isolation.
3. **Hero 1 AOE shape differentiation.** Four AOE rows (black/red/yellow/white) need distinct AOE shapes/patterns. Plan should expand the projectile helper beyond its current 4 hero profiles to carry per-row shape data, and define where shape geometry is authored (placeholder simple shapes only — final VFX still governed by PPF/manifest/parity).
4. **Idol visual matrix + the rarity dimension (biggest scope risk).** 16 canonical rows × 4 rarity = 64 conceptual variants, but traveler profiles (slots 4–19) and the single idol overlay profile have **no rarity dimension** today. The plan must explicitly decide: is rarity a new authored slot axis (64 slots, real perf/authoring cost) or a runtime modifier (scale/intensity/tint) layered over the existing 16? This is the one decision most likely to be under-specified — Codex should pick one and justify it, not leave it implicit.
5. **Process compliance.** State that these are infrastructure placeholders only; final Niagara/material VFX stays under the combat-VFX contracts (PPF, artifact parity, mechanism manifest, impact-context proof, visual/damage alignment, Unreal-owned capture, staged verification). Plan should respect `Gameplay/GAMEPLAY_AGENTS.md` and read `CombatVFXAuthoringProcedure.md` before any later asset work.
6. **Sequencing/staging.** Reflect the user's "one by one on the shapes" — plan delivers infrastructure + matrix scaffolding, then each shape lands as a separate reviewed step.

## Evidence Checked
- `original_prompt.md` and the pass-1 review prompt (no draft plan present).
- `Gameplay/GAMEPLAY_AGENTS.md` — combat ownership, read-first/VFX-authoring rules.
- `Gameplay/Combat/` index incl. `OutgoingTravelerVisualProfilesAuthoring.md` — confirms 16 element/delivery slots (4–19), no rarity axis, single Niagara selector, Phase-2 visual-proof standard.
- Grepped `Source/` for Electricity/Ice/color/legacy usages (broad spread across UI + gameplay) — supports the "audit other color consumers" step.

## Questions Or Blockers
None requiring the user. The deferred per-shape selections are by the user's own design ("one by one") and do not block delivering the plan.

## Caveats
- I could not literally review a Codex plan because none is in the folder yet; the above is the standard it should be measured against.
- Rarity dimension (16→up-to-64) is the highest-risk under-specification — verify the plan commits to slot-axis vs. runtime-modifier and notes the traveler-pool perf implication.
- Electricity color change needs a consumer audit (grep showed many color sites); don't assume the helper is the only source of truth.
- Temp shapes must not drift into final-VFX work; confirm the plan keeps the contract boundary explicit.

</validator_independent_answer>

<codex_draft>
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
     Planned implementation: centralize weapon black and idol element colors: Fire red, Ice ice-blue, Electricity purple, Nature green.
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
   - Hero 1 weapon attacks: black.
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
   - 4 Hero 1 AOE weapon rows.
   - 64 idol rows: 16 canonical idols x 4 rarities.
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
   - Fire DOT/AOE/Pierce/Bounce across Black/Red/Yellow/White.
   - Ice DOT/AOE/Pierce/Bounce across Black/Red/Yellow/White.
   - Electricity DOT/AOE/Pierce/Bounce across Black/Red/Yellow/White.
   - Nature DOT/AOE/Pierce/Bounce across Black/Red/Yellow/White.

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
5. Validator confirms Hero 1 rows use black.
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

</codex_draft>
