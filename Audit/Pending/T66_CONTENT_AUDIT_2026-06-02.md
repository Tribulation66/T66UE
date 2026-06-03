# T66 / Chadpocalypse Content Audit

Date: 2026-06-02  
Project version audited: `alpha-0.9` from [DefaultGame.ini](C:/UE/T66/Config/DefaultGame.ini:7)  
Audit lane: CONTENT  
Operator: Codex  
Validator: Claude Code  
Audit mode: descriptive source-grounded pass. Suspected problems are flagged; no fixes were made.

## 1. Scope And Release Assumptions

This audit describes the current working-tree game from a player/design perspective. It includes the main game and the minigame-class modules `T66Mini`, `T66TD`, `T66Idle`, and `T66Deck`. The current project declares all five runtime gameplay modules plus the editor module in [T66.uproject](C:/UE/T66/T66.uproject:6), with `T66Mini` at [T66.uproject](C:/UE/T66/T66.uproject:13), `T66TD` at [T66.uproject](C:/UE/T66/T66.uproject:18), `T66Idle` at [T66.uproject](C:/UE/T66/T66.uproject:23), and `T66Deck` at [T66.uproject](C:/UE/T66/T66.uproject:28). Evidence tier: READ.

The current release assumption is Steam demo behavior because `bForceDemoMode=true` is set in [DefaultDemoMode.ini](C:/UE/T66/Config/DefaultDemoMode.ini:2), and `UT66ReleaseVariantSubsystem::GetEffectiveReleaseVariant()` returns `SteamDemo` when that flag is set in [T66ReleaseVariantSubsystem.cpp](C:/UE/T66/Source/T66/Core/T66ReleaseVariantSubsystem.cpp:103). Evidence tier: STATIC_TRACE.

Demo-visible content is intentionally smaller than the authored content library. Current demo allow-lists expose heroes `Hero_1` through `Hero_5` in [DefaultDemoMode.ini](C:/UE/T66/Config/DefaultDemoMode.ini:8), only `Easy` difficulty in [DefaultDemoMode.ini](C:/UE/T66/Config/DefaultDemoMode.ini:13), companions `Companion_01` through `Companion_04` in [DefaultDemoMode.ini](C:/UE/T66/Config/DefaultDemoMode.ini:14), and casino games `CoinFlip`, `RockPaperScissors`, and `BlackJack` in [DefaultDemoMode.ini](C:/UE/T66/Config/DefaultDemoMode.ini:22). Evidence tier: READ.

Minigames are in scope for this audit. They are not treated as a boundary note. In current demo mode, they are hidden/blocked, not deprecated: [DEMO_GATED_INVISIBLE_CONTENT.md](C:/UE/T66/Demo/DEMO_GATED_INVISIBLE_CONTENT.md:79) states the Minigames tab and Mini / TD / Idle / Deck / Versus screen family are present in full game and hidden/blocked in demo. Evidence tier: READ + STATIC_TRACE.

## 2. Shared Schema Used

Lifecycle status tags use only these tokens: `ACTIVE`, `DEMO_GATED`, `HIDDEN_RUNTIME`, `PARTIAL`, `DEPRECATED`, `COMPAT_LEGACY`, `BROKEN`, `STUB`, `ORPHAN_SUSPECT`, `UNKNOWN`.

Evidence tiers use only these tokens:

- `READ`: direct file/data read without tracing runtime route.
- `STATIC_TRACE`: route/function/data flow traced in code, but not run in this pass.
- `PRIOR_ARTIFACT`: prior proof/report referenced as context, not freshly verified here.
- `RUNTIME_VERIFIED`: current runtime/editor/gameplay verification performed in this pass.

This audit produced no fresh `RUNTIME_VERIFIED` claims. All claims are `READ` or `STATIC_TRACE` unless explicitly marked otherwise.

Visibility route values used by this content audit:

- `VISIBLE_DEMO`: visible or reachable under current forced-demo settings.
- `DEMO_OMITTED`: intentionally removed from the demo-visible UI/list.
- `DIRECT_NAV_BLOCKED_DEMO`: direct screen navigation is blocked in demo.
- `FULL_GAME_ROUTE`: traceable route exists when not in demo.
- `WORLD_INTERACTION`: reached through spawned in-world interactable or combat flow.
- `RUN_REWARD`: reached as boss, stage, loot, altar, shop, vendor, or post-run reward.
- `DATA_ONLY`: authored data exists but no player route was confirmed.
- `FALLBACK_ROUTE`: runtime synthesizes content from a fallback when a primary table/row is missing.
- `COMPAT_ONLY`: retained for serialized/data compatibility, not a visible content feature.
- `DEPRECATED_DISABLED`: centrally disabled in all builds by deprecated-feature settings.

Element IDs use `CONTENT-{AREA}-{NNN}`. Finding IDs use `CONTENTFIND-{NNN}`.

## 3. Audit Question Set Answered

These are the repo-generated questions this audit answers.

| Question | Answer location | Evidence tier |
|---|---|---|
| What is the current player-visible release slice, and what is hidden by demo mode? | Sections 1, 4, 5, 9 | READ + STATIC_TRACE |
| Which authored heroes, companions, difficulties, stages, bosses, weapons, idols, items, enemies, and minigames are actually reachable? | Sections 5, 6, 8, 9 | READ + STATIC_TRACE |
| Which content is present but hidden only because of demo gating? | Sections 5, 8, 9 | READ + STATIC_TRACE |
| Which content is deprecated rather than demo-gated? | Sections 5, 8, 11 | READ + STATIC_TRACE |
| Which data exists without a confirmed player route? | Sections 10, 11, 12 | READ + STATIC_TRACE |
| Which UI routes exist but are blocked by release variant or deprecated gates? | Sections 4, 5, 9, 10 | STATIC_TRACE |
| Where do docs/comments lag current code behavior? | Sections 10, 11, 12 | READ + STATIC_TRACE |
| How do minigames work now, at full depth, and how does a player reach them? | Sections 9, 10, 12 | READ + STATIC_TRACE |
| What later runtime checks should validate this descriptive pass? | Section 13 | READ |

## 4. What The Game Currently Is - Player Walkthrough

### 4.1 Boot And Frontend

[STATIC_TRACE] The project boots into a frontend/main-game stack with demo mode forced by config. Demo mode is determined in release-variant code, not only by UI convention: `GetEffectiveReleaseVariant()` honors `bForceDemoMode` in [T66ReleaseVariantSubsystem.cpp](C:/UE/T66/Source/T66/Core/T66ReleaseVariantSubsystem.cpp:103), and `IsDemoModeActive()` returns the Steam demo state in [T66ReleaseVariantSubsystem.cpp](C:/UE/T66/Source/T66/Core/T66ReleaseVariantSubsystem.cpp:140).

[STATIC_TRACE] The main menu can send the player to a new game, save slots, Daily Descent, Power Up, Minigames, Settings, etc., but Daily Descent returns early in demo before navigation in [T66MainMenuScreen.cpp](C:/UE/T66/Source/T66/UI/Screens/T66MainMenuScreen.cpp:2042). The Minigames main-menu handler still navigates to `ET66ScreenType::Minigames` in [T66MainMenuScreen.cpp](C:/UE/T66/Source/T66/UI/Screens/T66MainMenuScreen.cpp:2059), but the UI manager blocks that screen in demo via [T66UIManagerReleaseVariant.cpp](C:/UE/T66/Source/T66/UI/T66UIManagerReleaseVariant.cpp:54).

[STATIC_TRACE] The top bar omits the Minigames tab in demo: `bShowMinigamesTab = !T66DemoModeUI::IsDemoModeActive(this)` in [T66FrontendTopBarWidget.cpp](C:/UE/T66/Source/T66/UI/T66FrontendTopBarWidget.cpp:938), and the tab is only added when that boolean is true in [T66FrontendTopBarWidget.cpp](C:/UE/T66/Source/T66/UI/T66FrontendTopBarWidget.cpp:1184).

[STATIC_TRACE] Minigame demo gating is also declared at the widget-game descriptor layer, not only as a top-bar symptom: `MakeFrontendDescriptor()` assigns `DemoGateKind = FrontendMinigameLocked` in [T66WidgetGameRegistry.cpp](C:/UE/T66/Source/T66/UI/WidgetGames/T66WidgetGameRegistry.cpp:112), frontend minigames are registered with those descriptors in [T66WidgetGameRegistry.cpp](C:/UE/T66/Source/T66/UI/WidgetGames/T66WidgetGameRegistry.cpp:320), and `ResolveDemoGateID()` resolves the descriptor gate ID in [T66WidgetGameRegistry.cpp](C:/UE/T66/Source/T66/UI/WidgetGames/T66WidgetGameRegistry.cpp:381).

### 4.2 First Main-Game Run

[READ + STATIC_TRACE] The authored hero roster has 12 rows in [Heroes.csv](C:/UE/T66/Content/Data/Heroes.csv:1), with `Hero_1` at [Heroes.csv](C:/UE/T66/Content/Data/Heroes.csv:2) and `Hero_12` at [Heroes.csv](C:/UE/T66/Content/Data/Heroes.csv:13). Current demo config allows only `Hero_1` through `Hero_5` in [DefaultDemoMode.ini](C:/UE/T66/Config/DefaultDemoMode.ini:8), and the release subsystem filters hero IDs through `FilterHeroIDs()` in [T66ReleaseVariantSubsystem.cpp](C:/UE/T66/Source/T66/Core/T66ReleaseVariantSubsystem.cpp:247). The experienced demo roster is therefore 5 visible heroes and 7 demo-gated heroes.

[READ + STATIC_TRACE] The authored companion roster has 16 rows in [Companions.csv](C:/UE/T66/Content/Data/Companions.csv:1). `Companion_01` through `Companion_04` are default-unlocked rows at [Companions.csv](C:/UE/T66/Content/Data/Companions.csv:2), [Companions.csv](C:/UE/T66/Content/Data/Companions.csv:3), [Companions.csv](C:/UE/T66/Content/Data/Companions.csv:4), and [Companions.csv](C:/UE/T66/Content/Data/Companions.csv:5), and the same four IDs are the demo allow-list in [DefaultDemoMode.ini](C:/UE/T66/Config/DefaultDemoMode.ini:14). The experienced demo companion roster is 4 visible companions and 12 demo-gated companions.

[READ + STATIC_TRACE] The authored difficulty/stage loop has five difficulty bands in [DifficultyTuning.json](C:/UE/T66/Content/Data/DifficultyTuning.json:1): Easy stages 1-4 use Black base idol/weapon rarity at [DifficultyTuning.json](C:/UE/T66/Content/Data/DifficultyTuning.json:3), Medium stages 5-8 use Red at [DifficultyTuning.json](C:/UE/T66/Content/Data/DifficultyTuning.json:12), Hard stages 9-12 use Yellow at [DifficultyTuning.json](C:/UE/T66/Content/Data/DifficultyTuning.json:21), VeryHard stages 13-16 use White at [DifficultyTuning.json](C:/UE/T66/Content/Data/DifficultyTuning.json:30), and Impossible stages 17-20 use White plus final sequence at [DifficultyTuning.json](C:/UE/T66/Content/Data/DifficultyTuning.json:39). Current demo allows only Easy in [DefaultDemoMode.ini](C:/UE/T66/Config/DefaultDemoMode.ini:13).

### 4.3 Combat, Rewards, And Stage Loop

[READ + STATIC_TRACE] The main run is a stage-based horde/action loop backed by 20 stage rows in [Stages.csv](C:/UE/T66/Content/Data/Stages.csv:1). `Stage_01` is Easy/Dungeon with `Dungeon_SewerSlimeKing` at [Stages.csv](C:/UE/T66/Content/Data/Stages.csv:2), and `Stage_20` is Impossible/Hell with `Hell_GreatDragon` at [Stages.csv](C:/UE/T66/Content/Data/Stages.csv:21). Current demo exposes the Easy band, so stages 1-4 are the visible demo run path and stages 5-20 are demo-gated.

