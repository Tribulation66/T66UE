Operator Packet: COMPLETE

# Weapon/Idol Combat VFX — Read-Only Infrastructure Assessment
Profile: read-only Operator (`C:\UE\T66`). Codex = Validator. No files written, no shell/Unreal/editor actions. No Mini/minigame scope. All line anchors confirmed by direct file inspection this pass.

## 1. Current State Summary
- **Production binding lane works but is single-row.** `ResolveCombatVFXBinding` (`T66CombatComponent.cpp:1060`) is fully general over `(SourceType, SourceID, AttackCategory)` via `DT_CombatVFXBindings`. Only one production row exists: `Hero_1_black_aoe → NS_Hero1AxeAOE_MeshSlash` (WeaponBase/AOE). All other weapon categories (Pierce/Bounce/DOT) are packet/proof infrastructure with **no production rows**, so they fall through to placeholder/imported-pack paths.
- **Idol impact lane is already category-general but Water-named.** Dispatch switches on `FIdolData.Category` and emits neutral `CombatImpactChainDiagnostic`, yet it is wrapped in stale Water-era identifiers and **three duplicated hardcoded idol allowlists** that must stay hand-synced.
- **Idol category proofs spawn reused placeholder primitives, not authored idol Niagara.** `SpawnIdolImpactPlaceholderVFX` (`T66CombatVFX.cpp:1319`) reuses base Pierce/Bounce/DOT pixel primitives; `SpawnWaterIdolImpactPlaceholderVFX` (`:1244`) spawns a basic blue sphere mesh "area read." `TrySpawnBoundIdolImpactVFX` (`:1147`) is the real production path but has no idol rows feeding it yet.
- **`GetIdolNiagaraEffectPath` (`:434`) is a hardcoded 15-idol path table** into the Stylized_VFX_StPack, used by the legacy Idol Pierce/AOE/Bounce/DOT spawners (`:1414–1618`). This is imported-asset scaffolding, parallel to (and bypassed by) the binding table.
- **Hero per-ID variant pixel emitters** (`TrySpawnHero{Pierce,AOE,Bounce,DOT}VariantPixels`, `:666–873`) hardcode `Hero_2..Hero_12` geometry — placeholder presentation, not production-bound.
- **Validator coverage is Hero-1-AOE-specific.** Impact-context and visual/damage alignment contracts are doc-only, not enforced. `CombatVFXBindings.csv`/`.uasset` are untracked by git (inventory §8.1).

## 2. Inventory Table
| Item | Anchor | Class | Notes |
|---|---|---|---|
| Generic binding resolver | `T66CombatComponent.cpp:1060` | Production (keep) | General over source/category; correct seam. |
| Bound idol impact spawn | `T66CombatVFX.cpp:1147` `TrySpawnBoundIdolImpactVFX` | Production (keep, unfed) | No idol rows route here yet. |
| Water sphere placeholder | `T66CombatVFX.cpp:1244` `SpawnWaterIdolImpactPlaceholderVFX` | Placeholder | Blue sphere "area read", lifespan 1.25s; comment self-flags as temp. |
| Category placeholder dispatch | `T66CombatVFX.cpp:1319` `SpawnIdolImpactPlaceholderVFX` | Placeholder | Reuses base Pierce/Bounce/DOT primitives by `Category`. |
| Hardcoded idol→Niagara path table | `T66CombatVFX.cpp:434` `GetIdolNiagaraEffectPath` | Scaffolding | 15 imported StPack paths; bypassed by binding table. |
| Legacy idol Pierce/AOE/Bounce/DOT spawners | `T66CombatVFX.cpp:1414/1455/1506/1563` | Scaffolding | Drive imported-pack paths + rarity scale; pre-binding. |
| Hero variant pixel emitters | `T66CombatVFX.cpp:666–873` | Placeholder | Hardcoded `Hero_2..12` shapes. |
| **Allowlist #1 (runtime)** | `T66CombatComponent.cpp:2172` `ImpactPresentationProofIdols` | Duplicate | Water/Light/Electric/Poison. |
| **Allowlist #2 (overlay)** | `T66PlayerController_Overlays.cpp:3366` `SupportedProofIdols` | Duplicate | Same 4 + Water fallback default `:3375`. |
| **Allowlist #3 (runner)** | `RunHero1AxeIdolCategoryNativeImpactProof.ps1:69–113` | Duplicate | Per-idol cases incl. Water-regression + Earth-neutral. |
| Water-named general locals | `T66CombatComponent.cpp:3329,3699,3737,3743` | Stale naming | `WaterIdolImpactContextCount`, `bWaterContextParity`. |
| Hardcoded `SourceID=Idol_Water` aggregate | `T66CombatComponent.cpp:3747` `CombatIdolImpactDiagnostic` | Stale/needs-decision | Logs `Idol_Water` even for Light/Electric/Poison runs. |
| Suppress-reason string | `T66CombatComponent.cpp:3184` `...OwnsWaterPlaceholder` | Stale naming | Codex-flagged (`codex_validator_report.md:96`). |
| Production binding validator | `Scripts/ValidateCombatVFXProductionBindings.py` | Production (narrow) | Hero-1-AOE-specific; no idol/alignment checks. |

