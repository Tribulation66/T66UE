Verdict: REVISE

## Blockers
None. No damage-authority violation, no production binding row creation, no final-VFX promotion of the placeholder.

## Major Issues
- **Unenumerated AOE-idol blast radius.** The plan says it will "Replace the AOE idol `WeaponHitActors` path with context-driven AOE target query/damage for Water and future AOE idols." `Idols.csv` may contain other rows with `Category=AOE` today. If any exist, this is a behavior change to live idols on the same pass that "only visually proves Water," which contradicts the stated scope ("this pass only visually proves Water AOE"). The plan must either (a) enumerate every current `Category=AOE` idol and prove each one didn't regress, or (b) narrow the target-query swap to `Idol_Water` only and leave other AOE idols on the existing path until they get their own proof.
- **"As much as possible" non-AOE preservation is too soft.** "Preserve existing non-AOE idol damage behavior as much as possible while routing through context/logging" leaves room for silent behavior drift on Pierce/Bounce/DOT idols. The contract should be: zero gameplay-observable change for non-AOE idols this pass; logging/context plumbing only. Anything stronger requires its own proof.
- **Placeholder vs. existing blue cube collision.** Live evidence calls out an existing "blue cube attached to the projectile lane" as the current idol overlay placeholder. The plan adds a *new* blue sphere placeholder at the idol impact point. Without an explicit suppression contract, both could spawn for Water and the proof becomes ambiguous about which artifact is the impact-context marker. The plan mentions "Suppress the old temporary idol projectile lane for Water" but doesn't bind that suppression to the same impact-context resolution path — spell out the exact mutual-exclusion rule and log it.

## Minor Issues
- CVar name, scope, and default state for impact logging are unspecified. Name it, default it off, and require the proof mode to set it on.
- The impact context lists `radius/inner radius/sector metadata` as common fields, but those are AOE-shaped. Either mark them `TOptional` / category-tagged, or document which categories populate which fields, to prevent future readers from treating them as universally meaningful.
- `Reports/Proof/CombatVFX/WaterIdolImpactStructure_<timestamp>/` — confirm the timestamp format matches existing combat proof bundle conventions so the directory sorts with prior captures.
- "Add a bounded pending issue" to `pending_issues_Combat.md` *only if a concrete gap appears* — that gate is fine, but pre-commit to what counts as "concrete" so it doesn't become a dumping ground.
- `MASTER_COMBAT.md` update item should explicitly note the placeholder boundary (so a later reader doesn't mistake the sphere for a shipped Water effect).

## Clarifying Questions
1. Does `Idols.csv` currently contain any `Category=AOE` row other than `Idol_Water`? If yes, are those rows in-scope for the target-query swap this pass, or excluded until their own proof?
2. Is the existing temporary projectile-lane blue cube being removed *globally* for AOE idols, only for `Idol_Water`, or only when an impact-context placeholder spawns for the same idol?
3. Where does the placeholder sphere mesh come from — runtime engine primitive, an existing `T66TemporaryProjectileSystem` shape, or a new asset? (A new asset would expand scope.)
4. Does the proof mode fully revert equipped Water idol + any state mutation on exit, so normal play sessions after capture are clean?

## Required Verification
- Focused build: `T66Editor` Development Win64.
- `Scripts/ValidateCombatVFXProductionBindings.py` if any binding-resolver code path is touched, to confirm no production row was added.
- Unreal-owned capture via `CaptureT66GameplayVideo.ps1` with proof mode `hero1axeaoewateridolimpact` and evidence bundle enabled.
- Log excerpt must show the *full discriminator sequence* (weapon context → derived idol context → `SourceID=Idol_Water` damage → `CombatVFXIdolImpactPlaceholderSpawned`) — sphere-spawn alone fails acceptance per the plan's own anti-lookalike rule.
- HP-delta pass/fail on both Water-hit and Water-miss staged targets (negative case required).
- `DamageBySource` (or equivalent) showing `AutoAttack` and `Idol_Water` as separate totals.
- Confirm `CombatVFXBindings.csv` / `DT_CombatVFXBindings` were not modified.
- Refresh staged standalone via `Scripts/StageStandaloneBuild.ps1` per `Gameplay/GAMEPLAY_AGENTS.md` since runtime gameplay code changes affect the playable build.
- After capture, diff `Idols.csv` and the binding CSV against `HEAD` to confirm zero data drift.

## Rationale
The structural design is sound: impact-context publication from weapons, derivation by idols, preserved damage authority via `ApplyDamageToTargetHandle(..., SourceID=IdolID)`, future seam through `ResolveCombatVFXBinding(IdolModifier, ...)`, and a clearly-labeled placeholder kept off the production binding path. Artifact-parity treatment of "structure EQUIVALENT, final Niagara DEFERRED" is correct, the anti-lookalike discriminator is strong, and the proof-mode/CVar/Unreal-owned-capture plan matches `CombatVFXDefinitionOfDone.md`. What pushes this to REVISE rather than APPROVE is scope ambiguity around other AOE idols, the soft "as much as possible" guarantee for non-AOE idols, and the unresolved placeholder/projectile-lane overlap — each is a small fix in the plan text but each is exactly the kind of silent narrowing or silent widening the reviewer instructions flag. Tighten those three, answer the four clarifying questions in the plan, and this is approvable.

