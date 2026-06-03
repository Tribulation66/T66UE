# Phase 1 Completion Packet — Enemy Roster Restructure Implementation

- Operator: Claude (`claude-opus-4-8`, FullOperator)
- Validator/Finisher: Codex
- Date: 2026-05-29 (revised same-day after Codex validation pass)
- Status: **Phase 1 source/data implementation complete; the 5 Codex validation gaps are now closed.** This is a work artifact for Codex re-validation, NOT a self-greenlight.
- Stop condition outcome: All five findings applied cleanly without violating user constraints (no editor build, no Git writes, no binary-asset mutation, no scope expansion). No blocker forced a stop. Remaining items are genuine Phase 2 (editor/uasset) deferrals, listed explicitly.

> Attribution note: the working tree was already dirty at session start with many unrelated pre-existing modifications. The "Exact files changed" lists below are scoped to files this Phase 1 pass (plus this revision) actually edited, not the full `git status`. No Git write operations were performed (commit/stage/push/tag/reset/clean all excluded).

---

## Revision Log — Codex Validation Gaps Closed

This revision addresses the five gaps Codex raised against the prior packet. Each is summarized here and detailed in the section it touches.

1. **Finding 1 — UniqueDebuff projectile path fully removed.** The prior packet *retained* `AT66UniqueDebuffProjectile` as a deviation. It is now deleted along with `FT66TemporaryProjectileSystem::ProfileUniqueDebuff`, the active-projectile counter contributions in the perf/lag subsystems, the Backrooms cleanup filter, and the debug-preview spawn in `T66PlayerController_Overlays.cpp`. Verified 0 hits.
2. **Finding 2 — gameplay-floor → mob-floor rename completed.** The three previously-deferred DataTable/UPROPERTY field renames (`GameplayFloorsPerStage`, `InitialEnemiesPerGameplayFloor`, `InitialTowerEnemiesPerGameplayFloor`) are now applied in source/data, bridged by documented `[CoreRedirects] +PropertyRedirects` so existing `DT_PlayerExperience` keeps binding until the Phase 2 uasset rebuild. `PlayerExperience.json` and the `ValidateEnemyBossRosterData.py` required-key list were updated to match.
3. **Finding 3 — VendorToken is now the canonical runtime name.** Runtime-facing `GamblersToken` symbols renamed to `VendorToken` (functions, members, helpers, the active enum case in live paths). Save/serialization compatibility preserved through explicitly-marked legacy fields (`ActiveGamblersTokenLevel`, `GamblersTokenUnlockedLevel`), the `Item_GamblersToken` legacy item-ID alias, the deprecated `ET66SecondaryStatType::GamblerToken` enum value, and `+FunctionRedirects` for the two renamed `UFUNCTION`s.
4. **Finding 4 — stale archetype pending docs closed.** `Exploder`/`Stutterer`/`Burrower` notes in `pending_issues_Enemies.md` and `Source/T66/Data/pending_issues_Data.md` updated to record that those speculative special archetypes were dropped by the restructure (live roster uses only `Melee`/`Rush`/`Flying`/`Ranged`), so they no longer contradict the cleanup.
5. **Finding 5 — this packet rewritten** with section-by-section status, exact remaining exceptions, the verification greps + results, and the Phase 2 deferral list.

---

## Summary By Section

