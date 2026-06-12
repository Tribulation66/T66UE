# Idol-Proc Infrastructure Map + Black/Grey Mesh Root Cause (2026-06-10, READ-ONLY)

## PART A — weapon-impact-driven idol procs

### Verdict up front
The design ("weapon projectile hits an enemy → equipped idol procs at that impact point") is
**already half-built and running in production for 4 idols**, as the "category-native idol
impact-presentation lane" inside `UT66CombatComponent::TryFire`. The lane is gated by a
centralized proof allowlist; extending the design is mostly *widening the allowlist and
finishing per-category dispatch*, not building new plumbing.

### 1. Where a weapon attack knows it hit
- **One narrow struct carries everything**: `FT66CombatImpactContext`
  (T66CombatComponent.h:240–263) — `ImpactPoint` (+ `bImpactPointValid`), `DamageCenter`,
  `AttackOrigin`, `Forward`, `Radius/InnerRadius/HalfAngle/LineLength/TubeRadius`,
  `EffectiveDamage`, `SourceID` (= the weapon ROW, e.g. `Hero_1_black_aoe`), `HeroID`,
  `AttackCategory`, `PrimaryTargetHandle` + `HitTargetHandles` array.
- Built per attack inside `TryFire` (T66CombatComponent.cpp:2080–4869, one giant function)
  and published via the local lambda `PublishWeaponImpactContext`
  (T66CombatComponent.cpp:2565–2580): appends to `WeaponImpactContexts` and captures the
  first primary as `PrimaryWeaponImpactContext`. Publish callsites: pierce :2964, slash
  full-contact :3219, slash :3228/:3233.
- **Ranged single projectiles** (`AT66HeroProjectile::OnSphereOverlap`,
  T66HeroProjectile.cpp:445+): has `OtherActor` + its own location at overlap; applies damage
  directly (`Enemy->TakeDamageFromHero(Damage, SourceID, ...)` :481) — does NOT build an
  impact context today. Pooled travelers report arrival via
  `FT66OutgoingTravelerArrivalEvent` (T66OutgoingTravelerPoolSubsystem.h:117–140):
  `ArrivalPosition`, `ResolvedTargetHandle`, `bHitLiveTarget` — a per-projectile arrival
  callback with the impact position (used at T66CombatComponent.cpp:4189).

