# memory.md — T66 agent context

**Purpose:** This file is **context for Cursor agents** — key rules, where to look, and current state. Use it to onboard quickly and avoid breaking conventions. Full change history lives in **git log**.

---

## For Cursor agents

- **Read this file first** before making changes.
- **Acknowledge the localization rule** in your next message (see HARD RULE below).
- **Build command (PowerShell):**  
  `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development "C:\UE\T66\T66.uproject" -waitmutex`
- **Repo path:** `c:\UE\T66` (Windows).

---

## HARD RULE (Non‑Negotiable) — Localize every new player-facing string

**Any time you add or change player-facing text, complete the full localization pipeline in the same change-set.**

1. **Author:** Use **`LOCTEXT` / `NSLOCTEXT`** or String Tables. Never `FText::FromString(TEXT("..."))` for shipping UI. No per-language `switch(CurrentLanguage)`.
2. **Gather:** Run GatherText so keys land in `Content/Localization/T66/`.
3. **Translate:** Populate all 22 cultures (e.g. `.archive` or `Scripts/AutoTranslateLocalizationArchives.py`).
4. **Compile:** Regenerate `Content/Localization/T66/<culture>/T66.locres`.
5. **Verify:** Confirm text changes when switching culture.

**Acknowledge in your next message:** e.g. “Acknowledged: new player-facing text will be localized (author → gather → translate → compile → verify).”

---

## Where to look (quick reference)