[READ + STATIC_TRACE] Combat rewards include enemy loot bags, chests, crates, and loot wheels tuned in [PlayerExperience.json](C:/UE/T66/Content/Data/PlayerExperience.json:3). Easy has `LevelUpXPThreshold` 100 at [PlayerExperience.json](C:/UE/T66/Content/Data/PlayerExperience.json:4), enemy loot bag base chance 0.1 at [PlayerExperience.json](C:/UE/T66/Content/Data/PlayerExperience.json:8), chests per stage at [PlayerExperience.json](C:/UE/T66/Content/Data/PlayerExperience.json:19), crates per stage at [PlayerExperience.json](C:/UE/T66/Content/Data/PlayerExperience.json:48), and loot wheels per stage at [PlayerExperience.json](C:/UE/T66/Content/Data/PlayerExperience.json:58).

[STATIC_TRACE] Weapon progression is hero-locked rather than free-branch: `BuildWeaponOffers()` reads the selected hero's `PrimaryCategory` and adds only one matching weapon ID for the current rarity in [T66WeaponManagerSubsystem.cpp](C:/UE/T66/Source/T66/Core/T66WeaponManagerSubsystem.cpp:52) and [T66WeaponManagerSubsystem.cpp](C:/UE/T66/Source/T66/Core/T66WeaponManagerSubsystem.cpp:69). The authored weapon table has 48 rows in [Weapons.csv](C:/UE/T66/Content/Data/Weapons.csv:1), one branch/rarity progression per hero rather than all branches per hero.

[READ + STATIC_TRACE] Idols are a separate altar/stock/equipped system. The main idol table has 16 rows in [Idols.csv](C:/UE/T66/Content/Data/Idols.csv:1), all using `Delivery=Traveler` in their rows, with `Idol_Fire_DOT` at [Idols.csv](C:/UE/T66/Content/Data/Idols.csv:2) and `Idol_Nature_Bounce` at [Idols.csv](C:/UE/T66/Content/Data/Idols.csv:17). The live idol manager supports 4 equipped slots and 16 stock slots in [T66IdolManagerSubsystem.h](C:/UE/T66/Source/T66/Core/T66IdolManagerSubsystem.h:22), and stock generation adds every idol ID into the stock array in [T66IdolManagerSubsystem.cpp](C:/UE/T66/Source/T66/Core/T66IdolManagerSubsystem.cpp:322).

[STATIC_TRACE] Boss death spawns pet capture, an idol altar, and a stage gate unless the run has reached the difficulty clear summary path. The post-boss path is visible in [T66GameMode_BossFlow.cpp](C:/UE/T66/Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp:492). The stage gate spawn path begins in [T66GameMode_BossFlow.cpp](C:/UE/T66/Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp:560).

### 4.4 Tower / Map Structure

[STATIC_TRACE] The run also has a tower-layout implementation path. When the tower main map layout is active, the game spawns descent holes in [T66GameMode_Tower.cpp](C:/UE/T66/Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp:359). Each descent hole is initialized with from-floor and destination-floor data, and may require weapon selection or guardian defeat in [T66GameMode_Tower.cpp](C:/UE/T66/Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp:820). Entering a tower descent hole updates the current floor, starts miasma after the first mob floor, spawns tower-floor enemies, and marks boss-floor entry in [T66GameMode_Tower.cpp](C:/UE/T66/Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp:1043). Evidence tier: STATIC_TRACE.

[STATIC_TRACE] The tower path can spawn vendors on mob floors and casinos with a 0.45 per-floor chance in [T66GameMode_WorldInteractables.cpp](C:/UE/T66/Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:1356) and [T66GameMode_WorldInteractables.cpp](C:/UE/T66/Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:1372). It can also spawn idol altars after tower guardian defeat in [T66GameMode_Tower.cpp](C:/UE/T66/Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp:923).

### 4.5 Economy, Vendor, Gambler

[STATIC_TRACE] The shop/vendor economy displays 5 shop slots and 5 buyback slots via constants in [T66RunStateSubsystem.h](C:/UE/T66/Source/T66/Core/T66RunStateSubsystem.h:124). Shop stage stock resets per stage and has fallback stock if the game instance is missing in [T66RunStateSubsystem_EconomyInventory.cpp](C:/UE/T66/Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp:51). Normal shop slot rarities are Black, Black, Black, Red, Yellow in [T66RunStateSubsystem_EconomyInventory.cpp](C:/UE/T66/Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp:156). Buyback display pages through the buyback pool in [T66RunStateSubsystem_EconomyInventory.cpp](C:/UE/T66/Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp:381).

[STATIC_TRACE] Stealing from the shop is a real content route, not just UI text. `ResolveShopStealAttempt()` calculates a buy price, uses player-experience/luck systems, and resolves success/failure in [T66RunStateSubsystem_EconomyInventory.cpp](C:/UE/T66/Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp:278). Vendor-token pickups are special-cased into `ApplyVendorTokenPickup()` rather than normal item inventory in [T66RunStateSubsystem_EconomyInventory.cpp](C:/UE/T66/Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp:643).

[READ + STATIC_TRACE] The casino/gambler content has six widget-game descriptors: Coin Flip, Rock Paper Scissors, BlackJack, Lottery, Plinko, and Box Opening in [T66WidgetGameRegistry.cpp](C:/UE/T66/Source/T66/UI/WidgetGames/T66WidgetGameRegistry.cpp:259). Demo config allows only CoinFlip, RockPaperScissors, and BlackJack in [DefaultDemoMode.ini](C:/UE/T66/Config/DefaultDemoMode.ini:22), so Lottery, Plinko, and Box Opening are demo-gated.

### 4.6 Pets And Companions

[READ + STATIC_TRACE] Companions are selected from the companion roster before or around the main run; 4 are demo-visible and 12 are demo-gated as described above. Pets are a separate boss-capture system. Boss death calls `TrySpawnPetCaptureForBoss()` in [T66GameMode_BossFlow.cpp](C:/UE/T66/Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp:492), which resolves a pet from the boss ID in [T66GameMode_BossFlow.cpp](C:/UE/T66/Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp:512), checks whether it was already captured in [T66GameMode_BossFlow.cpp](C:/UE/T66/Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp:520), and spawns a capture interactable in [T66GameMode_BossFlow.cpp](C:/UE/T66/Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp:536). Interacting with that capture object calls `Achievements->CapturePet()` and destroys the interactable in [T66PetCaptureInteractable.cpp](C:/UE/T66/Source/T66/Gameplay/T66PetCaptureInteractable.cpp:54).

[STATIC_TRACE] Pet data has an important fallback route: `GetPetData()` first checks `PetsDataTable`, but if no pet row exists it synthesizes `FPetData` from boss data in [T66GameInstance.cpp](C:/UE/T66/Source/T66/Core/T66GameInstance.cpp:736) and [T66GameInstance.cpp](C:/UE/T66/Source/T66/Core/T66GameInstance.cpp:753). No loose `Content/Data/Pets.csv` exists in this working tree. This is not necessarily broken, but it is a content/inventory cross-check risk.

## 5. Content Surface Map By Domain

| Domain | Final count | Current status summary | Visibility route | Evidence |
|---|---:|---|---|---|
| Heroes | 12 rows | 5 `ACTIVE`, 7 `DEMO_GATED` | `VISIBLE_DEMO`, `DEMO_OMITTED` | [Heroes.csv](C:/UE/T66/Content/Data/Heroes.csv:1), [DefaultDemoMode.ini](C:/UE/T66/Config/DefaultDemoMode.ini:8) |
| Weapons | 48 rows | `ACTIVE` for allowed hero/difficulty lanes; other hero/difficulty lanes `DEMO_GATED` | `RUN_REWARD`, `DEMO_OMITTED` | [Weapons.csv](C:/UE/T66/Content/Data/Weapons.csv:1), [T66WeaponManagerSubsystem.cpp](C:/UE/T66/Source/T66/Core/T66WeaponManagerSubsystem.cpp:52) |
| Idols | 16 rows | `ACTIVE`; rarity progression beyond Easy is difficulty-gated | `RUN_REWARD` | [Idols.csv](C:/UE/T66/Content/Data/Idols.csv:1), [T66IdolManagerSubsystem.h](C:/UE/T66/Source/T66/Core/T66IdolManagerSubsystem.h:22) |
| Main items | 30 rows | `ACTIVE`, with special/hidden runtime rows | `RUN_REWARD`, `WORLD_INTERACTION` | [Items.csv](C:/UE/T66/Content/Data/Items.csv:1), [T66RunStateSubsystem_EconomyInventory.cpp](C:/UE/T66/Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp:643) |
| Companions | 16 rows | 4 `ACTIVE`, 12 `DEMO_GATED` | `VISIBLE_DEMO`, `DEMO_OMITTED` | [Companions.csv](C:/UE/T66/Content/Data/Companions.csv:1), [DefaultDemoMode.ini](C:/UE/T66/Config/DefaultDemoMode.ini:14) |
| Enemies | 60 rows | Easy slice `ACTIVE`; non-Easy rows `DEMO_GATED`; status effects `PARTIAL` | `RUN_REWARD` | [Enemies.csv](C:/UE/T66/Content/Data/Enemies.csv:1), [pending_issues_Data.md](C:/UE/T66/Content/Data/pending_issues_Data.md:3) |
| Bosses | 23 rows | Easy bosses `ACTIVE`; non-Easy/finale rows `DEMO_GATED` | `RUN_REWARD` | [Bosses.csv](C:/UE/T66/Content/Data/Bosses.csv:1), [BossEncounters.csv](C:/UE/T66/Content/Data/BossEncounters.csv:1) |
| Stages | 20 rows | 4 `ACTIVE`, 16 `DEMO_GATED` | `VISIBLE_DEMO`, `DEMO_OMITTED` | [Stages.csv](C:/UE/T66/Content/Data/Stages.csv:1), [DifficultyTuning.json](C:/UE/T66/Content/Data/DifficultyTuning.json:1) |
| Boss attacks | 50 rows | `ACTIVE` data for boss runtime; non-demo bosses gated | `RUN_REWARD` | [BossAttacks.csv](C:/UE/T66/Content/Data/BossAttacks.csv:1) |
| Boss definitions | 25 attack definitions, 6 hazard definitions, 8 movement patterns, 20 encounters | `ACTIVE`, with non-demo use gated by boss/stage reachability | `RUN_REWARD` | [BossAttackDefinitions.csv](C:/UE/T66/Content/Data/BossAttackDefinitions.csv:1), [BossHazardDefinitions.csv](C:/UE/T66/Content/Data/BossHazardDefinitions.csv:1), [BossMovementPatterns.csv](C:/UE/T66/Content/Data/BossMovementPatterns.csv:1), [BossEncounters.csv](C:/UE/T66/Content/Data/BossEncounters.csv:1) |
| Status effects | 12 rows | `PARTIAL`; authored but production mob assignment flagged | `DATA_ONLY` for most effects | [StatusEffects.csv](C:/UE/T66/Content/Data/StatusEffects.csv:1), [pending_issues_Data.md](C:/UE/T66/Content/Data/pending_issues_Data.md:3) |
| Combat VFX bindings | 20 rows | `ACTIVE` weapon-base bindings, some placeholder reuse | `RUN_REWARD` | [CombatVFXBindings.csv](C:/UE/T66/Content/Data/CombatVFXBindings.csv:1) |
| Casino/gambler | 6 descriptor games | 3 `ACTIVE`, 3 `DEMO_GATED` | `WORLD_INTERACTION` | [T66WidgetGameRegistry.cpp](C:/UE/T66/Source/T66/UI/WidgetGames/T66WidgetGameRegistry.cpp:259), [DefaultDemoMode.ini](C:/UE/T66/Config/DefaultDemoMode.ini:22) |
| Arcade / Versus | 14 JSON interactable entries plus registry prototypes | `DEPRECATED` | `DEPRECATED_DISABLED` | [DefaultGame.ini](C:/UE/T66/Config/DefaultGame.ini:64), [DEPRECATED_CONTENT.md](C:/UE/T66/Demo/DEPRECATED_CONTENT.md:33) |
| Minigame hub | 5 cards | `DEMO_GATED`, one card also `DEPRECATED` for Versus/Arcade | `DIRECT_NAV_BLOCKED_DEMO`, `FULL_GAME_ROUTE` | [T66MinigamesScreen.cpp](C:/UE/T66/Source/T66/UI/Screens/T66MinigamesScreen.cpp:295), [T66UIManagerReleaseVariant.cpp](C:/UE/T66/Source/T66/UI/T66UIManagerReleaseVariant.cpp:11) |
| T66Mini | 12 heroes, 16 idols, 31 items, 24 companions, 4 enemies, 66 bosses, 50 stages, 50 waves, 295 tuning rows, 6 circus games | Current demo `DEMO_GATED`; full-game route `ACTIVE`; boss-table reachability `ORPHAN_SUSPECT` | `DIRECT_NAV_BLOCKED_DEMO`, `FULL_GAME_ROUTE` | [T66MiniDataSubsystem.cpp](C:/UE/T66/Source/T66Mini/Private/Core/T66MiniDataSubsystem.cpp:179), [T66MiniBattleScreen.cpp](C:/UE/T66/Source/T66Mini/Private/UI/Screens/T66MiniBattleScreen.cpp:367) |
| T66TD | 12 heroes, 12 hero-combat rows, 5 enemy archetypes, 96 battle-tuning rows, 5 difficulties, 20 maps, 20 stages, 20 layout records | Current demo `DEMO_GATED`; full-game route `ACTIVE` by static trace | `DIRECT_NAV_BLOCKED_DEMO`, `FULL_GAME_ROUTE` | [T66TDDataSubsystem.cpp](C:/UE/T66/Source/T66TD/Private/Core/T66TDDataSubsystem.cpp:31), [T66TDBattleScreen.cpp](C:/UE/T66/Source/T66TD/Private/UI/Screens/T66TDBattleScreen.cpp:1) |
| T66Idle | 5 heroes, 4 companions, 4 items, 4 idols, 4 enemies, 1 zone, 10 stages, 1 tuning row | Current demo `DEMO_GATED`; full-game route `PARTIAL` by content scale and no runtime check | `DIRECT_NAV_BLOCKED_DEMO`, `FULL_GAME_ROUTE` | [T66IdleDataSubsystem.cpp](C:/UE/T66/Source/T66Idle/Private/Core/T66IdleDataSubsystem.cpp:26), [T66IdleMainMenuScreen.cpp](C:/UE/T66/Source/T66Idle/Private/UI/Screens/T66IdleMainMenuScreen.cpp:337) |
| T66Deck | 3 heroes, 3 companions, 8 cards, 2 relics, 7 enemies, 3 items, 5 encounters, 3 starting decks, 1 stage, 1 tuning row | Current demo `DEMO_GATED`; full-game route `PARTIAL` by prototype-scale data | `DIRECT_NAV_BLOCKED_DEMO`, `FULL_GAME_ROUTE` | [T66DeckDataSubsystem.cpp](C:/UE/T66/Source/T66Deck/Private/Core/T66DeckDataSubsystem.cpp:44), [T66DeckMainMenuScreen.cpp](C:/UE/T66/Source/T66Deck/Private/UI/Screens/T66DeckMainMenuScreen.cpp:324) |

