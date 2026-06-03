# Review Packet: Item Special Primary Cleanup

## Working Goal

Remove the deprecated HpRegen and LifeSteal main-run item templates, introduce a Special primary item category for Quick Revive and Gambler's Token, refresh item data assets, and verify the focused T66 item-system changes.

## User Request

"Ok so first we can already do some infrastructure changes, we should remove HP regen and lifesteal items alltogehter not just depracate but delete them then, both BcakroomQuickRevive and gambler token should have a primary called Special. So that it doesnt fall into the other 7 like armor."

## Scope And Assumptions

- Main-run item system only. Mini/minigame systems remain excluded by root AGENTS default scope.
- "Gambler token" means the live `Item_GamblersToken` / Gambler's Token item.
- "Delete HpRegen and LifeSteal items" means remove the main-run item templates and future main item-art generation entries. The shared sprite assets will not be deleted in this pass because Mini data still references `/Game/Items/Sprites/Item_HpRegen_*` and `/Game/Items/Sprites/Item_LifeSteal_*`, and Mini is explicitly out of default scope.
- No PPF required: this is data/source infrastructure cleanup, not a visual/media/import authoring task.

## Instructions Read

- Root AGENTS instructions in the chat.
- `Gameplay/GAMEPLAY_AGENTS.md`
- `Gameplay/README.md`
- `Tools/README.md`
- `Reports/AGENTS.md`
- Pending files:
  - `Content/Data/pending_issues_Data.md`
  - `Source/T66/Data/pending_issues_Data.md`
  - `Source/T66/Core/pending_issues_Core.md`
  - `Source/T66/Core/RunState/pending_issues_RunState.md`

## Current Evidence

- `Content/Data/Items.csv` contains:
  - `Item_HpRegen,...,Armor,HpRegen,50,25`
  - `Item_LifeSteal,...,Evasion,LifeSteal,55,27`
  - `Item_BackroomsQuickRevive,...,Armor,None,0,0`
- `Source/T66/Data/T66DataTypes.h` has `ET66HeroStatType` with the seven main item-facing stats plus `Speed` and appended `Accuracy`, but no `Special`.
- `Source/T66/Core/T66GameInstance.cpp` synthesizes `Item_GamblersToken` with `PrimaryStatType = ET66HeroStatType::Luck` and `SecondaryStatType = ET66SecondaryStatType::GamblerToken`.
- `Source/T66/Core/T66GameInstance.cpp` excludes `Item_GamblersToken` and `Item_BackroomsQuickRevive` from random item pools by ID in `IsRandomItemPoolEligible`.
- `Source/T66/Core/RunState/T66RunStateSubsystem_Private.h` treats both `Item_GamblersToken` and `Item_BackroomsQuickRevive` as reward-only special items.
- `Tools/ArtPipeline/Items/T66ProcessReimaginedItemSheets.py` still contains `HpRegen` and `LifeSteal` item-art series.
- Shared sprite assets exist in `Content/Items/Sprites/Item_HpRegen_*.uasset` and `Content/Items/Sprites/Item_LifeSteal_*.uasset`.
- `Content/Mini/Data/T66Mini_Items.csv` and Mini runtime code reference HpRegen/LifeSteal and `Item_Alchemy` references the HpRegen sprite path; deleting the shared sprites would affect Mini, which is excluded.
- `Content/Data/pending_issues_Data.md` already notes staged smoke logs missing rows such as `Item_GamblersToken` from `/Game/Data/DT_Items`, but this pass will not add a row for `Item_GamblersToken` because no source or imported Gambler token item sprites currently exist; instead it keeps the existing synthetic item-data path and changes its primary stat to `Special`.

## Proposed Plan

1. Edit `Source/T66/Data/T66DataTypes.h`.
   - Append `Special` to `ET66HeroStatType` after existing values to preserve existing serialized enum ordinals.
   - Keep `Special` out of normal stat progression/buff switch cases; existing defaults should ignore it.

2. Edit `Content/Data/Items.csv`.
   - Remove the `Item_HpRegen` row.
   - Remove the `Item_LifeSteal` row.
   - Change `Item_BackroomsQuickRevive` `PrimaryStatType` from `Armor` to `Special`.

3. Edit `Source/T66/Core/T66GameInstance.cpp`.
   - Change synthetic `Item_GamblersToken` primary from `Luck` to `Special`.
   - Keep its secondary type `GamblerToken`, sell behavior, and random-pool exclusion unchanged.

4. Edit `Tools/ArtPipeline/Items/T66ProcessReimaginedItemSheets.py`.
   - Remove the `HpRegen` and `LifeSteal` `Series(...)` entries so future main item-art generation does not regenerate them.

5. Add a pending issue for shared sprite cleanup.
   - Record that the old shared HpRegen/LifeSteal sprite assets remain only because Mini still references them, and deleting them requires an explicit Mini-inclusive cleanup pass.
   - Proposed location: `Content/Items/Sprites/pending_issues_Sprites.md`.

6. Refresh Unreal data asset.
   - Run `UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="C:/UE/T66/Scripts/SetupItemsDataTable.py" -unattended -nop4 -nosplash`.
   - This should reload `/Game/Data/DT_Items` from `Content/Data/Items.csv`.

7. Verify.
   - Focused C++ build after source changes: UE 5.7 `Build.bat T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex`.
   - CSV count/filter check: confirm `Items.csv` has no `Item_HpRegen`/`Item_LifeSteal`, `Item_BackroomsQuickRevive` has `Special`, and normal live modifier rows are still 28.
   - Confirm source has `Item_GamblersToken` primary `Special`.
   - Because this affects playable standalone data/code, run `Scripts\StageStandaloneBuild.ps1` and verify the shortcut target remains `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.

## Out Of Scope

- Mini/minigame item tables and runtime behavior.
- Removing `ET66SecondaryStatType::HpRegen` and `ET66SecondaryStatType::LifeSteal`, because those stat values still exist in hero/base stats, temporary buffs, backend summary parsing, and Mini code.
- Deleting shared HpRegen/LifeSteal sprite assets while Mini references remain.
- Creating new Gambler's Token icons or moving it from synthetic source data into `Items.csv`; that would require an icon/source asset pass.

## Risks And Mitigations

- Risk: appending `Special` to `ET66HeroStatType` changes enum display but not existing ordinals. Mitigation: append only, do not reorder.
- Risk: if any code assumes every `PrimaryStatType` maps to progression stats, `Special` should be ignored by default switch behavior. Mitigation: inspect core switch consumers and build.
- Risk: deleting shared sprite assets would break Mini references. Mitigation: do not delete them in this non-Mini pass; document pending cleanup.
- Risk: `DT_Items` will reject `Special` if the editor uses stale compiled code. Mitigation: build first, then run the DataTable reload.

## Reviewer Ask

Review the plan strictly for scope, missing affected files, bad assumptions around adding `Special`, unsafe deletion, and whether verification is sufficient under the provided T66 AGENTS instructions.
