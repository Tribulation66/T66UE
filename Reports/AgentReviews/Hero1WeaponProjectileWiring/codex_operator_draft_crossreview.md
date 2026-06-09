Task: create and wire the four approved Hero 1 AOE weapon placeholder projectiles.

Operator result:
- Added Hero 1 AOE weapon data overrides in `Scripts/SetupWeaponsDataTable.py`.
- Regenerated `Content/Data/Weapons.csv` so:
  - `Hero_1_black_aoe`: `Hero1CrescentSingle`, projectile count 1, damage multiplier 1.20, bonus AOE radius 120.0, inner ratio 0.54.
  - `Hero_1_red_aoe`: `Hero1CrescentTriple`, projectile count 3, damage multiplier 1.44, bonus AOE radius 322.5, inner ratio 0.54.
  - `Hero_1_yellow_aoe`: `Hero1CrescentFive`, projectile count 5, damage multiplier 1.80, bonus AOE radius 416.3, inner ratio 0.54.
  - `Hero_1_white_aoe`: `Hero1CrescentFullContact`, projectile count 1, damage multiplier 2.40, bonus AOE radius 495.0, inner ratio 0.54.
- Updated `Scripts/ValidateCombatVFXProductionBindings.py` to enforce those four Hero 1 placeholder patterns and keep non-Hero-1 AOE rows on filled-sector inner ratio 0.
- Reworked `UT66CombatComponent::PerformSlash` in `Source/T66/Gameplay/T66CombatComponent.cpp`:
  - recognizes the four Hero 1 placeholder crescent pattern IDs;
  - uses connected side-by-side lobes for red and yellow rather than fan spread;
  - aggregates lobe hits before applying damage to avoid duplicate overlap damage;
  - applies primary damage to primary lobe points and 50% splash/body damage to non-primary body hits;
  - treats white full-contact as all-primary wherever the large slash touches;
  - publishes primary weapon impact contexts only for placeholder crescents and keeps `WeaponHitActors` primary-only for the legacy idol fallback path;
  - keeps the existing production Niagara mesh slash carrier via `TrySpawnBoundWeaponBaseSlashVFX`, so this is a gameplay/data placeholder pass, not final per-rarity Niagara authoring.
- Expanded `T66PlayerController_Overlays.cpp` automation proof targets for black/red/yellow/white and fixed the proof harness to lock onto the first primary-style label (`PrimaryCenter`) instead of requiring the literal label `Primary` or later overwriting to side lobes.

Verification performed:
- Focused editor compile passed after final harness patch:
  `Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex -NoHotReloadFromIDE`
- DataTable reload passed:
  `UnrealEditor-Cmd.exe ... -run=pythonscript -script=C:/UE/T66/Scripts/SetupWeaponsDataTable.py`
  Result: Python script executed successfully, 0 errors, 2 known `r.Upscale.Quality` warnings.
- Combat VFX production binding validator passed:
  `UnrealEditor-Cmd.exe ... -run=pythonscript -script=C:/UE/T66/Scripts/ValidateCombatVFXProductionBindings.py`
  Result: Python script executed successfully, 0 errors, 2 known `r.Upscale.Quality` warnings.
- Unreal-owned gameplay capture proofs completed:
  - Black: `C:\UE\T66\Saved\VideoCaptures\hero1axeaoevfxbinding_20260605_035151\hero1axeaoevfxbinding.mp4`
    Log: `C:\UE\T66\Saved\Logs\T66-backup-2026.06.05-06.52.26.log`
    Summary: 4 target PASS lines, 0 FAIL markers, primary 28 damage, body 14 damage, inner/outside controls unhit.
  - Red: `C:\UE\T66\Saved\VideoCaptures\hero1axeaoevfxbinding_20260605_040626\hero1axeaoevfxbinding.mp4`
    Log: `C:\UE\T66\Saved\Logs\T66-backup-2026.06.05-07.07.00.log`
    Summary: 5 target PASS lines, 0 FAIL markers, 3 primary lobe role lines, body overlap damage lines, primary 37 damage, body 19 damage.
  - Yellow: `C:\UE\T66\Saved\VideoCaptures\hero1axeaoevfxbinding_20260605_040703\hero1axeaoevfxbinding.mp4`
    Log: `C:\UE\T66\Saved\Logs\T66.log`
    Summary: 7 target PASS lines, 0 FAIL markers, 5 primary lobe role lines, body overlap damage lines, primary 54 damage, body 27 damage.
  - White: `C:\UE\T66\Saved\VideoCaptures\hero1axeaoevfxbinding_20260605_040241\hero1axeaoevfxbinding.mp4`
    Log: `C:\UE\T66\Saved\Logs\T66-backup-2026.06.05-07.03.12.log`
    Summary: 5 target PASS lines, 0 FAIL markers, all hit targets primary, 86 damage, outside control unhit.
- Staged standalone refresh passed:
  `powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\StageStandaloneBuild.ps1`
  Result: BuildCookRun successful, staged exe ready at `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Shortcut verification passed:
  - `C:\UE\T66\T66 Standalone.lnk` targets `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
  - Taskbar `T66 Standalone.lnk` targets the same staged exe.

Known scope/status:
- PPF status is partial for final art because this pass intentionally wires placeholders using the existing production Niagara mesh slash carrier, not final per-rarity custom Niagara silhouettes/materials.
- Gameplay/data behavior is complete for these four weapon placeholders.
- Legacy idol fallback now receives primary weapon hits only for these placeholder AOE weapons; full category-native idol fan-out across every published weapon impact context remains an idol-system follow-up, not claimed as complete here.

Please cross-review for missed repo constraints, implementation mistakes, evidence gaps, or overclaims in the intended final answer.