## 6. Element Cards - Main Game

### CONTENT-RELEASE-001 - Current Release / Demo Slice

- Player-visible behavior: forced demo mode filters hero, companion, difficulty, minigame, daily, lab, collector, and casino routes.
- Lifecycle status: `ACTIVE`.
- Evidence tier: READ + STATIC_TRACE.
- Visibility route: `VISIBLE_DEMO`, `DEMO_OMITTED`, `DIRECT_NAV_BLOCKED_DEMO`.
- Connections: all content-area cards inherit this release slice unless noted.
- Suspected issues: see `CONTENTFIND-001`, `CONTENTFIND-002`, `CONTENTFIND-003`.
- Evidence: [DefaultDemoMode.ini](C:/UE/T66/Config/DefaultDemoMode.ini:2), [T66ReleaseVariantSubsystem.cpp](C:/UE/T66/Source/T66/Core/T66ReleaseVariantSubsystem.cpp:145), [T66UIManagerReleaseVariant.cpp](C:/UE/T66/Source/T66/UI/T66UIManagerReleaseVariant.cpp:38).
- Cross-refs: TECH-RELEASE-001, INV-RELEASE-001.

### CONTENT-HERO-001 - Main Hero Roster

- Player-visible behavior: players choose one hero for a main run. Demo shows `Hero_1` to `Hero_5`; the authored table has 12 heroes.
- Lifecycle status: `ACTIVE`, `DEMO_GATED`.
- Evidence tier: READ + STATIC_TRACE.
- Visibility route: `VISIBLE_DEMO` for 5 allowed heroes; `DEMO_OMITTED` for 7 other heroes.
- Connections: selected hero drives weapon offer branch through `PrimaryCategory`, portraits, base stats, ultimate/passive, companion/pet context, and run start.
- Suspected issues: several hero rows use portrait paths whose folder numbers differ from row IDs, including at least demo-visible remaps `Hero_1` to `Hero_4` art, `Hero_4` to `Hero_6` art, and `Hero_5` to `Hero_1` art in [Heroes.csv](C:/UE/T66/Content/Data/Heroes.csv:2), [Heroes.csv](C:/UE/T66/Content/Data/Heroes.csv:5), and [Heroes.csv](C:/UE/T66/Content/Data/Heroes.csv:6). This may be intentional systematic remapping, but it is a content visibility risk for Inventory to verify. See `CONTENTFIND-014`.
- Evidence: [Heroes.csv](C:/UE/T66/Content/Data/Heroes.csv:1), [DefaultDemoMode.ini](C:/UE/T66/Config/DefaultDemoMode.ini:8), [T66ReleaseVariantSubsystem.cpp](C:/UE/T66/Source/T66/Core/T66ReleaseVariantSubsystem.cpp:247), [T66GameInstance.cpp](C:/UE/T66/Source/T66/Core/T66GameInstance.cpp:1253).
- Cross-refs: TECH-HERO-001, INV-HERO-001.

### CONTENT-WEAPON-001 - Hero-Locked Weapon Progression

- Player-visible behavior: each hero receives a weapon offer in the hero's locked attack category at the run's current rarity band. The authored table has four rarity rows per hero, for 48 total rows.
- Lifecycle status: `ACTIVE`, `DEMO_GATED`.
- Evidence tier: READ + STATIC_TRACE.
- Visibility route: `RUN_REWARD`; non-demo hero and non-Easy rarity bands are `DEMO_OMITTED` through hero/difficulty gating.
- Connections: hero `PrimaryCategory`, difficulty tuning `WeaponBaseRarity`, combat VFX bindings, combat component auto-attack behavior.
- Suspected issues: no issue confirmed in this pass; runtime proof should confirm every allowed demo hero gets exactly one valid Black-tier weapon offer.
- Evidence: [Weapons.csv](C:/UE/T66/Content/Data/Weapons.csv:1), [T66WeaponManagerSubsystem.cpp](C:/UE/T66/Source/T66/Core/T66WeaponManagerSubsystem.cpp:52), [DifficultyTuning.json](C:/UE/T66/Content/Data/DifficultyTuning.json:6).
- Cross-refs: TECH-WEAPON-001, INV-WEAPON-001.

### CONTENT-IDOL-001 - Main Idol System

- Player-visible behavior: idols are selected/equipped through altar or stock UI after boss/guardian progression. There are 16 idols, 4 equipped slots, and 16 stock slots. All current idol CSV rows use `Delivery=Traveler`.
- Lifecycle status: `ACTIVE`.
- Evidence tier: READ + STATIC_TRACE.
- Visibility route: `RUN_REWARD`, `WORLD_INTERACTION`.
- Connections: combat component traveler delivery, elemental power stats, boss altar spawns, difficulty rarity tuning.
- Suspected issues: source comments still say traveler delivery is reserved/inert, but current combat code dispatches traveler idols. See `CONTENTFIND-004`.
- Evidence: [Idols.csv](C:/UE/T66/Content/Data/Idols.csv:1), [T66IdolManagerSubsystem.h](C:/UE/T66/Source/T66/Core/T66IdolManagerSubsystem.h:22), [T66IdolManagerSubsystem.cpp](C:/UE/T66/Source/T66/Core/T66IdolManagerSubsystem.cpp:322), [T66CombatComponent.cpp](C:/UE/T66/Source/T66/Gameplay/T66CombatComponent.cpp:4105).
- Cross-refs: TECH-IDOL-001, INV-IDOL-001.

### CONTENT-ITEM-001 - Main Items, Shop, Drops, Special Items

- Player-visible behavior: 30 item rows feed loot, shop, buyback, pickups, and special effects. Shop presents 5 slots and buyback presents 5 slots. Special rows include Backrooms quick revive and Vendor Token.
- Lifecycle status: `ACTIVE`, `HIDDEN_RUNTIME`, `PARTIAL`.
- Evidence tier: READ + STATIC_TRACE.
- Visibility route: `RUN_REWARD`, `WORLD_INTERACTION`, `FALLBACK_ROUTE`.
- Connections: shop stock, mob/chest/crate/loot-wheel rewards, vendor failed-steal route, inventory/stat pipeline.
- Suspected issues: pending data issues report missing item row references and legacy sprite reuse; see `CONTENTFIND-007`.
- Evidence: [Items.csv](C:/UE/T66/Content/Data/Items.csv:1), [T66RunStateSubsystem.h](C:/UE/T66/Source/T66/Core/T66RunStateSubsystem.h:124), [T66RunStateSubsystem_EconomyInventory.cpp](C:/UE/T66/Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp:51), [T66RunStateSubsystem_EconomyInventory.cpp](C:/UE/T66/Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp:643), [pending_issues_Data.md](C:/UE/T66/Content/Data/pending_issues_Data.md:10).
- Cross-refs: TECH-ITEM-001, INV-ITEM-001.

### CONTENT-COMPANION-001 - Main Companions

- Player-visible behavior: players can select a companion for the run. Four companions are default-unlocked and demo-visible; 12 are authored but demo-gated.
- Lifecycle status: `ACTIVE`, `DEMO_GATED`.
- Evidence tier: READ + STATIC_TRACE.
- Visibility route: `VISIBLE_DEMO`, `DEMO_OMITTED`.
- Connections: hero selection/party selection, minigame copies, run context.
- Suspected issues: several later companion rows reuse early portrait paths, for example `Companion_09` references `Companion_01` art in [Companions.csv](C:/UE/T66/Content/Data/Companions.csv:10). This may be intentional placeholder reuse, but it needs Inventory confirmation. See `CONTENTFIND-014`.
- Evidence: [Companions.csv](C:/UE/T66/Content/Data/Companions.csv:1), [DefaultDemoMode.ini](C:/UE/T66/Config/DefaultDemoMode.ini:14), [T66ReleaseVariantSubsystem.cpp](C:/UE/T66/Source/T66/Core/T66ReleaseVariantSubsystem.cpp:272), [T66GameInstance.cpp](C:/UE/T66/Source/T66/Core/T66GameInstance.cpp:1356).
- Cross-refs: TECH-COMPANION-001, INV-COMPANION-001.

