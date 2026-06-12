# Claude Plan Review Packet - Pass 3

## Working Goal

Remove the deprecated HpRegen and LifeSteal main-run item templates, introduce a Special primary item category for Quick Revive and Gambler's Token, refresh item data assets, and verify the focused T66 item-system changes.

## User Request

The user asked to delete the HP regen and lifesteal items rather than only deprecating them. They also asked that both BackroomQuickRevive and Gambler Token have a primary called Special so they do not fall into the seven normal stat buckets such as Armor.

## Scope And Constraints

- Main-run item system only.
- Default Mini/minigame exclusion remains active. I will not inspect, edit, validate, or clean Mini-owned data except to note why shared sprite assets are not deleted in this pass.
- No production edits have been made yet.
- Data asset import/reload process applies because `Content/Data/Items.csv` backs `/Game/Data/DT_Items`.
- Staged standalone verification applies because these are playable item data/code changes.
- This is not a process-governed visual/media/import-art task. PPF is skipped as a trivial data/code infrastructure change where QA cares about the values and runtime behavior, not a replicated production method.

## Applicable Instructions Read

- Root `AGENTS.md` supplied by the user.
- `Gameplay/GAMEPLAY_AGENTS.md`.
- `Gameplay/README.md`.
- `Reports/AGENTS.md`.
- `Tools/README.md`.
- Pending issue files read before working in affected areas:
  - `Content/Data/pending_issues_Data.md`
  - `Source/T66/Data/pending_issues_Data.md`
  - `Source/T66/Core/pending_issues_Core.md`
  - `Source/T66/Core/RunState/pending_issues_RunState.md`

## Review History Addressed

Pass 1 requested deeper audits for item ID references, enum consumers, UI/HUD Special handling, save/load compatibility, staged verification, enum insertion safety, and pending issue routing.

Pass 2 requested:

- Read the real enum tail before inserting `Special`.
- Commit to real staged smoke launch instead of hedging.
- Confirm `PrimaryStatType` UI consumers.
- Confirm no sibling pipeline/orchestrator references remain.
- Name the shared sprite directory in pending issues.
- Strengthen legacy-save compatibility evidence.

## Current Evidence From Live Repo

### Enum Tail

`Source/T66/Data/T66DataTypes.h` currently has:

```cpp
/** Foundational hero stats that items can roll as their primary stat line (Line 1). */
UENUM(BlueprintType)
enum class ET66HeroStatType : uint8
{
    Damage UMETA(DisplayName = "Damage"),
    AttackSpeed UMETA(DisplayName = "AttackSpeed"),
    AttackScale UMETA(DisplayName = "AttackScale"),
    Armor UMETA(DisplayName = "Armor"),
    Evasion UMETA(DisplayName = "Evasion"),
    Luck UMETA(DisplayName = "Luck"),
    Speed UMETA(DisplayName = "Speed"),
    /** Appended to preserve serialized enum values for existing save data and data tables. */
    Accuracy UMETA(DisplayName = "Accuracy"),
};
```

Planned enum edit: append `Special` after `Accuracy`, preserving every existing serialized enum value.

### Main-Run Item Template References

Command used:

```powershell
rg -n "Item_HpRegen|Item_LifeSteal" Source Content Gameplay Scripts Tools -g "!*Mini*"
```

Main-run findings:

- `Content/Data/Items.csv` contains the two active rows to delete.
- `Gameplay/Stats/Accuracy_Item_And_TempBuff_Audit.md` contains historical documentation to update.
- No main-run C++ direct references to either item ID were found.

Broader searches show `HpRegen` and `LifeSteal` still exist as deprecated secondary stat mechanics and old-save compatibility fields. Those runtime secondary-stat mechanics are intentionally out of scope and should not be removed by this item-template cleanup.

### Pipeline And Source Art References

Command used:

```powershell
rg -n "HpRegen|LifeSteal|Armor_HpRegen|Evasion_LifeSteal|Item_HpRegen|Item_LifeSteal" Tools\ArtPipeline Scripts Config -g "!*Mini*"
```

Findings:

- Only `Tools/ArtPipeline/Items/T66ProcessReimaginedItemSheets.py` has main-run art generation series for `Armor_HpRegen_sheet.png` and `Evasion_LifeSteal_sheet.png`.
- `Scripts` and `Config` have no matching main-run references.
- `SourceAssets/ItemSprites` currently contains only `README.md`, so there are no source PNGs to delete there.
- Shared cooked/editor assets exist under `Content/Items/Sprites`:
  - `Item_HpRegen_black.uasset`
  - `Item_HpRegen_red.uasset`
  - `Item_HpRegen_white.uasset`
  - `Item_HpRegen_yellow.uasset`
  - `Item_LifeSteal_black.uasset`
  - `Item_LifeSteal_red.uasset`
  - `Item_LifeSteal_white.uasset`
  - `Item_LifeSteal_yellow.uasset`

Those shared sprite assets will not be deleted in this pass because Mini-owned data still references them and Mini scope is excluded. I will add a `[Minor]` pending issue to `Content/Data/pending_issues_Data.md` naming `Content/Items/Sprites/Item_HpRegen_*.uasset` and `Content/Items/Sprites/Item_LifeSteal_*.uasset` as shared asset cleanup that requires a Mini-inclusive pass.

### UI Consumers Of PrimaryStatType

Command used:

```powershell
rg -n "PrimaryStatType|GetPrimaryStatLabel|ET66HeroStatType" Source\T66\UI -g "!*Mini*"
```

Findings:

- `Source/T66/UI/T66ItemCardTextUtils.cpp` owns the item-card primary-stat label switch and needs a `Special` label in both localized and fallback switches.
- `Source/T66/UI/T66CollectorOverlayWidget.cpp` uses `StaticEnum<ET66HeroStatType>()->GetDisplayNameTextByValue(...)`, so the enum `DisplayName = "Special"` will display correctly there.
- Other UI stat/power surfaces use explicit eight-stat pools or labels and should not include `Special`, because `Special` is a non-stat reward-only category rather than a player stat.

### Runtime Consumers Of ET66HeroStatType

Command used:

```powershell
rg -n "case ET66HeroStatType|ET66HeroStatType::" Source\T66 -g "!*Mini*"
```

Findings:

- `RunStateSubsystem_Stats.cpp` switch functions have default/no-op behavior, so `Special` returns no stat contribution.
- `RunStateSubsystem_EconomyInventory.cpp` has default/no-op behavior in primary stat application and already treats reward-only special items separately.
- `RunStateSubsystem_Private.h` has primary-to-secondary mapping defaults to no secondaries, appropriate for `Special`.
- Buff, power-up, account status, and gameplay stat surfaces enumerate the real stat set explicitly and should not include `Special`.
- `Source/T66/Core/T66GameInstance.cpp` already excludes both `Item_GamblersToken` and `Item_BackroomsQuickRevive` from random item pool eligibility.

### Save/Load Compatibility

Current source evidence:

- `UT66RunStateSubsystem::ImportSavedRunSnapshot` copies saved inventory slots and then calls `RecomputeItemDerivedStats()`.
- `UT66RunStateSubsystem::RecomputeItemDerivedStats()` calls `GI->GetItemData(Slot.ItemTemplateID, D)` and continues when the row is missing.
- Therefore a saved run containing removed item template IDs should retain the serialized slot data but no longer apply missing-row stat effects during recompute.

Local fixture scan:

```powershell
rg -a -n "Item_HpRegen|Item_LifeSteal" Saved\StagedBuilds\Windows\T66\Saved\SaveGames Saved\SaveGames -g "*.sav"
```

This found no existing local or staged save fixture containing the two legacy item IDs.

Verification plan for compatibility:

- Keep the source proof above as the required compatibility gate.
- Re-run the binary-string scan after staging.
- If an existing fixture appears, launch the staged build against that fixture and report the result.
- I will not add production-only test hooks merely to force a legacy save load without a separate review, because current source behavior already handles missing item rows by skipping them and no local fixture exists.

## Planned Production Edits

