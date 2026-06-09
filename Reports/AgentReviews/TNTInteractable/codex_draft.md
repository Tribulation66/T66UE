# Codex Draft: TNT Interactable

Implemented a new non-trap world interactable named TNT.

Changed files:
- `Source/T66/Gameplay/T66TNTInteractable.h`
- `Source/T66/Gameplay/T66TNTInteractable.cpp`
- `Source/T66/Gameplay/GameMode/T66GameModePrivate.h`
- `Source/T66/Gameplay/GameMode/T66GameMode_Lab.cpp`
- `Source/T66/UI/T66CollectorOverlayWidget.cpp`

Behavior:
- `AT66TNTInteractable` derives from `AT66WorldInteractableBase`, so it uses the existing generic world-interactable controller flow.
- Pressing interact lights the fuse once, marks the interactable consumed during the fuse, changes its primitive visual color, and starts a `FuseSeconds` timer.
- When the timer expires, TNT applies radial damage by actor-distance checks:
  - hero HP through `UT66RunStateSubsystem::ApplyDamage(DamageHP, this, "TNTExplosion", this)`;
  - rich enemies through `AT66EnemyBase::TakeDamageFromEnvironment`;
  - lightweight mobs through their existing `TakeDamageFromHeroHitZone` API using a body hit-zone handle and TNT source/event IDs;
  - awakened living bosses through their existing hit-zone damage API using a core hit-zone handle.
- Explosion also emits a debug damage sphere and reuses `UT66CombatComponent::SpawnDeathBurstAtLocation` for a simple explosion burst.
- Non-showcase TNT hides/disables/destroys after exploding. Showcase reusable TNT resets after the explosion.
- Prompt target is `TNT`; prompt action is `light fuse`.

Integration:
- Added TNT to `AT66GameMode::SpawnLabInteractable(FName)` under the `TNT` ID.
- Added a TNT card to the lab Collector interactables tab.

Verification:
- Focused editor build passed:
  `C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex`
- First staged readiness attempt failed before smoke because an existing staged `T66.exe` locked `Saved\StagedBuilds\Windows\Engine\Binaries\ThirdParty\DbgHelp\dbghelp.dll`.
- Second staged readiness attempt reached the full wrapper:
  `Scripts\RunStagedBuildReadinessGate.ps1 -ClientConfig Development`
  - Stage: PASS
  - Staged executable: exists at `Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
  - ProjectRoot shortcut: PASS
  - PinnedTaskbar shortcut: PASS
  - Smoke suite: FAIL, unrelated frontend gate `05_TopBarPowerUpNavigation` missing marker `Frontend automation: widget dump wrote`.
  - Evidence summary: `Saved\StagedBuildReadiness\20260607_162706\summary.json`

Known caveat:
- I did not add a dedicated automated TNT interaction smoke. Runtime proof currently covers compile, cook/stage, and existing smoke wrapper behavior, but not a log-asserted TNT fuse/damage event.