### Section A — Removals (complete)
- **A1 Goblin Thief — fully removed.** No residual class/spawn/RNG/Lab/gold-steal references in source. Surviving `Goblin` references are the deprecated, `UMETA(Hidden)` `ET66SecondaryStatType::Goblin` enum value and its paired legacy loc/backend/`Item_Goblin` compat hooks — intentionally retained for save compat (listed in the grep table). No `Goblin` row remains in `Enemies.csv`.
- **A2 Debuff enemy AND projectile — fully removed (Finding 1, was a deviation, now closed).** `AT66UniqueDebuffEnemy` and `AT66UniqueDebuffProjectile` classes deleted; `FT66TemporaryProjectileSystem::ProfileUniqueDebuff` (decl + def + color/shape table entries) removed; active-enemy-projectile counter contributions removed from `T66PerformanceSubsystem.cpp` and `T66LagTrackerSubsystem.cpp`; Backrooms cleanup `IsA` filter removed from `T66GameMode_Backrooms.cpp`; the debug-preview spawn + include removed from `T66PlayerController_Overlays.cpp`. `grep UniqueDebuff|DebuffEnemy|DebuffProjectile|ProfileUniqueDebuff` → 0 hits in Source and Content/Data.
- **A3 Dormant miniboss-promotion tuning — removed.** `MiniBossChancePerWave` and `ActiveMiniBoss` → 0 residual hits. No `[Mm]iniboss` data rows. Tower guardian `ApplyMiniBossMultipliers` constants retained as the source of truth, as required. (The surviving `dormant` source hits are the unrelated boss-proximity-awaken mechanic, not a roster entry.)
- **A4 Archetype clears — done.** `Exploder|Stutterer|Burrower` in `Enemies.csv` → 0 hits. Stale pending docs now closed (Finding 4).
- **A5 `Feeling=MiniBossFeel` clears — done.** `MiniBossFeel` in `Enemies.csv` → 0 hits.

### Section B — Vendor Hidden Boss (complete)
- **B1 Repurpose GamblerBoss→VendorBoss — done.** `AT66GamblerBoss` replaced by `AT66VendorBoss`; old source deleted; no residual `GamblerBoss` class references. Mechanics/hit zones/AOE/token drop preserved.
- **B2 Trigger — ANY failed steal spawns the Vendor boss, no threshold.** (User-locked decision.)
- **B3 Casino anger system — fully removed.** `CasinoAnger|AngerLevel|ShopAnger` → 0 source hits. Casino gambling interactable kept functional (not redesigned).
- **B4 Token rename — now canonical at runtime (Finding 3).** Canonical token = `VendorToken` across item ID (`Item_VendorToken` in `Items.csv`, `SecondaryStatType=VendorToken`), runtime API (`ApplyVendorTokenPickup`, `GetActiveVendorTokenLevel`, `MaxVendorTokenLevel`, member `ActiveVendorTokenLevel`), and helpers (`T66_IsVendorTokenItem`, `T66_ClampVendorTokenLevel`, `GetExtraVendorTokenThresholds`). Save/serialization compat is preserved only through clearly-marked legacy hooks — see Loose Ends and the grep table. Exactly one hidden boss remains: Vendor.

### Section C — Mob-Floor Rename (complete, Finding 2 closed)
- **C1** "gameplay floor" → "mob floor" applied across identifiers, strings, logs, and docs (floor-role enum/flags/layout/NSLOCTEXT, etc., from the original pass) **plus** the three field renames that were previously deferred:
  - `FT66SpawnBudget::GameplayFloorsPerStage` → `MobFloorsPerStage`
  - `FT66SpawnBudget::InitialEnemiesPerGameplayFloor` → `InitialEnemiesPerMobFloor`
  - `AT66EnemyDirector::InitialTowerEnemiesPerGameplayFloor` → `InitialTowerEnemiesPerMobFloor`
  - Applied in `T66PlayerExperienceTypes.h`, `T66PlayerExperienceSubSystem_Spawning.cpp`, `T66StageProgressionSubsystem.cpp`, `T66EnemyDirector.h/.cpp`, `Content/Data/PlayerExperience.json` (all 5 stage blocks), and `Scripts/ValidateEnemyBossRosterData.py`'s required-key list.
  - **Compat bridge:** `Config/DefaultEngine.ini [CoreRedirects]` gains three `+PropertyRedirects` (old→new) so the existing `DT_PlayerExperience.uasset` still binds until the Phase 2 rebuild. This avoids a runtime binding break despite renaming the live UPROPERTY names now.
  - Deliberately **kept** the distinct `GameplayLevelNumber` / `ET66TowerGameplayLevelTheme` / `Resolve*GameplayLevel*` concepts (a separate non-tower-floor concept).
