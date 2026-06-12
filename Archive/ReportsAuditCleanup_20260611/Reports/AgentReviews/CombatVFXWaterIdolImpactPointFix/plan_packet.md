# Water Idol Impact Point Fix - Plan Packet

## Working Goal

Fix the Water idol blue-sphere placeholder so it spawns at the actual Hero 1 AOE weapon impact point instead of visually sitting on top of the hero, then verify with reviewed code and Unreal-owned capture evidence.

## User Constraint

The user identified this as a code bug, not just a capture-selection issue. Idols should work on top of weapons and be triggered at the weapon projectile/weapon visual impact point.

## Applicable Repo Instructions

- `AGENTS.md`: use a full working goal, inspect live repo state, use Claude review before implementation, report verification.
- `Gameplay/GAMEPLAY_AGENTS.md`: combat work is owned by Gameplay/Combat and runtime-facing changes require build verification.
- `Gameplay/Combat/VFX_PROCESS_INDEX.md`: combat VFX tasks start from the VFX process docs.
- `Gameplay/Combat/CombatVFXDefinitionOfDone.md`: gameplay capture must be Unreal-owned; hitbox/damage authority is combat code, not Niagara collision.
- `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md`: weapon VFX owns primary silhouette; idol context is spawned from a weapon impact point and owns its own source/query/damage/future chaining point.
- `Gameplay/Combat/MASTER_COMBAT.md`: Water is the first idol impact-context consumer; current proof uses a blue sphere placeholder until Water Niagara is authored.
- `Gameplay/Combat/pending_issues_Combat.md`: existing Water capture weak-proof note says the current placeholder dominates the view; this plan addresses the placement bug rather than only frame selection.

## Live Diagnosis

Current code:

- `Source/T66/Gameplay/T66CombatComponent.cpp`
  - `PerformSlash` sets `SlashCenter = GetTargetAimPoint(PrimaryHandle)`.
  - It sets `SlashImpactContext.ImpactPoint = SlashCenter`.
  - It publishes that context as `WeaponPrimary`.
  - It then spawns the production weapon slash through `TrySpawnBoundWeaponBaseSlashVFX(SlashCenter, SlashForward, ...)`.
- `Source/T66/Gameplay/T66CombatVFX.cpp`
  - `SpawnWaterIdolImpactPlaceholderVFX` places the blue sphere at `IdolImpactContext.ImpactPoint + FVector(0,0,76)`.
- `Content/Data/CombatVFXBindings.csv`
  - `Hero1Axe_AOE_Base` suppresses the old temporary projectile, so the primary weapon visual is the Niagara crescent slash.

Problem:

- For Hero 1 AOE, `SlashCenter` is the AOE damage/query center, not the visual impact point of the crescent band.
- The Hero 1 AOE crescent has an inner hollow (`AoeInnerRadiusRatio=0.54`). The visible weapon band is in front of the center, roughly midway between `EffectiveSlashInnerRadius` and `EffectiveSlashRadius`.
- Since the idol inherits the raw center as its impact point, the placeholder can appear over/near the hero instead of over the weapon slash impact.

## PPF CHECK

Objective: Correct the idol impact point source so Water's placeholder is driven by the weapon visual/projectile impact point.

Proven process: Combat VFX runtime binding/impact-context process in `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md` and `Gameplay/Combat/MASTER_COMBAT.md`.

My planned implementation: Keep damage authority in combat code. Do not make Niagara collision authoritative and do not re-enable the deprecated temporary projectile. Compute a weapon visual impact point only for frontal-sector crescent AOE attacks using the existing damage center, forward vector, inner radius, and outer radius, then publish that as the `FT66CombatImpactContext::ImpactPoint`. Water continues to use the weapon context impact point and owns its own idol context/damage query.

Same method class: YES

If NO, why: N/A

User approval required before proceeding: NO, because this preserves the existing combat-code impact-context method and corrects the impact point used by that method.