## 3. Prioritized Cleanup Plan (behavior-preserving)
**Phase 1 — Naming de-Watering (zero behavior change):**
- Rename suppress reason `ImpactPresentationOwnsWaterPlaceholder` → category-neutral (`:3184`). Codex pre-flagged.
- Rename Water-prefixed general locals/counters to category-neutral (`:3329,3699,3737,3743`).
- Resolve `CombatIdolImpactDiagnostic SourceID=Idol_Water` hardcode (`:3747`): emit actual idol ID or drop as redundant. **Needs-decision** — proof regexes (`runner:84–86`) match the literal string, so this is a coordinated source+runner change, not pure rename.

**Phase 2 — De-duplicate the 3 allowlists:**
- Hoist the Water/Light/Electric/Poison membership into one shared source of truth consumed by runtime (`:2172`) and overlay (`:3366`); keep the PS1 runner referencing the same set by name/comment. Preserve exact membership.

**Phase 3 — Consolidate scaffolding spawn paths (design, do not wire):**
- Document that `GetIdolNiagaraEffectPath` (`:434`) + legacy idol spawners (`:1414–1618`) are the pre-binding imported-pack lane; plan migration of idol rows onto `TrySpawnBoundIdolImpactVFX`/`DT_CombatVFXBindings` rather than extending the hardcoded table.

**Phase 4 — Validator generalization (Codex-owned):**
- Extend production validator with `IdolModifier` row checks; add impact-context + visual/damage-alignment assertions; track `CombatVFXBindings.csv`/`.uasset` in git or document the generated-asset exception.

## 4. Explicit No-Touch List (behavior, not cleanup)
- Allowlist **membership** (Water=AOE, Light=Pierce, Electric=Bounce, Poison=DOT) — rename container, never change contents.
- `AoeInnerRadiusRatio=0.54` crescent-band hitbox for `Hero_1_black_aoe`; all other AOE rows stay `0.00`.
- `0.20s` readable playback clamp; per-category rarity visual scale/quantity math (`:1439,1483,1593`).
- Imported-VFX per-frame budget logic (`TryConsumeCombatImportedVFXBudget`, `:148`) and EffectsQuality scaling.
- Placeholder marker scale/lifespan (`:1296–1298`), elevation offsets (`+72/+28/+24/+18`), `TranslucentSortPriority=13`.
- Automation HP/item grants and proof target staging in `hero1axeaoe*` modes.
- Hero variant emitter geometry (`Hero_2..12`) — presentation parity until each gets a reviewed packet.
- Damage authority paths: target selection, logical hitbox, `DamageBySource` — VFX stays presentation only.

## 5. Searchable Anchors
- Resolver/dispatch: `ResolveCombatVFXBinding`, `ShouldSuppressWeaponBaseProjectileVisual`, `TrySpawnBoundIdolImpactVFX`, `GetCombatVFXBindingData` (`T66GameInstance`).
- Placeholders/scaffolding: `SpawnWaterIdolImpactPlaceholderVFX`, `SpawnIdolImpactPlaceholderVFX`, `GetIdolNiagaraEffectPath`, `GetIdolBlueprintEffectClassPath`, `TrySpawnHeroPierceVariantPixels`/`AOE`/`Bounce`/`DOT`.
- Allowlists: `ImpactPresentationProofIdols`, `SupportedProofIdols`, `New-CategoryNativeCase`.
- Stale naming: `ImpactPresentationOwnsWaterPlaceholder`, `WaterIdolImpactContextCount`, `bWaterContextParity`, `CombatIdolImpactDiagnostic`.
- Diagnostics: `CombatImpactContext`, `CombatImpactChainDiagnostic`, `CombatVFXProductionSpawned`, `CombatVFXFallbackPlaceholder`, `CombatVFXIdolProjectileLaneSuppressed`.
- Files: `Source/T66/Gameplay/T66CombatVFX.cpp`, `T66CombatComponent.cpp`, `T66PlayerController_Overlays.cpp`; `Source/T66/Data/T66DataTypes.h` (`FT66CombatVFXBindingData`, `ET66CombatVFXBindingSourceType`); `Scripts/ValidateCombatVFXProductionBindings.py`, `RunHero1AxeIdolCategoryNativeImpactProof.ps1`; `Content/Data/CombatVFXBindings.csv` + `DT_CombatVFXBindings.uasset`.

Handoff: Phases 1–3 are behavior-preserving and Operator-proposable; Phase 1 `Idol_Water` aggregate and Phase 4 require Codex validation (proof-regex coupling + validator authoring). This is an assessment artifact, not a change approval.