### CONTENT-ENEMY-001 - Standard Enemy Roster

- Player-visible behavior: enemies populate stages/tower floors. The data table has 60 standard enemy rows across 5 difficulties and 5 themes.
- Lifecycle status: `ACTIVE`, `DEMO_GATED`, `PARTIAL`.
- Evidence tier: READ + STATIC_TRACE.
- Visibility route: `RUN_REWARD`; Easy enemies are demo-reachable through Easy stages; non-Easy enemy rows are demo-gated.
- Connections: stages, enemy director, mob families, status effects, boss/miniboss/tower floors.
- Suspected issues: status effects are authored but not assigned to production mobs per pending issue; spawn director still uses fallback-family behavior. See `CONTENTFIND-005` and `CONTENTFIND-006`.
- Evidence: [Enemies.csv](C:/UE/T66/Content/Data/Enemies.csv:1), [Stages.csv](C:/UE/T66/Content/Data/Stages.csv:2), [pending_issues_Data.md](C:/UE/T66/Content/Data/pending_issues_Data.md:3), [pending_issues_Gameplay.md](C:/UE/T66/Source/T66/Gameplay/pending_issues_Gameplay.md:116).
- Cross-refs: TECH-ENEMY-001, INV-ENEMY-001.

### CONTENT-BOSS-001 - Bosses And Boss Encounters

- Player-visible behavior: bosses end stages, spawn post-boss rewards, gate stage progression, and can spawn pet capture objects. There are 23 boss rows and 20 boss encounter rows.
- Lifecycle status: `ACTIVE`, `DEMO_GATED`.
- Evidence tier: READ + STATIC_TRACE.
- Visibility route: `RUN_REWARD`.
- Connections: stages, boss attacks, boss movement patterns, hazards, pet capture, idol altar, stage gate, victory summary.
- Suspected issues: no boss-route break confirmed in source pass, but non-Easy/finale boss content is currently demo-gated and needs runtime verification later.
- Evidence: [Bosses.csv](C:/UE/T66/Content/Data/Bosses.csv:1), [BossEncounters.csv](C:/UE/T66/Content/Data/BossEncounters.csv:1), [BossAttacks.csv](C:/UE/T66/Content/Data/BossAttacks.csv:1), [BossAttackDefinitions.csv](C:/UE/T66/Content/Data/BossAttackDefinitions.csv:1), [T66GameMode_BossFlow.cpp](C:/UE/T66/Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp:492).
- Cross-refs: TECH-BOSS-001, INV-BOSS-001.

### CONTENT-TOWER-001 - Tower / Stage Map Structure

- Player-visible behavior: when tower main-map layout is active, players move through tower floors using descent holes, fight floor populations/guardians, and trigger boss floor entry.
- Lifecycle status: `ACTIVE`, `PARTIAL`.
- Evidence tier: STATIC_TRACE.
- Visibility route: `RUN_REWARD`, `WORLD_INTERACTION`.
- Connections: stage data, enemy director, tower miasma, vendors, casinos, idol altars, boss entry.
- Suspected issues: pending gameplay issues flag possible floor seams and split drop-hole floor rectangles. See `CONTENTFIND-008`.
- Evidence: [T66GameMode_Tower.cpp](C:/UE/T66/Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp:359), [T66GameMode_Tower.cpp](C:/UE/T66/Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp:820), [T66GameMode_Tower.cpp](C:/UE/T66/Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp:1043), [pending_issues_Gameplay.md](C:/UE/T66/Source/T66/Gameplay/pending_issues_Gameplay.md:137).
- Cross-refs: TECH-TOWER-001, INV-TOWER-001.

### CONTENT-ECONOMY-001 - Gold, Loot, Shop, Buyback

- Player-visible behavior: players gain gold/material/value from drops and interactables, buy/sell shop items, page buyback, and can encounter luck/steal outcomes.
- Lifecycle status: `ACTIVE`.
- Evidence tier: READ + STATIC_TRACE.
- Visibility route: `RUN_REWARD`, `WORLD_INTERACTION`.
- Connections: item rows, player experience tuning, vendor, gambler/casino, anti-cheat/luck logs.
- Suspected issues: no direct economy break confirmed in this pass; pending tuning/load-order issue can affect player-experience requests. See `CONTENTFIND-006`.
- Evidence: [PlayerExperience.json](C:/UE/T66/Content/Data/PlayerExperience.json:3), [T66RunStateSubsystem.h](C:/UE/T66/Source/T66/Core/T66RunStateSubsystem.h:124), [T66RunStateSubsystem_EconomyInventory.cpp](C:/UE/T66/Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp:156), [pending_issues_Gameplay.md](C:/UE/T66/Source/T66/Gameplay/pending_issues_Gameplay.md:130).
- Cross-refs: TECH-ECONOMY-001, INV-ECONOMY-001.

### CONTENT-VENDOR-001 - Vendor And Failed-Steal Content

- Player-visible behavior: vendors spawn on tower mob floors and expose shop/buyback/steal behaviors; failed stealing can connect to hidden vendor boss/token routes.
- Lifecycle status: `ACTIVE`, `HIDDEN_RUNTIME`.
- Evidence tier: STATIC_TRACE.
- Visibility route: `WORLD_INTERACTION`.
- Connections: shop stock, vendor token item, hidden boss route, tower floors, gambler/casino shell.
- Suspected issues: the failed-steal/vendor-boss route is not obvious from top-level data and should be runtime-verified as a hidden content path. See `CONTENTFIND-015`.
- Evidence: [T66GameMode_WorldInteractables.cpp](C:/UE/T66/Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:1356), [T66RunStateSubsystem_EconomyInventory.cpp](C:/UE/T66/Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp:278), [T66RunStateSubsystem_EconomyInventory.cpp](C:/UE/T66/Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp:643).
- Cross-refs: TECH-VENDOR-001, INV-VENDOR-001.

### CONTENT-GAMBLER-001 - Casino / Gambler

- Player-visible behavior: casino interactables/NPC shell expose wagered minigames. Demo allows Coin Flip, Rock Paper Scissors, and BlackJack; Lottery, Plinko, and Box Opening are authored but demo-gated.
- Lifecycle status: `ACTIVE`, `DEMO_GATED`.
- Evidence tier: READ + STATIC_TRACE.
- Visibility route: `WORLD_INTERACTION`.
- Connections: casino interactables, widget-game registry, backend score/result reporting, buyback tab, vendor/casino shell.
- Suspected issues: no active break confirmed; exact availability of the three demo-allowed games needs runtime UI verification.
- Evidence: [T66WidgetGameRegistry.cpp](C:/UE/T66/Source/T66/UI/WidgetGames/T66WidgetGameRegistry.cpp:259), [DefaultDemoMode.ini](C:/UE/T66/Config/DefaultDemoMode.ini:22), [T66GameMode_WorldInteractables.cpp](C:/UE/T66/Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:346).
- Cross-refs: TECH-GAMBLER-001, INV-GAMBLER-001.

### CONTENT-PET-001 - Pets / Boss Capture

- Player-visible behavior: after defeating a boss, a capture interactable can spawn for the boss pet unless the pet was already captured; captured pets can be selected/equipped later.
- Lifecycle status: `ACTIVE`, `PARTIAL`.
- Evidence tier: STATIC_TRACE.
- Visibility route: `RUN_REWARD`, `FALLBACK_ROUTE`.
- Connections: boss data, achievements profile, pet selection screen, pet actor spawn in run.
- Suspected issues: no loose `Pets.csv` was found; current code can synthesize pet data from boss rows if no pets table row exists. This is functional by static trace, but it is a content/inventory risk. See `CONTENTFIND-013`.
- Evidence: [T66GameMode_BossFlow.cpp](C:/UE/T66/Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp:503), [T66GameInstance.cpp](C:/UE/T66/Source/T66/Core/T66GameInstance.cpp:729), [T66PetCaptureInteractable.cpp](C:/UE/T66/Source/T66/Gameplay/T66PetCaptureInteractable.cpp:54), [T66PetSelectionScreen.cpp](C:/UE/T66/Source/T66/UI/Screens/T66PetSelectionScreen.cpp:547).
- Cross-refs: TECH-PET-001, INV-PET-001.

### CONTENT-UI-001 - Player-Facing Screens And Flow

- Player-visible behavior: UI controls main menu, selection screens, top bar, Minigames hub, casino/gambler tab, pet selection, run summary, and minigame frontend screens.
- Lifecycle status: `ACTIVE`, `DEMO_GATED`, `PARTIAL`.
- Evidence tier: STATIC_TRACE.
- Visibility route: `VISIBLE_DEMO`, `DEMO_OMITTED`, `DIRECT_NAV_BLOCKED_DEMO`.
- Connections: release-variant subsystem, widget-game registry, top bar, screen navigation.
- Suspected issues: pending UI issues flag missing controller-focus contract and missing loot-wheel boost toast. See `CONTENTFIND-009`.
- Evidence: [T66UIManagerReleaseVariant.cpp](C:/UE/T66/Source/T66/UI/T66UIManagerReleaseVariant.cpp:38), [T66FrontendTopBarWidget.cpp](C:/UE/T66/Source/T66/UI/T66FrontendTopBarWidget.cpp:1184), [pending_issues_UI.md](C:/UE/T66/Source/T66/UI/pending_issues_UI.md:3).
- Cross-refs: TECH-UI-001, INV-UI-001.

### CONTENT-PROGRESSION-001 - Difficulty / Meta / Run Progression

- Player-visible behavior: difficulty bands map to stage ranges, rarity bands, final sequence status, and demo visibility. Demo only exposes Easy. Lab run and collector are disabled by demo config.
- Lifecycle status: `ACTIVE`, `DEMO_GATED`.
- Evidence tier: READ + STATIC_TRACE.
- Visibility route: `VISIBLE_DEMO`, `DEMO_OMITTED`.
- Connections: stages, bosses, weapon rarity, idol rarity, hero/companion filters, Daily Descent, lab, collector.
- Suspected issues: no progression break confirmed; player-experience tuning may be requested before table availability per pending issue.
- Evidence: [DifficultyTuning.json](C:/UE/T66/Content/Data/DifficultyTuning.json:1), [DefaultDemoMode.ini](C:/UE/T66/Config/DefaultDemoMode.ini:25), [T66ReleaseVariantSubsystem.cpp](C:/UE/T66/Source/T66/Core/T66ReleaseVariantSubsystem.cpp:220), [pending_issues_Gameplay.md](C:/UE/T66/Source/T66/Gameplay/pending_issues_Gameplay.md:130).
- Cross-refs: TECH-PROGRESSION-001, INV-PROGRESSION-001.

## 7. Element Cards - Deprecated / Compatibility Content

### CONTENT-DEPRECATED-001 - Arcade / Versus Prototype Content

- Player-visible behavior: the Minigames hub still includes a Versus card text for a direct arcade prototype launcher, but arcade games and arcade interactables are centrally disabled.
- Lifecycle status: `DEPRECATED`, `COMPAT_LEGACY`.
- Evidence tier: READ + STATIC_TRACE.
- Visibility route: `DEPRECATED_DISABLED`.
- Connections: deprecated feature settings, arcade interactable JSON, widget-game registry, Minigames hub Versus card.
- Suspected issues: demo config still allow-lists arcade IDs, but the central deprecated gate wins. See `CONTENTFIND-003`.
- Evidence: [DefaultGame.ini](C:/UE/T66/Config/DefaultGame.ini:64), [DEPRECATED_CONTENT.md](C:/UE/T66/Demo/DEPRECATED_CONTENT.md:33), [T66MinigamesScreen.cpp](C:/UE/T66/Source/T66/UI/Screens/T66MinigamesScreen.cpp:297), [T66MinigamesScreen.cpp](C:/UE/T66/Source/T66/UI/Screens/T66MinigamesScreen.cpp:478).
- Cross-refs: TECH-DEPRECATED-001, INV-DEPRECATED-001.

