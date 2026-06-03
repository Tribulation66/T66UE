# Claude Plan Review Packet - Pass 5

## Working Goal

Remove the deprecated HpRegen and LifeSteal main-run item templates, introduce a Special primary item category for Quick Revive and Gambler's Token, refresh item data assets, and verify the focused T66 item-system changes.

## Current State

No production edits have been made yet. Pass 5 addresses Pass 4's remaining Major issues:

- switch/case consumers of `ET66HeroStatType`;
- value rolling and stat application for `Special`;
- whether `Item_GamblersToken` exists in CSV as well as the synthetic path.

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
  - `Source/T66/UI/pending_issues_UI.md`

## Scope And Constraints

- Main-run item system only.
- No Mini/minigame edits.
- Shared cooked/editor item sprite assets will not be deleted in this pass.
- Data asset import/reload process applies because `Content/Data/Items.csv` backs `/Game/Data/DT_Items`.
- Staged standalone verification applies because these are playable item data/code changes.

## Evidence Since Pass 4

### Switch And Case Consumer Audit

Command:

```powershell
rg -n "case ET66HeroStatType::" Source\T66 -g "!*Mini*"
```

Relevant groups:

- `Source/T66/Core/RunState/T66RunStateSubsystem_Stats.cpp`
  - `GetItemPrimaryStatTenths`
  - `GetPermanentPrimaryBuffTenths`
  - `GetPrecisePrimaryStatTenths`
  - all have `default` fallback behavior and no `ensureMsgf` / `checkNoEntry`.
- `Source/T66/Core/RunState/T66RunStateSubsystem_Private.h`
  - `T66_GetSecondaryTypesForPrimary` has a static empty-array fallback, so `Special` maps to no secondaries.
  - `T66_GetHeroMainAttackSecondaryType` falls back to `None` for non-weapon primary families.
- `Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp`
  - `AddPrimaryBonusTenths` has `default: break`.
  - Planned edit adds an earlier explicit skip for `D.PrimaryStatType == ET66HeroStatType::Special`.
- `Source/T66/Core/T66BuffSubsystem.cpp`
  - power-up/permanent buff stat switches are only reached from explicit real-stat arrays in Account Status/Power Up and return safe defaults/null when outside the eight real stats.
- `Source/T66/UI/T66ItemCardTextUtils.cpp`
  - planned edit adds `Special` label and non-numeric category display.
- `Source/T66/UI/T66TopwarArcadeWidget.cpp`, `Source/T66/UI/T66LootWheelOverlayWidget.cpp`, `Source/T66/Gameplay/T66BoostInteractable.cpp`, `Source/T66/UI/Screens/T66PowerUpScreen.cpp`
  - these operate from explicit real-stat pools and should not include `Special`; their defaults remain real-stat fallback/empty UI for invalid stat values.

Command:

```powershell
rg -n "switch \((PrimaryStatType|StatType|Type)\)" Source\T66\Core Source\T66\Gameplay Source\T66\UI -g "!*Mini*"
```

This found the switch groups above plus unrelated enum switches. No `ET66HeroStatType` switch has an exhaustive failure path such as `checkNoEntry()` or `ensureMsgf(false, ...)`.

Additional command:

```powershell
rg -n "checkNoEntry|ensureMsgf|ensureAlways|ET66HeroStatType|PrimaryStatType|HeroStatType" Source\T66 -g "!*Mini*"
```

Findings:

- Existing `ensureMsgf` hits are unrelated to `ET66HeroStatType`.
- No hero-stat or primary-stat switch asserts on unknown enum values.

### Runtime Stat Application For Special

Current stat recompute path:

- `Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp:1024-1028`
  - `GI->GetItemData(Slot.ItemTemplateID, D)` must succeed or the slot is skipped.
  - `T66_IsRewardOnlySpecialItem(Slot.ItemTemplateID)` already skips Quick Revive and Gambler Token item IDs.
  - `D.SecondaryStatType == ET66SecondaryStatType::GamblerToken` also skips token effects from normal stat recompute.
