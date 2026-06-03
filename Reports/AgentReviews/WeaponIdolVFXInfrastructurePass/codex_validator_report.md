Verdict: APPROVE_WITH_CORRECTION

# Codex Validator Report — Weapon/Idol VFX Infrastructure Pass

## Packet Completeness Gate

- Claude packet first line: PASS (`Operator Packet: COMPLETE`)
- Read-only scope: PASS
- Mini/minigame exclusion: PASS
- File/symbol anchors included: PASS
- Correction required: YES

## Required Correction

Claude repeated a stale baseline claim that only Hero 1 AOE has a production binding row. Live source inspection shows:

- `Content/Data/CombatVFXBindings.csv` contains active `WeaponBase` rows for:
  - `Hero1Axe_AOE_Base` / `Hero_1_black_aoe` / `AOE`
  - `Hero1Axe_Pierce_Base` / `Hero_1_black_pierce` / `Pierce`
  - `Hero1Axe_Bounce_Base` / `Hero_1_black_bounce` / `Bounce`
- No active DOT row is present.
- `Content/Data/DT_CombatVFXBindings.uasset` has the same timestamp as the CSV, so the imported DataTable likely matches the CSV.
- `Gameplay/Combat/VFX_PROCESS_INDEX.md` still says Pierce and Bounce have no active production binding rows. That doc is stale relative to the live CSV/DataTable.

## Validated Findings

| Finding | Status | Evidence |
|---|---|---|
| Idol impact lane is category-general but Water-named | PASS | `UsesImpactPresentationForIdol`, `WaterIdolImpactContextCount`, `CombatImpactChainDiagnostic`, `CombatIdolImpactDiagnostic` in `Source/T66/Gameplay/T66CombatComponent.cpp` |
| Runtime proof-idol allowlist is duplicated in several places | PASS | `ImpactPresentationProofIdols`, `SupportedProofIdols`, `RunHero1AxeIdolCategoryNativeImpactProof.ps1` cases |
| Placeholder idol presentation still exists | PASS | `SpawnWaterIdolImpactPlaceholderVFX`, `SpawnIdolImpactPlaceholderVFX` in `Source/T66/Gameplay/T66CombatVFX.cpp` |
| Imported-pack idol path table is scaffolding parallel to binding table | PASS | `GetIdolNiagaraEffectPath`, legacy `SpawnIdolPierceVFX/AOEVFX/BounceVFX/DOTVFX` |
| Hero variant pixel emitters remain placeholder/scaffolding | PASS | `TrySpawnHeroPierceVariantPixels`, `TrySpawnHeroAOEVariantPixels`, `TrySpawnHeroBounceVariantPixels`, `TrySpawnHeroDOTVariantPixels` |
| Validator hardening gap remains | PASS | `pending_issues_Combat.md` documents impact-context and alignment contracts not yet validator-enforced |
| Process docs are stale against current binding source | PASS | `VFX_PROCESS_INDEX.md` says Pierce/Bounce no active row; CSV has rows |

## Behavior-Preserving Infrastructure Priorities

1. Rename/de-Water generalized idol impact code and logs without changing behavior.
2. Centralize proof-idol membership and proof metadata so runtime, overlay harness, and proof runner cannot drift.
3. Split proof-only placeholder presentation from production binding seams more explicitly.
4. Harden validator/proof wrappers for generalized impact-context and visual/damage-alignment contracts.
5. Refresh docs to reflect current live binding rows and clearly separate production, proof-only, placeholder, and deferred art states.

## No-Touch List

Do not change as part of an infrastructure cleanup:

- weapon/idol damage behavior,
- allowed proof idol membership,
- `AoeDelay` timing behavior,
- `AoeInnerRadiusRatio` values,
- falloff/default tuning,
- proof target HP/staging values,
- placeholder visual scale/lifespan,
- final Niagara art/content.

## Suggested Next Phase

Use one bounded behavior-preserving pass:

- rename Water-era generic variables/log text,
- preserve compatibility where existing proof scripts parse old Water diagnostics,
- centralize proof-idol metadata,
- update stale process docs,
- run focused compile plus the existing idol category-native proof runner.

This pass should not author new Niagara, add DOT production binding, or change gameplay damage/timing.