### CONTENT-COMPAT-001 - Legacy Data Fields And IDs

- Player-visible behavior: none by itself. These fields exist so old rows/save/data can still deserialize.
- Lifecycle status: `COMPAT_LEGACY`.
- Evidence tier: READ.
- Visibility route: `COMPAT_ONLY`.
- Connections: hero stat gain ranges, secondary stat enum values, legacy item template fields, idol max-level field, enemy projectile base, widget-game legacy IDs.
- Suspected issues: these fields can be confused with active content if audited by names alone.
- Evidence: [DEPRECATED_CONTENT.md](C:/UE/T66/Demo/DEPRECATED_CONTENT.md:50), [DEPRECATED_CONTENT.md](C:/UE/T66/Demo/DEPRECATED_CONTENT.md:58).
- Cross-refs: TECH-COMPAT-001, INV-COMPAT-001.

## 8. Element Cards - Minigame Access Layer

### CONTENT-MINI-001 - Minigames Hub / Route

- Player-visible behavior: in the full-game branch, the Minigames screen shows five cards: Versus, Mini, TD, Deck, and Idle. In current demo mode, the top-bar tab is omitted and direct navigation to Minigames/minigame screens is blocked.
- Lifecycle status: `DEMO_GATED`, `DEPRECATED` for the Versus/Arcade card.
- Evidence tier: READ + STATIC_TRACE.
- Visibility route: `DIRECT_NAV_BLOCKED_DEMO`, `FULL_GAME_ROUTE`, `DEPRECATED_DISABLED`.
- Connections: release variant, top bar, UI manager, widget-game registry, each minigame module. The registry source is `FrontendMinigameLocked`, not the trailing `MakeFrontendDescriptor()` booleans, which are capability flags (`bUsesCustomPaint`, `bUsesPersistentRun`) in [T66WidgetGameRegistry.cpp](C:/UE/T66/Source/T66/UI/WidgetGames/T66WidgetGameRegistry.cpp:100).
- Suspected issues: UI text still advertises Versus/Arcade prototypes while arcade is centrally disabled. See `CONTENTFIND-003`.
- Evidence: [T66MinigamesScreen.cpp](C:/UE/T66/Source/T66/UI/Screens/T66MinigamesScreen.cpp:295), [T66MinigamesScreen.cpp](C:/UE/T66/Source/T66/UI/Screens/T66MinigamesScreen.cpp:489), [T66UIManagerReleaseVariant.cpp](C:/UE/T66/Source/T66/UI/T66UIManagerReleaseVariant.cpp:11), [T66FrontendTopBarWidget.cpp](C:/UE/T66/Source/T66/UI/T66FrontendTopBarWidget.cpp:1184).
- Cross-refs: TECH-MINI-001, INV-MINI-001.

## 9. Element Cards - Minigame Modules

### CONTENT-MINI-002 - T66Mini / Chadpocalypse Mini

- Player-visible behavior: a full standalone 2D survivor-style minigame with its own hero, companion, difficulty, idol, shop, battle, save, summary, circus, runtime tuning, and visual systems. In current demo, the route is hidden/blocked.
- Lifecycle status: `DEMO_GATED`; full-game branch is `ACTIVE` by static trace; boss table reachability has `ORPHAN_SUSPECT`.
- Evidence tier: READ + STATIC_TRACE.
- Visibility route: `DIRECT_NAV_BLOCKED_DEMO`, `FULL_GAME_ROUTE`, `DATA_ONLY` for some boss rows.
- Counts: 12 heroes, 16 idols, 31 items, 24 companions, 4 enemies, 66 bosses, 50 stages, 50 waves, 295 runtime-tuning rows, 4 interactables, 6 circus games. Evidence starts at [T66Mini_Heroes.csv](C:/UE/T66/Content/Mini/Data/T66Mini_Heroes.csv:1), [T66Mini_Idols.csv](C:/UE/T66/Content/Mini/Data/T66Mini_Idols.csv:1), [T66Mini_Items.csv](C:/UE/T66/Content/Mini/Data/T66Mini_Items.csv:1), [T66Mini_Bosses.csv](C:/UE/T66/Content/Mini/Data/T66Mini_Bosses.csv:1), [T66Mini_Stages.csv](C:/UE/T66/Content/Mini/Data/T66Mini_Stages.csv:1), and [T66Mini_Waves.csv](C:/UE/T66/Content/Mini/Data/T66Mini_Waves.csv:1).
- Connections: Minigames hub, widget-game registry, mini frontend state, mini save, mini battle simulation, mini visual subsystem, mini data subsystem.
- Suspected issues: 66 boss rows are loaded, but current stage/wave route references only `Boss_05`, `Boss_10`, `Boss_15`, `Boss_20`, and `Boss_25` by CSV count. Current code finds bosses by stage/wave `BossID`, so the remaining boss rows are `ORPHAN_SUSPECT` until a runtime/randomization route is proven. See `CONTENTFIND-010`.
- Evidence: [T66Mini_MasterImplementation.md](C:/UE/T66/Gameplay/Minigames/Mini/T66Mini_MasterImplementation.md:166), [T66MiniBattleScreen.cpp](C:/UE/T66/Source/T66Mini/Private/UI/Screens/T66MiniBattleScreen.cpp:367), [T66MiniDataSubsystem.cpp](C:/UE/T66/Source/T66Mini/Private/Core/T66MiniDataSubsystem.cpp:179), [T66MiniBattleScreen.cpp](C:/UE/T66/Source/T66Mini/Private/UI/Screens/T66MiniBattleScreen.cpp:1953), [T66Mini_GapChecklist.md](C:/UE/T66/Gameplay/Minigames/Mini/T66Mini_GapChecklist.md:61).
- Cross-refs: TECH-MINI-002, INV-MINI-002.

### CONTENT-MINI-003 - T66TD / Chadpocalypse TD

- Player-visible behavior: a tower-defense minigame with hero placement, enemy waves, upgrades, maps, stages, save state, and a battle screen. In current demo, the route is hidden/blocked.
- Lifecycle status: `DEMO_GATED`; full-game branch is `ACTIVE` by static trace.
- Evidence tier: READ + STATIC_TRACE.
- Visibility route: `DIRECT_NAV_BLOCKED_DEMO`, `FULL_GAME_ROUTE`.
- Counts: 12 hero rows, 12 hero combat rows, 5 enemy archetypes, 96 battle-tuning rows, 5 difficulties, 20 maps, 20 stages, 5 theme modifier rows, and a JSON layout set. Evidence starts at [T66TD_Heroes.csv](C:/UE/T66/Content/TD/Data/T66TD_Heroes.csv:1), [T66TD_Maps.csv](C:/UE/T66/Content/TD/Data/T66TD_Maps.csv:1), and [T66TD_Stages.csv](C:/UE/T66/Content/TD/Data/T66TD_Stages.csv:1).
- Connections: Minigames hub, TD data subsystem, TD save subsystem, TD visual subsystem, TD battle screen.
- Suspected issues: no content break confirmed in source pass; runtime verification should prove stage/map selection and wave completion.
- Evidence: [T66WidgetGameRegistry.cpp](C:/UE/T66/Source/T66/UI/WidgetGames/T66WidgetGameRegistry.cpp:320), [T66TDDataSubsystem.cpp](C:/UE/T66/Source/T66TD/Private/Core/T66TDDataSubsystem.cpp:31), [T66TDBattleScreen.cpp](C:/UE/T66/Source/T66TD/Private/UI/Screens/T66TDBattleScreen.cpp:1), [T66TDSaveSubsystem.cpp](C:/UE/T66/Source/T66TD/Private/Save/T66TDSaveSubsystem.cpp:43).
- Cross-refs: TECH-MINI-003, INV-MINI-003.

### CONTENT-MINI-004 - T66Idle / Idle Chadpocalypse

- Player-visible behavior: an idle progression minigame with main menu, gameplay mode, summary, tap attack, upgrade buttons, passive tick, stage progression, purchases, local profile, and offline-capable save state. In current demo, the route is hidden/blocked.
- Lifecycle status: `DEMO_GATED`, `PARTIAL`.
- Evidence tier: READ + STATIC_TRACE.
- Visibility route: `DIRECT_NAV_BLOCKED_DEMO`, `FULL_GAME_ROUTE`.
- Counts: 5 heroes, 4 companions, 4 items, 4 idols, 4 enemies, 1 zone, 10 stages, 1 tuning row. Evidence starts at [T66Idle_Heroes.csv](C:/UE/T66/Content/Idle/Data/T66Idle_Heroes.csv:1) and [T66Idle_Stages.csv](C:/UE/T66/Content/Idle/Data/T66Idle_Stages.csv:1).
- Connections: Minigames hub, Idle data/save/frontend subsystems, profile save, backend leaderboard.
- Suspected issues: current authored data and gameplay code outgrew the master doc, which still says infrastructure-only/no finished gameplay loop. Idle gameplay is screen-owned inside `T66IdleMainMenuScreen` rather than a separate `BattleScreen` class, unlike Mini/TD, which is one reason this audit keeps the `PARTIAL` tag until runtime proof. `Hero_12` is named `Goblino Chad` in Idle data while main `Hero_12` is `Roach Chad` and main `Hero_9` is `Goblino Chad`. See `CONTENTFIND-011`.
- Evidence: [T66WidgetGameRegistry.cpp](C:/UE/T66/Source/T66/UI/WidgetGames/T66WidgetGameRegistry.cpp:348), [T66IdleDataSubsystem.cpp](C:/UE/T66/Source/T66Idle/Private/Core/T66IdleDataSubsystem.cpp:26), [T66IdleMainMenuScreen.cpp](C:/UE/T66/Source/T66Idle/Private/UI/Screens/T66IdleMainMenuScreen.cpp:337), [T66IdleMainMenuScreen.cpp](C:/UE/T66/Source/T66Idle/Private/UI/Screens/T66IdleMainMenuScreen.cpp:816), [T66IdleMainMenuScreen.h](C:/UE/T66/Source/T66Idle/Public/UI/Screens/T66IdleMainMenuScreen.h:40), [T66Idle_MasterImplementation.md](C:/UE/T66/Gameplay/Minigames/Idle/T66Idle_MasterImplementation.md:7), [T66Idle_Heroes.csv](C:/UE/T66/Content/Idle/Data/T66Idle_Heroes.csv:6).
- Cross-refs: TECH-MINI-004, INV-MINI-004.

### CONTENT-MINI-005 - T66Deck / Chadpocalypse Deck Builder

