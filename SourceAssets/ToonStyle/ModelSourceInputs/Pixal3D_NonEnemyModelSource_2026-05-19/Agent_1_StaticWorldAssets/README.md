# Agent 1 - Static World Assets

Agent 1 owns the static or mostly static world-facing Pixal3D source images in this folder.

Scope:
- Included: interactables, boost pickup coins, gates, and easy-difficulty visual props.
- Excluded: heroes, companions, enemies, failed iterations, contact sheets, and the skipped loot crate.
- PNG count: 24.

Production workflow:
- Use `C:\UE\T66\Model Generation\Instructions\09_PIXAL3D_TOONSTYLE_PRODUCTION_IMPORT_INSTRUCTIONS.md`.
- Build a production manifest from these PNGs and run the ToonStyle Pixal3D production path.
- Do not manually assign ToonStyle materials after import.
- Preserve existing gameplay/data behavior unless the task explicitly asks for behavior changes.

## Interactables

| Source image | Intended object | Runtime owner / data seam | Suggested model target |
| --- | --- | --- | --- |
| `Interactables/interactable_arcade_machine.png` | Shared arcade cabinet model for every arcade game | `Content/Data/ArcadeInteractables.json`, `DT_ArcadeInteractables`, arcade rows such as `Arcade_WhackAMole` | `/Game/World/Interactables/Arcade/Arcade_Machine/Arcade_Machine_Pixal3D` |
| `Interactables/interactable_vehicle.png` | Shared easy vehicle source, internally Vehicle not Tractor | `Content/Data/VehicleInteractables.json`, `DT_VehicleInteractables`, rows `Vehicle`, `Vehicle_Easy`, and later difficulty rows | `/Game/World/Interactables/Vehicles/Vehicle_Pixal3D` |
| `Interactables/interactable_chest.png` | Chest interactable shared visual | `AT66ChestInteractable`, `Source/T66/Gameplay/T66ChestInteractable.cpp`; rarity tuning stays in data | `/Game/World/Interactables/Chests/ChestModel/Chest_Pixal3D` |
| `Interactables/interactable_fountain.png` | Fountain interactable | `AT66FountainInteractable` | `/Game/World/Interactables/Fountain/Fountain_Pixal3D` |
| `Interactables/interactable_difficulty_totem.png` | Difficulty totem | `AT66DifficultyTotem` | `/Game/World/Interactables/DifficultyTotem/DifficultyTotem_Pixal3D` |
| `Interactables/interactable_quick_revive_vending_machine.png` | Quick Revive vending machine | `AT66QuickReviveVendingMachine` | `/Game/World/Interactables/Vending/QuickReviveVending_Pixal3D` |
| `Interactables/interactable_loot_wheel.png` | Loot Wheel interactable | `AT66LootWheelInteractable` | `/Game/World/Interactables/LootWheel/LootWheel_Pixal3D` |
| `Interactables/interactable_loot_bag_shared_rarity.png` | Shared loot bag model for all loot bag rarities | `AT66LootBagPickup`; update all rarity mesh soft pointers to the same model while preserving rarity logic | `/Game/World/LootBags/Shared/SM_LootBag_Shared_Pixal3D` |
| `Interactables/interactable_idol_altar.png` | Idol altar | Idol altar runtime/data references, existing verifier paths around `/Game/World/Interactables/SM_IdolAltar` | `/Game/World/Interactables/IdolAltar/IdolAltar_Pixal3D` |
| `Interactables/interactable_weapon_altar.png` | Weapon altar | `AT66WeaponAltar`, `Source/T66/Gameplay/T66WeaponAltar.cpp` | `/Game/World/Interactables/WeaponAltar/WeaponAltar_Pixal3D` |

Placement notes:
- World interactables are spawned by `Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp`.
- Keep arcade cabinet gameplay rows separate from the shared arcade machine model.
- Vehicle naming should stay `Vehicle` in data/runtime-facing paths. Do not introduce new user-facing Tractor naming.
- The weapon altar should have its own target mesh path instead of sharing the idol altar target.

## Boosts

These are source images for stat pickup or reward visuals. They match the stat reward types used by loot wheel boost rewards in `Source/T66/Gameplay/T66LootWheelInteractable.cpp`.

| Source image | Stat / intended object | Data or code seam |
| --- | --- | --- |
| `Boosts/boost_damage_strength.png` | Damage / Strength boost coin | `ET66HeroStatType::Damage` |
| `Boosts/boost_attack_speed.png` | Attack speed boost coin | `ET66HeroStatType::AttackSpeed` |
| `Boosts/boost_attack_scale.png` | Attack scale boost coin | `ET66HeroStatType::AttackScale` |
| `Boosts/boost_armor.png` | Armor boost coin | `ET66HeroStatType::Armor` |
| `Boosts/boost_evasion.png` | Evasion boost coin | `ET66HeroStatType::Evasion` |
| `Boosts/boost_luck.png` | Luck boost coin | `ET66HeroStatType::Luck` |
| `Boosts/boost_speed.png` | Speed boost coin | `ET66HeroStatType::Speed` |
| `Boosts/boost_accuracy.png` | Accuracy boost coin | `ET66HeroStatType::Accuracy` |

No dedicated boost pickup mesh data table was found during this source-packaging pass. If these become world pickup models, add or extend a runtime visual owner instead of burying mesh paths in code.

## Gates

| Source image | Intended object | Runtime owner / data seam | Suggested model target |
| --- | --- | --- | --- |
| `Gates/gate_stage.png` | Regular stage gate | `AT66StageGate`, `Source/T66/Gameplay/T66StageGate.cpp`; the tutorial end gate should use the regular stage gate visual | `/Game/World/Gates/StageGate_Pixal3D.StageGate_Pixal3D` |
| `Gates/gate_cowardice.png` | Cowardice gate clown-head gate | `AT66CowardiceGate`, `Source/T66/Gameplay/T66CowardiceGate.cpp` | `/Game/World/Gates/CowardiceGate_Pixal3D.CowardiceGate_Pixal3D` |

Placement notes:
- Gate placement comes from gameplay layout and game mode stage flow.
- Do not create a separate tutorial gate source model for this pass.

## Visual Props

These are non-interactable visual-only props for easy difficulty. They should use `AT66WorldVisualProp` and `Content/Data/WorldVisualProps.json` / `DT_WorldVisualProps`.

| Source image | Data row | Suggested model target |
| --- | --- | --- |
| `VisualProps/prop_wall_lamp_easy.png` | `WallLamp_Easy` | `/Game/World/VisualProps/Easy/WallLamp_Easy_Pixal3D.WallLamp_Easy_Pixal3D` |
| `VisualProps/prop_wall_torch_easy.png` | `WallTorch_Easy` | `/Game/World/VisualProps/Easy/WallTorch_Easy_Pixal3D.WallTorch_Easy_Pixal3D` |
| `VisualProps/prop_broken_vase_easy.png` | `BrokenVase_Easy` | `/Game/World/VisualProps/Easy/BrokenVase_Easy_Pixal3D.BrokenVase_Easy_Pixal3D` |
| `VisualProps/prop_skull_remains_easy.png` | `SkullRemains_Easy` | `/Game/World/VisualProps/Easy/SkullRemains_Easy_Pixal3D.SkullRemains_Easy_Pixal3D` |

Placement notes:
- Place via `DT_WorldVisualProps` row IDs and difficulty-specific prop placement rules.
- These are decorative props only. Do not give them gameplay interaction classes.
