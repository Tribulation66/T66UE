Phase 2 runtime wiring is complete and compiles. Operator work artifact below — not a greenlight; Codex validates the diff/build/log evidence.

## Changed files
- `Source/T66/Gameplay/T66CombatComponent.cpp` (runtime only)

No other files touched. No assets, CSV/DataTable, scripts, captures, Git, or scans. `MASTER_COMBAT.md` was not edited (the runtime change is self-documenting; no concise note was needed beyond in-code comments).

## Compile
- Command: `& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66Editor Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex -NoHotReloadFromIDE`
- Result: **Succeeded** (17.5s; `Module.T66.26.cpp` recompiled, `UnrealEditor-T66.dll` relinked).

## What changed (source anchors)

**1. Per-link PerChainLink contexts in `PerformBounce`** (`T66CombatComponent.cpp:1970-2030`)
- Replaced the single aggregated `BounceImpactContext` (old `ChainIndex=0`, all handles unioned, one publish) with a `PublishBounceLink` lambda that, per resolved link, builds one `FT66CombatImpactContext` with: `ImpactPoint`/`DamageCenter` = that link's aim point (both valid), `Forward` = planar dir from the previous chain position, `ChainIndex` (0 = locked primary, then 1,2,…), `EffectiveDamage` = that link's effective damage, and the link's single target handle.
- Each link calls `PublishWeaponImpactContext(..., bPrimary = ChainIndex==0)` and then `TrySpawnBoundWeaponBaseSlashVFX` once.
- **Damage authority and target selection preserved exactly**: same `BounceCount`/`Falloff` math, same `FindClosestTargetHandleInRange` walk, same `ApplyResolvedAutoAttackDamage`, same StaticCharge confusion roll. Only publication/spawn was split per link; the primary link (ChainIndex 0) feeds `PrimaryWeaponImpactContext`.

**2. `ImpactAnchored` Bounce branch in the dispatcher** (`T66CombatComponent.cpp:1135`, `1174-1184`, `1201`)
- New `bImpactAnchoredCarrier = (AttackCategory == Bounce)`. Anchors `VisualPivot` at the link `ImpactPoint`, fixed small footprint `VisualScaleVec = FVector(max(0.01, Binding.VisualScaleMultiplier))` (no radius/lane scaling), `SpawnRotation` along the chain forward, logs `VisualAnchorModel=ImpactAnchored`, and spawns directly at the impact point (no +70 lift). AOE/Pierce/Slash branches untouched.
- **Graceful no-binding**: unchanged binding resolution — if no `Hero1Axe_Bounce_Base` row resolves, the function returns false before spawning; the temporary projectile visual (spawned pre-damage at `:2571`, suppressed only when a binding sets `bSuppressTemporaryProjectile`) is preserved.

**3. Water-idol parity diagnostic robustness** (`T66CombatComponent.cpp:2729-2732`)
- The verbose Water-AOE-idol overlay diagnostic previously predicted `ImpactPresentationIdolSlots * EligibleWeaponImpactContexts`, which assumed exactly one weapon context per attack. Since the overlay is built once per slot from the *primary* context, PerChainLink Bounce (N contexts) would have produced a spurious `WaterIdolContextParity=FAIL`. Expected count now derives from primary-context validity (1 per slot), matching actual production. No gameplay/idol behavior or damage changed; `EligibleWeaponImpactContexts` is still logged. **Flagging this** as the one edit beyond the immediate Bounce path — it's a diagnostic regression my change would otherwise have introduced; please confirm it's within scope.

## AOE / Pierce / downstream preservation
- AOE (`PerformSlash`) and Pierce (`PerformPierce`) branches and their single-context publishes are unchanged.
- Downstream idol processing reads `WeaponImpactContexts` / `PrimaryWeaponImpactContext` as before; per-link Bounce contexts now populate that array so future idol/chaining systems can consume each link.

## Verification skipped
- No editor run / gameplay capture / per-link log capture was performed (excluded by scope; no production Bounce binding/assets exist yet, so a live `CombatVFXProductionSpawned VisualAnchorModel=ImpactAnchored` log cannot be produced until Phase 3/4 adds the binding). The spawn path is in place and compiles; runtime log proof is deferred to the asset/binding phase. If you want me to attempt a current editor run anyway, say so.