| Need to… | Look here |
|----------|-----------|
| **Skin ownership / purchase / equip** | `UT66SkinSubsystem` (hero + companion). `GetSkinsForEntity()`, `PurchaseSkin()`, `SetEquippedSkin()`, `OnSkinStateChanged`. |
| **AC balance / profile save** | `UT66AchievementsSubsystem`: `GetAchievementCoinsBalance()`, profile load/save. Skin *data* is in profile; skin *API* is in SkinSubsystem. |
| **Hero/companion definitions** | `UT66GameInstance` (GetHeroData, GetCompanionData, GetAllHeroIDs, etc.) + DataTables `DT_Heroes`, `DT_Companions` + CSVs under `Content/Data/`. |
| **Front-end UI obsidian 9-slice** | *(Stale — `EnsureObsidianBrushes()` was never implemented. `Content/UI/Obsidian.uasset` exists but is unused by code. Panels now use `FT66Style::MakePanel()` with texture-based dark/light brushes. Safe to delete the Obsidian asset.)* |
| **Dark/Light theme** | `FT66Style::SetTheme(ET66UITheme)` / `GetTheme()`. Tokens are mutable (non-const); `SetTheme` swaps palette, re-inits styles. Persisted in `T66PlayerSettingsSaveGame::bLightTheme`, getter/setter in `T66PlayerSettingsSubsystem`. Toggle buttons in MainMenu bottom-left (next to globe). `ToggleActive` button style for selected theme. Old style set kept alive (`GPreviousStyleSet`) to prevent dangling pointer crashes. |
| **Button debounce** | `FT66Style::DebounceClick(FOnClicked)` — wraps any click delegate with 150ms global cooldown. `MakeButton` uses it automatically; custom buttons should wrap with it too. Prevents double-fire, spam crashes, accidental double-nav. |
| **Buttons (create / style)** | **Generate:** `Scripts/GenerateButtonTextures.py` → PNGs in `SourceAssets/Images/Buttons/` (Dark/Light × N/H/P). **Import:** `Scripts/ImportButtonTextures.py` (editor) → `/Game/UI/Assets/`. **Procedural:** `Scripts/CreateButtonProceduralAssets.py` + `SourceAssets/Data/ButtonProcedural_MaterialInstances.json` → M_ButtonProcedural + 6 MIs. **Usage:** `FT66Style::MakeButton(...)` only; debounced via `DebounceClick`. See **guidelines.md §11**. |
| **Panels (create / style)** | **Generate:** `Scripts/GeneratePanelTextures.py` → `PanelDark.png`, `PanelLight.png` in `SourceAssets/Images/Panels/`. **Import:** `Scripts/ImportPanelTextures.py` (editor) → `/Game/UI/Assets/PanelDark.uasset`, `PanelLight.uasset`. **Usage:** `FT66Style::MakePanel(Content, FT66PanelParams)` or `MakePanel(Content, Type, Padding)`; types `Bg`, `Panel`, `Panel2`. Do not use raw `SBorder` + brush name. See **guidelines.md §11**. |
| **Procedural button material** | One UI material **M_ButtonProcedural** + 6 MIs (Dark/Light × N/H/P) in `/Game/UI/Assets/`. Created by **Scripts/CreateButtonProceduralAssets.py**; source of truth **SourceAssets/Data/ButtonProcedural_MaterialInstances.json**. When all 6 MIs load, T66Style uses them for Primary/Neutral/Danger/ToggleActive. Fallback: ButtonDark/ButtonLight textures, then rounded box. Button text shadow = Tokens::Border. |
| **Widget rebuild (DeferRebuild)** | `FT66Style::DeferRebuild(Widget, ZOrder)` in `T66Style.h/.cpp`. **Always** use this instead of raw `ReleaseSlateResources`/`TakeWidget`/`ForceRebuildSlate`. Defers rebuild to next tick, prevents dangling-pointer crashes from click/key handlers. `UT66ScreenBase::ForceRebuildSlate()` is a thin wrapper. See **guidelines.md §6.5**. |
| **Hero speed & animation** | `UT66HeroSpeedSubsystem`: speed still ramps (acceleration/deceleration unchanged). Animation is two-state only: **alert** when idle (speed 0), **run** when moving (any speed > 0). No walk state. Hero drives subsystem from movement input; companions copy same anim state. Params: `FHeroData::MaxSpeed`, `AccelerationPercentPerSecond` (Heroes.csv). |
| **Character visuals / meshes / animations** | `UT66CharacterVisualSubsystem` (ApplyCharacterVisual, alert/walk/run). GetMovementAnimsForVisual(VisualID, OutWalk, OutRun, OutAlert). Data: `DT_CharacterVisuals` / `CharacterVisuals.csv` (RunAnimation column). |
| **Lobby (co-op Duo/Trio)** | `UT66LobbyScreen`: party slots (You + Waiting for player…), Invite Friend (stub), Continue → Hero Selection. Reached from Party Size Picker when New Game + Duo/Trio. |
| **Hero selection UI** | `T66HeroSelectionScreen`: `RefreshSkinsList()`, `AddSkinRowsToBox()`, `SkinsListBoxWidget`, `ACBalanceTextBlock`, `PreviewSkinIDOverride`. Uses `UT66SkinSubsystem`. |
| **Companion selection UI** | `T66CompanionSelectionScreen`: same pattern (SkinsListBoxWidget, RefreshSkinsList, AddSkinRowsToBox); uses `UT66SkinSubsystem`. |
| **3D hero preview** | `AT66HeroPreviewStage` (Tick → CaptureScene after pawn anim). `T66HeroBase::InitializeHero(bPreviewMode)` → alert anim in preview. |
| **Preview materials** | `T66PreviewMaterials.h/.cpp`: `GetGroundMaterial()` / `GetSkyMaterial()` / `GetStarMaterial()` — loads from `/Game/UI/Preview/` or auto-creates in editor. All use single VectorParam→output (one hop, no chains). Used by both hero + companion preview stages. |
| **Localization** | `UT66LocalizationSubsystem::GetText_*()`. All UI strings via this or NSLOCTEXT; no hardcoded player-facing text. |
| **Run state (hearts, gold, inventory)** | `UT66RunStateSubsystem`. |
| **Pixelation (retro, scene only)** | `UT66PixelationSubsystem`: `SetPixelationLevel(0–10)`. Console: `Pixel0` (off), `Pixel1`–`Pixel10`. Post-process material at `Content/UI/M_PixelationPostProcess` (hand-built asset; C++ loads it). Only 3D scene pixelated; UI stays crisp. |
| **Save/load run** | `UT66SaveSubsystem`, `UT66RunSaveGame`. Profile (AC, skins, achievements): `UT66ProfileSaveGame`, saved by AchievementsSubsystem. |
| **Power Up Crystals** | `UT66PowerUpSubsystem`: `GetPowerCrystalBalance()`, `AddPowerCrystals()`, `UnlockPowerupSlice()`, `GetPowerupSlicesUnlocked()`, `GetPowerupStatBonuses()`. Save: `UT66PowerUpSaveGame` (slot `T66_PowerUp`). RunState: `GetPowerCrystalsEarnedThisRun()`, `AddPowerCrystalsEarnedThisRun()`; powerup bonuses in stat getters. Screen: `UT66PowerUpScreen` (main menu → Power Up). Run Summary: Power Crystals + AC blocks in right column. |
| **Damage Log (run)** | `UT66DamageLogSubsystem`: run-scoped damage by source. `RecordDamageDealt(FName SourceID, int32 Amount)`, `GetDamageBySourceSorted()` (returns `TArray<FDamageLogEntry>`), `ResetForNewRun()`. Source IDs: `SourceID_AutoAttack`, `SourceID_Ultimate`. Recorded at `TakeDamageFromHero` / `TakeDamageFromHeroHit`; projectile sets `DamageSourceID`. Run Summary: "DAMAGE BY SOURCE" panel to the right of Power Crystals. Snapshot: `UT66LeaderboardRunSummarySaveGame::DamageBySource` (SchemaVersion>=5). |
| **Combat (AoE slash attack)** | `UT66CombatComponent::TryFire()` → `PerformSlash` → `ApplyDamageToActor` + `SpawnSlashVFX`. Splash: `OverlapMultiByChannel` sphere at target. VFX: `AT66SlashEffect` (flat disc, fade-out). Material: `/Game/VFX/M_SlashDisc` (generated by `Scripts/CreateSlashEffectMaterial.py`). Params: `SlashRadius` (base 300u), `ProjectileScaleMultiplier` (from Scale stat). |
| **The Lab** | Entry: Hero Selection "THE LAB" button (above Enter the Tribulation) → sets `UT66GameInstance::bIsLabLevel`, opens `LabLevel`. `AT66GameMode::IsLabLevel()`; Lab-only BeginPlay (floor, light, hero+companion; no waves/NPCs/gates). `UT66LabOverlayWidget`: Items panel (unlocked only), Enemies tabs (NPC/Mobs/Stage Bosses), Reset Items/Enemies, Exit. Spawn: `SpawnLabMob`, `SpawnLabBoss`, `SpawnLabTreeOfLife`; `ResetLabSpawnedActors`. Unlocks: `UT66ProfileSaveGame::LabUnlockedItemIDs`, `LabUnlockedEnemyIDs`; `UT66AchievementsSubsystem::AddLabUnlockedItem/Enemy`, `IsLabUnlockedItem/Enemy`. RunState `ClearInventory()` for Lab Reset Items. |

