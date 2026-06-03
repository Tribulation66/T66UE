# Review Packet: Item Breakdown Rename List

## Working Goal

Produce a current main-run item breakdown list reflecting the requested item-category/name changes: Accuracy secondary Accuracy becomes Execute, EvasionChance becomes Dodge, Execute/Assassinate/Crush move to the end of their groups, Luck uses LootCrate/LootChest/LootBag/LootWheel while Stealing and Cheating are deprecated, without editing files yet.

## User Constraints

- Give the list again after the requested changes.
- Present the same major grouping order already requested by the user:
  - Weapon Modifiers: Damage, Attack Speed, Attack Scale.
  - Character Modifiers: Accuracy, Evasion, Armor.
  - Luck Modifier.
  - Special Items.
- Do not present HP Regen or Life Steal.
- Include Quick Revive and Vendor Token / Gambler's Token under Special Items.
- Default repo rule excludes Mini/minigame scope, so this answer is main-run item taxonomy only.

## Live Evidence Checked

- `Content/Data/Items.csv` currently has 29 rows:
  - Damage: `AoeDamage`, `BounceDamage`, `PierceDamage`, `DotDamage`.
  - AttackSpeed: `AoeSpeed`, `BounceSpeed`, `PierceSpeed`, `DotSpeed`.
  - AttackScale: `AoeScale`, `BounceScale`, `PierceScale`, `DotScale`.
  - Accuracy: `CritDamage`, `CritChance`, `AttackRange`, `Accuracy`.
  - Evasion: `Invisibility`, `CounterAttack`, `Assassinate`, `EvasionChance`.
  - Armor: `Taunt`, `ReflectDamage`, `Crush`, `DamageReduction`.
  - Luck: `LootCrate`, `TreasureChest`, `Cheating`, `Stealing`.
  - Special: `Item_BackroomsQuickRevive`.
- `Source/T66/Data/T66DataTypes.h` currently has `EvasionChance UMETA(DisplayName = "Dodge")`, so Dodge is a display rename over the same effect today.
- `Source/T66/Core/T66GameInstance.cpp` has synthetic `Item_GamblersToken` with primary `Special`.
- `Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp` uses `Item_BackroomsQuickRevive` as the quick revive special item and `Item_GamblersToken` as the special token pickup.
- `AGENTS.md:9` says default scope excludes Mini/minigame systems unless the user explicitly names them.
- User prompt defines Execute as "chance of a critical hit OHKO the enemy"; no source implementation has been changed yet.
- Narrow `rg` over `Source/T66`, `Content/Data/Items.csv`, and `Gameplay/Combat/MASTER_COMBAT.md` shows `Cheating` and `Stealing` are still wired in current code/data, so the answer must frame them as omitted from the target active list / to be deprecated, not already removed.

## Proposed Answer Scope

Return the intended post-change active list only. Do not claim source files were changed.

## Proposed Final List

### Weapon Modifiers

Damage:
- AOE Damage
- Bounce Damage
- Pierce Damage
- DOT Damage

Attack Speed:
- AOE Speed
- Bounce Speed
- Pierce Speed
- DOT Speed

Attack Scale:
- AOE Scale
- Bounce Scale
- Pierce Scale
- DOT Scale

### Character Modifiers

Accuracy:
- Crit Damage
- Crit Chance
- Attack Range
- Execute

Evasion:
- Invisibility
- Counter Attack
- Dodge
- Assassinate

Armor:
- Taunt
- Reflect Damage
- Damage Reduction
- Crush

### Luck Modifier

Luck:
- Loot Crate
- Loot Chest
- Loot Bag (new requested item stat; not yet a current CSV secondary)
- Loot Wheel (new requested item stat; not yet a current CSV secondary)

### Special Items

- Quick Revive (`Item_BackroomsQuickRevive`)
- Vendor Token / Gambler's Token (`Item_GamblersToken`, one Special item)

To deprecate / not in the target active list:
- Stealing
- Cheating

Notes:
- Execute replaces the old Accuracy secondary item stat and, per the user's prompt, means chance for a critical hit to OHKO the enemy.
- Dodge is the display name for the old Evasion Chance effect.
- Loot Chest keeps the current Treasure Chest effect.

## Risks / Caveats

- The user spelled `Assasinate`, but the repo spelling is `Assassinate`; use the repo spelling in the answer.
- `LootBag` and `LootWheel` are live gameplay objects/effects, but not currently item CSV secondaries. The list should present the requested intended item taxonomy and mark those two as not current CSV secondaries yet.
- `LootChest` maps to the current `TreasureChest` effect unless implementation later renames the secondary enum/data row.
- For Evasion and Armor, preserve the current CSV relative order and only move the user-named finisher items to the end: `Assassinate` for Evasion and `Crush` for Armor.

## Verification Needed

- Answer should be a list only, with a brief note that files were not edited yet.
- Do not include Mini/minigame items or code.
- Do not include HP Regen, Life Steal, Stealing, or Cheating in the active list.
