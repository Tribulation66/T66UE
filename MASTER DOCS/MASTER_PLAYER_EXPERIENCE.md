# T66 Master Player Experience

**Last updated:** 2026-04-18  
**Scope:** Single-source handoff for the T66 player-experience system: expected outcomes, progression pacing, tower stage structure, miasma pressure, interactable frequency, hero growth, item/alchemy stat pressure, UI presentation rules, and the current anti-cheat linkage.  
**Companion docs:** `MASTER DOCS/T66_MASTER_GUIDELINES.md`, `MASTER DOCS/MASTER_STATS.md`, `MASTER DOCS/MASTER_MAP_DESIGN.md`, `MASTER DOCS/MASTER_COMBAT.md`, `ANTI_CHEAT/MASTER_ANTI_CHEAT.md`  
**Maintenance rule:** Update this file after every material change to stage structure, pacing targets, difficulty starts, miasma timing, interactable frequency, hero XP curves, hero level-up behavior, item/alchemy stat grants, or expected-outcome telemetry.

## 1. Executive Summary

- `UT66PlayerExperienceSubSystem` is the authored tuning surface for expected player outcomes.
- `UT66RunStateSubsystem` is the live run-state authority for hero level, hero XP, primary and secondary stats, timers, anti-cheat expectations, and saved-run persistence.
- `AT66GameMode` is the runtime integrator for tower layout rules, interactable/NPC spawning, boss XP, timer activation, and tower miasma activation.
- `AT66EnemyDirector` is the main enemy pacing controller.
- `AT66MiasmaManager` is the flood-pressure controller.
- `AT66TowerMapTerrain` is the tower floor/theme/prop layout controller.
- `UT66RngSubsystem`, `UT66SkillRatingSubsystem`, and `UT66BackendSubsystem` consume the expected-outcome side of the system for biasing, telemetry, and backend export.

The current intended run shape is:

- Stages `1-20` are the full level climb.
- The run should gain `98` levels across those `20` stages on average.
- Average target is `4.9` levels per stage, not a forced exact `5`.
- Bosses should always contribute `1` full level.
- Stage `21` should begin at level `99`.

The current implemented tower stage shape is:

- Floor `1`: idol/start floor
- Floors `2-4`: gameplay floors
- Floor `5`: boss floor

The current implemented tower-theme rule is:

- `Easy -> Dungeon`
- `Medium -> Forest`
- `Hard -> Ocean`
- `VeryHard -> Martian`
- `Impossible -> Hell`

The current implemented pressure rule is:

- speedrun timer starts when the stage is entered
- stage timer stays separate
- tower miasma starts when the idol is taken or when the player descends without taking one
- tower miasma reaches full non-boss coverage by `5:00`

## 2. Canonical Files

- `Config/DefaultT66PlayerExperience.ini`
- `Source/T66/Core/T66PlayerExperienceSubSystem.h`
- `Source/T66/Core/T66PlayerExperienceSubSystem.cpp`
- `Source/T66/Core/T66RunStateSubsystem.h`
- `Source/T66/Core/T66RunStateSubsystem.cpp`
- `Source/T66/Core/T66RunSaveGame.h`
- `Source/T66/Core/T66BackendSubsystem.cpp`
- `Source/T66/Core/T66SkillRatingSubsystem.cpp`
- `Source/T66/Gameplay/T66GameMode.h`
- `Source/T66/Gameplay/T66GameMode.cpp`
- `Source/T66/Gameplay/T66EnemyDirector.cpp`
- `Source/T66/Gameplay/T66EnemyBase.h`
- `Source/T66/Gameplay/T66EnemyBase.cpp`
- `Source/T66/Gameplay/T66MiasmaManager.h`
- `Source/T66/Gameplay/T66MiasmaManager.cpp`
- `Source/T66/Gameplay/T66TowerMapTerrain.h`
- `Source/T66/Gameplay/T66TowerMapTerrain.cpp`
- `Source/T66/Data/T66DataTypes.h`
- `Source/T66/UI/T66StatsPanelSlate.h`
- `Source/T66/UI/T66StatsPanelSlate.cpp`
- `Source/T66/UI/T66GameplayHUDWidget.cpp`
- `Content/Data/Heroes.csv`

## 3. Ownership Model

### 3.1 PlayerExperience