---

## Current state

- **Project:** T66 (Tribulation 66) · **Engine:** UE 5.7
- **Active branch:** `version-0.9`
- **Build:** ✅ C++ compiles (ValidateFast as above).
- **Skins:** Centralized in `UT66SkinSubsystem`. Hero and companion selection use it; AC and skin inventory persist (profile, no one-time reset).
- **Pipelines:** Sprites (`Scripts/ImportSpriteTextures.py`), world models (`Scripts/ImportModels.py`), ground atlas (`Scripts/BuildGroundAtlas.py`, `ImportGroundAtlas.py`). Full setup: `Scripts/RunFullSetup.bat` or editor `T66Setup`.

---

## Guardrails

- Atomic change-sets; ValidateFast after each.
- Small, descriptive commits.
- UI: **event-driven** only (no per-frame UI logic).
- **All UI text localized** via `UT66LocalizationSubsystem::GetText_*()` or NSLOCTEXT.
- **After C++ changes:** Close the editor, run the build command, then reopen. Do not use Live Coding / Hot Reload — the Config UObject `UT66RngTuningConfig` can trigger "Cannot replace existing object of a different class" (CDO replace crash) if the module is reloaded while the editor is running.
- **Sync load (main menu):** The only allowed sync load is in **UT66MainMenuScreen::BuildSlateUI()**: **UT66UITexturePoolSubsystem::EnsureTexturesLoadedSync()** is called for main menu + Party Picker textures (MMDark, MMLight, T_LB_Global, T_LB_Friends, T_LB_Streamers, SoloDark, SoloLight, CoopDark, CoopLight) so the background, leaderboard filter icons, and Party Picker cards appear immediately with no white flash. This is a safe "loading window" (frontend, not gameplay); no other sync texture loads in UI.
- **Level transitions:** When transitioning to GameplayLevel from the frontend (Enter Tribulation, Load Game), use **`UT66GameInstance::TransitionToGameplayLevel()`** instead of raw `UGameplayStatics::OpenLevel`. This shows a loading screen, pre-loads heavy assets, then opens the level. In-game stage reloads (StageGate, CowardiceGate, etc.) can still use raw `OpenLevel` since assets are already warm.

---

## Open questions / blockers

- **LabLevel map:** Create in editor before using The Lab: File → New Level → Empty Level (or duplicate FrontendLevel). Add a **PlayerStart** at (0, 0, 200). Optionally add a simple floor (plane) and Directional Light + Sky Light for a white grid-room look. World Settings → GameMode Override: use same as GameplayLevel (e.g. BP_GameplayGameMode). Save as **LabLevel** in `Content/Maps/` so the path is `/Game/Maps/LabLevel`. Code opens via `OpenLevel(this, FName(TEXT("LabLevel")))`.
- **WBP_LanguageSelect / WBP_Achievements:** Optional; C++ works without. Re-run `T66Setup` if you add Blueprint overrides.
- **Leaderboard:** Placeholder until Steam.
- **Nav:** Enemies use Tick toward player; no nav required. For future pathfinding, add Nav Mesh Bounds in GameplayLevel.

