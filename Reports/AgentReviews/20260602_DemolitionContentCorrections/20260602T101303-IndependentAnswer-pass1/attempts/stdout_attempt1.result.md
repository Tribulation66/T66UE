Result: OK

## Independent Answer

This is a large, multi-module demolition + content-rewrite pass. The plan's sequencing (gate + shells compile → rewire entry points → delete roots last) is the correct shape. Below is the grounded risk map, the highest-leverage missed files, sequencing hazards, compile blockers, and where proof hooks belong.

### Module / surface reality (verified)
- Five runtime modules in `T66.uproject`: `T66`, `T66Mini`, `T66TD`, `T66Idle`, `T66Deck` (+ `T66Editor`). "Demote to main-module shell classes" means each demoted module's classes get re-homed under `Source/T66/...` and the module entries in `.uproject` plus the `*.Build.cs` dependency graph must be reconciled. **Do not delete the `.uproject` module entries until nothing references the module name** — a stale module entry or a dangling `PublicDependencyModuleNames`/`PrivateDependencyModuleNames` reference is the most likely UBT/link failure.
- An existing gate already exists: `T66DeprecatedFeatureSettings` (`bDisableArcadeGames`, `bDisableArcadeInteractables`) + `T66ReleaseVariantSubsystem` + `T66UIManagerReleaseVariant`. **The "central minigame/Versus/DailyDescent gate" should extend these, not introduce a parallel third gating mechanism.** A second independent gate is a real risk of contradictory states.

### Highest-risk shared/serialized files
- `Source/T66/Core/T66RunSaveGame.h` — casino enum `ET66AntiCheatGamblerGameType` (CoinFlip, RockPaperScissors, BlackJack, Lottery, Plinko, BoxOpening) and `FT66AntiCheatGamblerGameSummary` live here. Deleting RPS/BlackJack/Lottery/Plinko/BoxOpening members touches a `BlueprintType` UENUM consumed by the backend serializer/parser (`T66BackendRunSerializer.cpp`, `T66BackendRunSummaryParser.cpp`) and `T66LocalizationSubsystem`. Compile blocker risk: any `switch` over the enum will need its case arms removed in lockstep, and `UMETA`-named blueprint assets may hold stale enum-by-name references.
- `Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp` and `Source/T66/UI/WidgetGames/T66WidgetGameRegistry.cpp` — Lead-owned, named in plan. These are the integration chokepoints; treat as serialize-access (single writer).
- `T66ArcadeInteractableTypes.h` (`ET66ArcadeGameType`) — arcade teardown core enum, parallel deletion hazard to the casino enum.

### Missed / under-named files to confirm in scope
- Daily Descent spans more than UI: `T66SessionSubsystem.cpp`, `T66DirectEntry.cpp`, `T66GameInstance` surfaces, `T66DailyClimbScreen`, `T66AccountStatusScreen`, `T66FrontendTopBarWidget`, `T66MainMenuScreen`, `T66UIManager(.cpp/ReleaseVariant)`. Making backend/game-instance/run-save "inert under the gate" (not deleted) means each of these must route through the gate, not be stubbed independently.
- Companion unlock spans 15 files incl. `T66CompanionUnlockSubsystem(.h/.cpp)`, `T66CompanionUnlockSaveGame.h`, `T66GameMode_BossFlow.cpp`, `T66CompanionGridScreen/SelectionScreen`. The "boss caged companion free/interact unlock" rewrite must keep the SaveGame field contract intact (changing the unlock *trigger*, not the *persisted unlock set*) or old saves silently lose companions.
- Casino game widgets to retire: `T66BlackJackGameWidget`, `T66PlinkoGameWidget`, `T66LotteryGameWidget`, `T66BoxOpeningGameWidget`, `T66RpsGameWidget`, plus `T66CasinoGamblerTabWidget_*` partials (Build/Economy/Cheat/CoinGames/ChanceGames/BlackJack). The four replacement games (Coin Flip / Guess the Cup / Pick Stick / Find the Joker) need their own widgets + registry entries.
- Per-difficulty base rarity removal: `T66DifficultyTuningSubsystem(.h/.cpp)`, `T66IdolManagerSubsystem.cpp`, `T66LootWheelInteractable.cpp` reference base/difficulty rarity. Removing getters here will break callers in `T66PlayerController_Overlays/Combat` — fix call sites in the same change or it won't link.