- Owns difficulty starts and difficulty-end logic.
- Owns run-start help:
  - starting stage
  - starting gold bonus
  - start loot bags
  - start bonus hero levels
  - support vendor spawn permission
- Owns authored reward odds:
  - enemy loot-bag drop chance
  - loot-bag count
  - loot-bag rarity
  - chest counts, reward rarity odds, and gold bands
  - wheel counts and rarity
  - crate counts and rarity
  - gambler cheat base chance
  - vendor steal base chance

### 3.2 RunState

- Owns live hero XP and level.
- Owns precise primary stats in fixed-point tenths.
- Owns secondary stat bonus accumulation.
- Owns timers and speedrun pacing state.
- Owns run snapshot import/export.
- Owns anti-cheat expected-outcome accumulators and exports.

### 3.3 GameMode

- Owns how PlayerExperience rules are applied to the current stage.
- Owns tower-only overrides:
  - floor restrictions
  - guaranteed and excluded interactables
  - quick revive placement
  - saint placement
  - circus placement
  - miasma start condition
  - boss XP grant

### 3.4 EnemyDirector

- Owns spawn cadence and alive-budget pacing.
- Tower pacing is now explicitly overridden away from the old generic density-scaled path.

### 3.5 MiasmaManager

- Owns flood-grid construction and expansion coverage.
- Tower expansion is now source-driven rather than random batch patches.

## 4. Difficulty Bands And Expected Progression

From `DefaultT66PlayerExperience.ini`:

- `Easy` starts at stage `1`
- `Medium` starts at stage `6`
- `Hard` starts at stage `11`
- `VeryHard` starts at stage `16`
- `Impossible` starts at stage `21`

Current intended progression model:

- Easy stage `1` starts at level `1`
- Medium stage `6` starts at level `25`
- Hard stage `11` starts at level `50`
- VeryHard stage `16` starts at level `75`
- the run climbs from level `1` to level `99` across stages `1-20`
- Impossible stage `21` is the first stage that should begin at level `99`

Practical pacing target:

- `~4` levels should come from the `3` gameplay floors
- `1` level should come from the boss
- the stage average can vary around that target

## 5. Tower Stage Structure

The tower runtime is now a fixed `5`-floor stage:

- floor `1` is the idol floor
- floors `2-4` are the only mob floors
- floor `5` is the boss floor

Rules:

- no interactables or NPCs are allowed on the idol floor
- no interactables or NPCs are allowed on the boss floor
- gameplay floors are the only floors that can host normal tower content

Theme mapping is by difficulty, not by per-floor rotation:

- Easy: dungeon-only stage family
- Medium: forest-only stage family
- Hard: ocean-only stage family
- VeryHard: martian-only stage family
- Impossible: hell-only stage family

Prop rule:

- tower decorative props are now reduced to cube-based rock stepping stones
- the stepping stones only spawn on gameplay floors
- they are intended to remain usable after the floor is flooded

## 6. Timing And Miasma

Two clocks now matter:

- speedrun clock
- flood/stage-pressure clock

Current implemented behavior:

- speedrun clock starts on stage entry
- tower stage timer is still armed separately
- tower miasma starts when the idol state changes from untouched to selected
- if the player skips the idol and descends, tower miasma starts on descent

Current flood target:

- full non-boss coverage by `300` seconds
- boss floor excluded from coverage

Current tower spread model:

- start floor spreads from the altar anchor
- lower floors spread from explicit floor anchors or fallback arrival points
- tile ordering is floor-aware and distance-sorted from that anchor
- tower tile size is much smaller than before:
  - `max(Layout.PlacementCellSize * 0.25, 300)`

Important implementation note:

- this is still the existing miasma runtime, not final blood/liquid presentation
- behavior has been shifted toward the desired spread pattern, but final art treatment is still pending

## 7. Interactable And NPC Frequency Rules

### 7.1 Tower gameplay-floor rules

- chests: `1-3` per gameplay floor
- loot crates: `1-3` per gameplay floor
- fountains: `0-1` per gameplay floor
  - current implementation uses a per-floor deterministic roll and does not force one every stage
- quick revive: exactly once, always on floor `2`
- saint: exactly once per stage, random gameplay floor
- casino: once per gameplay floor
  - runtime object is currently `Circus`

