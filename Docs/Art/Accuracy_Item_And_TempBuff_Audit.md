# Accuracy Item And Temp Buff Audit

**Last updated:** 2026-04-11  
**Purpose:** Concrete follow-up after promoting `Accuracy` to a primary stat and deprecating `Alchemy`, `HpRegen`, and `LifeSteal`.

**Status note:** The canonical global prompt/index packs were updated later on 2026-04-11. References below to stale prompt docs are historical audit findings, not the current state.

## 1. Current Gameplay Truth

Live primary-to-secondary map:

- `Damage`
  - `AoeDamage`
  - `BounceDamage`
  - `PierceDamage`
  - `DotDamage`
- `AttackSpeed`
  - `AoeSpeed`
  - `BounceSpeed`
  - `PierceSpeed`
  - `DotSpeed`
- `AttackScale`
  - `AoeScale`
  - `BounceScale`
  - `PierceScale`
  - `DotScale`
- `Accuracy`
  - `CritDamage`
  - `CritChance`
  - `AttackRange`
  - `Accuracy`
- `Armor`
  - `Taunt`
  - `DamageReduction`
  - `ReflectDamage`
  - `Crush`
- `Evasion`
  - `EvasionChance`
  - `CounterAttack`
  - `Invisibility`
  - `Assassinate`
- `Luck`
  - `TreasureChest`
  - `Cheating`
  - `Stealing`
  - `LootCrate`

Deprecated legacy secondaries:

- `HpRegen`
- `LifeSteal`
- `Alchemy`

## 2. Item Pass

### 2.1 Items to deprecate from the live art/content set

These item rows still exist for compatibility and old saves, but they should be treated as legacy and excluded from any future live item-art batch:

- `Item_HpRegen`
- `Item_LifeSteal`
- `Item_Alchemy`

### 2.2 Items that stay live but moved families

These are still live items, but their primary family is now `Accuracy`:

- `Item_CritDamage`
- `Item_CritChance`
- `Item_AttackRange`

### 2.3 Item that must be added to the live art set

- `Item_Accuracy`

Recommended item object concept:

- `Item_Accuracy` -> `Laser Sight`

Reason:

- localization already names the Accuracy-base item as `Laser Sight`
- `Item_AttackRange` can remain the range-focused object (`Sniper Scope`)
- `Item_Accuracy` needs its own distinct silhouette instead of reusing the range art

### 2.4 Item art assets required

Generate a full 4-rarity item sheet for:

- `Item_Accuracy_black`
- `Item_Accuracy_red`
- `Item_Accuracy_yellow`
- `Item_Accuracy_white`

### 2.5 Item prompt-pack debt discovered during audit

`Docs/Art/Item_Icon_Prompt_Pack.md` and `Docs/Art/Item_Icon_Prompt_Index.csv` are stale relative to the current runtime.

They still include old/non-live prompt targets such as:

- `Close Range Damage`
- `Long Range Damage`
- `Spin Wheel`
- `Movement Speed`

They also do not yet include:

- `Item_Accuracy`

## 3. Temporary Buff Pass

### 3.1 Temporary buffs to deprecate from the live icon set

These should be treated as retired from the live shop/selection art set:

- `HpRegen`
- `LifeSteal`
- `Alchemy`

Current runtime-dependency files that become legacy:

- `RuntimeDependencies/T66/UI/PowerUp/SecondaryBuffs/hp-regen.png`
- `RuntimeDependencies/T66/UI/PowerUp/SecondaryBuffs/life-steal.png`
- `RuntimeDependencies/T66/UI/PowerUp/SecondaryBuffs/alchemy.png`

### 3.2 Temporary buffs that must exist in the live Accuracy family

The Accuracy row now consists of:

- `CritDamage`
- `CritChance`
- `AttackRange`
- `Accuracy`

### 3.3 What needs to be added and regenerated

Already exposed in code but missing unique art:

- `Accuracy` temp-buff icon

Needs regeneration so the full Accuracy row shares a coherent visual shell:

- `crit-damage.png`
- `crit-chance.png`
- `range.png`
- `accuracy.png`

Reason:

- right now those icons come from three old parent rows plus one missing Accuracy asset
- after the stat migration, the Accuracy row should look like one family, not a recycled mix of Damage / Attack Speed / Attack Scale imagery

### 3.4 Recommended new Accuracy-row shell

Recommended shell family for temp buffs:

- `Eyedrop bottle`

Reason:

- fits the established medical-object language used by the other temp-buff rows
- clearly maps to the new `EYES` / accuracy / head-targeting identity
- gives the new Accuracy row a clean silhouette distinct from vial / IV bag / inhaler

Recommended inner-emblem concepts:

- `CritDamage` -> cracked reticle star
- `CritChance` -> reticle sparkle
- `AttackRange` -> long sight line / ruler with arrow
- `Accuracy` -> eye with targeting reticle

### 3.5 Current temp-buff code state

Gameplay already supports the new live set correctly:

- `Source/T66/Core/T66BuffSubsystem.cpp`
  - `GetAllSingleUseBuffTypes()` filters deprecated secondaries out of the live list
- `Source/T66/UI/Screens/T66TemporaryBuffShopScreen.cpp`
  - builds from the filtered live temp-buff list
- `Source/T66/UI/Screens/T66TemporaryBuffSelectionScreen.cpp`
  - also builds from the filtered live temp-buff list

The remaining art-side code follow-up after new Accuracy icon art exists:

- `Source/T66/UI/T66TemporaryBuffUIUtils.cpp`
  - change `ET66SecondaryStatType::Accuracy` slug from `range` to `accuracy`
- `Source/T66/UI/Screens/T66ShopScreen.cpp`
  - change the shop-side Accuracy slug from `range` to `accuracy`

Do not make those slug changes until the new file exists, or the UI will temporarily lose the Accuracy icon.

## 4. Existing Reused Art That Should Be Replaced

Current placeholder reuse:

- `Content/Data/Items.csv`
  - `Item_Accuracy` currently points to the `Item_AttackRange_*` sprites
- `Source/T66/Core/T66GameInstance.cpp`
  - synthetic `Item_Accuracy` fallback also points to the `Item_AttackRange_*` sprites
- `Source/T66/UI/T66TemporaryBuffUIUtils.cpp`
  - `Accuracy` currently resolves to `range.png`
- `Source/T66/UI/Screens/T66ShopScreen.cpp`
  - shop-side `Accuracy` also resolves to `range.png`

This is acceptable as a temporary placeholder but should not be treated as final content.

## 5. ChatGPT Art Workflow Reference

Canonical workflow doc:

- `C:\UE\T66\Docs\Art\ChatGPT_Image_Production_Workflow.md`

The repo-standard workflow is:

- use the ChatGPT web UI through the local bridge
- preview one image first
- keep one conversation for the batch
- let rate limits cool down instead of resending prompts
- skip assets already present on disk
- harvest existing chat outputs before regenerating

This workflow does **not** use the OpenAI image API.

### 5.1 Item generation path

Canonical scripts:

- `C:\UE\T66\Tools\ChatGPTBridge\batch_generate.py`
- `C:\UE\T66\Tools\ChatGPTBridge\server.py`

Recommended item preview command:

```powershell
cd C:\UE\T66\Tools\ChatGPTBridge
.\.venv\Scripts\python.exe .\batch_generate.py `
  --grid-sheet `
  --same-chat `
  --filter "Accuracy" `
  --limit 1 `
  --pause-seconds 8
```

Item import staging path:

- `C:\UE\T66\SourceAssets\ItemSprites`

Item import command:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" `
  "C:\UE\T66\T66.uproject" `
  -ExecutePythonScript="C:\UE\T66\Scripts\RunImportItemSpritesAndExit.py"
```

### 5.2 Temporary buff generation path

Relevant existing references:

- `C:\UE\T66\Docs\Art\Secondary_Buff_Icon_Index.csv`
- `C:\UE\T66\Docs\Art\Secondary_Buff_Icon_Prompt_Pack.md`

Current runtime source path for temp-buff icons:

- `C:\UE\T66\RuntimeDependencies\T66\UI\PowerUp\SecondaryBuffs`

Current package target path:

- `/Game/UI/PowerUp/SecondaryBuffs`

Unlike item sprites, there is no dedicated temp-buff import runner documented in the workflow file. Follow the existing `RuntimeDependencies/T66/UI/PowerUp/SecondaryBuffs/<slug>.png` pattern and then import or refresh the matching assets under `/Game/UI/PowerUp/SecondaryBuffs`.

## 6. Recommended Execution Order

1. Update the item prompt index/pack so `Item_Accuracy` is part of the live batch and deprecated rows are no longer treated as live.
2. Update the secondary-buff prompt index/pack so the Accuracy row is a real four-icon family.
3. Generate and approve one preview:
   - `Item_Accuracy`
   - one Accuracy-row temp-buff icon using the new shell
4. Run the batch for:
   - `Item_Accuracy`
   - `crit-damage`
   - `crit-chance`
   - `range`
   - `accuracy`
5. Import the new item sprites.
6. Stage/import the new temp-buff icons.
7. Only then switch the Accuracy temp-buff slug from `range` to `accuracy` and point `Item_Accuracy` at its own imported textures.