- `Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp:1031-1038`
  - normal item line-1 and line-2 stat recompute happens only after those skips.

Planned additional production guard:

```cpp
if (D.PrimaryStatType == ET66HeroStatType::Special)
{
    continue;
}
```

This will be inserted before primary/secondary stat recompute. It makes a non-zero rolled `Line1RolledValue` harmless for any future `Special` row, including automation-forced Quick Revive pickup-card proof.

Value rolling itself does not need to be globally short-circuited because `AddItemWithRarity()` is generic inventory-slot construction and existing token/Quick Revive reward paths already have item-ID special handling. The important runtime invariant is: a rolled value on a `Special` primary must not apply to hero stats or derived secondary bonuses. The explicit recompute guard enforces that.

### Gambler's Token Data Source

Commands:

```powershell
Select-String -LiteralPath Content\Data\Items.csv -Pattern 'Item_GamblersToken|GamblersToken|Item_BackroomsQuickRevive|Item_HpRegen|Item_LifeSteal'
rg -n "Item_GamblersToken|GamblersToken" Content\Data Source\T66\Core Source\T66\Gameplay Source\T66\UI -g "!*Mini*"
```

Findings:

- `Content/Data/Items.csv` has no `Item_GamblersToken` row.
- `Item_GamblersToken` is intentionally synthetic in `Source/T66/Core/T66GameInstance.cpp`.
- `Content/Data/pending_issues_Data.md` already notes `Item_GamblersToken` as missing from `/Game/Data/DT_Items`, which matches the synthetic-row design.

Planned edit remains `T66GameInstance.cpp` synthetic `OutItemData.PrimaryStatType = ET66HeroStatType::Special`.

### UAsset ID String Scan

Command:

```powershell
rg -a -n "Item_HpRegen|Item_LifeSteal" Content Source Gameplay Scripts Tools -g "!*Mini*"
```

Current expected pre-edit matches:

- `Content/Data/Items.csv` rows to delete.
- `Content/Data/DT_Items.uasset`, which will be refreshed by `SetupItemsDataTable.py`.
- `Gameplay/Stats/Accuracy_Item_And_TempBuff_Audit.md`, which will be updated.
- `Content/Items/Sprites/Item_HpRegen_*.uasset` and `Content/Items/Sprites/Item_LifeSteal_*.uasset`, which self-reference their own asset names/import source and are deferred to the Mini-inclusive cleanup pending issue.

No other non-Mini `.uasset` was found hard-coding the two item IDs.

### Save Compatibility Source Points

- `Source/T66/Core/RunState/T66RunStateSubsystem_Snapshot.cpp:153` imports `InventorySlots = Snapshot.InventorySlots`.
- `Source/T66/Core/RunState/T66RunStateSubsystem_Snapshot.cpp:201-204` reinitializes stat tuning and syncs legacy hero stats.
- `Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp:1023-1028` skips any inventory slot whose item template row no longer resolves through `GI->GetItemData`.

Local `.sav` scan still has no existing fixture with `Item_HpRegen` or `Item_LifeSteal`; final verification will re-run it after staging.

## Planned Production Edits

1. `Source/T66/Data/T66DataTypes.h`
   - Append `Special UMETA(DisplayName = "Special")` after `Accuracy`.
   - Update the `FItemData::PrimaryStatType` comment to state that `Special` is for reward-only/non-stat templates.

2. `Content/Data/Items.csv`
   - Delete the `Item_HpRegen` row.
   - Delete the `Item_LifeSteal` row.
   - Change `Item_BackroomsQuickRevive` `PrimaryStatType` from `Armor` to `Special`.
   - Leave `Item_Alchemy` untouched because the user named only HP regen and lifesteal for deletion.
   - No `Item_GamblersToken` CSV edit is needed because no CSV row exists.

3. `Source/T66/Core/T66GameInstance.cpp`
   - Change synthetic `Item_GamblersToken` primary stat from `Luck` to `Special`.
   - Leave the existing random item pool exclusions intact.