### 7.2 Chest presentation and reward rules

- all treasure chests use the same yellow chest model in the world
- chest rarity is no longer authored as a spawn visual
- chest reward rarity is rolled when the chest is opened
- chest gold is rolled from the opened reward rarity band
- chest reward presentation always starts on a black background
- the reveal background upgrades by threshold:
  - black to red after the black gold cap is passed
  - red to yellow after the red gold cap is passed
  - yellow to white after the yellow gold cap is passed
- skip advances chest rewards one reveal color at a time until the rolled rarity is reached

### 7.3 Explicit tower exclusions

- idol floor: no NPCs or interactables
- boss floor: no NPCs or interactables

### 7.4 Tower-specific suppressions

The current tower path also suppresses:

- wheel spawns
- tower totem spawns

That suppression is runtime-local in `AT66GameMode`. The authored `PlayerExperience` config still contains general wheel/chest/crate ranges for the non-tower path.

## 8. XP And Spawn Pacing

### 8.1 Boss XP

Current implemented boss rule:

- on boss defeat, `GameMode` grants `RunState->GetHeroXPToNextLevel()`
- this guarantees one full level from the boss

### 8.2 Regular enemy XP

Current tower regular-enemy rule:

- enemy data default is still `XPValue = 20`
- tower runtime halves that award
- effective tower regular kill XP is therefore `10`

### 8.3 Tower director budget

Current tower pacing constants in `T66EnemyDirector.cpp`:

- initial enemies per gameplay floor: `4`
- runtime spawn interval: `9.0s`
- enemies per runtime wave: `1`
- max alive enemies: `12`

This is a deliberate move away from the old density-scaled tower behavior.

Why this matters:

- `3` gameplay floors x `4` initial enemies = `12` guaranteed regular enemies
- a perfect `300s` stage with a `9s` runtime cadence can inject roughly `33` additional regular enemies if the player keeps the board clear
- that yields an upper-end regular-kill budget of roughly `45` enemies per stage
- `45 x 10 XP = 450 XP`, or `~4.5` levels
- boss adds `100 XP`, or `1` level
- total upper-end pressure lands around `5.5` levels per stage, which is close enough to the intended `4.9` average once dead time, missed kills, pathing, pickups, and imperfect clears are accounted for

This is the current balancing basis for tower pacing.

## 9. Hero Growth Model

### 9.1 Internal precision

Hero primary stats are now internally stored in tenths through `FT66HeroPreciseStatBlock`.

Rules:

- tenths are authoritative internally
- display rounds upward
- display clamps to `/99`

### 9.2 Primary growth rules

Current implemented runtime rules:

- `Damage` defaults to `+0.5` to `+1.0` per level for every hero
- `Damage` is still uniform by design, but it is now authored through the hero table path as decimal ranges
- all other primaries, including `Speed`, use the authored hero gain ranges from hero tuning
- those ranges are stored as floats and rolled in tenths
- luck bias can push the roll toward the high end through `UT66RngSubsystem`

### 9.3 Secondary proxy growth rules

When a primary stat gains value, secondary bonus tenths are also granted.

Rules:

- if the primary is `Damage`, `AttackSpeed`, or `AttackScale`:
  - the hero's main attack-family secondary gains `70%-100%` of the primary gain
  - the other attack-family secondaries gain `10%-50%` of the primary gain
- if the primary is non-attack-family:
  - matching secondaries gain `50%-100%` of the primary gain

This is the current runtime implementation of the desired main-specialty versus off-specialty pattern.

### 9.4 Authored proficiency intent

The design intent is:

- hero base proficiency should tell the class story
- level-1 primary and secondary identity should range from `1-10`
- `10 = Excellent`
- `5 = Mid`
- `1 = Terrible`

Important current-state note:

- hero base stats and per-level growth ranges are sourced through the cooked Heroes DataTable path via `FHeroData`, `UT66GameInstance::GetHeroData`, and `UT66GameInstance::GetHeroStatTuning`
- runtime now supports tenths and range-based growth
- the hero row schema now supports decimal-authored primary gain ranges for all foundational stats, including `Speed`
- the current first-pass authoring rule seeds non-damage primary growth from the hero's current base proficiency and keeps `Damage` at `0.5-1.0`
- the final authored hero tables still need to be reviewed to fully align all heroes with the intended `1-10` base-proficiency story

