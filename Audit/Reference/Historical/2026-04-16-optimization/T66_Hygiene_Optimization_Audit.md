Below is a consolidated hygiene + optimization audit of the T66 Unreal project (UE 5.7, modules `T66`, `T66Mini`, `T66Editor`). I grouped findings by the three symptoms you described, plus general hygiene. Everything is backed by `file:line` refs so you can jump straight into fixes.

---

## 1. Why loading is slow

There is *good* plumbing already in place (`FStreamableManager::RequestAsyncLoad`, a persistent gameplay curtain, `UT66UITexturePoolSubsystem`), but a lot of code paths *bypass* it with `LoadSynchronous()` / `StaticLoadObject` / `LoadObject` / `EnsureTexturesLoadedSync`. Every single one of those, if hit on the game thread, = a hitch.

### Party invite → join path

The accept path itself is clean:

```2107:2130:Source/T66/Core/T66SessionSubsystem.cpp
// HandleSessionUserInviteAccepted -> JoinPartySession -> SessionInterface->JoinSession
```

No asset loading in `UT66PartySubsystem` or the invite modal. The hitch you feel is almost entirely the **blocking travel** at the end:

- `UT66SessionSubsystem::StartGameplayTravel` → `World->ServerTravel` (`T66SessionSubsystem.cpp:547`)
- Fallback + standalone paths use `UGameplayStatics::OpenLevel` (`T66SessionSubsystem.cpp:219, 254, 799, 1702` and `T66GameInstance.cpp:1408`)

`OpenLevel` / `ServerTravel` are engine-blocking by design. What stacks on top of that is:

1. `UT66CharacterVisualSubsystem::ResolveVisual` sync-loading hero meshes/anims when the party pawn initializes on the destination map  
   (`Source/T66/Core/T66CharacterVisualSubsystem.cpp:434–444, 598, 605, 679–723`). Note the explicit comment “this is synchronous” in the code — that's your per-hero hitch.
2. `PreloadGameplayAssets` completion re-enters `PreloadCharacterVisual` which just calls `ResolveVisual` again (`T66GameInstance.cpp:1300–1310`) — essentially wasting the async batch you already queued.
3. Party re-invite broadcasts force `CurrentModal->ForceRebuildSlate()` when an update arrives while the modal is open:

```733:738:Source/T66/Gameplay/T66PlayerController.cpp
if (CurrentModalType == ET66ScreenType::PartyInvite)
{
    if (UT66ScreenBase* CurrentModal = UIManager->GetCurrentModal())
    {
        CurrentModal->ForceRebuildSlate();
    }
    return;
}
```

**Fixes, in priority order:**
- Make `ResolveVisual` genuinely async: when `PreloadGameplayAssets` completes, the objects are already resident, so `ResolveVisual` must early-out and just `Get()` the resolved pointers.
- Every call to `GetXxxDataTable()` that falls back to `LoadSynchronous()` (`T66GameInstance.cpp:542–817`) should *block on `AreCoreDataTablesLoaded()`* or stall behind the loading curtain instead — a sync fallback in production means a guaranteed hitch if order-of-init changes.
- Update the `PartyInviteModal` in place (store `STextBlock` handles, update text) instead of `ForceRebuildSlate()` on every invite status change.

### Hero selection → game path

The transition itself is well-designed:

```1389:1410:Source/T66/Core/T66GameInstance.cpp
ShowPersistentGameplayTransitionCurtain();
UT66LoadingScreenWidget* LoadingWidget = CreateWidget<...>(World, UT66LoadingScreenWidget::StaticClass());
	if (LoadingWidget)
	{
		LoadingWidget->AddToViewport(9999);
	}
	...
	World->GetTimerManager().SetTimer(PreloadTimerHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		...
	}), 0.05f, false);
```

But it gets undone by:

- `AT66HeroPreviewStage::UpdatePreviewPawn → InitializeHero → ApplyCharacterVisual → ResolveVisual` (sync) every time the hero changes on the select screen (`T66HeroPreviewStage.cpp:307–346`). That is the hitch you feel on the select screen itself.
- `T66PreviewStageEnvironment.cpp:286, 306` — `LoadObject` of fallback material + per-decor mesh synchronously.
- `UT66HeroSelectionScreen` calls `EnsureTexturesLoadedSync` during Slate build for balance icons & party portraits (`T66HeroSelectionScreen.cpp:1181–1187, 1302–1305`) — even though directly adjacent code (line 1213–1218) uses `T66SlateTexture::BindSharedBrushAsync`. The pattern is already there, just inconsistently applied.
- `UT66PlayerController_Frontend` uses `LoadClass<UT66ScreenBase>` per-Mini-screen (`T66PlayerController_Frontend.cpp:468–483, 707`), which syncs the whole module/package on first open of the Mini frontend.
- `T66MainMapTerrain.cpp:945–960` (and the theme loads at ~160 and 1782) has a big block of `LoadObject` for meshes/materials/textures when the terrain is built. Moving these to preloaded soft references is a big win at “first spawn into gameplay”.
- `T66PlayerController.cpp:163–164` — `JumpVFXNiagara`/`PixelVFXNiagara` `LoadSynchronous()` at PC setup.
- `T66CombatVFX.cpp:764–765` — `WarmupVFXSystems` uses `LoadSynchronous`.
- `T66CombatComponent.cpp:260` — `ShotSfx.LoadSynchronous()`.

**Rule of thumb:** do one grep-pass for `.LoadSynchronous(`, `TryLoad(`, `StaticLoadObject(`, `LoadObject<`, `LoadClass<`, `EnsureTexturesLoadedSync`. Any match that can run during gameplay is a hitch candidate. The codebase already has `RequestAsyncLoad` plumbing at:
- `T66GameInstance.cpp:299, 450, 525, 1289`
- `UT66UITexturePoolSubsystem::RequestTexture` at `T66UITexturePoolSubsystem.cpp:145–150`

so the task is mostly consistency, not new infrastructure.

### Mini character select (worst offender on select→game)

```544:565:Source/T66Mini/Private/UI/Screens/T66MiniCharacterSelectScreen.cpp
for (const FT66MiniHeroDefinition& Hero : Heroes)
{
    ...
    if (UTexture2D* HeroTexture = VisualSubsystem->LoadHeroTexture(Hero.DisplayName))
    {
        Brush->SetResourceObject(HeroTexture);
    }
}
```

`LoadHeroTexture` → `LoadImportedOrLooseTexture` → `StaticLoadObject` (`T66MiniVisualSubsystem.cpp:250–272`). That's `N_heroes` sync texture loads per `BuildSlateUI()`, and the grid is a dense `SUniformGridPanel` inside an `SScrollBox`, not a virtualized list. Mirror `UT66UITexturePoolSubsystem::RequestTexture` here and/or preload the set once during frontend init.

---

## 2. Why FPS drops in game

The highest-impact issues cluster in `T66Mini`. All of these run **per frame**:

### Full-world scans in Tick

- `AT66MiniEnemyBase::FindBestTargetPawn` → `TActorIterator<AT66MiniPlayerPawn>` every tick, for every enemy (`T66MiniEnemyBase.cpp:211–254, 404–430`). This is O(enemies × players_in_world) per frame.
- `AT66MiniPickup::Tick` → full `TActorIterator` every tick, for every pickup (`T66MiniPickup.cpp:165–211, 253–279`).
- `AT66MiniInteractable::Tick` → same pattern (`T66MiniInteractable.cpp:172–204, 313–339`).
- `AT66MiniHazardTrap::ApplyPulse` → two world iterators per pulse, for pawns and enemies (`T66MiniHazardTrap.cpp:191–220`).
- `AT66MiniProjectile::ExplodeAt` + bounce targeting → full enemy iterator per explode/bounce (`T66MiniProjectile.cpp:322–346, 361–375`).
- `AT66MiniGameMode::Tick → PositionPartyPawns()` rebuilds the party list every frame even when formation hasn’t changed (`T66MiniGameMode.cpp:196–216, 434–437, 67–87`).
- `AT66MiniBattleHUD::DrawHUD` finds the boss with a `TActorIterator<AT66MiniEnemyBase>` on every HUD draw (`T66MiniBattleHUD.cpp:581–589`).