**Procedural terrain:** Implemented. Plan: `T66_Procedural_Landscape_Plan.md`. Editor: Window → T66 Tools → "Generate Procedural Hills Landscape (Dev)" (creates/updates Landscape in current level). Runtime: seed set in `UT66GameInstance::ProceduralTerrainSeed` when clicking "Enter the Tribulation"; `AT66GameMode::GenerateProceduralTerrainIfNeeded()` applies hills on GameplayLevel load. **Caveat:** Height apply uses editor-only API; in **packaged** builds terrain is not regenerated (use level as saved). Logs use `[MAP]` prefix. **Landscape & foliage:** Optional consolidated folder **Content/T66MapAssets**: run **Window → T66 Tools → Setup T66 Map Assets** once to copy sources there. **Grass:** from Polytope (`SM_Grass_02`, `MI_Plants_Grass`) — HISM every quad, full ground coverage, no collision. **Landscape, trees, rocks:** from Cozy Nature (`MI_Landscape`, `Grass_LayerInfo`, trees/rocks meshes and MIs). Tool loads T66MapAssets first, then Polytope/CozyNature. Trees: **10 total**, rocks: **6 total**, placed **procedurally at random**. No cast shadow; actors tagged `T66ProceduralFoliage`. Material usage fix: Polytope M_Plants, Cozy M_Trees, M_Rocks.

---

## Known issues / tech debt

- **T66RngTuningConfig CDO crash:** If you see "Cannot replace existing object of a different class" for `Default__T66RngTuningConfig`, close the editor, build from command line, then reopen. Avoid Live Coding when changing T66 C++; full restart prevents the CDO replace crash.
- Coliseum: `ColiseumLevel.umap` may be missing; code falls back to GameplayLevel.
- Optional: move ID-keyed copy (achievements, hero/companion names) to String Tables for designers.

---

## Architecture (summary)

- **Core:** `T66GameInstance` (hero/companion/data tables), `T66AchievementsSubsystem` (profile, AC, save), `T66SkinSubsystem` (all skin API), `T66CharacterVisualSubsystem` (meshes/anim), `T66LocalizationSubsystem`, `T66RunStateSubsystem`, `T66SaveSubsystem`.
- **UI:** `T66UIManager`, `T66ScreenBase`; screens under `UI/Screens/` (Slate in `BuildSlateUI()`). Hero/Companion selection: skin list + AC from SkinSubsystem; refresh via `RefreshSkinsList()`.
- **Gameplay:** `T66GameMode`, `T66HeroBase`, `T66CompanionBase`, `T66EnemyBase`, preview stages in `Gameplay/`.
- **Data:** `Source/T66/Data/T66DataTypes.h` (FHeroData, FCompanionData, FSkinData, etc.). DataTables and CSVs in `Content/Data/`.
- **Flow:** Frontend (MainMenu → PartySizePicker → [Solo: HeroSelection | Duo/Trio New Game: Lobby → HeroSelection] → CompanionSelection → Enter) → GameplayLevel. Run state in RunStateSubsystem; death → RunSummary.

---

## Keystone audit (Bible vs repo)

- Checklist: `T66_Keystone_Audit_Report.md`
- Reconciled sections listed in that report; keep Bible and repo in sync.

---

## Recent context (for continuity)

- **Map 100k × 100k:** Playable area is 100,000 × 100,000 UU (half-extent 50,000). `T66MiasmaBoundary::SafeHalfExtent = 50000`; procedural landscape MainMap preset 1009×1009, origin -50400; all spawn positions, gates, arenas, NPC corners, start/boss/tutorial/Coliseum/Boost scaled. HUD minimap full bounds ±50000. Lab half-extent 12500. **Miasma walls:** Pass-through (no collision), black smoke-like look: WallHeight 2400, WallThickness 500, very dark color. **Fog:** Density 0.12, MaxOpacity 0.96 for very short visibility (~5–6k).
- **Power Up Crystals system:** New meta-progression: **Power Crystals** earned by killing bosses (10 per boss, stage or Coliseum). Persisted in **UT66PowerUpSubsystem** (own save slot `T66_PowerUp`). **Power Up** screen (main menu, below Load Game): 6 stats (Damage, Attack Speed, Attack Size, Evasion, Armor, Luck), each with 10 unlockable slices; 10 crystals per slice; each slice adds +1 to that stat for all heroes. Run Summary shows Power Crystals earned this run + total, and Achievement Coins stubs (0 earned, 10,000 total). Main menu order: New Game, Load Game, Power Up, Achievements, Settings. RunState: `PowerCrystalsEarnedThisRun`, `PowerupStatBonuses` (refreshed at run start); stat getters include powerup bonus. Boss kill: `HandleBossDefeated` → `AddPowerCrystalsEarnedThisRun(10)`. Run end: crystals added to subsystem before showing Run Summary.

