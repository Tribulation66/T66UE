Result: OK

## Independent Answer

Build TNT as a new `AT66TNTInteractable : public AT66WorldInteractableBase` (single-file `.h/.cpp` under `Source/T66/Gameplay/`), following the existing subclass pattern (e.g. `T66CrateInteractable`). Wire it as:

1. **Interaction → fuse → explosion.** Override `Interact(APlayerController*)`. On first interact, guard against re-trigger (`bConsumed`/an `bArmed` flag, respecting `bShowcaseReusable`), then start a timer (`GetWorldTimerManager().SetTimer`, fuse seconds as an `EditAnywhere` C++ default — a narrow first-pass default is sanctioned by the contract). Return whatever the base's consume semantics expect. On timer expiry, run the explosion then hide/destroy (or reset if `bShowcaseReusable`).

2. **Damage both factions — do NOT route through the trap utils.** `FT66TrapDamageUtils` is hard-typed to `AT66TrapBase`, and the contract explicitly says TNT is not a trap, so it can't be reused. Instead call the same primitives those utils call:
   - Gather targets with a sphere overlap (`UKismetSystemLibrary::SphereOverlapActors` or a transient `USphereComponent`/`GetOverlappingActors`) at an `EditAnywhere` radius.
   - Heroes: `UT66RunStateSubsystem::ApplyDamage(Damage, this, FName("TNT"/...), this)` via `GameInstance->GetSubsystem<UT66RunStateSubsystem>()`.
   - Enemies: `Cast<AT66EnemyBase>(Target)->TakeDamageFromEnvironment(Damage, this)` (signature at `T66EnemyBase.h:129`).
   - Consider optional falloff and a self-only-once guard; flat radius damage is acceptable for a first pass.

3. **Lab Collector spawnability — two edit sites, both required.** Add the ID→class branch in `AT66GameMode::SpawnLabInteractable` (`T66GameMode_Lab.cpp:244-252`, e.g. `else if (InteractableID == FName(TEXT("TNT"))) ClassToSpawn = AT66TNTInteractable::StaticClass();` plus the include), and add the matching `AddSpawnCard(...OnSpawnInteractable(FName(TEXT("TNT")))...)` in `T66CollectorOverlayWidget.cpp` near lines 379–381. Missing either one leaves it unspawnable or uncompiled.

4. **Visuals.** Base already assigns a primitive cube; placeholder is in-scope, so no Pixal3D work. Optional tint/scale tweak only.

5. **Verification.** Run a focused module compile, then a staged standalone Lab run: spawn TNT via Collector, interact, confirm the fuse delay and that nearby heroes **and** enemies both take damage. If the runtime damage-to-both-factions path isn't actually observed in a staged run, call that out explicitly as unverified per the stop condition — compile-only is not sufficient here.

## Evidence Checked

- `T66WorldInteractableBase.h/.cpp` — base API: `Interact` is pure-virtual, `bConsumed`/`bShowcaseReusable`, cube visual default, registry + prompt lifecycle.
- `T66TrapDamageUtils.cpp` — damage primitives: hero `RunStateSubsystem->ApplyDamage`, enemy `TakeDamageFromEnvironment`; all entry points require `AT66TrapBase`.
- `T66EnemyBase.h:129` — `TakeDamageFromEnvironment` signature.
- `T66GameMode_Lab.cpp:238-253` — `SpawnLabInteractable` ID→class switch.
- `T66CollectorOverlayWidget.cpp:76-381` — spawn-card UI → `OnSpawnInteractable` → `SpawnLabInteractable`.
- `T66CrateInteractable.h` — minimal subclass shape.
- World/global routers — no rule against this; TNT is not world-generation scope.

## Questions Or Blockers

None that require the user. Tuning defaults (fuse seconds, radius, damage, falloff) are unspecified but the contract authorizes scoped C++ defaults, so Codex can pick sensible values and leave them `EditAnywhere`.

## Caveats

- Confirm the exact `UT66RunStateSubsystem::ApplyDamage` and `TakeDamageFromEnvironment` signatures/`DeliveryMethod` conventions at implementation time rather than copying the trap call verbatim — the trap path applies trap-specific scaling/`CanAffect` gates that TNT should not inherit.
- Friendly-fire to heroes is intended per the prompt ("damages everything around it, heroes and enemies"); make sure no global hero-damage guard silently no-ops it.
- The two Lab-integration edit sites are the most likely thing to be half-done; both must land for live testing.
- Result is OK on the assumption the staged runtime explosion check is actually executed; if it can't be run in this environment, that limitation must be surfaced, not papered over.
