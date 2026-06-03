# Claude Plan Review Packet - Pass 4

## Working Goal

Remove the deprecated HpRegen and LifeSteal main-run item templates, introduce a Special primary item category for Quick Revive and Gambler's Token, refresh item data assets, and verify the focused T66 item-system changes.

## Current State

No production edits have been made yet. Pass 4 addresses Pass 3's remaining Major issues:

- enum-iteration consumers were not fully surveyed;
- the item art-pipeline edit needed a clearer ownership statement across the Mini exclusion boundary;
- `Special` needed a saner item-card/collector treatment than a numeric stat line.

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
- Default Mini/minigame exclusion remains active.
- No Mini-owned paths will be edited.
- Shared cooked/editor item sprite assets will not be deleted in this pass.
- Data asset import/reload process applies because `Content/Data/Items.csv` backs `/Game/Data/DT_Items`.
- Staged standalone verification applies because these are playable item data/code changes.
- PPF is skipped because this is not visual/media/import-art replication work; QA cares that data/category/runtime behavior changes correctly.

## New Evidence Since Pass 3

### Enum Iteration And Pool Audit

Command:

```powershell
rg -n "StaticEnum<ET66HeroStatType>|GetEnumeratorIndex|GetMaxEnumValue|NumEnums|GetIndexByName|TEnumRange|EnumRange|TArray<ET66HeroStatType>|ET66HeroStatType>" Source\T66 -g "!*Mini*"
```

Relevant result:

```text
Source\T66\UI\T66CollectorOverlayWidget.cpp:334: const FText PrimaryStat = StaticEnum<ET66HeroStatType>()->GetDisplayNameTextByValue(...)
Source\T66\UI\Screens\T66ChallengesScreen.cpp:687: for (int32 EnumIndex = 0; EnumIndex < Enum->NumEnums() - 1; ++EnumIndex)
Source\T66\UI\Screens\T66ChallengesScreen.cpp:708: for (int32 EnumIndex = 0; EnumIndex < Enum->NumEnums() - 1; ++EnumIndex)
Source\T66\UI\Screens\T66AccountStatusScreen.cpp:1313: const TArray<ET66HeroStatType> PowerStats = {
Source\T66\UI\Screens\T66PowerUpScreen.cpp:1492: const TArray<ET66HeroStatType> PermanentCardOrder = {
Source\T66\UI\T66TopwarArcadeWidget.cpp:31: static const TArray<ET66HeroStatType>& T66GetTopwarStatPool()
Source\T66\Gameplay\T66ArcadeInteractableBase.cpp:74: static const TArray<ET66HeroStatType>& T66GetBoostStatPool()
Source\T66\Gameplay\T66LootWheelInteractable.cpp:53: static const TArray<ET66HeroStatType>& T66GetLootWheelBoostStatPool()
```

`T66ChallengesScreen.cpp` lines 680-708 use `StaticEnum<ET66PassiveType>()` and `StaticEnum<ET66UltimateType>()`, not `ET66HeroStatType`; the grep hit comes from the `NumEnums()` token only.

Additional command:

```powershell
rg -n "ET66HeroStatType.*MAX|MAX.*ET66HeroStatType|HeroStatType.*Num|Num.*HeroStatType|PrimaryStatType.*Num|Num.*PrimaryStatType|HeroStatTypes|PrimaryStatTypes" Source\T66 -g "!*Mini*"
```

Result: no matches.

Interpretation:

- There is no enum-driven loop that will automatically include appended `Special` in a real stat pool.
- Real stat pools are explicit arrays in Account Status, Power Up, Topwar Arcade, Arcade Interactable, and Loot Wheel. Those arrays intentionally remain the eight real stat values and will not be edited to include `Special`.
- `T66CollectorOverlayWidget` is an item information card context, not a general stat grid. It should show `Special` as a non-stat category label, not as `Line 1: +Special`; planned UI edit below handles that.

### Item Card And Collector Treatment

Current `Source/T66/UI/T66ItemCardTextUtils.cpp` behavior:

- Primary stat lines are numeric: `{Label}: +{MainValue}`.
- `GamblerToken` already bypasses normal primary stat rendering and shows its token-effect line.
- `BuildPrimaryStatLine()` returns empty when `MainValue <= 0`, but automation can force-add Quick Revive through `T66GameplayAutoPickupCard`, which gives it a rolled line-1 value.

Planned UI edit:

- Add `Special` label support in `GetPrimaryStatLabel()`.
- In `BuildItemCardDescription()`, if `PrimaryStatType == Special` and the item is not `GamblerToken`, return the `Special` label as a category description instead of a numeric `+value` stat line.
- Keep `GamblerToken`'s existing effect text.
- In `T66CollectorOverlayWidget`, render `Special` as a primary/category label rather than `Line 1: +Special`.

This makes Quick Revive pickup-card proof meaningful: it should display a sane `Special` category line and not a stat roll.

### Art Pipeline Ownership

`Tools/README.md` routes tools as:

```text
- ArtPipeline/Items/: item sprite and item-art processing tools.
- ArtPipeline/Minigames/: shared minigame sprite-sheet tools.
```

`Tools/ArtPipeline/Items/T66ProcessReimaginedItemSheets.py` is hardwired to main item data/art paths:

```python
LIVE_SPRITES_DIR = ROOT / "SourceAssets" / "ItemSprites"
ITEMS_CSV = ROOT / "Content" / "Data" / "Items.csv"
```

It promotes processed sprites by looking up item IDs from `Content/Data/Items.csv` secondary stat rows:

```python
item_ids = live_item_ids_by_secondary_stat()
for series in SERIES:
    item_id = item_ids.get(series.series)
    if not item_id:
        missing.append(f"{series.series}: no live item row")
        continue
```

Therefore, after deleting the `Item_HpRegen` and `Item_LifeSteal` CSV rows, keeping the two series in this main item script would make future `--promote-live` runs fail with `no live item row`. The script edit remains in scope because this script is the main item CSV/source-sprite pipeline, while Mini-owned source processing is routed under `Tools/ArtPipeline/Minigames/`.

I am not deleting the shared `Content/Items/Sprites/Item_HpRegen_*.uasset` or `Item_LifeSteal_*.uasset` assets in this pass. That asset cleanup still needs a Mini-inclusive ownership pass and will be documented as a pending issue.

### Documentation Sweep

Command:

```powershell
rg -n "HpRegen|LifeSteal|Item_HpRegen|Item_LifeSteal" Gameplay -g "!*Mini*"
```

Findings:

- `Gameplay/Stats/Accuracy_Item_And_TempBuff_Audit.md` is the only doc that names `Item_HpRegen` / `Item_LifeSteal` item rows and needs updating.
- `Gameplay/Combat/MASTER_COMBAT.md` and `Gameplay/Stats/MASTER_STATS.md` describe deprecated secondary stat mechanics, not active main-run item rows, and should stay unchanged.

### Pending Issue Routing

There is no `pending_issues_*.md` in `Content/Items/` or `Content/Items/Sprites/`. Existing content/data pending issues are already tracked in `Content/Data/pending_issues_Data.md`. Because the deferred asset cleanup is caused by removing rows from `Content/Data/Items.csv` while shared sprite assets remain referenced outside scope, `Content/Data/pending_issues_Data.md` is the closest existing content-side pending issue file.

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
   - Add `Special` to primary-label switches.
   - Render non-token `Special` items as the category text `Special`, not as `Special: +N`.

5. `Source/T66/UI/T66CollectorOverlayWidget.cpp`
   - Render `Special` item descriptions as category text instead of `Line 1: +Special`.

6. `Tools/ArtPipeline/Items/T66ProcessReimaginedItemSheets.py`
   - Remove the `Armor/HpRegen` and `Evasion/LifeSteal` item sheet series so the main item art pipeline still matches `Content/Data/Items.csv`.

7. `Gameplay/Stats/Accuracy_Item_And_TempBuff_Audit.md`
   - Update historical status so it no longer says HpRegen and LifeSteal still exist as compatibility rows in the main-run item CSV.

8. `Content/Data/pending_issues_Data.md`
   - Add a `[Minor]` pending issue for shared `Content/Items/Sprites/Item_HpRegen_*.uasset` and `Content/Items/Sprites/Item_LifeSteal_*.uasset` cleanup that requires a Mini-inclusive ownership pass.

## Out Of Scope

- No Mini/minigame data edits.
- No deletion of shared cooked/editor sprite assets while Mini-owned data may still reference them.
- No removal of deprecated `ET66SecondaryStatType::HpRegen` or `ET66SecondaryStatType::LifeSteal`, because those are runtime/stat compatibility mechanics rather than main-run item template rows.
- No addition of `Special` to real-stat HUD, power-up, account status, arcade, loot wheel, or boost stat pools.

## Verification Plan

1. CSV/static validation:
   - Parse `Content/Data/Items.csv` with Python.
   - Assert `Item_HpRegen` and `Item_LifeSteal` are absent.
   - Assert `Item_BackroomsQuickRevive.PrimaryStatType == Special`.

2. Reference validation:
   - Run a non-Mini grep for `Item_HpRegen|Item_LifeSteal` across `Source Content Gameplay Scripts Tools`, excluding the updated historical note and pending issue.
   - Re-run enum-iteration grep listed above and confirm no enum loop will inject `Special` into a real-stat pool.
   - Re-run `Tools/ArtPipeline` and `Scripts` grep for `HpRegen|LifeSteal|Armor_HpRegen|Evasion_LifeSteal` and confirm no main item tooling path can regenerate the removed series.

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
   - Use the resulting log/screenshot path to confirm Quick Revive pickup-card presentation does not crash and treats `Special` as category text, not as a normal stat roll.
   - Treat crash/fatal/assert/launch failure as failed verification, not as a skipped check.

7. Save compatibility:
   - Re-run the `.sav` binary-string scan for the two deleted item IDs.
   - If no fixture exists, report that no legacy-save fixture was available and cite the source-level missing-row skip proof.

## Reviewer Request

Please review this pass-4 plan for:

- Whether the enum-iteration audit closes the leak risk.
- Whether the item art-pipeline edit is justified without editing Mini-owned paths.
- Whether the updated item-card/collector plan treats `Special` as a non-stat category cleanly.
- Whether the verification plan is now sufficient.

Return `Verdict: APPROVE` only if no Blocker or Major issues remain.