- **Pixelation (post-process):** Replaced `r.ScreenPercentage`-based pixelation with a post-process material. `UT66PixelationSubsystem` adds a blendable to the level's post-process volume; material does floor(UV*PixelGridSize)/PixelGridSize and samples PostProcessInput0 with point filtering. Only the 3D scene is pixelated; UI stays crisp. **Pixel0** = off, **Pixel1**–**Pixel10** = levels. Material is a hand-built editor asset at `/Game/UI/M_PixelationPostProcess` (C++ loads it; no programmatic creation).
- **Preview vs gameplay look:** Hero and companion preview SceneCaptures copy the world's unbound `APostProcessVolume::Settings` into `SceneCapture->PostProcessSettings` in `EnsureCaptureSetup()` (so the capture gets the same auto exposure and saturation as the main view). `PostProcessBlendWeight = 1.0f`. Without this copy, SceneCaptureComponent2D only uses its own (empty) post process, not the level's PP volume.

- **Party Size Picker:** Only Solo and Co-op (no Duo/Trio). Co-op → Lobby (3 slots). New Game + Solo → Hero Selection; New Game + Co-op → Lobby; Load Game → Save Slots.
- **Lobby (Bible-style):** Left = players in lobby (hero portrait + You / Waiting for player…). Right = Friends list (stub). Bottom right = Select Hero, Ready Check → popup → Enter the Tribulation. Hero Selection from Lobby has no Enter button; Back saves hero to GI.
- **Lobby (legacy note):** New Game → Co-op now opens **Lobby** (`ET66ScreenType::Lobby`, `UT66LobbyScreen`) instead of going directly to Hero Selection. Lobby shows party slots (You + “Waiting for player…” for empty slots), INVITE FRIEND (stub), CONTINUE → Hero Selection, Back → Party Size Picker. No Steam backend yet; Continue proceeds to Co-op Hero Selection without session. Localization: `GetText_LobbyTitle`, `GetText_LobbyYou`, `GetText_LobbyWaitingForPlayer`, `GetText_LobbyInviteFriend`, `GetText_LobbyContinue`.

- **Map / interactable placement:** World interactables (trees, trucks, wheels, totems) and stage effect tiles use **run-level seed** (`ProceduralTerrainSeed` from GameInstance, or `FMath::Rand()` when 0) so positions change every time "Enter the Tribulation" or PIE is started. **NPCs:** same 4 corner positions; which NPC (Vendor/Gambler/Ouroboros/Saint) is at which corner is **shuffled per run**. **Plateaus:** `AT66SpawnPlateau` (flat disc) is spawned under each world interactable, each corner NPC, and each stage effect tile so they sit on flat ground even on hills. **Grass:** procedural landscape editor no longer spawns grass HISM (`bSpawnGrassAssets = false` in `T66ProceduralLandscapeEditorTool.cpp`); landscape and trees/rocks unchanged.

- **Central skin subsystem:** `UT66SkinSubsystem` holds all skin logic (hero + companion). AchievementsSubsystem keeps profile and delegates skin calls to SkinSubsystem. Hero and companion selection use `GetSkinsForEntity()`, `RefreshSkinsList()`, dynamic AC display.
- **Persistence:** Profile (AC, owned/equipped skins) persists; one-time reset that cleared purchases was removed.
- **Hero selection:** When switching heroes, skin is validated for the *new* hero (if not owned, falls back to Default); `PreviewSkinIDOverride` cleared on switch.
- **Companion selection:** Same skin/AC wiring as hero selection (SkinsListBoxWidget, RefreshSkinsList, AddSkinRowsToBox, SkinSubsystem).

**Grass on landscape:** StylizedGrassByMayu pack copied to `Content/World/StylizedGrassByMayu/`. Generate Procedural Hills Landscape (Dev) now assigns the grass landscape material and imports the Grass weight layer at full coverage so the ground is covered with grass.

**Landscape look (less crunchy, more atmosphere):** GameMode `SpawnLightingIfNeeded()` spawns **Exponential Height Fog** (gentle density, soft blue-grey) and an unbound **PostProcessVolume** (exposure + slight desaturation) when missing, so distant terrain reads with atmospheric depth. Grass/ground material tuning (reduce normal strength, increase roughness, macro variation, blend variants): see **T66_Grass_Stylization_Plan.md** section “Procedural map (Polytope grass + Cozy landscape)”. Optional script: `Scripts/TuneGrassAndLandscapeMaterials.py` (run in editor) to set normal/roughness on T66MapAssets MIs if base materials expose those parameters.

**Fonts:** Single theme with 6 options (index in T66Style.cpp: GThemeFontIndex). 0=Caesar Dressing, 1=Cinzel, 2=Cormorant SC, 3=Germania One, 4=Grenze, 5=Alagard (default). Console command **T66NextFont** cycles to next font and re-inits style. Paths under `Content/Slate/Fonts/New Fonts/`. Alagard source: `SourceAssets/Font/Alagard/alagard.ttf`. Aztec, Almendra, Uncial removed (code and assets).

**UI borders:** All T66 panel/button brushes (Bg, Panel, Panel2, Stroke, Circle, Primary/Neutral/Danger buttons) use white outline (Tokens::Border, Tokens::BorderWidth) so each element is visually distinct.