- **C2** Gate guardians confirmed on mob-floor descent transitions 2→3, 3→4, 4→5; gate index math now reads `FirstMobFloorNumber`. No behavior change.

### Section D — Add 10 Mobs And Expand Stages (complete)
- **D1** Stage schema expanded `EnemyA..EnemyJ` → `EnemyA..EnemyL` (struct +2 `FName` slots; `Stages.csv` header +2 columns = 22 total; resolver/preload/director read EnemyK/EnemyL).
- **D2** 10 `Enemies.csv` rows added (`ModelStatus=Placeholder`, `StatusEffectOnHit=None`, `Rarity=Core`, `XPValue=20`): Dungeon CursedCrow(Flying)/FamishedGhoul(Rush); Forest WillOWisp(Flying)/GoreStag(Rush); Ocean GullDiver(Flying)/Hammerjaw(Melee); Martian ReconOrb(Flying)/CarapaceBrute(Melee); Hell CinderWraith(Flying)/BrimstoneBrute(Melee).
- **D3** Each theme's stage rows populated; every roster MobID in `Stages.csv` exists in `Enemies.csv`.

### Section E — Mega-Mob Gate Assignment (complete)
- **E1** Hardcoded `ConfigureAsMob("Slime")` replaced with per-gate assignment via `T66ResolveTowerGateGuardianMobID(...)`. `ApplyMiniBossMultipliers` and must-kill behavior unchanged. `"Slime"` retained only as fallback.
- **E2** Data-driven, no new uasset: gate index from descent context; roster pulled from existing `DT_Stages`. `SlotIndex = (LocalStage-1)*3 + GateIndex` distributes 12 theme mobs across 12 gate encounters.
- **E3** `ResolveEnemyClass` resolves a concrete family for every roster MobID; the 10 new IDs were added to `FT66EnemyFamilyResolver::ResolveFamily` keyed by CSV `FamilyID` (family/behavior routing only — real models deferred to the DataTable rebuild).

---

## Files Changed In This Revision (Findings 1–4)

