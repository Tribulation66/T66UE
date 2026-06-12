# BLACK-Tier Projectile Meshes — Proof (2026-06-10)

Mission: wire the two FriendSlop projectile meshes as the visuals for the BLACK-tier Hero-1
basic slash and the BLACK Fire Pierce idol ONLY, gated, and prove them live with the
physical-knockback test.

## Pipeline

- **STEP 1 (Blender 5.1, headless)** — `Model Generation/Scripts/Batches/Weapons/FriendSlopProjectiles/export_friendslop_projectiles_unreal_ready.py`
  (modeled on `export_weapon_projectiles_unreal_ready.py`): import GLBs from
  `Model Generation/Runs/Pixal3D/FriendSlopProjectiles_20260609_0659/Outputs/`,
  conservative clean (merge 0.1 mm + degenerate dissolve, UVs untouched), shade smooth +
  Weighted Normal applied (master is LIT), long axis normalized to +X / 2.0 m.
  Exports: `SourceAssets/Import/Weapons/Projectiles/FriendSlop/UnrealReady/*.fbx` + raw
  base-color PNGs. Gotcha fixed: the GLB-embedded textures were **WebP bytes in .png
  clothing** — converted to real PNG via ffmpeg before UE would import them.
  Manifest: `.../FriendSlopProjectiles_20260609_0659/Notes/FriendSlopProjectiles_UnrealReadyManifest.json`
  (WeaponProjectile 193,477 tris; IdolProjectile 187,504 tris — Trellis-heavy, candidates
  for decimation later).

- **STEP 2 (UE)** — assets at `/Game/Weapons/Projectiles/FriendSlop/`:
  `SM_WeaponProjectile_Black` + `MI_WeaponProjectile_Black`, `SM_IdolProjectile_FireBlack` +
  `MI_IdolProjectile_FireBlack` (instances of `M_FriendSlop_FallGuys`, BaseColorTexture = raw
  albedo; assigned as the meshes' slot defaults). Import route that works in UE 5.7
  automation: full editor + `-ExecutePythonScript` (the `-run=pythonscript` commandlet
  crashes — Interchange needs Slate; legacy FbxImportUI options also crash).

## Wiring (all behind `t66.Combat.ProjectileMeshes`, int32, default 1)

- `AT66HeroProjectile::ApplyCustomVisualMeshOverride(UStaticMesh*)`
  (T66HeroProjectile.h/.cpp) — sets VisualMesh, clears override materials so the slot MI
  shows, resets relative scale/rotation, hides the AccentMesh overlay.
- **Slash horn** — `T66CombatComponent.cpp`, PerformSlash payload, right after
  `TrySpawnBoundWeaponBaseSlashVFX` (~line 3256): the bound slash visual is pure Niagara, so
  the horn rides a 0.25 s visual-only `SpawnVisualTravelProjectile` along the slash arc.
  Gate: `AttackCategory == AOE && PatternID == "Hero1CrescentSingle"` — that pattern exists
  ONLY on the `Hero_1_black_aoe` weapon row, so the weapon row itself is the black-tier
  selector; higher tiers (Triple/Five/FullContact patterns) never enter.
- **Fire streak** — `T66CombatVFX.cpp`, `SpawnIdolPierceVFX` (~line 2000): new branch ABOVE
  the force-enabled primitive-placeholder branch (which had been swallowing it) and above
  Niagara. Gate: `IdolID == Idol_Fire_Pierce && Rarity == Black`. Travels the official
  pierce line on a visual-only travel projectile (dist/2400 s). Collision, damage,
  ProjectileMovement, Niagara assets: untouched.

## Live proof (TestRoom, PhysicalKnockbackTest=1, capture `proj14_on`)

- `CombatBlackTierSlashHorn Origin=(850,166) Center=(949,166)` — horn spawned on the slash.
- `CombatVFXBlackTierMeshStreak SourceID=Idol_Fire_Pierce Travel=0.86s` — streak on the pierce line.
- `T66PhysicalKnockbackTest_FirePierce ... Targets=2` — two EASY mobs launched (point-blank
  pack before the volley; arena empty by the 2.35 s frame).
- `proj14_on_testroom_action1.png` — **both meshes in flight in one frame**, textured + lit.

## Regression (meshes OFF / other tiers)

- `-ExecCmds="t66.Combat.ProjectileMeshes 0"` (`proj5_off`/`proj6_off` logs):
  `CombatVFXPrimitiveIdolPlaceholderSpawned` returns — legacy visuals restored.
- Gate triad log (`CombatBlackTierSlashHornGate`, verbose-only) proved refusal when
  conditions don't match; non-black rows carry different pattern IDs and never qualify.

## Capture-harness additions (automation-only, no production impact)

- `-T66HeroLookDevIdol=<IdolID>` — equips the idol (base tier) + the Hero_1 black weapon,
  refreshes combat caches (`RecomputeFromRunStateForAutomation`, new !UE_BUILD_SHIPPING
  wrapper), drives `TryFire` every 1.2 s (auto-attack is suppressed in automation; hero
  resolved fresh per tick — entry flags change possession timing), locks the nearest mob.
- Four action shots (1.1/2.35/3.55/5.95 s) with a camera that frames any live
  `AT66HeroProjectile` (fallback: elevated 3/4 lane view); quit at 8.2 s.
- Default automation hero is **Hero_2** — remember this for any future hero-gated tests.

## Tuning knobs for next pass (report-only, not changed)

1. **Scale** — both meshes normalized to 2.0 m read LARGE in flight (~2× hero height for
   the horn). First knob: per-call scale on `ApplyCustomVisualMeshOverride` (≈0.6 horn,
   ≈0.8 streak) or re-export at smaller target length.
2. **Orientation** — horn flies crescent-up with the flat face leading; a relative roll
   (~90°) would read as a horizontal slash wave. Streak head leads correctly.
3. Tri counts (~190 k each) — decimate in a later Blender pass if perf asks.