- Player-visible behavior: a dungeon-descent deckbuilder minigame with main menu, hero/companion select, map choice, gameplay combat UI, reward UI, save/load, starting decks, cards, relics, enemies, items, and leaderboard hooks. In current demo, the route is hidden/blocked.
- Lifecycle status: `DEMO_GATED`, `PARTIAL`.
- Evidence tier: READ + STATIC_TRACE.
- Visibility route: `DIRECT_NAV_BLOCKED_DEMO`, `FULL_GAME_ROUTE`.
- Counts: 3 heroes, 3 companions, 8 cards, 2 relics, 7 enemies, 3 items, 5 encounters, 3 starting decks, 1 stage, 1 tuning row. Evidence starts at [T66Deck_Cards.csv](C:/UE/T66/Content/Deck/Data/T66Deck_Cards.csv:1), [T66Deck_Stages.csv](C:/UE/T66/Content/Deck/Data/T66Deck_Stages.csv:1), and [T66Deck_Tuning.csv](C:/UE/T66/Content/Deck/Data/T66Deck_Tuning.csv:1).
- Connections: Minigames hub, Deck data/save/frontend subsystems, backend leaderboard, local run save.
- Suspected issues: current code/data outgrew the master doc, which still says no screen registration/gameplay combat/data ingestion. Deck gameplay is screen-owned inside `T66DeckMainMenuScreen` rather than a separate `BattleScreen` class, unlike Mini/TD, which is one reason this audit keeps the `PARTIAL` tag until runtime proof. Content scale is prototype-small: one stage and one tuning row with `DifficultyID=Prototype`. See `CONTENTFIND-012`.
- Evidence: [T66WidgetGameRegistry.cpp](C:/UE/T66/Source/T66/UI/WidgetGames/T66WidgetGameRegistry.cpp:334), [T66DeckDataSubsystem.cpp](C:/UE/T66/Source/T66Deck/Private/Core/T66DeckDataSubsystem.cpp:44), [T66DeckMainMenuScreen.cpp](C:/UE/T66/Source/T66Deck/Private/UI/Screens/T66DeckMainMenuScreen.cpp:324), [T66DeckMainMenuScreen.cpp](C:/UE/T66/Source/T66Deck/Private/UI/Screens/T66DeckMainMenuScreen.cpp:589), [T66DeckMainMenuScreen.h](C:/UE/T66/Source/T66Deck/Public/UI/Screens/T66DeckMainMenuScreen.h:36), [T66Deck_MasterImplementation.md](C:/UE/T66/Gameplay/Minigames/Deck/T66Deck_MasterImplementation.md:7), [T66Deck_Tuning.csv](C:/UE/T66/Content/Deck/Data/T66Deck_Tuning.csv:2).
- Cross-refs: TECH-MINI-005, INV-MINI-005.

## 10. Hidden / Deprecated / Partial Register

| ID | Content | Status | Why it is not plainly active demo content | Evidence tier | Evidence |
|---|---|---|---|---|---|
| CONTENT-HERO-001 | Heroes 6-12 | `DEMO_GATED` | filtered out by demo allow-list | READ + STATIC_TRACE | [DefaultDemoMode.ini](C:/UE/T66/Config/DefaultDemoMode.ini:8), [T66ReleaseVariantSubsystem.cpp](C:/UE/T66/Source/T66/Core/T66ReleaseVariantSubsystem.cpp:247) |
| CONTENT-COMPANION-001 | Companions 5-16 | `DEMO_GATED` | filtered out by demo allow-list | READ + STATIC_TRACE | [DefaultDemoMode.ini](C:/UE/T66/Config/DefaultDemoMode.ini:14), [T66ReleaseVariantSubsystem.cpp](C:/UE/T66/Source/T66/Core/T66ReleaseVariantSubsystem.cpp:272) |
| CONTENT-PROGRESSION-001 | Medium/Hard/VeryHard/Impossible | `DEMO_GATED` | only Easy is allowed in demo | READ + STATIC_TRACE | [DefaultDemoMode.ini](C:/UE/T66/Config/DefaultDemoMode.ini:13), [T66ReleaseVariantSubsystem.cpp](C:/UE/T66/Source/T66/Core/T66ReleaseVariantSubsystem.cpp:297) |
| CONTENT-UI-001 | Daily Descent | `DEMO_GATED` | main menu returns before navigation and UI manager blocks screen | STATIC_TRACE | [T66MainMenuScreen.cpp](C:/UE/T66/Source/T66/UI/Screens/T66MainMenuScreen.cpp:2042), [T66UIManagerReleaseVariant.cpp](C:/UE/T66/Source/T66/UI/T66UIManagerReleaseVariant.cpp:61) |
| CONTENT-PROGRESSION-001 | Lab run | `DEMO_GATED` | config disallows Lab | READ + STATIC_TRACE | [DefaultDemoMode.ini](C:/UE/T66/Config/DefaultDemoMode.ini:25), [T66ReleaseVariantSubsystem.cpp](C:/UE/T66/Source/T66/Core/T66ReleaseVariantSubsystem.cpp:220) |
| CONTENT-PROGRESSION-001 | Collector | `DEMO_GATED` | config disallows Collector | READ + STATIC_TRACE | [DefaultDemoMode.ini](C:/UE/T66/Config/DefaultDemoMode.ini:26), [T66ReleaseVariantSubsystem.cpp](C:/UE/T66/Source/T66/Core/T66ReleaseVariantSubsystem.cpp:236) |
| CONTENT-GAMBLER-001 | Lottery / Plinko / Box Opening | `DEMO_GATED` | registry descriptors exist; demo allow-list excludes them | READ + STATIC_TRACE | [T66WidgetGameRegistry.cpp](C:/UE/T66/Source/T66/UI/WidgetGames/T66WidgetGameRegistry.cpp:289), [DefaultDemoMode.ini](C:/UE/T66/Config/DefaultDemoMode.ini:22) |
| CONTENT-MINI-001 | Minigames hub and minigame screens | `DEMO_GATED` | top-bar omitted and direct navigation blocked in demo | STATIC_TRACE | [T66FrontendTopBarWidget.cpp](C:/UE/T66/Source/T66/UI/T66FrontendTopBarWidget.cpp:938), [T66UIManagerReleaseVariant.cpp](C:/UE/T66/Source/T66/UI/T66UIManagerReleaseVariant.cpp:54) |
| CONTENT-DEPRECATED-001 | Arcade games / arcade interactables | `DEPRECATED` | central deprecated feature settings disable them in all builds | READ | [DefaultGame.ini](C:/UE/T66/Config/DefaultGame.ini:64), [DEPRECATED_CONTENT.md](C:/UE/T66/Demo/DEPRECATED_CONTENT.md:33) |
| CONTENT-COMPAT-001 | legacy serialized fields/IDs | `COMPAT_LEGACY` | retained for compatibility, not player-facing content | READ | [DEPRECATED_CONTENT.md](C:/UE/T66/Demo/DEPRECATED_CONTENT.md:50) |
| CONTENT-MINI-002 | T66Mini unused boss rows | `ORPHAN_SUSPECT` | 66 boss rows load, but stage/wave route references 5 boss IDs by CSV count | READ + STATIC_TRACE | [T66Mini_Bosses.csv](C:/UE/T66/Content/Mini/Data/T66Mini_Bosses.csv:1), [T66MiniBattleScreen.cpp](C:/UE/T66/Source/T66Mini/Private/UI/Screens/T66MiniBattleScreen.cpp:1953) |
| CONTENT-MINI-004 | T66Idle current state | `PARTIAL` | real screen-owned gameplay exists, but data scale is small and docs are stale | READ + STATIC_TRACE | [T66IdleMainMenuScreen.cpp](C:/UE/T66/Source/T66Idle/Private/UI/Screens/T66IdleMainMenuScreen.cpp:337), [T66Idle_MasterImplementation.md](C:/UE/T66/Gameplay/Minigames/Idle/T66Idle_MasterImplementation.md:7) |
| CONTENT-MINI-005 | T66Deck current state | `PARTIAL` | real screen-owned gameplay exists, but one stage and prototype tuning | READ + STATIC_TRACE | [T66DeckMainMenuScreen.cpp](C:/UE/T66/Source/T66Deck/Private/UI/Screens/T66DeckMainMenuScreen.cpp:589), [T66Deck_Stages.csv](C:/UE/T66/Content/Deck/Data/T66Deck_Stages.csv:2) |

## 11. Mismatch Hunt Findings

### CONTENTFIND-001 - Authored Content And Demo-Visible Content Are Easy To Confuse

- Status tags: `DEMO_GATED`.
- Evidence tier: READ + STATIC_TRACE.
- What was found: current data has 12 heroes, 16 companions, 20 stages, 5 difficulties, 23 boss rows, and full minigame modules, but current forced-demo settings expose only 5 heroes, 4 companions, Easy difficulty, Easy stage band, and no minigame screens.
- Why it matters: a player/design description that counts authored rows without release gates will overstate the current player-facing demo.
- Evidence: [DefaultDemoMode.ini](C:/UE/T66/Config/DefaultDemoMode.ini:2), [DefaultDemoMode.ini](C:/UE/T66/Config/DefaultDemoMode.ini:8), [DefaultDemoMode.ini](C:/UE/T66/Config/DefaultDemoMode.ini:13), [T66UIManagerReleaseVariant.cpp](C:/UE/T66/Source/T66/UI/T66UIManagerReleaseVariant.cpp:54).
- Verification backlog: launch current demo build and screenshot/select all visible hero/companion/difficulty/minigame routes.

### CONTENTFIND-002 - Minigames Are Demo-Gated, Not Deprecated

- Status tags: `DEMO_GATED`.
- Evidence tier: READ + STATIC_TRACE.
- What was found: docs and current code route minigames as present in full game but hidden/blocked in demo. This differs from older project habits where Mini/minigames were excluded/deprecated.
- Why it matters: tagging minigames as deprecated would misclassify active full-game content and hide four real modules from planning.
- Evidence: [DEMO_GATED_INVISIBLE_CONTENT.md](C:/UE/T66/Demo/DEMO_GATED_INVISIBLE_CONTENT.md:79), [T66UIManagerReleaseVariant.cpp](C:/UE/T66/Source/T66/UI/T66UIManagerReleaseVariant.cpp:11), [T66WidgetGameRegistry.cpp](C:/UE/T66/Source/T66/UI/WidgetGames/T66WidgetGameRegistry.cpp:320).
- Verification backlog: full-game variant run should confirm Minigames tab appears and each minigame card opens its target screen.

### CONTENTFIND-003 - Arcade Allow-List Exists In Demo Config, But Arcade Is Deprecated/Disabled

- Status tags: `DEPRECATED`, `COMPAT_LEGACY`.
- Evidence tier: READ + STATIC_TRACE.
- What was found: `DefaultDemoMode.ini` still lists four `AllowedArcadeGameIDs`, but `DefaultGame.ini` disables arcade games and arcade interactables globally.
- Why it matters: this is a route/data mismatch: the allow-list can look like demo-visible arcade content, but the deprecated feature gate wins.
- Evidence: [DefaultDemoMode.ini](C:/UE/T66/Config/DefaultDemoMode.ini:18), [DefaultGame.ini](C:/UE/T66/Config/DefaultGame.ini:64), [DEPRECATED_CONTENT.md](C:/UE/T66/Demo/DEPRECATED_CONTENT.md:33), [T66MinigamesScreen.cpp](C:/UE/T66/Source/T66/UI/Screens/T66MinigamesScreen.cpp:478).
- Verification backlog: full-game route should confirm Versus/Arcade card stays blocked while deprecated settings remain true.

### CONTENTFIND-004 - Idol Traveler Comments Are Stale

- Status tags: `ACTIVE`.
- Evidence tier: STATIC_TRACE.
- What was found: comments still say traveler delivery is reserved/inert, but current combat code handles `ET66IdolDelivery::Traveler`, fires traveler requests, and applies idol damage on arrival.
- Why it matters: a doc/comment-only audit would incorrectly tag all 16 current idols as inert/reserved.
- Evidence: stale comment at [T66DataTypes.h](C:/UE/T66/Source/T66/Data/T66DataTypes.h:1031), stale comment at [T66DataTypes.h](C:/UE/T66/Source/T66/Data/T66DataTypes.h:1970), active traveler branch at [T66CombatComponent.cpp](C:/UE/T66/Source/T66/Gameplay/T66CombatComponent.cpp:4105), active dispatch at [T66CombatComponent.cpp](C:/UE/T66/Source/T66/Gameplay/T66CombatComponent.cpp:4139).
- Verification backlog: current runtime capture should verify each idol category route: Pierce, Bounce, DOT, and AOE.

### CONTENTFIND-005 - Status Effects Are Authored But Not Production-Assigned