### Sequencing hazards
1. **Build.cs before .uproject.** Reconcile module dependency lists first; remove `.uproject` module entries only after a clean compile against the re-homed shells.
2. **Enum deletion is not "delete last."** It's a compile-fan-out: delete enum members + every `switch`/case + serializer field handling + any DataTable/blueprint enum-by-name in one atomic step, then build.
3. **Gate before stubs.** The central gate must compile and be the single source of truth before any entry point is rewired; otherwise inert surfaces drift.
4. **Companion save schema is a trap.** Treat the persisted unlock set as frozen; only swap the unlock pathway.
5. **Shop rarity tunables** (Black 70 / Red 25 / Yellow 4.5 / White 0.5) must be named config constants summing to 100.0 with an explicit assertion — independent per-slot rolls, 4 buy/sell/buyback slots, buy slots never empty.

### Proof-hook locations (deterministic)
- Shop rarity: a deterministic-seed RNG path + a log/proof marker emitting the rolled rarity per slot, so distribution over N rolls is verifiable (assert weights sum, log seed).
- Central gate: a single boolean-resolver function with a logged gate state at startup (covering minigame/Versus/DailyDescent) so "inert" is provable from logs, not inferred.
- Companion unlock: a boss-cage-free event marker + companion-id, distinct from the old direct-unlock path, so the new mechanism is evidenced.
- Build/cook/stage/smoke: per AGENTS "Staged standalone verification" — `Scripts\StageStandaloneBuild.ps1`, verify shortcut target `...\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`, smoke logs. Source compile alone is **not** acceptable proof.

### Recommended checkpoints (phase gates)
- **G1:** central gate + re-homed shells compile (no entry-point rewire yet). Build log.
- **G2:** all entry points route through gate; Daily Descent inert-under-gate; old casino enum/save/backend refs removed; clean compile + link.
- **G3:** content rewrites (4 casino games, 4 shop slots, companion cage unlock, base-rarity removal) compile with proof hooks emitting.
- **G4:** module roots deleted from `.uproject`/Build.cs; clean full build.
- **G5:** cook/stage + staged smoke with logs; shortcut target verified.

## Evidence Checked
- `original_prompt.md`, `AGENTS.md` (read fully), `OPERATOR_VALIDATOR_PROTOCOL.md` (presence confirmed).
- `T66.uproject` module list; `Source/*/*.Build.cs` (5 runtime + editor modules).
- Casino enum block in `T66RunSaveGame.h:206-239`; grep fan-out for RPS/BlackJack/Lottery/Plinko/BoxOpening (25 files incl. backend serializer/parser, localization).
- `T66DeprecatedFeatureSettings.h` (existing gate); Versus/Arcade (45 files), Daily Descent (12 files), companion unlock (15 files), base-rarity (7 files) grep maps.
- Lead-owned `T66GameMode_WorldInteractables.cpp` and `T66WidgetGameRegistry.cpp` located.

## Questions Or Blockers
None requiring the user — the three previously-open decisions (Daily Descent shelve-not-delete, casino enum deletion, 4-slot shop with named weights) are already user-resolved in the prompt. Items for Codex to handle inline: confirm the central gate extends `T66DeprecatedFeatureSettings`/`ReleaseVariantSubsystem` rather than adding a third gate; confirm companion SaveGame field schema is preserved while only the unlock trigger changes.

## Caveats
- I did not open every one of the 45 Versus/Arcade or 15 companion files; the risk map names the chokepoints, not an exhaustive edit list — Codex must still grep call sites before deleting any getter or enum member.
- "Demote to main-module shell" has two valid readings (physically move sources into `T66` vs. leave module but reduce to shells); the `.uproject` shows the modules still present, so confirm the intended end state before deleting module entries.
- This is read-only; no build/cook/stage/smoke was run. Per AGENTS, current build + staged smoke proof is mandatory and is owned by the Codex-approved FullOperator phase — prior evidence cannot substitute.
- Honored Mini-scope rule: Mini files are listed only because the prompt explicitly names `T66Mini` for demotion.
