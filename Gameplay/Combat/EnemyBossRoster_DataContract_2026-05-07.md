# Enemy/Boss Roster Data Contract

Status: Active

This is the live data contract for the enemy and boss roster after the placeholder cleanup. Image generation and Trellis mesh generation are intentionally separate follow-up work.

## Stage Layout

- `Easy`: stages `1-4`, theme `Dungeon`
- `Medium`: stages `5-8`, theme `Forest`
- `Hard`: stages `9-12`, theme `Ocean`
- `VeryHard`: stages `13-16`, theme `Martian`
- `Impossible`: stages `17-20`, theme `Hell`

Every difficulty has four stages. Local stage `4` is the boss-only finale stage for that difficulty.

## Regular Enemies

Each difficulty has ten regular enemies. The source of truth is `Content/Data/Enemies.csv`.

The normal per-difficulty pattern is:

- `5` core enemies
- `2` rare enemies
- `3` late enemies

Most difficulties use the core mix of `2` melee, `1` ranged, `1` rush, and `1` flying enemy. Impossible/Hell is the current exception: its core mix intentionally uses `2` rush, `1` melee, and `2` flying enemies, with no standard ranged core mob.

Stage rosters are authored in `Content/Data/Stages.csv` through `EnemyA` through `EnemyJ`. The ten slots are progressively filled inside each difficulty:

- Local stage `1`: `7` authored mobs, then `3` `None` slots.
- Local stage `2`: `8` authored mobs, then `2` `None` slots.
- Local stage `3`: `9` authored mobs, then `1` `None` slot.
- Local stage `4`: all `10` authored mobs, while also marked as the boss-only finale stage for that difficulty.

The `None` slots in local stages `1-3` are intentional authored gaps, not missing data. Runtime spawn code filters them out before choosing mobs.

## Negative Status Effects

The source of truth is `Content/Data/StatusEffects.csv`.

- `Webbed`: movement slow with web portrait icon.
- `Poisoned`: damage over time with poison icon.
- `Rooted`: brief movement lock with root icon.
- `Thorned`: thorn damage pressure with thorn icon.
- `Bleeding`: physical damage over time with blood icon.
- `ArmorCracked`: increased incoming damage with cracked shield icon.
- `Cursed`: damage/recovery impairment with curse icon.
- `Shocked`: auto-attack cadence disruption with lightning icon.
- `Chilled`: movement slow with snowflake icon.
- `Burning`: fire damage over time with flame icon.
- `Dazed`: short stun/interrupt with daze icon.

## Bosses

The source of truth is:

- `Content/Data/Bosses.csv`
- `Content/Data/BossEncounters.csv`
- `Content/Data/BossEncounterMembers.csv`

Stages `1-16` use themed bosses for local stages `1-3` and demonic Chad-like Goetia capstones for local stage `4`:

- `Bael Fallen Chad`
- `Buer Verdant Chad`
- `Focalor Drowned Chad`
- `Stolas Astral Chad`

Impossible uses the apocalypse chain:

- Stage `17`: `The Four Horsemen`, four simultaneous boss members.
- Stage `18`: `The False Prophet`.
- Stage `19`: `The Antichrist`.
- Stage `20`: `The Great Dragon`.

## Import And Validation

Reload combat roster DataTables after C++ is compiled:

```powershell
UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="C:/UE/T66/Scripts/SetupCombatRosterDataTables.py"
```

Validate source data without launching Unreal:

```powershell
python Scripts/ValidateEnemyBossRosterData.py
```

The old placeholder sources are archived at `Archive/DataReorg_2026-05-07_EnemyBossPlaceholderArchive`.

## Next Steps

1. Generate enemy and boss concept images from the `ImagePrompt` and `VisualConcept` fields.
2. Use Trellis to generate meshes from the approved images.
3. For boss meshes, run `Scripts/RunImportQuadRetroBossVisualsAndExit.py` in Unreal Editor. This imports the 23 `Stage02_Bosses_QuadRetroManifest.json` GLBs and upserts BossID-keyed rows in `Content/Data/CharacterVisuals.csv`.
4. Validate boss visual wiring with `python Scripts/ValidateBossQuadRetroVisuals.py`.
5. Implement the live hero-status runtime effects and portrait icons against `StatusEffects.csv`.