- Status tags: `PARTIAL`.
- Visibility route: `DATA_ONLY`.
- Evidence tier: READ.
- What was found: 12 status-effect rows exist, but pending issue says production mobs currently assign `StatusEffectOnHit=None` and normal production mobs do not yet consume the hit-path status effect.
- Why it matters: status effects are content data, but the player likely does not experience most of them in the normal enemy loop yet.
- Evidence: [StatusEffects.csv](C:/UE/T66/Content/Data/StatusEffects.csv:1), [pending_issues_Data.md](C:/UE/T66/Content/Data/pending_issues_Data.md:3).
- Verification backlog: enemy-hit runtime proof for each non-None status effect, or explicit design sign-off that they remain authored-only.

### CONTENTFIND-006 - Enemy Spawn Director Still Uses Fallback-Family Behavior

- Status tags: `PARTIAL`.
- Evidence tier: READ.
- What was found: pending gameplay issue says `T66EnemyDirector` reads up to 10 stage slots but still routes production archetype selection through fallback families; another pending issue says Hell core has no ranged mob and old logic expecting a ranged slot may behave differently.
- Why it matters: the 60-row enemy roster may not be experienced with its full intended archetype variety.
- Evidence: [pending_issues_Gameplay.md](C:/UE/T66/Source/T66/Gameplay/pending_issues_Gameplay.md:116), [pending_issues_Gameplay.md](C:/UE/T66/Source/T66/Gameplay/pending_issues_Gameplay.md:123).
- Verification backlog: per-difficulty spawn-composition proof from current stage data to runtime enemy families.

### CONTENTFIND-007 - Item/Data References Have Known Stale Or Missing Assets/Rows

- Status tags: `PARTIAL`, `ORPHAN_SUSPECT`.
- Evidence tier: READ.
- What was found: pending data issue reports missing `/Game/Data/DT_HouseNPCs`, missing world visual prop references, and item rows such as `Item_GamblersToken` and `Item_Alchemy` missing from `/Game/Data/DT_Items`; another issue says Headshot still points to CritDamage sprites.
- Why it matters: data may exist in source CSV but not resolve cleanly in cooked/DataTable asset paths, and some visuals may present as fallback/old icons.
- Evidence: [pending_issues_Data.md](C:/UE/T66/Content/Data/pending_issues_Data.md:10), [pending_issues_Data.md](C:/UE/T66/Content/Data/pending_issues_Data.md:24).
- Verification backlog: Technical/Inventory audits should reconcile CSV rows, DataTable assets, staged logs, and icon asset existence.

### CONTENTFIND-008 - Tower Floors And Drop-Hole Floors Have Known Visual Seam Risks

- Status tags: `PARTIAL`.
- Evidence tier: READ.
- What was found: pending gameplay issues say inter-walkable-box floor seams remain possible, and drop-hole floors are split around openings.
- Why it matters: tower traversal is an active route, but its visual/layout clarity may be degraded in live play.
- Evidence: [pending_issues_Gameplay.md](C:/UE/T66/Source/T66/Gameplay/pending_issues_Gameplay.md:137), [pending_issues_Gameplay.md](C:/UE/T66/Source/T66/Gameplay/pending_issues_Gameplay.md:144).
- Verification backlog: current tower run capture focused on mob floors and descent-hole floor boundaries.

### CONTENTFIND-009 - Frontend Focus And Loot-Wheel Boost Presentation Are Known UI Gaps

- Status tags: `PARTIAL`.
- Evidence tier: READ.
- What was found: pending UI issues say frontend screens lack a central controller-focus contract and loot-wheel boost rewards lack a focused result toast.
- Why it matters: content can be active but hard to operate or poorly communicated, especially on Steam Deck/controller.
- Evidence: [pending_issues_UI.md](C:/UE/T66/Source/T66/UI/pending_issues_UI.md:3), [pending_issues_UI.md](C:/UE/T66/Source/T66/UI/pending_issues_UI.md:10).
- Verification backlog: UI smoke for first-focus/back/accept on content screens, and loot-wheel boost-result presentation capture.

### CONTENTFIND-010 - T66Mini Boss Table Has 66 Rows But Current Stage/Wave Routes Reference Five Boss IDs

- Status tags: `ORPHAN_SUSPECT`.
- Evidence tier: READ + STATIC_TRACE.
- What was found: `T66Mini_Bosses.csv` has 66 rows, but current stage/wave boss references resolve to five IDs by CSV count: `Boss_05`, `Boss_10`, `Boss_15`, `Boss_20`, `Boss_25`. Battle code picks `Stage->BossID` or `Wave->BossID` and then calls `FindBoss()`.
- Why it matters: 61 mini boss rows may be authored but unreachable under the current stage/wave route, unless another runtime selector not found in this pass uses them.
- Evidence: [T66Mini_Bosses.csv](C:/UE/T66/Content/Mini/Data/T66Mini_Bosses.csv:1), [T66Mini_Stages.csv](C:/UE/T66/Content/Mini/Data/T66Mini_Stages.csv:1), [T66Mini_Waves.csv](C:/UE/T66/Content/Mini/Data/T66Mini_Waves.csv:1), [T66MiniBattleScreen.cpp](C:/UE/T66/Source/T66Mini/Private/UI/Screens/T66MiniBattleScreen.cpp:1953), [T66MiniDataSubsystem.cpp](C:/UE/T66/Source/T66Mini/Private/Core/T66MiniDataSubsystem.cpp:270).
- Verification backlog: full mini run over all 50 waves or code trace proving random/alternate boss selection.

### CONTENTFIND-011 - T66Idle Docs Are Stale, And Idle Hero Naming Conflicts With Main Hero IDs

- Status tags: `PARTIAL`.
- Evidence tier: READ + STATIC_TRACE.
- What was found: `T66Idle_MasterImplementation.md` says infrastructure-only/no production data/finished gameplay loops, but current code loads authored CSVs and has gameplay UI. Idle gameplay is screen-owned inside `T66IdleMainMenuScreen` rather than a separate battle-screen class. Also `T66Idle_Heroes.csv` names `Hero_12` as `Goblino Chad`, while main `Hero_12` is `Roach Chad` and main `Hero_9` is `Goblino Chad`.
- Why it matters: stale docs understate current Idle maturity, while the hero-ID/name mismatch can confuse cross-audit ID mapping.
- Evidence: [T66Idle_MasterImplementation.md](C:/UE/T66/Gameplay/Minigames/Idle/T66Idle_MasterImplementation.md:7), [T66IdleDataSubsystem.cpp](C:/UE/T66/Source/T66Idle/Private/Core/T66IdleDataSubsystem.cpp:26), [T66IdleMainMenuScreen.cpp](C:/UE/T66/Source/T66Idle/Private/UI/Screens/T66IdleMainMenuScreen.cpp:337), [T66Idle_Heroes.csv](C:/UE/T66/Content/Idle/Data/T66Idle_Heroes.csv:6), [Heroes.csv](C:/UE/T66/Content/Data/Heroes.csv:13).
- Verification backlog: design decision on whether Idle intentionally remaps hero IDs/names; update docs later if confirmed.

### CONTENTFIND-012 - T66Deck Docs Are Stale, And Deck Content Is Prototype-Scale

- Status tags: `PARTIAL`.
- Evidence tier: READ + STATIC_TRACE.
- What was found: `T66Deck_MasterImplementation.md` says no shared screen registration/gameplay combat/data ingestion, but current code registers the frontend descriptor, loads authored CSVs, and builds gameplay/reward screens. Deck gameplay is screen-owned inside `T66DeckMainMenuScreen` rather than a separate battle-screen class. Data scale remains prototype-small: one stage and tuning row with `DifficultyID=Prototype`.
- Why it matters: Deck is not a pure stub, but it is also not full-depth content comparable to main run or T66Mini.
- Evidence: [T66Deck_MasterImplementation.md](C:/UE/T66/Gameplay/Minigames/Deck/T66Deck_MasterImplementation.md:7), [T66WidgetGameRegistry.cpp](C:/UE/T66/Source/T66/UI/WidgetGames/T66WidgetGameRegistry.cpp:334), [T66DeckDataSubsystem.cpp](C:/UE/T66/Source/T66Deck/Private/Core/T66DeckDataSubsystem.cpp:44), [T66DeckMainMenuScreen.cpp](C:/UE/T66/Source/T66Deck/Private/UI/Screens/T66DeckMainMenuScreen.cpp:589), [T66Deck_Stages.csv](C:/UE/T66/Content/Deck/Data/T66Deck_Stages.csv:2), [T66Deck_Tuning.csv](C:/UE/T66/Content/Deck/Data/T66Deck_Tuning.csv:2).
- Verification backlog: full-game Deck launch, start descent, enter encounter, play cards, claim reward, save/load.

### CONTENTFIND-013 - Pet Content Uses Boss Fallback When No Pets CSV/Table Row Is Found

- Status tags: `PARTIAL`.
- Visibility route: `FALLBACK_ROUTE`.
- Evidence tier: STATIC_TRACE.
- What was found: no loose `Content/Data/Pets.csv` exists, and `GetPetData()` synthesizes pet data from boss rows if the pets table does not produce a row.
- Why it matters: pets can be active while not appearing as explicit CSV content; Inventory must check DataTable assets and fallback behavior rather than counting CSVs only.
- Evidence: [T66GameInstance.cpp](C:/UE/T66/Source/T66/Core/T66GameInstance.cpp:736), [T66GameInstance.cpp](C:/UE/T66/Source/T66/Core/T66GameInstance.cpp:753), [T66GameInstance.cpp](C:/UE/T66/Source/T66/Core/T66GameInstance.cpp:770), [T66GameInstance.cpp](C:/UE/T66/Source/T66/Core/T66GameInstance.cpp:1366).
- Verification backlog: boss kill capture, pet selection, pet actor spawn, and DataTable asset existence check.

### CONTENTFIND-014 - Some Hero/Companion Rows Reuse Or Remap Visual Paths

- Status tags: `PARTIAL`, `ORPHAN_SUSPECT`.
- Evidence tier: READ.
- What was found: main hero portrait remapping spans at least three demo-visible heroes: `Hero_1` references `Hero_4` art, `Hero_4` references `Hero_6` art, and `Hero_5` references `Hero_1` art. Later companion rows also reuse earlier companion portrait paths. This may be intentional placeholder reuse, a model/portrait remap, or stale content.
- Why it matters: player-visible identity can diverge from ID/name data even when the row itself is active.
- Evidence: [Heroes.csv](C:/UE/T66/Content/Data/Heroes.csv:2), [Heroes.csv](C:/UE/T66/Content/Data/Heroes.csv:5), [Heroes.csv](C:/UE/T66/Content/Data/Heroes.csv:6), [Companions.csv](C:/UE/T66/Content/Data/Companions.csv:10), [Companions.csv](C:/UE/T66/Content/Data/Companions.csv:17).
- Verification backlog: Inventory audit should resolve portrait asset ownership and current runtime presentation for each demo-visible and hidden row.

### CONTENTFIND-015 - Vendor Failed-Steal / Hidden Boss Route Is Active But Not Obvious From Data

- Status tags: `HIDDEN_RUNTIME`.
- Evidence tier: STATIC_TRACE.
- What was found: shop stealing has a live resolver; tower verification code references vendor-boss spawn and vendor-token drop paths; vendor token is special-cased as a pickup.
- Why it matters: a player could experience a hidden vendor/boss branch that is not obvious from high-level item/economy data counts.
- Evidence: [T66RunStateSubsystem_EconomyInventory.cpp](C:/UE/T66/Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp:278), [T66GameMode_Tower.cpp](C:/UE/T66/Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp:553), [T66RunStateSubsystem_EconomyInventory.cpp](C:/UE/T66/Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp:643).
- Verification backlog: focused runtime route: spawn vendor, fail steal, confirm hidden boss identity, token drop, and UI feedback.

