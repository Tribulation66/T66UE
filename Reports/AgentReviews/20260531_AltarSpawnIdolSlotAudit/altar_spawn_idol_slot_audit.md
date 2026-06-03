# T66 Altar Spawn and Idol Slot Audit

Date: 2026-05-31  
Operator: Codex  
Validator: Claude pending  
Scope: read-only audit/report for the requested altar spawn, idol opportunity, and equipped idol count changes. No gameplay code, data tables, assets, config, or saves were changed.

## Requested Target Rules

- Stage/floor rule: on stage 1 floor 1, only the weapon altar should be present among reward/interactable spawns.
- Stage 4 exception: on floor 1 of local stage 4, the weapon altar and idol altar should both be present because killing the stage 4 boss ends the difficulty and should not spawn a post-boss idol altar.
- Miniboss reward rule: each of the three placed tower minibosses should spawn an idol altar when killed.
- Stage boss reward rule: local stages 1, 2, and 3 should spawn an idol altar after the stage boss dies; local stage 4 should not spawn a post-boss idol altar.
- Per-difficulty target count before the final boss: 4 weapon altar interactions and 16 idol altar interactions.
- Inferred rarity cadence from the requested count: local stage 1 gives black-tier weapon/idols, local stage 2 gives red-tier weapon/idols, local stage 3 gives yellow-tier weapon/idols, and local stage 4 gives white-tier weapon/idols. This is the only way the stated "4 white rarity idols" before the final boss lines up with 16 idol altar visits.
- Equipped idol target: the character needs enough held/equipped idol capacity to retain the 16 idol pickups for the difficulty.

## Current Count Summary

- Current normal tower layout: 5 floors: floor 1 start, floors 2-4 mob floors, floor 5 boss floor (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:58-62`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4860-4870`).
- Current difficulty-ending stage layout: 2 floors because `IsBossRushFinaleStage()` is true when the current stage equals the selected difficulty end stage, and `BuildLayout(..., bBossRushFinaleStage=true)` makes floor 2 the boss floor and removes normal mob floors (`Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp:7-21`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4861-4863`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4870`).
- Current weapon altar opportunities per selected difficulty: 4 stage-start weapon altar spawns in tower stages, because tower stage start bypasses the non-tower "difficulty start only" guard (`Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:1973-2002`).
- Current weapon altar rarity cadence: one base rarity per selected difficulty, not one rarity per local stage (`Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:2049-2055`, `Content/Data/DifficultyTuning.json:3-44`).
- Current idol opportunities before the difficulty-ending boss: 12 from local stages 1-3 if all three minibosses and each stage boss are cleared; local stage 4 currently contributes no normal miniboss idol altars and no post-boss idol altar under the boss-rush/difficulty-clear path (`Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp:800-805`, `Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp:861-902`, `Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp:451-493`).
- Current equipped idol capacity: 3 slots (`Source/T66/Core/T66IdolManagerSubsystem.h:22`, `Source/T66/Core/T66IdolManagerSubsystem.cpp:9-37`).
- Current idol altar visible offers: 3 cards per page, 4 pages/categories, backed by 12 stock slots (`Source/T66/UI/T66IdolAltarOverlayWidget.h:35-37`, `Source/T66/UI/T66IdolAltarOverlayWidget.cpp:312-327`, `Source/T66/UI/T66IdolAltarOverlayWidget.cpp:1084-1097`).

## Current Implementation Inventory

### Tower Floors and Stage 4

- The static normal tower floor constants are floor 1 start, floors 2-4 mob floors, and floor 5 boss (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:58-62`).
- Normal layout assignment copies those constants into the generated layout (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:4860-4863`) and marks only non-start/non-boss floors as mob floors (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:4876-4887`).
- The current map design reference says floor 1 is "weapon altar only," floors 2-4 are gameplay floors, and floor 5 is boss flow only (`Gameplay/World/T66_MAP_DESIGN_REFERENCE.md:13-18`).
- The code currently treats every selected difficulty's end stage as a boss-rush finale (`Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp:7-21`). In that mode, the generated tower has only 2 floors and no normal floor 2-4 miniboss sequence (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:4861-4870`).
- Boss-rush finale also skips world interactable population and enemy director spawning (`Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:470-474`, `Source/T66/Gameplay/GameMode/T66GameMode_Bootstrap.cpp:702-715`).