1. `Source/T66/Data/T66DataTypes.h`
   - Append `Special UMETA(DisplayName = "Special")` after `Accuracy`.
   - Update the `FItemData::PrimaryStatType` comment to state that `Special` is for reward-only/non-stat templates.

2. `Content/Data/Items.csv`
   - Delete the `Item_HpRegen` row.
   - Delete the `Item_LifeSteal` row.
   - Change `Item_BackroomsQuickRevive` `PrimaryStatType` from `Armor` to `Special`.
   - Leave `Item_Alchemy` untouched because the user named only HP regen and lifesteal for deletion.

3. `Source/T66/Core/T66GameInstance.cpp`
   - Change synthetic `Item_GamblersToken` primary stat from `Luck` to `Special`.
   - Leave the existing random item pool exclusions intact.

4. `Source/T66/UI/T66ItemCardTextUtils.cpp`
   - Add `Special` to the item-card primary label switch in both localized and fallback paths.

5. `Tools/ArtPipeline/Items/T66ProcessReimaginedItemSheets.py`
   - Remove the `Armor/HpRegen` and `Evasion/LifeSteal` item sheet series so future main-run art runs do not regenerate these deleted templates.

6. `Gameplay/Stats/Accuracy_Item_And_TempBuff_Audit.md`
   - Update historical status so it no longer says HpRegen and LifeSteal still exist as compatibility rows in the main-run item CSV.

7. `Content/Data/pending_issues_Data.md`
   - Add a `[Minor]` pending issue for shared `Content/Items/Sprites/Item_HpRegen_*.uasset` and `Content/Items/Sprites/Item_LifeSteal_*.uasset` cleanup that requires Mini-inclusive ownership.

## Out Of Scope

- No Mini/minigame data edits.
- No deletion of shared cooked/editor sprite assets while Mini-owned data still references them.
- No removal of deprecated `ET66SecondaryStatType::HpRegen` or `ET66SecondaryStatType::LifeSteal`, because those are runtime/stat compatibility mechanics rather than main-run item template rows.
- No redesign of item UI grouping beyond making `Special` display as a non-stat primary value.

## Verification Plan

1. CSV/static validation:
   - Parse `Content/Data/Items.csv` with Python.
   - Assert `Item_HpRegen` and `Item_LifeSteal` are absent.
   - Assert `Item_BackroomsQuickRevive.PrimaryStatType == Special`.

2. Reference validation:
   - Run a non-Mini grep for `Item_HpRegen|Item_LifeSteal` across `Source Content Gameplay Scripts Tools`, excluding the updated historical note and pending issue.
   - Run a tooling grep for `Armor_HpRegen`, `Evasion_LifeSteal`, and the removed series forms.
   - Confirm no production main-run references remain.

3. Build:
   - Run focused editor build:
     `C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex`

4. Data reload:
   - Run `Scripts/SetupItemsDataTable.py` through `UnrealEditor-Cmd.exe` against `C:\UE\T66\T66.uproject`.
   - Require successful `/Game/Data/DT_Items` save/log evidence.

5. Staged standalone:
   - Run `Scripts\StageStandaloneBuild.ps1`.
   - Verify taskbar shortcut `T66 Standalone.lnk` targets:
     `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`

6. Staged smoke launches:
   - Launch the staged exe to main menu and capture/log the boot.
   - Launch the staged exe with `-T66Entry=Run:TestRoom` and capture/log entering a gameplay run.
   - Treat crash/fatal/assert/launch failure as failed verification, not as a skipped check.

7. Save compatibility:
   - Re-run the `.sav` binary-string scan for the two deleted item IDs.
   - If no fixture exists, report that no legacy-save fixture was available and cite the source-level missing-row skip proof.

## Reviewer Request

Please review this pass-3 plan for:

- Missing production references to the deleted main-run item templates.
- Unsafe enum/data-table ordering.
- Any UI path that would show `Special` incorrectly or incorrectly include it in real stat surfaces.
- Any contradiction with the Mini exclusion.
- Whether the staged verification plan is strong enough.
- Whether the compatibility evidence is acceptable given no existing local legacy-save fixture contains these item IDs.

Return `Verdict: APPROVE` only if no Blocker or Major issues remain.
