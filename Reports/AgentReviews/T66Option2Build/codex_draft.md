Working task:
Operator: Codex
Validator: Claude per `.t66/operator-state.json`
Scope: Implement option 2 only: Stream A full, Stream C layout half, Stream D/idol data-schema/UI/status/stat work that does not require Foundation traveler rendering; explicitly defer Stream B traveler adapter and any `FireOutgoingTraveler`/`CancelOutgoingTraveler` calls; no Mini/minigame scope; no git operations.
Stop condition: Scoped code/data/UI changes are made, current verification is attempted, and deferred traveler work plus remaining caveats are reported with token usage.

Summary of implemented work:
- Stream A:
  - Reduced generated weapon pool to 48 rows in `Content/Data/Weapons.csv`: one hero-locked category per hero across 4 rarities.
  - `UT66WeaponManagerSubsystem::BuildWeaponOffers` now offers only the selected hero's `PrimaryCategory`.
  - Added shared local-stage rarity helpers in `UT66DifficultyTuningSubsystem`; weapon altar and idol stock both use local stage 1-4 rarity mapping.
  - Reclassified VFX bindings: Hero 1 AOE baseline, Hero 2 Pierce, Hero 4 Bounce, Hero 5 DOT, and Hero 3 AOE placeholder reuse of Hero 1 AOE. Hero 3 reuse rows are explicitly flagged in notes.
  - ULT path now uses charge (`UltimateChargeRequired=100`) instead of cooldown gating, and HUD displays charge/ready state.
  - Hero CSV primary-category tuning updated for per-hero category/stat distribution.
- Stream C layout half:
  - Added `ShouldUseBossRushFinaleLayout()` so `IsBossRushFinaleStage()` can still own final-boss difficulty-clear behavior while tower layout remains normal 5-floor layout.
  - Replaced layout/floor/altar call sites in MainMap, Bootstrap, Tower, and WorldInteractables with the new layout predicate while leaving BossFlow final-clear orchestration on `IsBossRushFinaleStage()`.
  - Tower direct-entry smoke showed 5 generated floors with floor 5 boss role.
  - Stage-entry weapon altar rarity now uses local-stage weapon rarity; tower stage 4 floor-entry idol altar is allowed while post-final-boss altar remains blocked by the final-boss flow.
- Stream D non-traveler:
  - Added `ET66IdolElement`, `ET66IdolDelivery`, and clean 16-idol grid data with Element + Delivery columns.
  - Added elemental-power stats (`FirePower`, `IcePower`, `ElectricityPower`, `NaturePower`) with 5% per point scaling against idol damage and behavior scale.
  - Increased equipped idol cap to 4 and normalized saved/equipped arrays append-only.
  - Added No Idol selection stack path and saved `NoIdolSelectionStacks`/`NoIdolPrimaryStatBonuses` append-only in run snapshots.
  - Removed runtime idol-specific status behavior by making `ApplyIdolSpecialBehavior` return without applying legacy status effects.
  - Idol altar UI now pages 4 cards per element plus a No Idol page; HUD/run summary use the 4-slot cap.
  - `FIdolData.AoeDelay` and hero `AoeDelay` are applied at runtime, scaled by attack speed; DOT rule is explicit: Attack Scale/property controls tick count, Attack Speed controls tick interval, and duration is `tickCount * tickInterval`.

Explicitly deferred:
- No Stream B traveler adapter was built.
- No `FireOutgoingTraveler` or `CancelOutgoingTraveler` calls were added.
- Existing Foundation traveler lab/proof names remain untouched; they are pre-existing Foundation/VFXLab artifacts, not a gameplay adapter.

Verification performed:
- `C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex -NoHotReloadFromIDE` passed after the final source patch.
- Data reload commandlets passed:
  - `Scripts/ImportHeroDataTable.py`
  - `Scripts/SetupWeaponsDataTable.py`
  - `Scripts/SetupIdolsDataTable.py`
  - `Scripts/SetupCombatVFXBindingsDataTable.py`
- Combat VFX validator commandlet passed:
  - `Scripts/ValidateCombatVFXProductionBindings.py`
- `Scripts/StageStandaloneBuild.ps1 -SkipCook` passed, rebuilt/staged `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`, and refreshed both standalone shortcuts.
- Packaged stat-pipeline smoke passed with Hero 2:
  - JSON: `C:\UE\T66\Saved\Codex\Option2BuildSmoke\stat_pipeline_smoke_hero2.json`
  - `ok=true`, including `WeaponID=Hero_2_black_pierce Selected=1` and live auto-attack headshot stun.
- Packaged stat-pipeline smoke passed with Hero 1 after updating the smoke harness to wait for the authored AOE delay:
  - JSON: `C:\UE\T66\Saved\Codex\Option2BuildSmoke\stat_pipeline_smoke_hero1_aoe_delay.json`
  - `ok=true`, including `WeaponID=Hero_1_black_aoe Selected=1` and live delayed AOE headshot stun.
- Data count checks:
  - Weapons: 48 rows.
  - Weapon branch distribution: AOE 12, Bounce 16, DOT 12, Pierce 8.
  - Idols: 16 rows, 4 each Fire/Ice/Electricity/Nature.
  - Combat VFX binding rows: 20 rows, with 4 flagged Hero 3 AOE reuse rows.
- `git diff --check` over the scoped touched text files returned no whitespace errors; it emitted only expected LF/CRLF warnings.
- `rg "FireOutgoingTraveler|CancelOutgoingTraveler"` over `Source/T66 Content/Data Scripts` returned no matches.

Caveats / review focus:
- A pre-existing Foundation/VFXLab traveler proof file had a malformed `UE_LOG` severity expression that blocked the packaged target compile. Codex fixed only that compile syntax; no gameplay traveler adapter, renderer, or `FireOutgoingTraveler`/`CancelOutgoingTraveler` path was added.
- `Scripts\StageStandaloneBuild.ps1 -SkipCook` initially hit a transient UnrealBuildTool mutex conflict, then passed on retry after checking no UBT process remained.
- The staged build still reports an existing Niagara deprecation warning in `T66Hero1AxeAOEVFXLabActor.cpp` for `FNiagaraEmitterInstance::IsReadyToRun`.
- There are many pre-existing tracked modifications in the repo outside this scope. Codex did not revert or stage anything.