### Stage Start / Floor 1 Spawns

- Stage bootstrap always calls `SpawnWeaponAltarForPlayer()` for the first player (`Source/T66/Gameplay/GameMode/T66GameMode_Bootstrap.cpp:447-460`).
- In tower layout, bootstrap then calls `SpawnWorldInteractablesForStage()` (`Source/T66/Gameplay/GameMode/T66GameMode_Bootstrap.cpp:462-469`).
- Tower world interactables build `TowerMobFloorNumbers` only from `T66IsNormalTowerInteractableFloor(...)`, which requires `Floor.bMobFloor`, excludes start and boss floors, and requires the floor to be between first and last mob floor (`Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:87-96`, `Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:475-493`).
- Chests, crates, loot wheels, fountains, loot bags, arcade machines, vendors, casino interactables, vehicles, saints, and totems in the tower branch are distributed over those mob floors rather than floor 1 (`Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:739-849`, `Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:1536-1564`).
- The start-gallery showcase path is explicitly skipped at tower stage start with the log "floor 1 is reserved for the Weapon Altar" (`Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:1563-1564`).
- Backrooms entry doors are spawned through a separate bootstrap call, but the door floor is selected only from mob floors (`Source/T66/Gameplay/GameMode/T66GameMode_Bootstrap.cpp:733-740`, `Source/T66/Gameplay/GameMode/T66GameMode_Backrooms.cpp:504-518`).
- Tower descent holes are also spawned. The floor 1 descent hole is initialized with `bRequiresWeaponSelection` because floor 1 is the start floor (`Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp:773-790`). This is a progression actor that currently coexists with the floor 1 weapon altar.

### Weapon Altar

- `SpawnWeaponAltarForPlayer()` spawns one `AT66WeaponAltar` if the global `WeaponAltar` pointer is not already valid (`Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:1973-1984`, `Source/T66/Gameplay/T66GameMode.h:374-377`).
- Non-tower stages only allow the weapon altar on the selected difficulty start stage and only when no weapon is equipped; tower stages bypass that guard because `bTowerStageStart = IsUsingTowerMainMapLayout()` (`Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:1996-2002`).
- In tower layout, the weapon altar uses the tower start anchor and is snapped/tagged to the start floor (`Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:2004-2035`, `Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:2064-2068`).
- Weapon offer rarity is currently `DifficultyTuning->GetDifficultyWeaponBaseRarity(SelectedDifficulty)`, so every weapon altar within a selected difficulty uses the same base rarity (`Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:2049-2055`).
- Difficulty tuning currently defines base weapon rarity by selected difficulty: Easy black, Medium red, Hard yellow, VeryHard white, Impossible white (`Content/Data/DifficultyTuning.json:3-44`; built-in defaults mirror this at `Source/T66/Core/T66DifficultyTuningSubsystem.cpp:37-72`).

### Idol Altar Spawn Paths

- `SpawnIdolAltarForPlayer()` exists as a stage-entry spawn helper, but it immediately returns for tower layout with "floor 1 is reserved for the weapon altar" (`Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:1863-1874`).
- No live caller was found for `SpawnIdolAltarForPlayer()` in `Source/T66` besides the function definition (`rg -n "SpawnIdolAltarForPlayer\\(" Source/T66` returned only the declaration/definition).
- Tower miniboss floors are current normal tower mob floors and are disabled on boss-rush finale stages (`Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp:800-805`).
- Entering a mob floor spawns the placed tower miniboss for that floor and starts floor population (`Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp:1007-1047`).
- Killing a tower gate guardian/miniboss spawns an idol altar at the guardian death location with `bAllowMultiple=true`, snaps/tags it to the guardian floor, and syncs the miasma source anchor (`Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp:861-902`).
- `SpawnIdolAltarAtLocation(...)` grants one selection by setting `RemainingSelections = 1` (`Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:2348-2385`). The altar actor itself also defaults to one remaining selection (`Source/T66/Gameplay/T66IdolAltar.h:57-59`).
- Stage boss defeat spawns an idol altar and stage gate only after non-difficulty-clear boss kills (`Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp:451-493`).
- Difficulty-clear boss kills return after opening the run summary, so no post-boss idol altar is spawned on the difficulty-ending stage (`Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp:453-489`).