**Dark/Light theme system:** Dark (default) = black bg, yellow text; Light = grey bg, white text. `ET66UITheme` enum in `T66Style.h`. `FT66Style::SetTheme()` updates mutable `Tokens::` colors, keeps old `FSlateStyleSet` alive in `GPreviousStyleSet` (prevents `ACCESS_VIOLATION` from dangling brush pointers), then re-initializes styles. `UT66PlayerSettingsSubsystem::SetLightTheme()` persists, applies theme, saves. MainMenu has "DARK"/"LIGHT" toggle buttons (next to language selector). Theme change calls `ForceRebuildSlate()` which does `RemoveFromParent()` + `AddToViewport()` to re-create the entire widget tree, ensuring instant visual update. All buttons globally debounced via `FT66Style::DebounceClick` (150ms cooldown) integrated into `MakeButton`.

**Risk fixes (batch):**
- **AC economy fix:** Removed dev hack that restored AC balance to 10,000 on every profile load (`T66AchievementsSubsystem.cpp`). 10k starting grant now only applies to fresh profiles (first creation). Players who spend AC below 10k will stay below across sessions.
- **Leaderboard texture lifetime:** `ST66LeaderboardPanel` icon/GIF textures now load via `UT66UITexturePoolSubsystem` + `T66SlateTexture::BindSharedBrushAsync` instead of raw `LoadObject` + `SetResourceObject`. Prevents potential GC crash where Slate outlives UObject.
- **GameMode sync load removed:** Removed redundant `LoadSynchronous()` block in `SpawnFloorIfNeeded()` for ground floor materials; the existing async fallback path in the same function handles loading and re-application.
- **Leaderboard localization:** Dropdown display text (party size, difficulty, type) now uses the existing NSLOCTEXT-backed getter functions (`GetPartySizeText`, `GetDifficultyText`, `GetTypeText`) instead of raw `FText::FromString`. Internal keys remain English FStrings for handler mapping.
- **Heros/Heroes path:** All `Heros` paths fully renamed to `Heroes`: music subsystem, Characters, Sprites, CSVs, and import scripts. Only `SourceAssets/Heros/` and the script filename `ImportHeros.py` still use the old spelling (cosmetic, not runtime).

---

## Known issues / tech debt

- **T66RngTuningConfig CDO crash:** If you see "Cannot replace existing object of a different class" for `Default__T66RngTuningConfig`, close the editor, build from command line, then reopen. Avoid Live Coding when changing T66 C++; full restart prevents the CDO replace crash.
- Coliseum: `ColiseumLevel.umap` may be missing; code falls back to GameplayLevel.
- Optional: move ID-keyed copy (achievements, hero/companion names) to String Tables for designers.
- **Combat world scans (deferred):** `T66CombatComponent::TryFire()` uses 3x `TActorIterator<>` (GamblerBoss, BossBase, EnemyBase) per fire tick (up to 20/s). Currently only 1 combat component active (hero only), so perf is acceptable. When companion combat or higher enemy counts are added, replace with a cached enemy registry (enemies register in BeginPlay, unregister in EndPlay). Files: `T66CombatComponent.cpp:271`.
- **WebView2 HTTP hardening (deferred):** `T66WebView2Host.cpp:304` allows both HTTP and HTTPS. Should reject plain HTTP (TikTok serves HTTPS only). Also `storage.googleapis.com` is overly broad — tighten to specific bucket prefix. Low urgency (requires network-level MITM to exploit).
- **Heros → Heroes folder rename (complete):** Both `Content/Characters/Heros/` and `Content/UI/Sprites/Heros/` renamed to `Heroes/` in UE Editor. All CSV paths, import scripts, and code updated. Only `SourceAssets/Heros/` folder and `ImportHeros.py` filename still use old spelling (cosmetic).
- **Boss/Enemy models removed:** `Content/Characters/Bosses/` and `Content/Characters/Enemies/` deleted; corresponding rows removed from `CharacterVisuals.csv`. Enemy/boss actors fall back to code-driven placeholder shapes (FT66VisualUtil). New models can be added by creating rows in CharacterVisuals.csv + importing assets.
- **Theme: in-game lighting (deferred):** Dark/Light theme currently only changes UI palette (+ sun/moon directional light toggle). Future intent: extend further (e.g. HDRI swap for day/night).