Verification evidence: focused compile plus Unreal-owned `hero1axeaoewateridolimpact` capture/log proof showing `WeaponPrimary` and `IdolPrimary` impact points moved from the old damage-center XY to the crescent band XY and the blue sphere is no longer on top of the hero.

## ARTIFACT PARITY GATE

Reference artifact/category: Weapon-driven idol placeholder proof.

Role: Primary for this temporary Water proof only.

Required: YES

Planned artifact/path: Existing blue sphere placeholder in `SpawnWaterIdolImpactPlaceholderVFX`, driven from corrected `IdolImpactContext.ImpactPoint`.

Status: SAME

Evidence: Unreal-owned capture plus log line `CombatVFXIdolImpactPlaceholderSpawned ... ImpactPoint=...`.

## MECHANISM MANIFEST

Reference/source: `CombatVFXIdolOverlayArchitecture.md`

Required mechanisms:

1. Mechanism: Weapon impact context drives idol impact context.
   Required: YES
   Planned implementation: For the current Hero 1 black AOE crescent, change the weapon impact context to publish the crescent-band visual impact point instead of the damage-center point. The concrete predicate is `bUseHeroOneFrontalSector && EffectiveSlashInnerRadius > KINDA_SMALL_NUMBER && EffectiveSlashRadius > EffectiveSlashInnerRadius`; current weapon data makes that true only for `Hero_1_black_aoe` because `AoeInnerRadiusRatio=0.54` is authored only for that row and all other AOE rows are `0.00`.
   Evidence needed: `WeaponPrimary SourceID=Hero_1_black_aoe` and `IdolPrimary SourceID=Idol_Water ParentSourceID=Hero_1_black_aoe` with matching corrected impact points.

2. Mechanism: Idol owns independent damage/query source.
   Required: YES
   Planned implementation: Keep existing Water idol impact context, query, damage, and `SourceID=Idol_Water` application path unchanged except for the center inherited from the corrected weapon impact point.
   Evidence needed: `DamageBySource SourceID=Idol_Water` and Water proof target pass/fail logs.

3. Mechanism: Weapon VFX remains primary silhouette.
   Required: YES
   Planned implementation: Do not re-enable `Hero1Axe_AOE_Base` temporary projectile. Keep production Niagara slash binding unchanged.
   Evidence needed: `CombatVFXProductionSpawned Binding=Hero1Axe_AOE_Base`.

## Planned Code And Doc Change

Edit `Source/T66/Gameplay/T66CombatComponent.cpp`:

1. In `PerformSlash`, keep `SlashCenter` for target gathering, target query, and production slash spawn. Do not mutate `SlashCenter`.
2. Add a local `SlashContextImpactPoint` initialized to `SlashCenter`.
3. Use this concrete gate before moving the context point: `bUseHeroOneFrontalSector && EffectiveSlashInnerRadius > KINDA_SMALL_NUMBER && EffectiveSlashRadius > EffectiveSlashInnerRadius`.
4. If the gate is true, set `SlashContextImpactPoint = SlashCenter + SlashForward * ((EffectiveSlashInnerRadius + EffectiveSlashRadius) * 0.5f)`.
5. Treat the `0.5f` annulus midpoint as a tuned placeholder approximation for the current Hero 1 crescent-band proof, not as final Water Niagara art direction. It is based on the current combat band mechanics (`EffectiveSlashInnerRadius=236.26`, `EffectiveSlashRadius=437.52` in the proof capture) and moves the context to the middle of the visible/damage-authoritative band without sampling final Water pixels.
6. Set `SlashImpactContext.ImpactPoint = SlashContextImpactPoint`.
7. Leave `TrySpawnBoundWeaponBaseSlashVFX(SlashCenter, ...)` unchanged.

Edit `Gameplay/Combat/pending_issues_Combat.md`:

1. Mark the earlier "Water Idol Impact Capture Weakly Shows Base Weapon VFX" item as resolved by this runtime placement correction plus a recapture.
2. Add a short follow-up marker that the current annulus-midpoint impact is a placeholder/tuned bridge for the blue-sphere proof and should be revisited against the final Water Niagara effect when that asset is authored.

This should move the Water placeholder from the raw damage center toward the actual crescent-band impact point without changing the weapon's AOE sector damage center, target gathering, or production Niagara spawn origin.

## Risks

- Water's idol-owned AOE damage center will move because it correctly inherits the weapon impact point. Current proof target expectations may need to be updated if they assumed the old center.
- Logs will show the weapon impact point no longer equals the production VFX spawn `Location` base center. This is expected because the VFX spawn origin is the slash system origin, while the context impact point is the visual/projectile impact within that system.
- Future non-crescent AOE weapons should keep the center impact behavior unless they define an inner-band/visual impact model.
- `SpawnWaterIdolImpactPlaceholderVFX` currently adds `FVector(0,0,76)` and bound idol VFX spawn adds `FVector(0,0,72)`. Those presentation Z offsets are intentionally unchanged because `FT66CombatImpactContext::ImpactPoint` remains the planar/grounded damage-source point; this pass fixes the XY source point only.

## Proof Target Scope

Updating proof target expectations is in scope only if the current `hero1axeaoewateridolimpact` assertions fail solely because the idol damage center correctly moved with the impact point. The desired proof outcome remains: weapon-only targets prove the base crescent, Water-only targets prove the independent idol damage source, and the capture proves Water is spawned from the corrected weapon context. If a target fails for a different gameplay reason, document it before changing expectations.

## Verification Plan

1. `git diff --check -- Source/T66/Gameplay/T66CombatComponent.cpp Gameplay/Combat/pending_issues_Combat.md`
2. Focused compile:
   `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex`
3. Unreal-owned capture:
   `Scripts\CaptureT66GameplayVideo.ps1 -CaptureMode hero1axeaoewateridolimpact ... -EvidenceBundle`
4. Inspect video/contact sheet for blue sphere placement.
5. Check log lines:
   - `EquippedAoeWeapon=Hero_1_black_aoe Success=1`
   - `CombatImpactContext Phase=WeaponPrimary ... ImpactPoint=...`
   - `CombatImpactContext Phase=IdolPrimary SourceID=Idol_Water ParentSourceID=Hero_1_black_aoe ... ImpactPoint=...`
   - `CombatVFXIdolImpactPlaceholderSpawned ... ImpactPoint=...`
   - `DamageBySource SourceID=Idol_Water`
6. Compare old captured impact point `V(X=360.00, Y=0.00, Z=64.00)` against the post-fix `WeaponPrimary` and `IdolPrimary` XY. With the current proof values, the expected new point is approximately `SlashCenter + Forward * 336.89`, because `(236.26 + 437.52) * 0.5 = 336.89`.
7. Negative scope check: confirm from `Content/Data/Weapons.csv` / `Hero1AxeAOESlashMechanismPacket.md` that `AoeInnerRadiusRatio=0.54` is currently authored only for `Hero_1_black_aoe`; all other AOE rows use `0.00`, so the new predicate leaves them at center impact behavior. If time permits, rerun `Scripts\ValidateCombatVFXProductionBindings.py` as an additional static/process check.

## Out Of Scope

- Authoring the final Water Niagara asset.
- Re-enabling the deprecated Hero 1 AOE temporary projectile.
- Changing Water `AoeDelay`.
- Changing the Hero 1 AOE slash visual asset.
- Mini/minigame systems.

## Reviewer Request

Please review the diagnosis and planned code change for correctness. Focus on whether moving the published Hero 1 crescent AOE impact context to the band midpoint is the right code seam, whether this preserves weapon damage authority and idol damage authority, and whether any other file needs to change before implementation.

Return first non-empty line exactly `Verdict: APPROVE`, `Verdict: REVISE`, `Verdict: NEEDS_HUMAN_DECISION`, or `Verdict: BLOCK`.
