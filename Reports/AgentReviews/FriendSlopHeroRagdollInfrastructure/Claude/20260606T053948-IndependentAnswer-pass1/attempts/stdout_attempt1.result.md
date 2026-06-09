Result: OK

## Independent Answer

**Recommended shape: a new hero/character-owned `UT66KnockbackComponent` (ragdoll/knockback) plus a data-authored tuning struct, with TestRoom reduced to a trigger.** The current ~600 lines of ragdoll machinery living as file-local structs/free-functions inside `T66GameMode_TestRoom.cpp` (the `FWipeoutArmTrapState`, `EWipeoutArmRagdollPhase`, bone-selection helpers, PAC create/drive, follow/settle/recover loop) is exactly the reusable infrastructure that should move onto the hero.

**1. Ownership — files/classes:**
- New component `Source/T66/Gameplay/Combat/T66KnockbackComponent.{h,cpp}` (or `Gameplay/Movement/` to sit beside `T66HeroMovementComponent`). It owns: phase enum, sim-root/follow/velocity bone selection, the PAC create+drive, the per-tick follow/settle/recover state machine, and the timer that currently lives in the GameMode. This is the single home so bosses/elites later just add the same component.
- New tuning struct `FT66KnockbackProfile` (USTRUCT) — follow the existing `FT66HeroMovementTuning` pattern: a struct exposed as an `EditDefaultsOnly` UPROPERTY on the component, **not** new C++ constants. This satisfies the data-authored-over-hardcoded constraint without forcing a full DataTable in this pass. Fields: launch impulse magnitude/direction model, below-bodies impulse fraction (currently the `*0.15f`), min-incap seconds, max-ragdoll seconds, settle speed, settle-hold seconds, blend-out seconds, PAC enable/drive-mode/strength/activation-delay. Keep the existing `t66.TestRoom.*` CVars as runtime debug overrides that read through to the profile so the current tuning surface isn't lost.
- `UT66HeroBase` owns a `TObjectPtr<UT66KnockbackComponent>` constructed in the ctor (mirroring how `CombatComponent`/`HeroMovementComponent` are wired). The PAC is created/parented to the hero by the component, as it already is today (`Hero->AddInstanceComponent`).

**2. Minimal `AT66HeroBase` API:**
- `UT66KnockbackComponent* GetKnockbackComponent() const;`
- A thin pass-through `void ApplyKnockback(const FVector& LaunchVelocity, const FT66KnockbackProfile* Override = nullptr);` (or just let callers go through the component getter — one of the two, not both). Internally it: enables mesh physics from the selected sim-root, applies the impulse, suppresses movement + calls `CombatComponent->SetAutoAttackSuppressed(true)`, and registers the per-tick update. Recovery restores both.
- `bool IsKnockedDown() const;` for callers/anim/UI gating.
- Suppression must be reference/flag-restored on recovery so overlapping traps don't permanently disable attacks.

**3. How TestRoom calls the real path:** TestRoom keeps only trap geometry/overlap detection and the swing timing. On impact it computes the launch vector and calls `Hero->ApplyKnockback(LaunchVelocity)` (or `Hero->GetKnockbackComponent()->Knockback(...)`). It should **delete** `FWipeoutArmTrapState`'s ragdoll fields, the PAC helpers, the bone selectors, and `UpdateWipeoutArmRagdollState` — leaving the trap state holding only swing/cooldown data. Net: TestRoom no longer references `PhysicalAnimationComponent.h`, `AddImpulse`, or `SetAllBodiesBelowSimulatePhysics`.

