# Review Packet: Requested Item Breakdown Order

## Working Goal

Re-present the live non-minigame T66 item breakdown in the requested header order, excluding HpRegen and LifeSteal and treating Quick Revive plus the vendor token as special items.

## User Request

"Ok so do a few things for me, present the breakdown in the following order, Damage, Attack Speed and Attack Scale under a header Weapon Modifiers. And then Accuracy, Evasion and Armor as Character Modifiers and then Luck as Luck Modifier. Also dont present hp regen and life steal, but also there should be another that is the vendor token or something, and present Item Quick Revive and the Vendor Token as special items header."

## Scope And Assumptions

- Main-run `Content/Data/Items.csv` / `DT_Items` item system only.
- Default Mini/minigame exclusion still applies because the user did not name Mini/minigames.
- "Vendor Token" is interpreted from live source as `Item_GamblersToken` / `Gambler's Token`.
- No production edits, build, staging, or runtime capture are needed.

## Evidence

- `Content/Data/Items.csv` lists normal modifier rows by primary/secondary stat:
  - Damage: `Item_AoeDamage`, `Item_BounceDamage`, `Item_PierceDamage`, `Item_DotDamage`
  - AttackSpeed: `Item_AoeSpeed`, `Item_BounceSpeed`, `Item_PierceSpeed`, `Item_DotSpeed`
  - AttackScale: `Item_AoeScale`, `Item_BounceScale`, `Item_PierceScale`, `Item_DotScale`
  - Accuracy: `Item_CritDamage`, `Item_CritChance`, `Item_AttackRange`, `Item_Accuracy`
  - Evasion: `Item_Invisibility`, `Item_CounterAttack`, `Item_Assassinate`, `Item_EvasionChance`
  - Armor: `Item_Taunt`, `Item_ReflectDmg`, `Item_Crush`, `Item_DamageReduction`
  - Luck: `Item_LootCrate`, `Item_TreasureChest`, `Item_Cheating`, `Item_Stealing`
- `Item_HpRegen` and `Item_LifeSteal` are present in the CSV but are intentionally omitted per user request and because they use deprecated secondary stats.
- `Item_BackroomsQuickRevive` is present in the CSV with `Armor,None,0,0`.
- `Item_GamblersToken` is not a normal CSV row. It is a source-defined special item:
  - `Source/T66/Data/T66DataTypes.h` has `GamblerToken UMETA(DisplayName = "Gambler's Token")`.
  - `Source/T66/Core/T66GameInstance.cpp` synthesizes special item data for `Item_GamblersToken`.
  - `Source/T66/Core/T66RunStateSubsystem.h` documents it as an active Gambler's Token level for the current run.
  - `Source/T66/Gameplay/T66GamblerBoss.cpp` drops `Item_GamblersToken`.

## Proposed Final Answer

State:

- Presented count: 30 total presented items = 28 normal modifier items + 2 special items.
- Raw data caveat: `Items.csv` still has 31 rows, but the answer omits `Item_HpRegen` and `Item_LifeSteal` and includes the source-defined `Item_GamblersToken`.
- Use this order:

Weapon Modifiers
- Damage: AoeDamage, BounceDamage, PierceDamage, DotDamage
- Attack Speed: AoeSpeed, BounceSpeed, PierceSpeed, DotSpeed
- Attack Scale: AoeScale, BounceScale, PierceScale, DotScale

Character Modifiers
- Accuracy: CritDamage, CritChance, AttackRange, Accuracy
- Evasion: Invisibility, CounterAttack, Assassinate, EvasionChance
- Armor: Taunt, ReflectDmg, Crush, DamageReduction

Luck Modifier
- Luck: LootCrate, TreasureChest, Cheating, Stealing

Special Items
- Item_BackroomsQuickRevive / Quick Revive
- Item_GamblersToken / Gambler's Token, the vendor token the user was remembering

## Verification Performed

- Re-read live CSV data with `Import-Csv Content\Data\Items.csv`.
- Searched source for `GamblersToken`, `VendorToken`, and `QuickRevive`.
- Confirmed no `ANTHROPIC_API_KEY` is set in Process/User/Machine before reviewer execution.

## Risks

- "Vendor Token" could be user-facing shorthand for a future renamed token, but live source currently identifies the matching special item as `Gambler's Token`.