**Fix pattern:** keep a single authoritative list of `LivePawns` + `LiveEnemies` on the GameMode (you already have `LiveEnemies` there in main T66). Register on spawn, unregister on death. Replace every per-frame iterator with a read of that list. For pickups/interactables, use an `UPrimitiveComponent` overlap or a spatial hash that you populate only when a pawn moves a meaningful distance.

### Main T66 Tick hotspots

- `AT66EnemyBase::Tick → T66ResolveClosestPlayerPawn(this)` every frame, which iterates `GetPlayerControllerIterator()` per enemy (`T66EnemyBase.cpp:868–871, 124–151`). Even with 4 players, that's `enemies × 4` chasing pointers every frame. Cache on GameState instead.
- `AT66MiasmaManager::Tick` calls `UpdateFromRunState()` + `EnsureSpawnedCount()` + `SetScalarParameterValue("Brightness", ...)` every frame (`T66MiasmaManager.cpp:93–118, 283–340`). The MID scalar push only needs to happen when brightness actually changes; the spawn checks should be driven by a "stage-step-changed" event.
- `AT66LavaPatch::Tick` – `SetScalarParameterValue` every frame (`T66LavaPatch.cpp:127–136`).
- `UT66MiniHitFlashComponent` – `PrimaryComponentTick.bCanEverTick = true` and `ApplyTintedMaterial` every tick while flashing (`T66MiniHitFlashComponent.cpp:9–12, 94–108`). Disable the component tick when `FlashRemaining <= 0` — free win.

### VFX / actor churn (no pooling in Mini)

- `UT66MiniVFXSubsystem::SpawnPulse` allocates a new `AT66MiniFlipbookVFXActor` every call (`T66MiniVFXSubsystem.cpp:25–69`).
- Each of those actors ticks and calls `ApplyTintedMaterial` every frame, which re-sets 2 texture params + 3 vector params on the MID even if nothing changed (`T66MiniFlipbookVFXActor.cpp:29–49`, `T66MiniVfxShared.h:46–85`).
- `AT66MiniGroundTelegraphActor` — same pattern (`T66MiniGroundTelegraphActor.cpp:29–44`).

Main T66 already has the right pattern via `UT66FloatingCombatTextPoolSubsystem` — port that to Mini's flipbook VFX. Also, only push MID parameters on actual change (track `LastTint`, `LastAlpha`, early-out).

Minor: `UT66FloatingCombatTextPoolSubsystem::AcquireActor` calls `CompactPools()` on every acquisition (`T66FloatingCombatTextPoolSubsystem.cpp:22–23`). Compact periodically or only above a watermark.

### UMG / HUD overdraw

- Every `AT66EnemyBase` owns **two** `UWidgetComponent`s (health bar + lock indicator) and `InitWidget()` is called for both in `BeginPlay` — even though they're hidden (`T66EnemyBase.cpp:222–237, 469–479`). That's `2 × enemy_count` Slate trees allocated up-front.
  - Combine into a single widget or, better, draw these via an `AHUD`/Canvas pass that projects world positions for *only visible* enemies.
- `UT66GameplayHUDWidget::NativeTick` runs every frame with: pause checks, `RefreshHUD()` (full), `RefreshSpeedRunTimers()`, DPS accumulation, tower-map reveal update, pickup-card fade, presentation queues, scoped-ult text formatting, crosshair follow (`T66GameplayHUDWidget.cpp:2460–2494`).
  - `RefreshSpeedRunTimers` already has a centisecond-guard pattern (line ~4424–4432) — apply the same guard to the scoped-ult/shot text update at `2550–2563`. The `FString::Printf + SetText` every frame is a measurable GC-churn source.
- `T66MiniBattleHUD::DrawHUD` also does per-frame `T66MiniLoadHudTexture` on cache miss (`StaticLoadObject` — `T66MiniBattleHUD.cpp:42–62`) and per-frame inventory stack rebuilds. Preload HUD textures once at level start; update inventory on change events.
- `UT66GameplayHUDWidget::RefreshBossBar` does `BossPartBarsBox->ClearChildren()` then adds fresh slots whenever it refreshes (`T66GameplayHUDWidget.cpp:4459–4499`). If it refreshes often, diff by part-id and update existing slots.
- `UT66HeroGridScreen::BuildSlateUI` + `UT66MiniCharacterSelectScreen` build dense grids without virtualization (`T66HeroGridScreen.cpp:96–121`). For larger hero/companion rosters you'll want `STileView` / `STableViewBase`.