### Idol Capacity and Held Idol Display

- The idol manager's hard cap is `MaxEquippedIdolSlots = 3` (`Source/T66/Core/T66IdolManagerSubsystem.h:22`).
- `NormalizeEquippedArrays()` pads or truncates `EquippedIdolIDs` and `EquippedIdolLevels` to that hard cap (`Source/T66/Core/T66IdolManagerSubsystem.cpp:9-37`).
- Selecting from the altar or stock requires a free slot and rejects duplicate idol IDs (`Source/T66/Core/T66IdolManagerSubsystem.cpp:221-243`, `Source/T66/Core/T66IdolManagerSubsystem.cpp:332-370`).
- Restore/save resume also normalizes to the same hard cap, so any saved/restored idol array larger than the cap would be truncated until the cap changes (`Source/T66/Core/T66IdolManagerSubsystem.cpp:445-470`).
- HUD idol arrays are sized to `UT66IdolManagerSubsystem::MaxEquippedIdolSlots` (`Source/T66/UI/HUD/T66GameplayHUDWidget_Build.cpp:76-81`).
- The current HUD idol panel is a 2-column grid using 68px slots inside a 152px by 152px bottom-left side panel (`Source/T66/UI/HUD/T66GameplayHUDWidget_Build.cpp:653-725`, `Source/T66/UI/HUD/T66GameplayHUDWidget_Build.cpp:1538-1558`, `Source/T66/UI/HUD/T66GameplayHUDWidget_Private.h:699-703`).
- HUD refresh loops over the built idol slot widgets and binds idol icons/tooltips from the idol manager/run state (`Source/T66/UI/HUD/T66GameplayHUDWidget_Refresh.cpp:1055-1109`).
- Run summary has two idol presentations: one hardcoded flat summary with 4 idol slots (`Source/T66/UI/Screens/T66RunSummaryScreen.cpp:2148-2158`) and another row that loops `MaxEquippedIdolSlots` (`Source/T66/UI/Screens/T66RunSummaryScreen.cpp:3196-3267`).
- Combat caches all equipped idols dynamically from the idol manager/run state and iterates the cached array for idol projectile lanes and idol effects (`Source/T66/Gameplay/T66CombatComponent.cpp:924-965`, `Source/T66/Gameplay/T66CombatComponent.cpp:3225-3268`, `Source/T66/Gameplay/T66CombatComponent.cpp:3343-3452`).
- Local run saves, leaderboard snapshots, and run snapshots copy dynamic idol arrays from the idol manager/run state (`Source/T66/Core/T66SessionSubsystem.cpp:789-802`, `Source/T66/Core/T66LeaderboardSubsystem.cpp:932-941`, `Source/T66/Core/RunState/T66RunStateSubsystem_Snapshot.cpp:91-99`).
- Backend run summary serialization writes `equipped_idols` as a dynamic array of idol IDs (`Source/T66/Core/Backend/T66BackendRunSerializer.cpp:511-516`).

### Idol Altar Offer UI

- The current altar widget uses `OfferSlotCount = 12`, `OfferSlotsPerCategory = 3`, and `OfferCategoryCount = 4` (`Source/T66/UI/T66IdolAltarOverlayWidget.h:35-37`).
- It constructs exactly 3 visible offer cards per page (`Source/T66/UI/T66IdolAltarOverlayWidget.cpp:300-360`).
- Reroll advances the active offer category modulo 4; it does not create a fifth "No Idol" page or 4-card element pages (`Source/T66/UI/T66IdolAltarOverlayWidget.cpp:1084-1097`).
- Current visible card text includes idol name plus rarity name on separate lines (`Source/T66/UI/T66IdolAltarOverlayWidget.cpp:649-660`).
- Taking an idol checks for remaining selections, duplicate ownership, and empty equipped slot before committing the selection animation (`Source/T66/UI/T66IdolAltarOverlayWidget.cpp:721-813`).
- The eventual commit uses `SelectIdolFromStock(...)` for normal offers, then consumes the altar's one-selection budget (`Source/T66/UI/T66IdolAltarOverlayWidget.cpp:1036-1073`).