## 10. Item And Alchemy Stat Pressure

### 10.1 Item rarity flat bonuses

Current implemented item flat bonuses:

- Black: `+1` primary, `+1` direct secondary
- Red: `+3` primary, `+3` direct secondary
- Yellow: `+9` primary, `+9` direct secondary
- White: `+27` primary, `+27` direct secondary

### 10.2 Alchemy rarity flat bonuses

Current implemented alchemy flat bonuses:

- Black: `+1`
- Red: `+4`
- Yellow: `+10`
- White: `+30`

### 10.3 Proxy behavior

Items do not only add their explicit secondary.

When an item adds primary stat value:

- the direct secondary gets its flat item bonus
- secondary proxy gains are also rolled from the primary increase
- this mirrors the same primary-to-secondary behavior used by level-ups

Implementation note:

- inventory slots now store deterministic roll seeds and optional explicit secondary-flat overrides
- saved runs now persist precise stats, the hero-stat RNG seed, and persistent secondary bonus entries

## 11. Level-Up Presentation And Burst

Current implemented level-up effects:

- full heal to max HP
- `P_Bifrost` spawned at the hero location
- nearby enemies killed in a `375 UU` radius
- floating combat text level-up event shown

Important gameplay note:

- the current nearby-kill implementation uses normal enemy death routing
- nearby kills still award score, XP, loot, and telemetry
- this means level-up burst chaining can still amplify room clears if a level-up happens inside a dense pack

That is currently accepted behavior, but it is a known exploit-pressure point.

## 12. UI Presentation Rules

Current implemented visual rules:

- `Level` now displays as `current/99`
- displayed primary stats show `current/99`
- primary-stat adjectives are back visually for the primary stat lines only
- secondary stat rows do not show adjectives
- secondary category headers show the owning primary stat value but not the adjective
- chest HUD presentation always uses yellow chest art, regardless of the rolled reward rarity
- chest reward reveal background always starts at black
- chest reward reveal background upgrades by reveal stage:
  - black -> red -> yellow -> white
  - stage changes occur when the displayed gold counter crosses the authored chest gold minimum for that rarity tier
  - reveal staging never exceeds the rolled chest reward rarity
- chest skip behavior is chest-specific:
  - one skip advances exactly one rarity reveal stage
  - skip only closes the chest presentation if it is already showing the final rolled rarity stage
- wheel, crate, and pickup-card skip behavior stays unchanged

Current adjective model:

- adjectives are determined from the stronger of:
  - hero innate base proficiency
  - current progression toward the cap

That means:

- a hero can start naturally strong in a stat
- a weak base stat can still become visually strong if itemization and leveling push it far enough

HUD rule:

- the main gameplay HUD level readout also now uses `/99`

## 13. Expected Outcomes And Anti-Cheat Link

This system is not isolated from anti-cheat.

The relationship is:

- `PlayerExperience` defines the intended generosity and pacing surface
- `RngSubsystem` executes luck-biased rolls
- `RunState` records expected chance telemetry and pressure-window expectations
- `SkillRatingSubsystem` consumes expected dodge pressure
- `BackendSubsystem` serializes those expected-outcome values to the backend payload
- Ranked policy now treats challenges and mods as expected unranked variants, not as part of the pristine-build ranked pool.
- The next anti-cheat phase will also use this system's authored expectations for compact provenance checks and boundary integrity validation instead of per-tick scanning.

Important expected-outcome fields already in the run-state/backend path:

- expected chance for bool outcomes such as cheat and steal
- replay rarity weights for weighted quality rolls
- expected dodges in anti-cheat pressure windows
- stage pacing points and cumulative active time

## 14. Current Open Gaps

- Final blood/liquid flood presentation is not done yet. Current work changes behavior and pressure, not the final art stack.
- `PlayerExperience` config still contains non-tower wheel/chest/crate ranges even though tower now uses stronger runtime overrides.
- The hero data tables still need an authored pass to fully align all heroes with the intended `1-10` base-proficiency story and the new specialty math.
- Level-up burst kills currently preserve full XP and loot routing, which can create chaining pressure spikes.
- This doc is now the master SOT for player experience, but it must stay in sync with `MASTER_STATS.md` whenever stat-growth math changes.