4. `Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp`
   - Add an explicit `D.PrimaryStatType == ET66HeroStatType::Special` skip before item line-1/line-2 stat recompute.

5. `Source/T66/UI/T66ItemCardTextUtils.cpp`
   - Add `Special` to primary-label switches with `NSLOCTEXT("T66.Stats", "Special", "Special")`.
   - Render non-token `Special` items as the category text `Special`, not as `Special: +N`.

6. `Source/T66/UI/T66CollectorOverlayWidget.cpp`
   - Render `Special` item descriptions as category text instead of `Line 1: +Special`.

7. `Tools/ArtPipeline/Items/T66ProcessReimaginedItemSheets.py`
   - Remove the `Armor/HpRegen` and `Evasion/LifeSteal` item sheet series so the main item art pipeline still matches `Content/Data/Items.csv`.

8. `Gameplay/Stats/Accuracy_Item_And_TempBuff_Audit.md`
   - Update historical status so it no longer says HpRegen and LifeSteal still exist as compatibility rows in the main-run item CSV.

9. `Content/Data/pending_issues_Data.md`
   - Add a `[Minor]` pending issue for shared `Content/Items/Sprites/Item_HpRegen_*.uasset` and `Content/Items/Sprites/Item_LifeSteal_*.uasset`.
   - The issue will explicitly say cleanup is deferred because a Mini-inclusive ownership/reference pass has not been done.

## Out Of Scope

- No Mini/minigame data edits.
- No deletion of shared cooked/editor sprite assets while Mini-owned data may still reference them.
- No removal of deprecated `ET66SecondaryStatType::HpRegen` or `ET66SecondaryStatType::LifeSteal`, because those are runtime/stat compatibility mechanics rather than main-run item template rows.
- No addition of `Special` to real-stat HUD, power-up, account status, arcade, loot wheel, boost, or temporary-buff stat pools.

## Verification Plan

1. CSV/static validation:
   - Parse `Content/Data/Items.csv` with Python.
   - Assert `Item_HpRegen` and `Item_LifeSteal` are absent.
   - Assert `Item_BackroomsQuickRevive.PrimaryStatType == Special`.
   - Assert there is no `Item_GamblersToken` CSV row.
   - Assert `Special` exists as an `ET66HeroStatType` enumerator token in `Source/T66/Data/T66DataTypes.h`, not only as display text.

2. Reference validation:
   - Run a non-Mini grep for `Item_HpRegen|Item_LifeSteal` across `Source Content Gameplay Scripts Tools`, excluding the updated historical note, pending issue, and the deferred sprite assets.
   - Re-run enum iteration and switch/case greps.
   - Re-run `Tools/ArtPipeline` and `Scripts` grep for `HpRegen|LifeSteal|Armor_HpRegen|Evasion_LifeSteal`.
   - Re-run binary `.uasset` ID-string scan and confirm only deferred sprite assets remain outside data/docs after `DT_Items` reload.

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
   - Launch staged gameplay with existing automation flags:
     `-T66Entry=Run:TestRoom -T66HudReviewMode=default -T66GameplayAutoPickupCard=Item_BackroomsQuickRevive -T66GameplayAutoPickupCardRarity=black`
   - Confirm Quick Revive pickup-card presentation does not crash and treats `Special` as category text, not as a normal stat roll.
   - Check staged logs for `Fatal`, `Error`, `Ensure failed`, `ensureMsgf`, `PrimaryStatType`, and `HeroStatType`.

7. Save compatibility:
   - Re-run the `.sav` binary-string scan for the two deleted item IDs.
   - If no fixture exists, report that no legacy-save fixture was available and cite the source-level missing-row skip proof above.

## Reviewer Request

Please review this pass-5 plan for:

- Whether switch/case consumers are sufficiently audited.
- Whether the explicit stat recompute skip handles rolled values on `Special` safely.
- Whether the Gambler Token synthetic-only source is sufficiently addressed.
- Whether the verification plan is now sufficient.

Return `Verdict: APPROVE` only if no Blocker or Major issues remain.
