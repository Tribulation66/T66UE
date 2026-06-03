# Claude Review Packet: Combat VFX Idol Impact Plan

## Working Goal

Analyze the live T66 combat VFX/idol architecture from commit `27e148ae`, run Claude cross-review, and explain what is missing plus next steps for impact-point-driven idol Niagara effects without implementing changes.

## Output Scope To Review

Read-only architectural answer to Pablo. No file edits, no Unreal asset work, no implementation plan approval beyond next-step sequencing. The answer should say whether Codex and Claude understand the requested vision and identify what must change before a first idol overlay can be implemented.

## User Constraints And Assumptions

- Pablo explicitly requested discussion before implementation.
- Do not implement anything in this pass.
- Default scope excludes Mini/minigame systems.
- Use live repo state, not stale memory.
- Claude review is required by repo process and explicitly requested by Pablo.
- Safe assumption: start with one existing AOE idol, likely `Idol_Earth`, `Idol_Water`, or `Idol_Storm`, because the live idol table has those as `AOE`. `Idol_Lava` is currently `DOT`, so treating it as the "fire explosion" idol would require a design/data decision or a new/changed idol row.

## Applicable Instructions Read

- `AGENTS.md`
- `Gameplay/GAMEPLAY_AGENTS.md`
- `Gameplay/README.md`
- `Gameplay/Combat/VFX_PROCESS_INDEX.md`
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`
- `Gameplay/Combat/CombatVFXDefinitionOfDone.md`
- `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md`
- `Gameplay/Combat/CombatVFXGeneratedAssetPolicy.md`
- `Gameplay/Combat/MASTER_COMBAT.md`
- `Gameplay/Combat/CombatVFXInfrastructureInventory.md`
- `Gameplay/Combat/Hero1AxeAOESlashMechanismPacket.md`
- `Gameplay/Combat/Hero1AxeVFXPlan.md`
- `Gameplay/Combat/Hero1AxeSharedAuraMaterialResearchPlan.md`
- `Gameplay/Combat/pending_issues_Combat.md`
- `Source/T66/Gameplay/pending_issues_Gameplay.md`
- `Reports/AGENTS.md`

## Current Live Findings

1. The user vision is coherent and matches the architecture direction already documented:
   - Weapon VFX owns the primary attack silhouette.
   - Idol overlay VFX owns an additive secondary layer.
   - Idol overlays must not revive the temporary projectile placeholder path as production.
   - Overlay rows/assets require their own effect packet, source evidence, visual proof, and validator coverage.
   - Source: `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md:11-15`.

2. The current baseline says Hero 1 AOE has a production binding, but idol overlays are architecture-only:
   - `Gameplay/Combat/VFX_PROCESS_INDEX.md:24-30`.
   - `Content/Data/CombatVFXBindings.csv:2` contains only `Hero1Axe_AOE_Base`, with notes saying idol overlays are deferred.
   - `Gameplay/Combat/CombatVFXGeneratedAssetPolicy.md:32` also says this is the only active combat VFX production binding.

3. The data type already has a future idol binding seam:
   - `ET66CombatVFXBindingSourceType` includes `WeaponBase` and `IdolModifier`.
   - `FT66CombatVFXBindingData` includes `SourceType`, `SourceID`, `AttackCategory`, `NiagaraSystem`, `EffectPacketID`, `VFXProfile`, suppress/fallback flags, and scale/timing fields.
   - Source: `Source/T66/Data/T66DataTypes.h:31-83`.

4. Runtime lookup already supports any source type/category combination:
   - `UT66GameInstance::GetCombatVFXBindingData` loops rows and matches `SourceType`, `SourceID`, and `AttackCategory`.
   - Source: `Source/T66/Core/T66GameInstance.cpp:916-973`.
   - Missing: no runtime caller currently resolves `IdolModifier` rows, and no active row exists.

5. The AOE weapon currently has a local impact center but not a reusable impact context:
   - `TryFire` computes `AttackOrigin` at `Source/T66/Gameplay/T66CombatComponent.cpp:1244-1245`.
   - AOE slash computes `PrimaryHandle`, `SlashCenter = GetTargetAimPoint(PrimaryHandle)`, `EffectiveSlashRadius`, `SlashForward`, `EffectiveSlashInnerRadius`, builds slash targets, and spawns the bound weapon VFX from `SlashCenter`.
   - Source: `Source/T66/Gameplay/T66CombatComponent.cpp:1587-1607`.
   - Missing: `SlashCenter`, `SlashForward`, radius, inner radius, hit handles, timing, and binding source are local variables, not a named struct/event that an idol overlay can consume.

6. Logical authority is already separated from Niagara and should stay that way:
   - AOE target gathering uses overlap plus sector checks and debug draw in `BuildSlashTargets`.
   - Source: `Source/T66/Gameplay/T66CombatComponent.cpp:1365-1460`.
   - The DoD says hitbox/damage authority must be combat query/log proof, not Niagara collision/render mesh/material opacity.
   - Source: `Gameplay/Combat/CombatVFXDefinitionOfDone.md:19`.

7. Production weapon Niagara spawn exists only for weapon-base slash:
   - `TrySpawnBoundWeaponBaseSlashVFX` resolves `WeaponBase`, loads the Niagara, scales by `EffectiveSlashRadius / BaseVisualRadius`, clamps playback for readability, spawns at `Location + Z70`, and logs `CombatVFXProductionSpawned`.
   - Source: `Source/T66/Gameplay/T66CombatComponent.cpp:1028-1192`.
   - Missing: no equivalent `TrySpawnBoundIdolOverlayVFX` or generic bound combat VFX spawn helper exists.

8. Current idol runtime behavior is not the desired overlay model:
   - `RecomputeFromRunState` loads equipped idols into `CachedIdolSlots`.
   - Source: `Source/T66/Gameplay/T66CombatComponent.cpp:892-902`.
   - `TryFire` still spawns a visual-only placeholder projectile lane per idol before damage resolution.
   - Source: `Source/T66/Gameplay/T66CombatComponent.cpp:2263-2291`.
   - `SpawnWeaponProjectileVisual` uses `AT66HeroProjectile` with `FT66TemporaryProjectileSystem::ProfileIdolOverlay`, which is a temporary blue cube overlay.
   - Source: `Source/T66/Gameplay/T66CombatComponent.cpp:1486-1534` and `Source/T66/Gameplay/T66TemporaryProjectileSystem.cpp:74-80,157-164`.
   - Idol payload damage then loops `WeaponHitActors`, applies damage to each actor with `ApplyDamageToActor`, and passes `Hit->GetActorLocation()` to `ApplyIdolSpecialBehavior`.
   - Source: `Source/T66/Gameplay/T66CombatComponent.cpp:2366-2418`.
   - Missing: idol effects do not consume a single impact point, do not use a combat impact context, do not spawn bound Niagara overlays, and do not preserve target handles for idol damage/status routing.

9. Existing idol data has AOE rows, but no "Fire" AOE idol:
   - `Idol_Earth`, `Idol_Water`, and `Idol_Storm` are `AOE`.
   - `Idol_Lava` is currently `DOT`.
   - Source: `Content/Data/Idols.csv:3,8-10`.

10. The setup/validator tooling is still Hero 1 AOE-centric:
   - `Scripts/SetupCombatVFXBindingsDataTable.py` enforces the `Hero1Axe_AOE_Base` row and reloads the DataTable.
   - `Scripts/ValidateCombatVFXProductionBindings.py` validates active rows generically but hard-checks Hero 1 AOE assets/row and required assets.
   - Missing: idol-specific asset/path validation, no `/Game/VFXLab` dependency checks for future idol Niagara paths, no proof wrapper for base-only/base+idol states, and no active-row acceptance for an idol overlay.

## Codex Proposed Answer

Codex should answer that the vision is understood:

- The weapon attack should produce an official impact context.
- The base weapon VFX remains the primary silhouette and combat authority stays in C++ queries.
- The idol should subscribe to that context as an additive overlay, e.g. a burst/explosion at the impact point, using production Niagara assets and binding rows.

What is missing:

1. A runtime impact context/struct, probably around `UT66CombatComponent`, containing:
   - attack category,
   - weapon ID/source binding ID,
   - hero ID,
   - attack origin,
   - primary target handle,
   - impact point from `GetTargetAimPoint`,
   - forward/normal/orientation,
   - outer radius, inner radius, half angle, and effective scale,
   - effective damage/playback timing,
   - hit target handles or at least actors,
   - whether this is primary impact, secondary splash, bounce hop, pierce endpoint, or DOT anchor.

2. A small refactor so each weapon path returns or fills that context:
   - AOE first, because `SlashCenter` already exists and Hero 1 AOE has proof infrastructure.
   - Later Pierce/Bounce/DOT can define their own impact anchors.

3. A generic or idol-specific bound VFX spawn helper:
   - resolve `ET66CombatVFXBindingSourceType::IdolModifier`,
   - source ID is the idol ID,
   - category can initially be the weapon attack category or the idol category, but this is a design choice to settle before row authoring,
   - spawn overlay Niagara at the context impact point,
   - pass scale/timing/orientation/user params,
   - log a distinct `CombatVFXIdolOverlaySpawned` record.

4. Binding data/schema may be enough for one first pass, but the architecture doc's future fields are not present yet:
   - no weapon binding ID compatibility,
   - no compatible attack category list,
   - no stacking/priority rule,
   - no overlay material parameter set,
   - no explicit damage/status authority note beyond `Notes`.
   For a first idol, one row keyed by `SourceType=IdolModifier`, `SourceID=<IdolID>`, `AttackCategory=AOE` is probably enough if the effect packet records the limitations. For a scalable system, add the missing fields later.

5. Current idol damage/status semantics need a decision:
   - existing code applies idol damage/specials to every weapon-hit actor after the weapon attack resolves.
   - the user's explosion-at-impact idea sounds like a single secondary AOE centered on the weapon impact point.
   - Therefore the first implementation should either be visual-only overlay with existing idol damage unchanged, or a real idol AOE combat rule with separate query/log proof. Do not mix visual overlay and new damage authority silently.

6. Current placeholder idol projectile visuals should be suppressed/replaced when a bound idol overlay exists:
   - otherwise the production overlay would coexist with the old blue cube projectile and violate the architecture's "do not resurrect placeholder path as production" rule.

7. Tooling/docs missing:
   - first idol effect packet,
   - source evidence and mechanism manifest,
   - generated/promoted Niagara/material assets under the right lab/production paths,
   - setup script preserving/adding idol rows,
   - validator coverage for idol active rows/assets/no lab dependency,
   - capture/proof mode that equips Hero 1 AOE plus one idol and proves base-only vs base+idol separately.

Recommended next steps:

1. Make a small design decision before implementation: for the first pass, choose `Idol_Earth`, `Idol_Water`, or `Idol_Storm` as the existing AOE idol. If Pablo specifically wants fire/explosion, decide whether to treat `Idol_Lava` as the first visual-only fire overlay despite its current `DOT` data or add/rename a proper AOE fire idol row later.
2. Write the first idol effect packet and PPF/material/mechanism gates. It can reuse the Hero 1 AOE process but the primary carrier is likely `SupportImpact`/explosion burst, not `ArcSlash`, because the weapon keeps the slash silhouette.
3. Refactor only the combat context first: create the impact context, fill it from Hero 1 AOE, preserve current behavior.
4. Add a bound idol overlay spawn path using `IdolModifier` rows and suppress the placeholder idol projectile only when the bound overlay spawns.
5. Add one first production row and validator/proof harness for the chosen idol overlay.
6. Implement the Niagara/material asset through the normal lab to production VFX process, then capture base-only and base+idol evidence.

## Specific Questions For Claude

1. Is the proposed answer consistent with the repo instructions and live code?
2. Are there missing code seams or risks Codex should mention before giving this answer to Pablo?
3. Should Codex recommend using the existing binding schema for the first idol row, or should it say schema extension is required before any implementation?
4. Are there any Blocker/Major issues that should prevent presenting this as the next-step recommendation?

## Verification For This Read-Only Pass

- Verified HEAD with `git rev-parse --short HEAD`: `27e148ae`.
- Read requested docs and additional required routers/docs listed above.
- Inspected live code/data paths listed in findings.
- Verified `ANTHROPIC_API_KEY` is unset in Process/User/Machine before invoking Claude.
- No build/test/capture is required because no implementation is being performed.