## Requirement-by-Requirement Implications

### 1. Stage 1 Floor 1 Should Only Have the Weapon Altar

Already present in intent:
- The design reference explicitly says floor 1 start is "weapon altar only" and has no normal interactable scatter (`Gameplay/World/T66_MAP_DESIGN_REFERENCE.md:13-18`).
- The tower interactable floor predicate excludes the start floor (`Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:87-96`).
- Tower reward/utility interactables are routed through mob-floor lists, not floor 1 (`Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:475-493`, `Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:739-849`).

Partially present / structurally distributed:
- The clean-floor rule is enforced by multiple individual systems rather than one central floor-1 allowlist. The current code relies on each spawn path honoring `TowerMobFloorNumbers`, plus separate skips for start gallery and Backrooms.
- Floor 1 also has a required tower descent hole today, gated by weapon selection (`Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp:773-790`). If "only weapon altar" literally excludes the descent hole, the traversal design would need a separate progression path. If the intended meaning is "only reward/utility interactables," the descent hole can remain as progression infrastructure.

Missing for a durable fix:
- A central authoritative floor-1 spawn policy/check does not currently exist. The audit did not find normal tower reward interactables intentionally routed to floor 1, but a future implementation would need to consolidate the rule so any actor on floor 1 must be explicitly allowed: weapon altar, required descent/progression actor, player start/terrain infrastructure, and the local-stage-4 idol altar exception.

### 2. Stage 4 Floor 1 Should Have Weapon Altar + Idol Altar

Already present:
- Weapon altar spawning at tower stage start already happens every stage, including the difficulty-ending stage (`Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:1973-2002`).
- `SpawnIdolAltarAtLocation(...)` can create an idol altar with one selection and return the spawned actor (`Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:2348-2385`).

