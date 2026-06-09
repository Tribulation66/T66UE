I have enough to assess this. The implementation is fully feasible internally—no user decision required.

Result: OK

## Independent Answer

Yes, this is straightforward and fully implementable within the existing HUD/minimap architecture. The pieces already exist:

- **Trap source**: `UT66TrapSubsystem::GetRegisteredTraps()` returns `TArray<TWeakObjectPtr<AT66TrapBase>>` (Source/T66/Core/T66TrapSubsystem.h:34). Traps self-register, so no world scan is needed. Note traps are **not** in `UT66ActorRegistrySubsystem`, so Codex must pull from the trap subsystem, not the actor registry used for the other markers.
- **Marker mechanics**: `RefreshMapData()` (T66GameplayHUDWidget_Map.cpp:112) already builds `FT66MapMarker` entries with `Color` + `Visual`. The `ET66MapMarkerVisual::Dot` visual (T66GameplayHUDWidget_Private.h:412) is exactly the "red circle" requested — used today for enemies with `DrawSize` ~6px. A trap marker is `Visual = Dot`, `Color = red`, added the same way enemy/POI markers are.

Recommended approach for Codex:
1. In the `bNeedsFullRefresh` cache-build block, fetch `UT66TrapSubsystem::Get(World)` and iterate `GetRegisteredTraps()`, adding cache entries (either a new `EMapCacheMarkerType::Trap` or a Dot-style entry with a red color).
2. In the per-marker build loop, render as `Dot` with a red `FLinearColor` and a small `DrawSize`.
3. Apply `ShouldShowTowerObjectMarker(Location)` so trap markers respect the active-floor view in tower layout (per the world-map reference rule).
4. Focused compile/build verification per repo rules (runtime-facing UI change).

## Evidence Checked
- `T66GameplayHUDWidget_Map.cpp` — full `RefreshMapData()` marker-build pipeline (NPC/Gate/Miasma/POI/Enemy paths).
- `T66ActorRegistrySubsystem.h` — confirmed no trap registration exists there.
- `T66TrapSubsystem.h` — confirmed `GetRegisteredTraps()` provides the trap list.
- `T66GameplayHUDWidget_Private.h` — `ET66MapMarkerVisual` enum (Dot/Icon/Cross) and `FT66MapMarker` struct; Dot is the existing small-circle marker.

## Questions Or Blockers
None requiring the user. Two judgment calls Codex can make without escalating:
- Whether traps show always vs. proximity-only on the minimap (enemies use a ~2400-unit radius cap). "Wherever there is a trap" suggests following the always-visible object-marker pattern with active-floor filtering.
- Whether to gate trap markers behind a discovery/reveal condition. Default to always-show unless the user later wants traps hidden until discovered.

## Caveats
- Confirm the `Dot` visual renders as a filled circle (not a square) at the chosen size — verify visually in standalone, since "red circle" is the explicit ask.
- Trap density could be high; the enemy path has a `MaxMinimapEnemyMarkers` cap (48) — consider whether traps need a similar cap to avoid minimap clutter, though traps are typically fewer.
- Don't forget active-floor filtering in tower layout; skipping it would show traps from other floors, contradicting the world-map reference rule.