**3D Hero/Companion preview environment:** Both `AT66HeroPreviewStage` and `AT66CompanionPreviewStage` include a sky dome (inverted sphere, dynamically scaled), ambient light, theme-reactive lighting, and parameterized materials. **Simplified material pattern (v3):** Every material uses a single VectorParameter → one output pin (no multi-node chains, which fail silently in C++ material construction). Three materials in `/Game/UI/Preview/`: `M_PreviewGround` (BaseColor param → BaseColor, DefaultLit), `M_PreviewSky` (SkyColor param → EmissiveColor, self-lit), `M_PreviewStar` (StarColor param → EmissiveColor, self-lit). Auto-created in editor via `T66PreviewMaterials::GetGroundMaterial()` / `GetSkyMaterial()` / `GetStarMaterial()` (WITH_EDITORONLY_DATA). **Stars:** 20 small UStaticMeshComponent spheres (engine Sphere mesh) scattered on upper hemisphere via fixed seed (7777), hidden in day mode, shown in dark mode with bright emissive. **Dynamic dome scaling (fix):** Backdrop dome now scales dynamically in `FrameCameraToPreview()` so it always encloses the camera; characters with large mesh bounds push the camera far away — old fixed-size dome left the camera in void (black preview). Stars, ambient light attenuation, and key/fill/rim attenuation also scale. **Blue sky (fix):** Sky is now blue in both themes: Day = FLinearColor(2.0, 3.5, 6.0), Night = FLinearColor(1.2, 2.0, 4.0), matching gameplay level's SkyAtmosphere look. Star color = FLinearColor(8, 8, 7) warm white. **Preview diagnostics:** `ApplyCharacterVisual` logs material names/classes per slot when `bIsPreviewContext` is true, and forces texture streaming (`bForceMipStreaming`, `StreamingDistanceMultiplier=50`). Orbit bounds logged with `[PREVIEW]` prefix. Old assets must be deleted when material graph changes (C++ recreates on next PIE).

- **AoE Slash attack (replaces homing projectile):** Hero auto-attack changed from single-target homing projectile to an **instant AoE slash** centered on the target. `TryFire()` finds the nearest enemy (same Gambler > Boss > Enemy priority), then: (1) deals full damage to the primary target, (2) does an `OverlapMultiByChannel` sphere query at the target's location (radius = `SlashRadius * ProjectileScaleMultiplier`, default 300u base) to damage all nearby enemies/bosses, (3) spawns `AT66SlashEffect` VFX (translucent disc, scale-up + fade-out over 0.25s), (4) plays shot SFX. New files: `T66SlashEffect.h/.cpp` (short-lived disc actor), `Scripts/CreateSlashEffectMaterial.py` (generates `/Game/VFX/M_SlashDisc` — Translucent Unlit, "Color" → Emissive, "Opacity" → Opacity). New helper: `UT66CombatComponent::ApplyDamageToActor()` centralizes damage dispatch. `AT66HeroProjectile` class retained but no longer spawned by default (available for future ranged heroes). **To generate the material asset:** run `Scripts/CreateSlashEffectMaterial.py` in-editor or via `-run=pythonscript`.

- **Guidelines Section 10 — Material creation pattern:** Added to `guidelines.md`. When a new material `.uasset` is needed, Cursor should proactively offer to create it via a Python editor script (`Scripts/`) following the `CreatePreviewMaterials.py` pattern. Script is source of truth (git-tracked), `.uasset` is generated artifact.

**Procedural UI buttons:** M_ButtonProcedural + 6 MIs; CreateButtonProceduralAssets.py + ButtonProcedural_MaterialInstances.json. T66Style prefers MIs, then ButtonDark/ButtonLight textures, then rounded box.

**UI panels (centralized):** All panels use **FT66Style::MakePanel()** with **ET66PanelType** (Bg, Panel, Panel2). Textures: **GeneratePanelTextures.py** → `SourceAssets/Images/Panels/PanelDark.png`, `PanelLight.png`; **ImportPanelTextures.py** (editor) → `/Game/UI/Assets/PanelDark.uasset`, `PanelLight.uasset`. Style uses 9-slice brushes when textures exist, else procedural. No raw `SBorder` + `BorderImage("T66.Brush.Panel")`; see **guidelines.md §11**.

**DeferRebuild (widget rebuild safety):** **Never** call `ReleaseSlateResources`, `TakeWidget()`, or raw `RemoveFromParent + Release + AddToViewport` directly. Always use **`FT66Style::DeferRebuild(Widget, ZOrder)`** — defers rebuild to next tick so Slate finishes processing click/key events before the widget tree is torn down. `ForceRebuildSlate()` on `UT66ScreenBase` is now a thin wrapper around `DeferRebuild`. Replaced all 18+ dangerous sites across Lab/Collector overlays, HeroSelection, Settings, PowerUp, ThemeToggle, Lobby, MainMenu, Achievements, CompanionSelection, LanguageSelect, UIManager. See **guidelines.md §6.5**.