Missing:
- No current stage-start tower idol altar path exists. `SpawnIdolAltarForPlayer()` is skipped for all tower layouts and has no live caller (`Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:1863-1874`).
- There is no local-stage-4 exception that places an idol altar on floor 1 beside the weapon altar.
- Current `IsBossRushFinaleStage()` makes local stage 4 a two-floor boss-rush stage, which conflicts with the requested 16-idol pre-final-boss count because stage 4 currently has no floors 2-4 miniboss sequence (`Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp:7-21`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4861-4870`).

Code/data implications:
- Add a local-stage resolver for "stage within selected difficulty" or reuse `FStageData.LocalStageNumber` / `UT66StageProgressionSubsystem` local stage index.
- Add a tower start-floor idol altar spawn path only when local stage is 4, with a placement offset that does not overlap the weapon altar or descent hole.
- Keep the difficulty-clear boss path from spawning a post-boss idol altar, because the requested replacement is the floor 1 stage 4 idol altar.
- Decide whether local stage 4 should continue to be `IsBossRushFinaleStage()` or use the normal five-floor tower layout. The requested 16-idol count requires local stage 4 to have the three miniboss floors before the final boss.

### 3. Idol Altar After the Three Minibosses

Already present for normal tower stages:
- Floors 2-4 are placed-miniboss floors in normal tower layout (`Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp:800-805`).
- Entering each mob floor spawns that floor's placed tower miniboss (`Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp:1007-1047`).
- Killing that miniboss spawns one idol altar and floor-tags it (`Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp:861-902`).

Missing for the requested final-stage count:
- Because local stage 4 is currently boss-rush finale, `IsPlacedTowerMinibossFloor()` returns false and the three stage 4 miniboss altar opportunities do not exist (`Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp:800-805`).

Code/data implications:
- Preserve the existing guardian-defeat-to-idol-altar path for local stages 1-3.
- Extend the same normal tower floor/miniboss path to local stage 4 if the requested final-stage pre-boss count is authoritative.

### 4. Idol Altar After Stage Boss, Except Difficulty-Ending Boss

Already present:
- Non-difficulty-clear stage boss kills spawn an idol altar and stage gate (`Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp:451-493`).
- Difficulty-clear boss kills return after summary/final-sequence handling and do not spawn the post-boss idol altar (`Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp:441-489`).

Missing:
- Nothing is missing for the "no post-final-boss idol altar" portion.
- The missing piece is the stage 4 floor 1 replacement idol altar before the final boss, described above.

### 5. Weapon/Idol Rarity Cadence by Local Stage

Already present:
- Difficulty tuning has start/end stages for each difficulty (`Content/Data/DifficultyTuning.json:3-44`).
- Stage data contains `LocalStageNumber` 1-4 for each difficulty band (`Content/Data/Stages.csv:1-21`).
- Idol rarity/tier conversion already supports four tiers: black, red, yellow, white (`Source/T66/Core/T66IdolManagerSubsystem.cpp:107-128`).

Missing:
- Weapon altar rarity currently comes from selected difficulty base rarity, not local stage rarity (`Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:2049-2055`).
- Idol altar stock rarity currently comes from selected difficulty base rarity, not local stage rarity (`Source/T66/Core/T66IdolManagerSubsystem.cpp:286-307`).
- Difficulty tuning currently stores one idol and weapon base rarity per selected difficulty (`Content/Data/DifficultyTuning.json:3-44`).

Code/data implications:
- Add or derive a local-stage rarity mapping: local stage 1 = black, 2 = red, 3 = yellow, 4 = white.
- Apply that mapping to `BuildWeaponOffers(...)` and idol stock tier values.
- Decide whether this belongs in difficulty tuning data, stage progression tuning, or a code-level helper shared by weapon/idol altar systems.

### 6. Equipped Idol Count and Where Idols Show Up

Already present:
- Idol state is centralized in `UT66IdolManagerSubsystem`.
- Saves, run snapshots, and leaderboard snapshots copy dynamic idol arrays once the idol manager supplies them.
- Combat consumes the equipped idol array dynamically and does not appear to hardcode exactly three slots in the combat cache path.

Missing:
- `MaxEquippedIdolSlots` is 3; the desired 16-idol build cannot be held today (`Source/T66/Core/T66IdolManagerSubsystem.h:22`).
- The idol manager truncates restored/equipped arrays to the cap (`Source/T66/Core/T66IdolManagerSubsystem.cpp:9-37`, `Source/T66/Core/T66IdolManagerSubsystem.cpp:445-470`).
- The bottom-left HUD panel is sized for the current small slot count and uses two columns at 68px per slot inside a 152px panel (`Source/T66/UI/HUD/T66GameplayHUDWidget_Build.cpp:653-725`, `Source/T66/UI/HUD/T66GameplayHUDWidget_Private.h:699-703`).
- Run summary flat mode has a hardcoded 4-idol presentation (`Source/T66/UI/Screens/T66RunSummaryScreen.cpp:2148-2158`).

Code/data implications:
- Increase the idol cap to 16, then adjust every slot-based presentation that uses the cap.
- Redesign the HUD idol display so 16 idols are readable and fit in the intended location. A 4-by-4 grid is the direct structural match for "16 held idols," but the current 68px, 2-column, 152px panel dimensions cannot hold that without resizing or shrinking.
- Update run summary idol display to handle 16 consistently in both flat and non-flat layouts.
- Confirm save, snapshot, leaderboard, and backend consumers still accept 16 dynamic entries after the manager no longer truncates to 3.
- Combat will iterate more idol slots once equipped; that is structurally supported by the dynamic loop, but it changes the number of idol effects fired per attack and should be handled by the separate projectile/rendering throughput work.

## Desired vs Current Opportunity Table

| Local stage | Current floor structure | Current idol opportunities | Desired idol opportunities |
|---|---:|---:|---:|
| Stage 1 | Normal 5-floor tower | 3 miniboss altars + 1 post-boss altar = 4 | 3 miniboss altars + 1 post-boss altar = 4 black |
| Stage 2 | Normal 5-floor tower | 3 miniboss altars + 1 post-boss altar = 4 | 3 miniboss altars + 1 post-boss altar = 4 red |
| Stage 3 | Normal 5-floor tower | 3 miniboss altars + 1 post-boss altar = 4 | 3 miniboss altars + 1 post-boss altar = 4 yellow |
| Stage 4 | Current boss-rush finale, 2 floors | 0 normal miniboss altars + 0 post-boss altar = 0 | 1 floor-1 altar + 3 miniboss altars + 0 post-boss altar = 4 white |
| Total before difficulty final boss | Current path | 12 idol altar opportunities | 16 idol altar opportunities |

## Implementation Surface Map

- Stage/floor classification:
  - `Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp`
  - `Source/T66/Gameplay/T66TowerMapTerrain.cpp`
  - `Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp`
  - `Source/T66/Core/T66StageProgressionSubsystem.cpp`
- Floor 1 allowlist / start-floor spawn routing:
  - `Source/T66/Gameplay/GameMode/T66GameMode_Bootstrap.cpp`
  - `Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp`
  - `Source/T66/Gameplay/GameMode/T66GameMode_Backrooms.cpp`
  - `Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp`
- Weapon altar rarity and offers:
  - `Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp`
  - `Source/T66/Core/T66DifficultyTuningSubsystem.cpp`
  - `Content/Data/DifficultyTuning.json`
- Idol altar stock and equipped state:
  - `Source/T66/Core/T66IdolManagerSubsystem.h`
  - `Source/T66/Core/T66IdolManagerSubsystem.cpp`
  - `Source/T66/UI/T66IdolAltarOverlayWidget.h`
  - `Source/T66/UI/T66IdolAltarOverlayWidget.cpp`
- Idol display:
  - `Source/T66/UI/HUD/T66GameplayHUDWidget_Build.cpp`
  - `Source/T66/UI/HUD/T66GameplayHUDWidget_Refresh.cpp`
  - `Source/T66/UI/HUD/T66GameplayHUDWidget_Private.h`
  - `Source/T66/UI/Screens/T66RunSummaryScreen.cpp`
- Save/summary/backend surfaces:
  - `Source/T66/Core/T66SessionSubsystem.cpp`
  - `Source/T66/Core/T66LeaderboardSubsystem.cpp`
  - `Source/T66/Core/RunState/T66RunStateSubsystem_Snapshot.cpp`
  - `Source/T66/Core/Backend/T66BackendRunSerializer.cpp`
- Combat effects from larger idol count:
  - `Source/T66/Gameplay/T66CombatComponent.cpp`

## Audit Conclusions

- The miniboss-to-idol altar path already exists for normal tower floors 2-4.
- The non-final-stage boss-to-idol altar path already exists.
- The final boss not spawning a post-boss idol altar already matches the requested rule.
- The stage 4 floor 1 idol altar does not currently exist.
- The requested 16 idol opportunities require changing current final-stage/boss-rush behavior, because the current difficulty-ending stage has no floors 2-4 miniboss sequence.
- The current equipped idol cap is 3 and must be raised/reworked for 16 held idols.
- The current HUD and run-summary idol presentations are not sized for 16 idols.
- Current weapon and idol rarity are difficulty-based, not local-stage-based; the requested black/red/yellow/white sequence needs a local-stage rarity resolver applied to both systems.

## Verification Performed

- Read root task/process instructions from the user-provided `AGENTS.md` text.
- Read `.t66/operator-state.json`; operator is Codex and validator is Claude.
- Read gameplay/world/report routing instructions:
  - `Gameplay/GAMEPLAY_AGENTS.md`
  - `Gameplay/README.md`
  - `Gameplay/World/WORLD_AGENTS.md`
  - `Gameplay/World/T66_MAP_DESIGN_REFERENCE.md`
  - `Reports/AGENTS.md`
- Performed read-only source/data inspection with `rg` and `Get-Content`.
- No code, data, asset, config, or save changes were made.
- No runtime/editor capture was performed; this is a source-architecture audit only.