### 2. The existing proc infrastructure (what Pablo remembers)
- **The impact-presentation lane** — comment at T66CombatComponent.cpp:2581–2587: idols in
  the allowlist are "driven from the official weapon impact point" and dispatch
  category-native presentation/damage. Allowlist:
  `T66CombatShared::GetImpactPresentationProofIdols()` (T66CombatShared.cpp:99–110) =
  `Idol_Ice_AOE, Idol_Electricity_Pierce, Idol_Electricity_Bounce, Idol_Nature_DOT`
  (Idol_Nature_AOE intentionally absent as control; **Idol_Fire_Pierce is NOT in the lane**
  — it's only in the wider `GetSupportedProofIdols()` traveler grid, :112+).
- The lane consumes `PrimaryWeaponImpactContext`, builds an `IdolImpactContext` per proc
  (T66CombatComponent.cpp:~4400–4760; pierce variant seeds
  `IdolImpactContext.ImpactPoint` via `BuildPierceTargets(...)` :4533 and reuses the weapon's
  `PrimaryTargetHandle` :4548), logs `LogCombatImpactContext(..., "IdolPrimary")`, and has
  full parity diagnostics (`EmitIdolChainDiagnostic`, skip counters
  `IdolSkippedNoWeaponContext`/`IdolSkippedInvalidImpactPoint`/`IdolLegacyFallbackCount`,
  :3990–4005). There is a written contract doc referenced in-code:
  `CombatVFXImpactContextContract.md`.
- **Data-driven impact VFX already keyed by idol**: `FT66CombatVFXBindingData`
  (T66DataTypes.h:45+) with `SourceType` enum `WeaponBase | IdolModifier` (:32–36), rows in
  `Content/Data/CombatVFXBindings.csv` (e.g. `Hero1Axe_AOE_Base` → Niagara at the impact).
  Consumers: `TrySpawnBoundWeaponBaseSlashVFX` + `TrySpawnBoundIdolImpactVFX`
  (T66CombatComponent.h:267–268, impl T66CombatVFX.cpp:1706; called from the lane at
  :4576/:4646/:4689/:4748 for Pierce/?, range, DOT, AOE).
- **On-hit proc precedent** (chance-on-hit effects already living at the damage waist):
  life steal (T66CombatComponent.cpp:4955), taunt armor-debuff proc (:4987) — both inside
  `ApplyDamageToTargetHandle`; headshot-stun + invisibility-on-hit chances in run state
  (T66RunStateSubsystem.h:1057/:1081).
- **Schema**: Idols.csv has NO proc columns (cadence/damage/AoE fields only); Weapons.csv has
  NO on-hit-idol fields. The allowlist is code, not data — a data column ("ProcOnWeaponHit")
  would be new.

### 3. How Idol_Fire_Pierce fires TODAY
- No idol cooldown/timer fields exist (grep: none). **Idols fire on the auto-attack
  cadence**: the idol sections are INSIDE `TryFire` (timer at 1.0s/attack-speed,
  T66CombatComponent.cpp:675), iterating `CachedIdolSlots` (built by
  `RecomputeFromRunState`, :1145–1187, from `UT66IdolManagerSubsystem::GetEquippedIdols`,
  4 slots).
- Fire Pierce is in the LEGACY (non-impact-lane) section: switch on `IdolData.Category`
  ~:4035; Pierce branch dispatches pooled travelers (per-target homing) whose arrival
  callback (:4189) and synchronous fallback (:4195) call `SpawnIdolPierceVFX`
  (T66CombatVFX.cpp:1938) — aim-anchored at the HERO, not at any weapon impact. The
  physical-knockback test branch (:4049+) replaces travelers with the straight sweep, also
  hero-anchored (`LowOrigin` from hero feet).

### 4. Damage narrow waist
`UT66CombatComponent::ApplyDamageToTargetHandle` (T66CombatComponent.cpp:4889): receives
`TargetHandle + DamageAmount + EventType + SourceID` only. `DamageOrigin` is recomputed as
the HERO's location (:4894) — **the impact position does NOT survive to the waist**; only
the enemy ref does. Existing on-hit procs there (life steal/taunt) are position-less. For
position-accurate procs the insertion must be upstream (the impact-context sites) or the
waist must gain an optional impact-position parameter.

### 5. Multi-hit shape
- One slash/AoE = ONE `FT66CombatImpactContext` per projectile lobe with ALL victims in
  `HitTargetHandles` (and damage applied per-enemy via `SlashDamageEntries`,
  T66CombatComponent.cpp:2997–3010). Multi-lobe patterns and chain attacks publish one
  context per lobe/link ("PerChainLink Bounce publishes one per link", comment :3977).
- So both mechanics are natively expressible: proc-once-per-attack (use the primary context)
  vs proc-per-enemy (iterate `HitTargetHandles`, or per published context). The existing
  lane proc's ONCE per idol slot per attack from the primary context
  (`ExpectedIdolImpactContexts = ImpactPresentationIdolSlots * (primary valid ? 1 : 0)`,
  :3989).

### 6. Honest assessment — cleanest insertion points
- **Cleanest: extend the existing lane.** Add idols (e.g. Fire) to
  `GetImpactPresentationProofIdols()` and give the lane's category switch a Fire/Pierce
  presentation (it already has Pierce dispatch for Electricity). The "fire streak from the
  hit position" = the lane's pierce branch with `IdolImpactContext.AttackOrigin =
  PrimaryWeaponImpactContext.ImpactPoint` — the context machinery, diagnostics, VFX
  bindings, and damage-ownership flags all exist.
- **What must be built**: per-idol-category presentation for the non-proof elements
  (Fire currently legacy-only); a data field if membership should be CSV-driven instead of
  the code Set; impact-position plumbed into `SpawnIdolPierceVFX`-style calls (today they
  derive from hero aim); optionally arrival-event procs for ranged weapons
  (`FT66OutgoingTravelerArrivalEvent.ArrivalPosition` is ready-made).
- **Systems that would fight it**:
  1. Idol cadence == attack cadence (no independent timers) — proc-on-hit doubles down on
     this coupling; fine for the design, but "proc chance" needs new state if desired.
  2. The legacy traveler path and the impact lane are PARALLEL branches keyed off the same
     allowlist — an idol must move lanes, not run both (the lane logs
     `IdolLegacyFallbackCount` for exactly this hygiene).
  3. `t66.Combat.PhysicalKnockbackTest` (default 1) currently REPLACES the Fire Pierce
     production path with the hero-anchored sweep — any Fire impact-proc work must decide
     how the test branch composes with it.
  4. Pooling: traveler visuals come from `T66OutgoingTravelerPoolSubsystem`; per-impact
     spawned actors (like the mesh streak) bypass the pool — heavy proc rates may want the
     pool instead.

## PART B — black/grey projectile meshes: ROOT CAUSE CONFIRMED (hypothesis corrected)

**It is NOT a persisting material override.** The override chain works as designed:
- `ApplyProfileToMesh` ends in `FT66VisualUtil::ApplyT66Color`
  (T66TemporaryProjectileSystem.cpp:236 → T66VisualUtil.cpp:75–93) which sets a flat-color
  MID as a COMPONENT override (`Mesh->SetMaterial(0, Mat)`, :91).
- `AT66HeroProjectile::BeginPlay` (T66HeroProjectile.cpp:57–69) additionally creates a tint
  MID — but BeginPlay runs inside `FinishSpawning`, i.e. BEFORE both hook callsites.
- The hook helper `ApplyCustomVisualMeshOverride` (T66HeroProjectile.cpp:242–266) calls
  `VisualMesh->EmptyOverrideMaterials()` AFTER `SetStaticMesh` — all overrides are cleared
  and the SLOT DEFAULT renders. Order verified at both callsites
  (T66CombatComponent.cpp:3278; T66CombatVFX.cpp:2035).

**The actual root cause: the mesh assets' slot defaults were never durably set to the MIs.**
Read-only probe (`Saved/CombatTest/FallGuys/projectile_materials_probe.json`):
- `SM_WeaponProjectile_Black` slot 0 → `/Engine/EngineMaterials/WorldGridMaterial` (grey).
- `SM_IdolProjectile_FireBlack` slot 0 → `/Game/Weapons/Projectiles/FriendSlop/Material_0`
  (Interchange junk auto-material, dark grey).
- The MIs themselves are correct and orphaned: parent `M_FriendSlop_FallGuys`,
  `BaseColorTexture` set to the right albedos — assigned to nothing.

The import-setup script wrote `static_materials` and saved, but the assignment did not
persist (the `junk_material_refs` readback in `finish_projectiles.json` flagged it at the
time and was misread as stale asset-registry data). Visual confirmation: the fire-streak
albedo `T_IdolProjectile_FireBlack_BaseColor` is BLAZING orange/red — nothing orange has
ever appeared in any capture. (The horn's albedo is authored near-black, which masked the
bug: grey WorldGrid at small scale reads like the intended dark horn.)

**Fix when commissioned (one of):** re-assign slot 0 on both mesh assets via
`StaticMesh.set_material(0, MI)` + save (editor pass), or belt-and-suspenders at runtime:
`VisualMesh->SetMaterial(0, LoadedMI)` in `ApplyCustomVisualMeshOverride` after
`EmptyOverrideMaterials`. (Either way the cook already contains both MIs + textures since
the /Game/Weapons cook rule landed.)