### Math / microscale

- `AT66MiniEnemyBase::Tick` uses `ToPlayer.Size2D()` (sqrt) every frame (`T66MiniEnemyBase.cpp:257–258`). Use `SizeSquared2D` for range checks and only `Size` when you need the normalized direction.
- `AT66EnemyBase::Tick` does `UE_LOG(..., Verbose, ...)` every tick (`T66EnemyBase.cpp:870–871`). Fine with `Verbose` off but remove it if not wanted in dev builds.

---

## 3. C++ hygiene (dev-time & structural)

Not runtime wins, but they compound — faster iteration makes it easier to keep the game tight.

### Header fan-out (big compile-time tax)

- `Source/T66/Core/T66RunStateSubsystem.h:5–12` pulls in `Data/T66DataTypes.h`, `T66IdolManagerSubsystem.h`, `T66RunSaveGame.h`, `T66Rarity.h`. Since `T66DataTypes.h` includes `Engine/DataTable.h`, every consumer drags in the whole data-table machinery.
- `Source/T66/Core/T66GameInstance.h:5–11` — same pattern.
- `Source/T66/Core/T66BackendSubsystem.h:5–10` — also includes `IHttpRequest.h` / `IHttpResponse.h`. Any UI widget that mentions the backend type pays for HTTP headers.
- `Source/T66/Core/T66AchievementsSubsystem.h:7–8` vs `:12` — includes `T66LocalizationSubsystem.h` *and* forward-declares the same class. Drop the include.
- `Source/T66/Core/T66LagTrackerSubsystem.h:8, 92–128` — includes `Engine/World.h` only because `FLagScopedScope` is inline. Move the impl to the `.cpp` (or an `.inl`).

Forward-declare class pointers in these hub headers, move `#include`s to the `.cpp`s. On a project this size the link and IWYU savings are substantial.

### `.Build.cs` dependency surface

- `Source/T66/T66.Build.cs:12–22` lists `SlateCore`, `UMG`, `HTTP`, `Json` in `PublicDependencyModuleNames`. Every consumer of T66 inherits those. Unless T66's *public headers* actually expose Slate/UMG/HTTP/Json types in signatures, move them to `PrivateDependencyModuleNames`.
- `Source/T66Mini/T66Mini.Build.cs:10–18` exposes `T66` publicly, so everything consuming Mini inherits the whole main-module public surface too. Consider private unless Mini's API really exposes T66 types.

### API hygiene

- `SetPendingPickerSnapshots(TArray<...> Snapshots)` at `T66LeaderboardSubsystem.h:81` passes by value — should be `const TArray&`.
- Many subsystem getters return `TArray` by value (`T66RunStateSubsystem.h:384 GetInventory()`, `T66BackendSubsystem.h:304`, `T66MiniLeaderboardSubsystem.h:56`, `T66PartySubsystem.h:101–102`). If the callers are native, prefer `const TArray&` accessors; keep `UFUNCTION` copies only where BP needs them.
- `T66BackendSubsystem.h:374–387` and `T66MiniLeaderboardSubsystem.h:116–117` have multiple `TMap<FString, …>` caches. If keys are stable (difficulty, board id, steam token), `FName` is faster and zero-alloc.
- `T66RunStateSubsystem.h` has a *very* large set of `DECLARE_DYNAMIC_MULTICAST_DELEGATE` macros (`:14–39` and the whole file). Dynamic multicasts are slower than `DECLARE_MULTICAST_DELEGATE`. For events with only C++ listeners, use the native version; keep dynamic only where BP truly subscribes.
- `T66Style.cpp:531` uses `check(StyleInstance.IsValid())`. If style can fail in Shipping, this fires only in Dev. Replace with explicit validation + fallback.

### Subsystem choice