## 12. Verification Backlog For Later Active Checks

These checks are not fixes. They are the recommended runtime validation targets for this descriptive audit.

1. Current forced-demo frontend: capture visible main menu, hidden Daily/Minigames, hero list, companion list, and difficulty list. Target findings: `CONTENTFIND-001`, `CONTENTFIND-002`.
2. Full-game release variant: capture Minigames tab, Minigames hub, and launch each minigame screen. Target cards: `CONTENT-MINI-001` through `CONTENT-MINI-005`.
3. Main demo run smoke: start each demo-visible hero, confirm one correct Black-tier hero-locked weapon offer, Easy stage progression, boss death altar/gate, and pet capture route. Target cards: `CONTENT-HERO-001`, `CONTENT-WEAPON-001`, `CONTENT-BOSS-001`, `CONTENT-IDOL-001`, `CONTENT-PET-001`.
4. Idol runtime category proof: equip/fire each idol category and verify traveler arrival/damage for Pierce, Bounce, DOT, AOE. Target finding: `CONTENTFIND-004`.
5. Enemy/status pass: confirm which `StatusEffects.csv` rows are actually visible in normal combat, boss combat, minigames, or not at all. Target finding: `CONTENTFIND-005`.
6. Tower traversal: capture descent-hole route, guardian route, floor seams, drop-hole floor boundary, vendor/casino spawns. Target cards: `CONTENT-TOWER-001`, `CONTENT-VENDOR-001`, `CONTENT-GAMBLER-001`.
7. Vendor failed steal: force/perform failed steal, confirm hidden boss/token path. Target finding: `CONTENTFIND-015`.
8. Casino/gambler: verify demo allows Coin Flip, RPS, BlackJack and hides/blocks Lottery, Plinko, Box Opening. Target card: `CONTENT-GAMBLER-001`.
9. T66Mini boss reachability: run or trace all 50 mini waves and prove whether only five boss IDs appear. Target finding: `CONTENTFIND-010`.
10. T66Idle: full-game launch, start run, attack, upgrade, collect, finish summary, save/load. Target finding: `CONTENTFIND-011`.
11. T66Deck: full-game launch, hero/companion selection, map node, combat, reward, save/load. Target finding: `CONTENTFIND-012`.
12. Visual identity audit: verify hero/companion portrait remaps/placeholders against actual UI presentation. Target finding: `CONTENTFIND-014`.

## 13. Cross-Reference Appendix

| Content ID | Subject | Technical ID to reuse | Inventory ID to reuse | Primary evidence |
|---|---|---|---|---|
| CONTENT-RELEASE-001 | release/demo gating | TECH-RELEASE-001 | INV-RELEASE-001 | [DefaultDemoMode.ini](C:/UE/T66/Config/DefaultDemoMode.ini:2), [T66ReleaseVariantSubsystem.cpp](C:/UE/T66/Source/T66/Core/T66ReleaseVariantSubsystem.cpp:84) |
| CONTENT-HERO-001 | main heroes | TECH-HERO-001 | INV-HERO-001 | [Heroes.csv](C:/UE/T66/Content/Data/Heroes.csv:1) |
| CONTENT-WEAPON-001 | hero-locked weapons | TECH-WEAPON-001 | INV-WEAPON-001 | [Weapons.csv](C:/UE/T66/Content/Data/Weapons.csv:1), [T66WeaponManagerSubsystem.cpp](C:/UE/T66/Source/T66/Core/T66WeaponManagerSubsystem.cpp:52) |
| CONTENT-IDOL-001 | idols | TECH-IDOL-001 | INV-IDOL-001 | [Idols.csv](C:/UE/T66/Content/Data/Idols.csv:1), [T66IdolManagerSubsystem.h](C:/UE/T66/Source/T66/Core/T66IdolManagerSubsystem.h:22) |
| CONTENT-ITEM-001 | items/shop/special rows | TECH-ITEM-001 | INV-ITEM-001 | [Items.csv](C:/UE/T66/Content/Data/Items.csv:1), [T66RunStateSubsystem_EconomyInventory.cpp](C:/UE/T66/Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp:51) |
| CONTENT-COMPANION-001 | companions | TECH-COMPANION-001 | INV-COMPANION-001 | [Companions.csv](C:/UE/T66/Content/Data/Companions.csv:1) |
| CONTENT-ENEMY-001 | standard enemies | TECH-ENEMY-001 | INV-ENEMY-001 | [Enemies.csv](C:/UE/T66/Content/Data/Enemies.csv:1), [pending_issues_Gameplay.md](C:/UE/T66/Source/T66/Gameplay/pending_issues_Gameplay.md:116) |
| CONTENT-BOSS-001 | bosses/encounters | TECH-BOSS-001 | INV-BOSS-001 | [Bosses.csv](C:/UE/T66/Content/Data/Bosses.csv:1), [BossEncounters.csv](C:/UE/T66/Content/Data/BossEncounters.csv:1) |
| CONTENT-TOWER-001 | tower/run map | TECH-TOWER-001 | INV-TOWER-001 | [T66GameMode_Tower.cpp](C:/UE/T66/Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp:359) |
| CONTENT-ECONOMY-001 | economy/rewards | TECH-ECONOMY-001 | INV-ECONOMY-001 | [PlayerExperience.json](C:/UE/T66/Content/Data/PlayerExperience.json:3), [T66RunStateSubsystem.h](C:/UE/T66/Source/T66/Core/T66RunStateSubsystem.h:124) |
| CONTENT-VENDOR-001 | vendor/steal | TECH-VENDOR-001 | INV-VENDOR-001 | [T66RunStateSubsystem_EconomyInventory.cpp](C:/UE/T66/Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp:278) |
| CONTENT-GAMBLER-001 | casino/gambler | TECH-GAMBLER-001 | INV-GAMBLER-001 | [T66WidgetGameRegistry.cpp](C:/UE/T66/Source/T66/UI/WidgetGames/T66WidgetGameRegistry.cpp:259) |
| CONTENT-PET-001 | pets/capture | TECH-PET-001 | INV-PET-001 | [T66GameMode_BossFlow.cpp](C:/UE/T66/Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp:503), [T66GameInstance.cpp](C:/UE/T66/Source/T66/Core/T66GameInstance.cpp:729) |
| CONTENT-UI-001 | UI screens/routes | TECH-UI-001 | INV-UI-001 | [T66UIManagerReleaseVariant.cpp](C:/UE/T66/Source/T66/UI/T66UIManagerReleaseVariant.cpp:38), [T66FrontendTopBarWidget.cpp](C:/UE/T66/Source/T66/UI/T66FrontendTopBarWidget.cpp:938) |
| CONTENT-PROGRESSION-001 | difficulty/meta progression | TECH-PROGRESSION-001 | INV-PROGRESSION-001 | [DifficultyTuning.json](C:/UE/T66/Content/Data/DifficultyTuning.json:1) |
| CONTENT-DEPRECATED-001 | arcade deprecated content | TECH-DEPRECATED-001 | INV-DEPRECATED-001 | [DefaultGame.ini](C:/UE/T66/Config/DefaultGame.ini:64), [DEPRECATED_CONTENT.md](C:/UE/T66/Demo/DEPRECATED_CONTENT.md:33) |
| CONTENT-COMPAT-001 | legacy compatibility fields | TECH-COMPAT-001 | INV-COMPAT-001 | [DEPRECATED_CONTENT.md](C:/UE/T66/Demo/DEPRECATED_CONTENT.md:50) |
| CONTENT-MINI-001 | minigame hub/access | TECH-MINI-001 | INV-MINI-001 | [T66MinigamesScreen.cpp](C:/UE/T66/Source/T66/UI/Screens/T66MinigamesScreen.cpp:295), [T66UIManagerReleaseVariant.cpp](C:/UE/T66/Source/T66/UI/T66UIManagerReleaseVariant.cpp:11) |
| CONTENT-MINI-002 | T66Mini | TECH-MINI-002 | INV-MINI-002 | [T66MiniDataSubsystem.cpp](C:/UE/T66/Source/T66Mini/Private/Core/T66MiniDataSubsystem.cpp:179), [T66MiniBattleScreen.cpp](C:/UE/T66/Source/T66Mini/Private/UI/Screens/T66MiniBattleScreen.cpp:367) |
| CONTENT-MINI-003 | T66TD | TECH-MINI-003 | INV-MINI-003 | [T66TDDataSubsystem.cpp](C:/UE/T66/Source/T66TD/Private/Core/T66TDDataSubsystem.cpp:31), [T66TDBattleScreen.cpp](C:/UE/T66/Source/T66TD/Private/UI/Screens/T66TDBattleScreen.cpp:1) |
| CONTENT-MINI-004 | T66Idle | TECH-MINI-004 | INV-MINI-004 | [T66IdleDataSubsystem.cpp](C:/UE/T66/Source/T66Idle/Private/Core/T66IdleDataSubsystem.cpp:26), [T66IdleMainMenuScreen.cpp](C:/UE/T66/Source/T66Idle/Private/UI/Screens/T66IdleMainMenuScreen.cpp:337) |
| CONTENT-MINI-005 | T66Deck | TECH-MINI-005 | INV-MINI-005 | [T66DeckDataSubsystem.cpp](C:/UE/T66/Source/T66Deck/Private/Core/T66DeckDataSubsystem.cpp:44), [T66DeckMainMenuScreen.cpp](C:/UE/T66/Source/T66Deck/Private/UI/Screens/T66DeckMainMenuScreen.cpp:324) |

## 14. Final Counts

These counts are from live CSV/JSON/source reads in this pass.

- Main game data: 12 heroes, 48 weapons, 16 idols, 30 items, 16 companions, 60 standard enemies, 23 boss rows, 20 boss encounters, 20 stages, 50 boss attack rows, 25 boss attack definitions, 6 boss hazard definitions, 8 boss movement patterns, 12 status-effect rows, 20 combat VFX binding rows. Evidence tier: READ.
- Current demo-visible main slice: 5 heroes, 4 companions, 1 difficulty band, 4 stages, 3 casino games, no Minigames tab/screens. Evidence tier: READ + STATIC_TRACE.
- Current demo-gated main slice: 7 heroes, 12 companions, 4 difficulty bands, 16 stages, non-Easy boss/enemy/progression content, 3 casino games, Daily Descent, Lab, Collector, all minigame modules. Evidence tier: READ + STATIC_TRACE.
- Minigame data: T66Mini has 12 heroes / 16 idols / 31 items / 24 companions / 4 enemies / 66 bosses / 50 stages / 50 waves / 295 tuning rows / 4 interactables / 6 circus games; T66TD has 12 heroes / 12 hero-combat rows / 5 enemy archetypes / 96 battle tuning rows / 5 difficulties / 20 maps / 20 stages / 5 theme-modifier rows; T66Idle has 5 heroes / 4 companions / 4 items / 4 idols / 4 enemies / 1 zone / 10 stages / 1 tuning row; T66Deck has 3 heroes / 3 companions / 8 cards / 2 relics / 7 enemies / 3 items / 5 encounters / 3 starting decks / 1 stage / 1 tuning row. Evidence tier: READ.
- Deprecated-disabled content: arcade games and arcade interactables are centrally disabled. Evidence tier: READ.

## 15. Audit Caveats

- No runtime/editor/gameplay verification was run in this pass, so the document does not claim `RUNTIME_VERIFIED` behavior.
- Current code comments and docs were treated as claims to check, not as authority. This materially changed the idol traveler, Idle, and Deck tags.
- This is a content/experience audit. Technical root cause and asset inventory reconciliation belong to the parallel Technical and Inventory audits using the cross-reference IDs above.
