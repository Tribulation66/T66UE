# Section 6 - Memory and Loading

## Level Structure

The game uses whole-map flow, not runtime level streaming.

Configured maps:

- `/Game/Maps/FrontendLevel`
- `/Game/Maps/GameplayLevel`

Map files found:

- `Content/Maps/FrontendLevel.umap`
- `Content/Maps/GameplayLevel.umap`

No runtime usage found for:

- `LoadStreamLevel`
- `ULevelStreaming`
- source-level World Partition runtime flow

Only editor World Partition settings were found in config.

## Floor Loading

The seven-floor stage structure is generated at runtime inside the active gameplay map.

`T66TowerMapTerrain` and GameMode paths spawn floors, shell walls, maze walls, props, and lighting. Floor/chunk streaming was not found.

Stage/gate transitions currently reopen the current level in some paths rather than streaming floor chunks.

## Async Loading

Async loading exists through `UAssetManager::GetStreamableManager().RequestAsyncLoad`.

Static call sites include:

- Core DataTables in `T66GameInstance`.
- Core presentation assets in `T66GameInstance`.
- Gameplay transition assets.
- Second-stage visual warmup.
- Boss classes.
- UI texture pool.
- Audio subsystem.
- VFX caches.

## Synchronous Loading Hotspots

Sync-load risk remains in:

- `ResolveCachedDataTable`, which can call `WaitUntilComplete()` on the core DataTable handle.
- `T66CharacterVisualSubsystem`, which uses `LoadObject` and `LoadSynchronous` for shared material, mob VAT table, meshes, materials, and textures.
- Gameplay/world helper paths for terrain/world setup.
- Boss flow material/texture/mesh setup.

`StaticLoadObject` had no hits in `Source/T66`.

## Texture and Mesh Streaming

Config:

- Texture streaming enabled.
- Mesh streaming disabled.
- Texture pool sizes set by quality tier: 400, 600, 800, 1000 MB.

Asset risk:

- Some UI textures are `NeverStream=True`, appropriate for UI but important for memory accounting.
- Several character/environment textures are 512-2048 and streamable.
- Loose runtime dependencies include large UI/video folders.

## Garbage Collection

No overrides found for:

- `gc.TimeBetweenPurgingPendingKillObjects`
- other `gc.*` CVars

## Runtime Memory Footprint

No `stat memory`, MemReport, or memory artifact was found.

Required next capture:

- Packaged standalone MemReport at frontend idle.
- Gameplay level after entering stage.
- Stage with combat/VFX active.
- After several floor transitions or stage resets.