Overall the split between `UGameInstanceSubsystem` (session-scoped — run state, backend, save, rng) and `UWorldSubsystem` (world-scoped — enemy pool, trap, pixel VFX, actor registry) is sound. Nothing obvious to move.

### Module duplication

Main T66 and T66Mini have parallel-but-separate `RunStateSubsystem`, `LeaderboardSubsystem`, `VFXSubsystem`, `VisualSubsystem` etc. That's fine structurally, but the two VFX pipelines diverge in quality: main T66 has pooling, Mini doesn't. Mini should borrow the pool patterns wholesale (it's already linking against T66).

---

## 4. Prioritized action plan

If you want to spend engineering time by impact, I'd tackle it in this order:

| # | Task | Expected payoff |
|---|------|-----------------|
| 1 | Kill all `TActorIterator` calls in every `Tick()` / `DrawHUD` in `T66Mini` (enemies, pickups, interactables, HUD boss lookup). Replace with a `LivePawns` / `LiveEnemies` list on the GameMode. | **Biggest in-game FPS win**, especially as enemy count grows. |
| 2 | Pool `AT66MiniFlipbookVFXActor` + `AT66MiniGroundTelegraphActor` and early-out `ApplyTintedMaterial` when tint/alpha didn't change. | Removes the per-pulse alloc churn and MID-param spam. |
| 3 | Make `UT66CharacterVisualSubsystem::ResolveVisual` async-first: if assets are resident (post-preload), just bind; never call `LoadSynchronous` on the game thread. | Kills hero-select hitch and the hitch on party join. |
| 4 | Sweep `LoadSynchronous` / `StaticLoadObject` / `EnsureTexturesLoadedSync` and replace with `UT66UITexturePoolSubsystem::RequestTexture` / `FStreamableManager::RequestAsyncLoad`. High-value call sites: `T66HeroSelectionScreen.cpp:1181, 1302`, `T66MainMenuScreen.cpp:408, 1536`, `T66MiniCharacterSelectScreen.cpp:544–565`, `T66MiniVisualSubsystem.cpp:250–272`, `T66MainMapTerrain.cpp:945–960`, `T66PlayerController.cpp:163–164`, `T66CombatVFX.cpp:764–765`, `T66CombatComponent.cpp:260`. | Removes the hitches you feel on first spawn, opening screens, entering the frontend. |
| 5 | Replace per-enemy `UWidgetComponent` health + lock bars with a single `AHUD`/Canvas pass for on-screen bars (or a single shared widget per target). Same for hero cooldown bar. | Steady-state cost drops with enemy count; less first-spawn hitching. |
| 6 | Apply the centisecond guard (already used for speedrun timer) to scoped-ult and scoped-shot text; make MID scalar pushes (`Miasma`, `LavaPatch`) *change-gated*. | Stops FString/FText allocation every frame; removes render-thread parameter-set work. |
| 7 | Gate `MiniHitFlashComponent` tick with `SetComponentTickEnabled(false)` when idle. | Free win, scales with enemy count. |
| 8 | Header hygiene pass on `T66RunStateSubsystem.h`, `T66GameInstance.h`, `T66BackendSubsystem.h`. Forward-declare instead of include. Move HTTP/UMG/Slate out of T66's *public* `.Build.cs`. | Faster iteration, cleaner module boundaries. |
| 9 | Replace BP-only dynamic multicasts in `T66RunStateSubsystem.h` with native multicasts for C++-only listeners; keep dynamic for BP. | Smaller broadcast cost on every state change. |
| 10 | Virtualize Hero Grid + Mini character select grid (`STileView`). Diff-update boss part bars instead of `ClearChildren`. | Smoother UI, cheaper rebuilds when hero roster grows. |

Items 1–4 are the ones that will most directly address your three complaints (“joining party”, “loading into game after hero selection”, “in-game FPS drops”). 5–7 give you smooth steady-state performance once you're in combat. 8–10 are quality-of-life and long-term hygiene.

If you'd like, switch me out of Ask mode and point me at one of these (I'd recommend #1 or #3 as the single biggest felt improvement) and I'll implement it with tests and a before/after stat.