### Source — C++
- `Source/T66/Gameplay/T66UniqueDebuffProjectile.{h,cpp}` — **deleted** (F1).
- `Source/T66/Gameplay/T66TemporaryProjectileSystem.{h,cpp}` — removed `ProfileUniqueDebuff` decl/def + color/shape table entries (F1).
- `Source/T66/PerformanceSystem/T66PerformanceSubsystem.cpp`, `Source/T66/Core/T66LagTrackerSubsystem.cpp` — removed projectile-count contribution + include (F1).
- `Source/T66/Gameplay/GameMode/T66GameMode_Backrooms.cpp` — removed `IsA(AT66UniqueDebuffProjectile)` cleanup filter + include (F1).
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` — removed debug-preview spawn block + include (F1).
- `Source/T66/Core/PlayerExperience/T66PlayerExperienceTypes.h`, `.../T66PlayerExperienceSubSystem_Spawning.cpp`, `Source/T66/Core/T66StageProgressionSubsystem.cpp`, `Source/T66/Gameplay/T66EnemyDirector.{h,cpp}` — mob-floor field renames (F2).
- `Source/T66/Core/T66RunStateSubsystem.h` — `ApplyVendorTokenPickup` / `GetActiveVendorTokenLevel` / `MaxVendorTokenLevel` / member `ActiveVendorTokenLevel` (F3).
- `Source/T66/Core/RunState/T66RunStateSubsystem_Private.h` — `T66_IsVendorTokenItem` / `T66_ClampVendorTokenLevel` / `MaxVendorTokenLevel`; legacy `T66LegacyGamblersTokenItemID` + deprecated-enum marker retained (F3).
- `Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp` — runtime symbol renames; recompute-skip now matches canonical `VendorToken` AND legacy `GamblerToken` (F3).
- `Source/T66/Core/RunState/T66RunStateSubsystem.cpp`, `.../T66RunStateSubsystem_Snapshot.cpp` — runtime member renames; save-field side (`*.ActiveGamblersTokenLevel`) preserved (F3).
- `Source/T66/UI/HUD/T66HUDPresentationController.cpp`, `Source/T66/Gameplay/T66PlayerController_Combat.cpp`, `Source/T66/Core/T66AchievementsSubsystem.cpp` — call-site/helper renames; legacy item-alias markers (F3).
- `Source/T66/Data/T66DataTypes.h`, `Source/T66/Core/T66LocalizationSubsystem.cpp`, `Source/T66/Core/Backend/T66BackendRunSummaryParser.cpp`, `Source/T66/UI/T66ItemCardTextUtils.cpp` — `GamblerToken` deprecated-enum/legacy-key markers (F3).

### Data / Config / Docs / Scripts
- `Content/Data/PlayerExperience.json` — mob-floor key renames in all 5 stage blocks (F2).
- `Config/DefaultEngine.ini` — `[CoreRedirects]` +3 `PropertyRedirects` (mob-floor, F2) +2 `FunctionRedirects` (VendorToken UFUNCTIONs, F3).
- `Scripts/ValidateEnemyBossRosterData.py` — required-spawn-budget keys renamed to mob-floor (F2).
- `Source/T66/Core/T66RunSaveGame.h`, `Source/T66/Core/T66ProfileSaveGame.h` — legacy-field-name markers on the kept save UPROPERTYs (F3).
- `Source/T66/Gameplay/Enemies/pending_issues_Enemies.md`, `Source/T66/Data/pending_issues_Data.md` — archetype notes closed (F4).

> CSV/JSON → `DT_*.uasset` regeneration remains **deferred to Phase 2** (requires editor). The redirects keep the old assets binding in the meantime.

---

## Verification Greps Run And Results

| Check | Scope | Result |
|---|---|---|
| `UniqueDebuff\|DebuffEnemy\|DebuffProjectile\|ProfileUniqueDebuff` | `Source/T66` | **0 hits** ✅ (F1) |
| `UniqueDebuff\|DebuffEnemy\|DebuffProjectile\|ProfileUniqueDebuff` | `Content/Data` | **0 hits** ✅ (F1) |
| `GameplayFloor` (any mob-floor field) | `Source/T66` | **0 hits** ✅ (F2) |
| `GameplayFloorsPerStage\|InitialEnemiesPerGameplayFloor\|InitialTowerEnemiesPerGameplayFloor` | repo (excl. `Reports/`) | only `DefaultEngine.ini` redirect OldName sides + one historical `Audit/Reference/.../Report.md` snapshot ✅ (F2) |
| `MobFloorsPerStage\|InitialEnemiesPerMobFloor` | `PlayerExperience.json` | present in all 5 blocks ✅ (F2) |
| `GamblersToken\|GamblerToken` | `Source/T66` | only legacy/deprecated compat hooks (see below) ✅ (F3) |
| `GamblersToken\|GamblerToken` | `Content/Data` | none in live data; one historical note in `pending_issues_Data.md` ✅ (F3) |
| `VendorToken` canonical item row | `Items.csv` | `Item_VendorToken,…,VendorToken` present ✅ (F3) |
| `Goblin` | `Enemies.csv` | **0 hits** ✅ |
| `Goblin` | `Source/T66` | deprecated `UMETA(Hidden)` enum + paired legacy loc/backend/`Item_Goblin` compat only ✅ |
| `[Mm]iniboss\|MiniBoss` | `Content/Data` | **0 hits** ✅ |
| `[Dd]ormant` | `Source/T66` | only unrelated boss-proximity-awaken mechanic + one terrain pending doc ✅ |

### Remaining `GamblersToken`/`GamblerToken` hits — all explicitly legacy (Finding 3 acceptable exceptions)
- **Save-serialized fields kept (renaming would break old saves), now marked legacy:** `T66RunSaveGame.h` `ActiveGamblersTokenLevel`; `T66ProfileSaveGame.h` `GamblersTokenUnlockedLevel` (+ its `T66AchievementsSubsystem.cpp` read/write/clamp sites).
- **Deprecated enum value retained for save/DataTable compat, marked:** `ET66SecondaryStatType::GamblerToken` in `T66DataTypes.h` (DEPRECATED comment + listed in `T66IsDeprecatedSecondaryStatType`); paired legacy `case`s in `T66LocalizationSubsystem.cpp` (×3), `T66ItemCardTextUtils.cpp` (×2), the recompute-skip in `T66RunStateSubsystem_EconomyInventory.cpp`, and the deprecated-enum guard in `T66RunStateSubsystem_Private.h`.
- **Legacy item-ID alias for old saves, marked:** `Item_GamblersToken` in `T66RunStateSubsystem_Private.h` (`T66LegacyGamblersTokenItemID`), `T66GameInstance.cpp` (`LegacyGamblersTokenItemID`), `T66PlayerController_Combat.cpp` / `T66HUDPresentationController.cpp` (legacy-alias checks), the `T66PlayerController_Overlays.cpp` legacy-alias smoke check, and the legacy backend key in `T66BackendRunSummaryParser.cpp`.

No compiler/editor build was run (Phase 2 per prompt; not needed as a blocker check). Greps were run via ripgrep over the working tree.

---

## Deferred To Phase 2 (uasset / DataTable / editor rebuilds)

1. **`DT_PlayerExperience.uasset`** rebuild from `PlayerExperience.json` — finalizes the mob-floor key rename in the binary asset and lets the `PropertyRedirects` be retired. Until then the redirects bridge the old field names. (F2)
2. **`DT_Stages.uasset`** rebuild from `Stages.csv` — surfaces the new EnemyK/EnemyL columns to the runtime DataTable.
3. **`DT_Enemies.uasset`** rebuild from `Enemies.csv` — materializes the 10 new placeholder mob rows; family routing already works via the resolver map, but DataTable-backed visual/model stays placeholder until rebuild.
4. **`DT_Items.uasset`** rebuild — re-keys the canonical `Item_VendorToken` / `VendorToken` rows if the editor needs it.
5. **Retire the compat redirects** (`PropertyRedirects` + `FunctionRedirects`) once all uassets/Blueprints are rebuilt and re-saved against the new names; optionally drop the deprecated `GamblerToken` enum value and legacy save fields only behind an explicit save-migration pass.
6. **New-mob real models / behavior tuning** — out of scope; Phase 2+.

---

## Blockers / Deviations From Approved Plan

- **None outstanding.** The prior packet's two open items are resolved this revision:
  - The A2 `AT66UniqueDebuffProjectile` retention deviation is closed — the projectile and all its call sites (player-side debug spawn, perf/lag counters, Backrooms filter) were removed, so the class could be deleted cleanly (F1).
  - The three deferred mob-floor field renames are applied now, with `PropertyRedirects` standing in for the not-yet-rebuilt DataTable (F2) — no runtime binding break.
- The recompute-skip in `T66RunStateSubsystem_EconomyInventory.cpp` was broadened to also skip the canonical `VendorToken` secondary-stat (previously only the deprecated `GamblerToken`), fixing a latent double-count for canonical vendor-token items. Flagging for Codex sign-off as the one behavior-affecting change made while closing F3.

No scope expansion into Minigames, no Git writes, no binary-asset mutation, no editor commandlets.

---

## Token Ledger

Not reliably available to the operator at write time (no in-session token accounting surfaced). Omitted rather than estimated.