**4. Impulse/PAC defaults to stop the stretching:** the smear is the classic symptom of (a) impulse applied at one bone via `bVelChange=true` while neighbors lag, and/or (b) PAC driving toward a stale/garbage pose. Defaults the component should ship:
- Drive **all bodies below the sim-root to simulate** and set blend weight to 1 *before* applying impulse (the code already does `SetAllBodiesBelowSimulatePhysics` + `SetAllBodiesBelowPhysicsBlendWeight(...,1)` — keep that ordering).
- Apply the main impulse with `bVelChange=true` but **also** push the below-bodies impulse at a higher coherence fraction than the current `0.15` (try `0.5–0.7`) so the whole chain leaves at a similar velocity instead of the velocity-bone whipping away from a near-static pelvis. Make this the profile's `BelowBodiesImpulseFraction`.
- Keep PAC **off by default at impact** and only enable it after the existing activation delay once pose buffers are valid (`HasWipeoutArmPhysicalAnimationPoseBuffers`). Driving PAC against an unrefreshed pose is a prime stretch cause — the delayed-activation guard already present should be the default, not an opt-in CVar.
- Default `PhysicalAnimationDriveMode` to drive the **core chain** (mode 2) rather than all-below-root, and a moderate strength, then let data tune up. This avoids over-driving extremities that produce smear.
- These are starting points; the user owns the *feel*, so expose them in `FT66KnockbackProfile` for iteration rather than baking final numbers.

**5. Compile/proof checks for Codex:**
- Build the `T66`/`T66Editor` targets (UBT) — the Operator's standard compile step — and confirm zero new warnings around the moved code.
- Grep-confirm `T66GameMode_TestRoom.cpp` no longer references `PhysicalAnimation`, `AddImpulse`, `SetAllBodiesBelow*`, or the deleted struct fields.
- Staged standalone validation (playable standalone is affected): launch the standalone TestRoom, trigger the wipeout arm on the selected hero, and capture the existing `UE_LOG` "wipeout arm active ragdoll impact" + recovery lines to prove the component path fires, plus a visual check that the body launches and settles without stretch/smear, then recovers control and re-enables auto-attack.
- Confirm a second consecutive hit still knocks down and recovers (suppression flag restored).

## Evidence Checked
- `T66HeroBase.h` — confirms `CombatComponent` + `HeroMovementComponent` ownership pattern and ctor-wired components; no existing knockback/ragdoll API.
- `T66GameMode_TestRoom.cpp` (grep) — confirms TestRoom currently owns the full ragdoll/PAC/follow/recover machinery, `AddImpulse(...,true)`, `AddImpulseToAllBodiesBelow(...,0.15f,...,true)`, and `t66.TestRoom.*` CVars.
- `T66CombatComponent.h:39` — `SetAutoAttackSuppressed(bool)` exists for suppression.
- `T66HeroMovementComponent.h:52-53` — `FT66HeroMovementTuning` as an `EditDefaultsOnly` USTRUCT is the established data-authored tuning pattern to mirror for the profile.
- `Content/Data/*.csv` — DataTable authoring exists (Heroes, Bosses, Enemies, StatusEffects), available if a CSV-backed profile is preferred later for per-actor scaling.

## Questions Or Blockers
None that require the user. Scope is hero-only, well-bounded, and the path is decidable from repo evidence. Codex can implement directly.

## Caveats
- Profile-in-component (struct) vs. full DataTable is a judgment call: struct is the right *minimal* call for this pass and matches `FT66HeroMovementTuning`; a `Knockback.csv` DataTable becomes worthwhile only when bosses/elites need per-row scaling — defer it, but name/shape the struct so it can become a DataTable row type later.
- The exact impulse/PAC numbers above are directional fixes for the stretch, not final feel; the user owns Fall-Guys feel tuning, so land them as editable profile defaults, not constants.
- I did not open `T66KnockbackComponent` (doesn't exist) or read every line of the 1300-line GameMode beyond the grep — Codex should confirm no TestRoom-only state (swing timing, overlap bookkeeping) gets accidentally pulled into the shared component during extraction.
- Verify the PAC component lifetime when moved: it's currently created on the hero via `AddInstanceComponent`; ensure the new component cleans it up on recovery/EndPlay to avoid leaking an instance component across runs.
