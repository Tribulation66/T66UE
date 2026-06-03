# Deprecated Content

Tracks content/features that are **deprecated** (turned off or removed as a
feature decision), as distinct from content that is merely **demo-gated**.
Demo-only gating lives in `Demo/DEMO_GATED_INVISIBLE_CONTENT.md`.

Difference vs. demo gating:
- **Deprecated** = disabled in all builds via a feature setting or removed
  entirely; not expected to ship without an explicit revival decision.
- **Demo-gated** = present in the full game, only hidden in the Steam demo via
  the release-variant gate.

This is a **documentation-only** inventory. Centrally declared arcade-deprecated
items are listed here as inventory entries; minigames are no longer part of this
deprecated category and now live in `Demo/DEMO_GATED_INVISIBLE_CONTENT.md`.

---

## Feature Gates (centrally declared deprecated features)

Central seam: `UT66DeprecatedFeatureSettings`
(`Source/T66/Core/T66DeprecatedFeatureSettings.h` / `.cpp`), backed by
`Config/DefaultGame.ini` `[/Script/T66.T66DeprecatedFeatureSettings]`.
Helper accessors: `T66DeprecatedFeatures::AreArcadeGamesDisabled()`,
`AreArcadeInteractablesDisabled()` (each defaults to disabled when the settings
object is missing).

| Feature | Config flag (current) | Accessor | Status |
|---|---|---|---|
| Arcade games | `bDisableArcadeGames=true` | `AreArcadeGamesDisabled()` | Deprecated / disabled |
| Arcade interactables (world machines) | `bDisableArcadeInteractables=true` | `AreArcadeInteractablesDisabled()` | Deprecated / disabled |

### Arcade games — DEPRECATED
- **Seam:** `Config/DefaultGame.ini` `bDisableArcadeGames=true`.
- **Consumers:** `Source/T66/Gameplay/T66ArcadeInteractableBase.cpp` (multiple guards), `Source/T66/UI/Screens/T66VersusArcadeScreen.cpp`, `Source/T66/UI/T66ArcadeSelectionWidget.cpp`.
- **Note (documentation-only):** `Config/DefaultDemoMode.ini` still carries `+AllowedArcadeGameIDs=` (`Arcade_WhackAMole`, `Arcade_Topwar`, `Arcade_GoldMiner`, `Arcade_BladeSweep`). These are demo allow-list entries retained for forward compatibility; they do **not** re-enable arcade because the central deprecated feature gate disables arcade games in all builds. Listed here, not in the demo-gated inventory, because the controlling decision is deprecation, not demo scope.
- **Revive:** Set `bDisableArcadeGames=false` (and review arcade runtime separately — out of scope for this pass).

### Arcade interactables — DEPRECATED
- **Seam:** `Config/DefaultGame.ini` `bDisableArcadeInteractables=true`.
- **Consumers:** `Source/T66/Gameplay/T66ArcadeMachineInteractable.cpp`, `Source/T66/Gameplay/T66ArcadeInteractableBase.cpp`, `Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp`, `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp`.
- **Revive:** Set `bDisableArcadeInteractables=false` (runtime review out of scope).

> Minigames are **no longer deprecated**. They moved to demo-gated-invisible
> status (present in the full game, hidden only in the Steam demo). See
> `Demo/DEMO_GATED_INVISIBLE_CONTENT.md`.

---

## Compatibility-Retained (deprecated fields kept for save/data compatibility)

These are **not feature gates**. They are deprecated data-model fields/values
that remain in source only so existing CSV rows, DataTables, and serialized save
data keep deserializing. They do not gate any visible feature and should not be
confused with the feature gates above. Reviving them is not meaningful; they are
listed so cleanup work does not mistake them for live or demo-gated content.

| Item | Location | Note (from source comment) |
|---|---|---|
| Per-level gain ranges (hero stat struct) | `Source/T66/Data/T66DataTypes.h:265` | "Deprecated per-level gain ranges retained for Heroes.csv/DataTable compatibility." |
| Random gain range (hero row) | `Source/T66/Data/T66DataTypes.h:713`, `:744` | "Deprecated random gain range retained for hero row compatibility." |
| Deprecated secondary stat types | `Source/T66/Data/T66DataTypes.h:902-942` (`T66IsDeprecatedSecondaryStatType`) | Enum values kept for serialized save/data compatibility (`Goblin`, `Leprechaun`, `Fountain`, `CloseRangeDamage`, `LongRangeDamage`, `SpinWheel`, `MovementSpeed`, `HpRegen`, `LifeSteal`, `Alchemy`, `Accuracy`, `Cheating`, `Stealing`, `VendorToken`). `T66IsLiveSecondaryStatType` is the live-only inverse. |
| Legacy item-template rows | `Source/T66/Data/T66DataTypes.h:985`, `:991` | "deprecated legacy rows kept for compatibility"; Line 2 multiplier preserved for legacy item-card text only. |
| Legacy per-idol max level | `Source/T66/Data/T66DataTypes.h:1622` | "Legacy per-idol max level. Live progression uses rarity tiers instead of XP levels." |
| Enemy ranged projectile base | `Source/T66/Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.h:17` | "DEPRECATED: Enemy Ranged projectiles are now owned by `UT66ProjectileManagerSubsystem`." |
| Widget-game legacy ID lookup | `Source/T66/Public/UI/WidgetGames/T66WidgetGameDescriptor.h:16` (`LegacyGameID`), `T66WidgetGameRegistry.h:16` (`FindByLegacyID`) | Legacy widget-game ID retained for backward lookup. |

> Note: numerous combat/diagnostic log strings in
> `Source/T66/Gameplay/T66CombatComponent.cpp` use "legacy"/"compatibility"
> vocabulary for preserved diagnostic field names. Those are logging/diagnostic
> labels, not deprecated features or data fields, and are intentionally omitted
> here.

---

## Out of Scope / Pending (recorded elsewhere)

Deprecated-code cleanup items already tracked in pending-issue files (not part of
this docs pass):
- `Source/T66/Gameplay/pending_issues_Gameplay.md` — deprecated Niagara emitter
  readiness API (`FNiagaraEmitterInstance::IsReadyToRun`), dormant legacy
  MainMapTerrain decoration names, and deprecated rich basic-mob routing branches
  pending cleanup.
