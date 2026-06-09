# Tooltip Infrastructure Phase 1/2 - Codex Patch Summary

## Scope

Implemented the approved tooltip infrastructure pass for Phase 1 and Phase 2:

- Shared tooltip model and renderer:
  - `Source/T66/UI/T66TooltipTypes.h/.cpp`
  - `Source/T66/UI/T66TooltipSlate.h/.cpp`
  - `Source/T66/UI/T66TooltipResolvers.h/.cpp`
- Metadata/dump support:
  - `FT66FlatWidgetMetadata` now carries tooltip id, kind, has-tooltip, and tooltip-required flags.
  - `FT66WidgetTreeWalker` now exports tooltip fields under both `interactivity` and `t66_metadata`.
- Helper consolidation:
  - `T66StatsPanelSlate` delegates to the shared tooltip renderer.
  - HUD `CreateCustomTooltip` and `CreateRichTooltip` delegate to the shared tooltip renderer.
  - `FT66FlatStyle::MakeFlatTooltipIcon` uses the shared tooltip payload path.
- Pilot coverage:
  - Stats panel lines continue to show rich stat descriptions through the shared renderer.
  - Gameplay HUD ultimate, equipped weapon, idol slots, inventory items, and mob loot use typed payloads.
  - Vendor shop stock, buyback, and sell-strip cards/actions use item, mob-loot, and vendor-action payloads.
  - Powerup permanent relic cards and temporary single-use buff cards use powerup payloads.

## Dirty Baseline Caveat

The worktree was already dirty before this pass, including unrelated HUD changes in files touched by this task. The tooltip edits are scoped to the infrastructure and pilot surfaces listed above. Do not treat unrelated pre-existing HUD diffs, such as ragdoll recovery or minimap drawing changes, as part of this tooltip implementation.

## Verification

Focused compile passed:

```text
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66Editor Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex -NoHotReloadFromIDE
Result: Succeeded
Compiled Module.T66.38.cpp, linked UnrealEditor-T66.lib and UnrealEditor-T66.dll.
Total execution time: 19.97 seconds.
```

Staged readiness was attempted and failed in the cook phase, before smoke:

```text
powershell -ExecutionPolicy Bypass -File .\Scripts\RunStagedBuildReadinessGate.ps1
Status: FAIL
Output root: C:\UE\T66\Saved\StagedBuildReadiness\20260608_043755
Error: StageStandaloneBuild.ps1 failed with exit code 25.
Smoke suite: NOT_RUN
Observed cook blockers: missing /Game/Data/DT_HouseNPCs and missing /Game/World/VisualProps/Easy/SM_BrokenVase_Easy_Pixal3D from DT_WorldVisualProps.
```

## Known Follow-Up

The wider tooltip philosophy still needs future coverage for gambler minigames, settings controls, leaderboard/run-count warnings, map markers, quest/objective prompts, pause menu controls, hero/companion selection, and world interactables. This patch establishes the shared infrastructure and first UI pilots.