**Performance / loading optimizations (batch):**
- **Blurry UI textures fixed:** `UT66UITexturePoolSubsystem::HandleLoaded()` and `EnsureTexturesLoadedSync()` now set `Tex->bForceMiplevelsToBeResident = true` on every UI texture after load, ensuring full-resolution mips stay resident. Applies to all textures loaded via the texture pool (Party Picker, items, idols, leaderboard icons, etc.).
- **PIE startup lag reduced:** `UT66GameInstance::Init()` now pre-warms all 9 main-menu + Party Picker textures (added SoloDark, SoloLight, CoopDark, CoopLight) so the sync fallback in `MainMenuScreen::BuildSlateUI()` is a no-op more often.
- **PIE resolution zoom flash fixed:** Frontend UI initialization in `AT66PlayerController::BeginPlay()` is deferred by 0.1s via `SetTimer` so the PIE viewport stabilizes its resolution before the first UI frame renders. Prevents the "zoomed-in then snaps to correct size" flash.
- **Enter Tribulation loading screen:** New `UT66LoadingScreenWidget` (Slate, `UI/T66LoadingScreenWidget.h/.cpp`). `UT66GameInstance::TransitionToGameplayLevel()` shows loading screen → pre-loads gameplay assets → opens level. Called from Hero Selection, Lobby, and Save Slots instead of raw `OpenLevel`. Loading text localized via `UT66LocalizationSubsystem::GetText_Loading()`.
- **Gameplay asset pre-loading:** `UT66GameInstance::PreloadGameplayAssets()` async-loads engine cube mesh, ground materials, and placeholder material before `OpenLevel`, so `GameMode::BeginPlay` finds them already resident.
- **Cached cube mesh:** `AT66GameMode::GetCubeMesh()` delegates to `FT66VisualUtil::GetBasicShapeCube()`.
- **LoadObject → FT66VisualUtil refactor:** All `LoadObject<UStaticMesh>` calls for `/Engine/BasicShapes/{Cube,Sphere,Cylinder}` and `LoadObject<UMaterialInterface>` for `M_PlaceholderColor` across ~20 gameplay files now use `FT66VisualUtil::GetBasicShapeCube/Sphere/Cylinder()` and `GetPlaceholderColorMaterial()`, which cache internally. Eliminates redundant per-actor sync loads.
- **Staggered spawning:** `AT66GameMode::SpawnLevelContentAfterLandscapeReady()` now runs in two phases across two frames: Phase 1 = structures + NPCs, Phase 2 (next tick) = combat systems + boss. Reduces single-frame hitch.

**Audit cleanup (batch):**
- **Deleted unused assets:** `Content/Characters/Heroes/` (Hero1–Hero9 Meshy AI exports, unreferenced), `Content/Characters/Test/` (Goodtest_rigged, unreferenced), `Content/Placeholder/tiktok.png` (unreferenced).
- **Audio path bug fixed:** `T66MusicSubsystem.cpp` hero music folder changed from `/Game/Audio/OSTS/Heros/` to `/Game/Audio/OSTS/Heroes/` to match the on-disk folder name. Hero theme music should now load correctly.
- **LoadObject centralization:** ~20 gameplay files refactored from direct `LoadObject<UStaticMesh/UMaterialInterface>` to `FT66VisualUtil::GetBasicShapeCube/Sphere/Cylinder()` and `GetPlaceholderColorMaterial()`. Each utility caches with a static TObjectPtr, eliminating per-actor duplicate sync loads.

**Lumen removed — HDRI cubemap SkyLight lighting:** Lumen GI and hardware ray tracing disabled (`DefaultEngine.ini`: `r.DynamicGlobalIlluminationMethod=0`, `r.ReflectionMethod=2`, `r.Lumen.HardwareRayTracing=False`, `r.RayTracing=False`). Characters and scene now lit by a **boosted SkyLight** (intensity 2.5, was 0.8) using a **studio HDRI cubemap** (`/Game/Lighting/TC_HDRI_Studio`) that replicates UE asset-preview lighting quality — even, omnidirectional ambient from all angles. Fallback: if cubemap not found, SkyLight uses captured scene (SkyAtmosphere) at same boosted intensity. Lower hemisphere fill enabled (`bLowerHemisphereIsBlack=false`, `LowerHemisphereColor=(0.4,0.4,0.4)`) to prevent dark undersides. Directional lights (sun/moon) and SkyAtmosphere remain for visual sky + direct light. **Result:** all characters (current and future imports) look consistently bright regardless of material quirks, and GPU load is significantly reduced. **Setup for new imports:** run `Scripts/AddEmissiveFillToCharacterMaterials.py` (sets EmissiveColorMap = DiffuseColorMap at weight 0.12 for guaranteed minimum brightness). **HDRI import:** `Scripts/ImportHDRI.py` imports `SourceAssets/studio_small_09_2k.exr` as HDR Texture2D; user must right-click → "Create Texture Cube" → name `TC_HDRI_Studio` in `/Game/Lighting/`.

**Full history:** `git log` (this file is context, not a full changelog).
