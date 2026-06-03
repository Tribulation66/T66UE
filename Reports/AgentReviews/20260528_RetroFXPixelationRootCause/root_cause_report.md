# RetroFX / Pixelation Unexpected Enablement Root Cause

## Resolution 2026-05-29

The recurrence root cause is resolved by `Reports/AgentReviews/20260529_RetroFXOffByDefaultFix/completion_packet.md`.

The fix makes gameplay RetroFX, real low resolution, and frontend fullscreen CRT off by default, removes the duplicate saved master flag, forces existing saves off through schema 24 migration, seals reset/safe-mode/UI reset/save-load/settings/world startup paths, and makes the off path restore `r.ScreenPercentage=100`.

## Review Status

- Working goal: diagnose why T66 retro/pixelation post-FX turns on unexpectedly, identify the side-effect trigger and source-of-truth problem, and produce a root-cause finding plus permanent fix proposal without unrelated edits.
- Claude review: `Reports/AgentReviews/20260528_RetroFXPixelationRootCause/20260528T212718-pass1/claude_review_pass1.md`
- Verdict: `APPROVE`
- Scope followed: diagnostic/report-only. No runtime code, content, config, or save edits were made by this pass.

## Executive Finding

Pixelation/retro-FX recurs because gameplay RetroFX is default-on in committed source and is re-applied by normal settings/world startup paths. The strongest visible symptom is not necessarily the T66 pixelation post-process material. The logs show `WorldPixelation=0.00` / PS1 scalars at zero while `RealLowRes=true` is applied, causing `UT66RetroFXSubsystem` to lower `r.ScreenPercentage` at runtime. That downscale presents as a pixelated/retro look even with scalar pixelation strengths at zero.

The trigger is any unrelated work that causes settings to be created, migrated, reset, safe-mode-applied, loaded on gameplay world settings change, or re-applied after RetroFX async asset preload. Those paths all route through defaults or saved state where gameplay RetroFX master and real-low-resolution are currently on.

This is a committed-code source-of-truth bug, amplified by persisted local/staged SaveGame state. It is not caused by the current dirty worktree in the RetroFX source/config/material paths.

## State Map

### `FT66RetroFXSettings`

File: `Source/T66/Core/T66RetroFXSettings.h`

- `bEnableRetroFXMaster = true` at line 20.
- `bUseRealLowResolution = true` at line 53.
- `TargetResolutionHeightPercent = 40.0f` at line 62.
- Visible PS1/pixelation scalars default to zero, for example `WorldPixelationPercent = 0.0f` at line 89 and `CharacterPixelationPercent = 0.0f` at line 92.

This means `FT66RetroFXSettings()` currently represents "gameplay RetroFX enabled with real low resolution active," even though most blendable effect strengths are zero.

### `UT66PlayerSettingsSaveGame`

File: `Source/T66/Core/T66PlayerSettingsSaveGame.h`

- Duplicate saved master flag: `bRetroFXMasterEnabled = true` at line 234.
- Full settings struct: `FT66RetroFXSettings RetroFXSettings` at line 237.

This creates two saved sources that can represent the same gameplay master state.

### `UT66PlayerSettingsSubsystem`

File: `Source/T66/Core/T66PlayerSettingsSubsystem.cpp`

Key state flow:

- Schema 21 copies `RetroFXSettings.bEnableRetroFXMaster` into `bRetroFXMasterEnabled` at line 234.
- Schema 22 resets `RetroFXSettings = FT66RetroFXSettings()` and copies its master at lines 241-242.
- Schema 23 repeats the reset/copy at lines 249-250.
- After migration, the struct master is overwritten from the duplicate saved flag at line 254.
- `SetRetroFXSettings()` writes both the struct and duplicate master at lines 958-963.
- `GetRetroFXSettings()` returns static default settings when no settings object exists at lines 966-972, and overwrites the returned struct master from the duplicate flag at lines 974-975.
- `ResetRetroFXSettingsToDefaults()` uses `FT66RetroFXSettings()` and copies that master at lines 979-984.
- `ApplySafeModeSettings()` also resets `RetroFXSettings = FT66RetroFXSettings()` and copies the default master at lines 1042-1049.

Because `FT66RetroFXSettings()` is enabled by default, migrations, resets, safe-mode, and fallback getters can all restore gameplay RetroFX to enabled.

### `UT66RetroFXSubsystem`

File: `Source/T66/Core/T66RetroFXSubsystem.cpp`

Runtime application path:

- `BuildEffectiveSettings()` returns settings unchanged when `bEnableRetroFXMaster` is true at lines 399-403.
- If master is false, it explicitly disables real low resolution and pixelation weights at lines 406-430.
- `HandleRetroAssetPreloadComplete()` calls `ApplyCurrentSettings(World)` at lines 659-665, so async asset loading can re-apply saved/default state after a normal startup path.
- `ApplyCurrentSettings()` fetches saved settings and calls `ApplySettings()` at lines 668-675.
- `ApplySettings()` applies blendable weights, runtime resolution, pixelation stencil masks, and forwards pixelation levels to `UT66PixelationSubsystem` at lines 719-785.
- `ApplyResolutionRuntime()` writes runtime resolution when `bUseRealLowResolution` is true at lines 1025-1059, including `r.ScreenPercentage` at line 1054.

This is the main runtime writer that turns the visible effect back on.

### `UT66PixelationSubsystem`

File: `Source/T66/Core/T66PixelationSubsystem.cpp`

- `SetPixelationLevels()` clamps world/character levels and applies zero weight when both are zero at lines 80-95.
- The pixelation post-process material is added to a PPV with initial blend weight `0.0f` at line 188.
- `ApplyLevelToBlendable()` sets blend weight to `1.0f` only when the current level is above zero at lines 197-217.

This subsystem is not default-on by itself. It only enables the pixelation material when `UT66RetroFXSubsystem` computes non-zero world/character pixelation levels or another caller directly sets levels.

### Post-Process Volumes / Content

Files:

- `Source/T66/Gameplay/T66WorldVisualSetup.cpp`
- `Content/UI/M_PixelationPostProcess.uasset`
- `Content/Materials/Retro/*`
- `Content/UE5RFX/Materials/PostProcess/*`

Findings:

- `FT66WorldVisualSetup::EnsureNeutralVisualSetupForWorld()` finds/creates an unbound neutral PPV and applies neutral settings, not RetroFX material enablement, at `T66WorldVisualSetup.cpp` lines 546-548.
- Theme visuals apply theme atmosphere post-process at lines 593-597, again not the T66 pixelation material.
- The content search found RetroFX/UE5RFX material assets but no map-level reference to `M_PixelationPostProcess` as an always-on map PPV source. Runtime code adds the pixelation blendable and RetroFX PPV.

Conclusion: no independent map/content PPV default was found that forces pixelation on. The active mechanism is runtime code applying settings.

### Config CVars

Files:

- `Config/DefaultEngine.ini`
- `Config/DefaultDeviceProfiles.ini`

Findings:

- `DefaultEngine.ini` has `r.ScreenPercentage.Default=100.000000` at line 116 and default modes at lines 117-120.
- It also has `r.SecondaryScreenPercentage.GameViewport=100` at line 111 and `r.ScreenPercentage.MinResolution=0` at line 112.
- `DefaultDeviceProfiles.ini` has one profile CVar `r.ScreenPercentage=85` at line 56.

The config defaults do not explain the recurring strong pixelation by themselves. The logs show RetroFX runtime writes driving `r.ScreenPercentage` much lower than these config values.

### Persisted Settings

Files:

- `Saved/SaveGames/T66_PlayerSettings.sav`
- `Saved/StagedBuilds/Windows/T66/Saved/SaveGames/T66_PlayerSettings.sav`

Findings:

- Both local and staged saves contain serialized `bRetroFXMasterEnabled`, `RetroFXSettings`, and `bEnableRetroFXMaster` field names.
- The binary save values were not edited or decoded in this pass, but their presence proves that local/staged persisted state can preserve and re-apply old on states.

Persisted state is an amplifier and trigger vector. The underlying bug is still the committed default/migration/reset behavior that treats default gameplay RetroFX as on.

## Intended-Off State

Correct current gameplay state should mean:

- `RetroFXSettings.bEnableRetroFXMaster == false`
- `RetroFXSettings.bUseRealLowResolution == false`
- Runtime `r.ScreenPercentage` restored to the project/user baseline path, not overridden by RetroFX. In many desktop paths that baseline is 100 from `ApplyCrispRenderingDefaults()` / `DefaultEngine.ini`, but `DefaultDeviceProfiles.ini` can set profile-specific values such as 85, so follow-up verification should assert "no RetroFX downscale override" rather than hard-code 100 for every profile.
- PS1/N64/chromatic/outline post-process blend weights are zero.
- `WorldPixelationPercent`, `CharacterPixelationPercent`, and legacy `T66PixelationPercent` are zero.
- `UT66PixelationSubsystem` world/character/current levels are zero and its blendable weight is zero.
- UI-only CRT flags are treated separately from gameplay world RetroFX so frontend experimentation cannot turn on gameplay world downscale by accident.

Current committed defaults violate that intended-off state because gameplay master and real low resolution default on.

## Writers And Triggers

### Implicit defaults

- `FT66RetroFXSettings()` creates enabled gameplay RetroFX by default: `bEnableRetroFXMaster=true`, `bUseRealLowResolution=true`, `TargetResolutionHeightPercent=40.0f`.
- `UT66PlayerSettingsSaveGame::bRetroFXMasterEnabled` also defaults true.

Trigger: new settings object, missing settings object fallback, default reset, schema migration, or any temporary pending settings initialized from defaults.

### Save migration

- Schema 22 and 23 reset `RetroFXSettings` to defaults and copy the master flag from those defaults.

Trigger: launching a build with an older save schema. This can recur across staged builds, restored saves, or any run using a stale SaveGame.

### Settings reset / safe mode

- `ResetRetroFXSettingsToDefaults()` writes `FT66RetroFXSettings()` and saves it.
- `ApplySafeModeSettings()` writes `FT66RetroFXSettings()` and saves it.

Trigger: user or automation using reset/safe-mode. This is especially wrong because safe mode should never turn a visual downscale effect on.

### UI RetroFX tab

Files:

- `Source/T66/UI/Screens/Settings/T66SettingsScreen_RetroFX.cpp`
- `Source/T66/UI/Screens/Settings/T66SettingsScreen_Build.cpp`

Writers:

- Reset button calls `HandleResetRetroFXClicked()` then `ResetPendingRetroFXToDefaults()` at lines 298-300 and 427-430.
- Apply writes `PS->SetRetroFXSettings(PendingRetroFXSettings)` and calls `RetroFX->ApplySettings()` at lines 365-396.
- Build-screen toggles can live-apply gameplay master changes via `MarkRetroFXEdited()` / `ApplyPendingRetroFX()` at `T66SettingsScreen_Build.cpp` lines 219-224.

Trigger: opening settings, resetting defaults, live preview mode, or applying pending settings.

### Hero Selection inline RetroFX panel

File: `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_RetroFX.cpp`

Writers:

- Missing settings fallback initializes pending inline settings to `FT66RetroFXSettings()` at line 403.
- Reset initializes pending inline settings to `FT66RetroFXSettings()` at line 465.
- Apply writes `PS->SetRetroFXSettings()` and calls `RetroFX->ApplySettings()` at lines 425-431.

Trigger: hero selection inline RetroFX panel reset/apply/close with dirty settings.

### Gameplay settings changed path

File: `Source/T66/Gameplay/T66GameMode.cpp`

- `HandleSettingsChanged()` calls `RetroFX->ApplySettings(RetroSettings, GetWorld())` for normal runs at lines 1475-1499.
- It only forces master/real-low-resolution off for test-room runs at lines 1491-1497.
- If the player settings subsystem is missing, it falls back to `RetroFX->ApplyCurrentSettings(GetWorld())` at lines 1500-1503.

Trigger: gameplay world settings changed/startup path. This is the likely "unrelated work" trigger: any smoke, stage, map load, or settings refresh that reaches `HandleSettingsChanged()` re-applies saved/default RetroFX.

### Async asset preload callback

File: `Source/T66/Core/T66RetroFXSubsystem.cpp`

- `HandleRetroAssetPreloadComplete()` calls `ApplyCurrentSettings(World)` at lines 659-665.

Trigger: RetroFX material/asset preload finishing after startup. This can make the effect appear to flip on later, not only at the initial settings call.

### Direct pixelation console helper

File: `Source/T66/Core/T66Pixelation.cpp`

- `SetPixelationLevel()` forwards to `UT66PixelationSubsystem`.

Trigger: explicit console/helper call only. This is not the recurring default-on path found here.

## Runtime Evidence

Saved standalone logs contain direct evidence of the recurrence pattern:

- `Saved/StandaloneLogs/VisualCleanup_Iteration01_Stage1.log:613`: `ApplySettings` applies `MasterEnabled=true`, all PS1/chromatic/pixelation-style strengths shown at `0.00`, `RealLowRes=true`, `TargetRes=40.00`.
- `Saved/StandaloneLogs/VisualCleanup_Iteration01_Stage1.log:615`: `ApplyResolutionRuntime` lowers `r.ScreenPercentage` to `31.11` on a 1080p viewport.
- `Saved/StandaloneLogs/VisualLock_Iteration01_Verify.log:913`: saved RetroFX settings are applied with `MasterEnabled=true`, `RealLowRes=true`, and all listed PS1 strengths at zero.
- `Saved/StandaloneLogs/VisualLock_Iteration01_Verify.log:920`: runtime resolution is enabled with `ScreenPercentage=46.67`.
- `Saved/StandaloneLogs/VisualLock_Iteration02_verify_first_boot.log:1112-1124`: `ApplyCurrentSettings` re-applies saved RetroFX to `GameplayLevel`, then `ApplyResolutionRuntime` lowers `ScreenPercentage=31.11`.

These lines answer the important distinction: the recurring visible "pixelation" can be caused by `bUseRealLowResolution=true` alone. The logs show the explicit pixelation/PS1 effect strengths at zero while the real low-resolution path is still active.

## Dirty-State Determination

Narrow checks were used to avoid broad LFS/content scans.

Clean / no diff:

- `Source/T66/Core/T66RetroFXSettings.h`
- `Source/T66/Core/T66PlayerSettingsSaveGame.h`
- `Source/T66/Core/T66PlayerSettingsSubsystem.cpp`
- `Source/T66/Core/T66RetroFXSubsystem.cpp`
- `Source/T66/Core/T66PixelationSubsystem.cpp`
- `Source/T66/UI/Screens/Settings/T66SettingsScreen_RetroFX.cpp`
- `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_RetroFX.cpp`
- `Config/DefaultEngine.ini`
- `Config/DefaultDeviceProfiles.ini`
- Targeted material/content paths checked: `Content/UI/M_PixelationPostProcess.uasset`, `Content/Materials/Retro`, `Content/UE5RFX/Materials/PostProcess`

Dirty but not root cause:

- `Source/T66/Gameplay/T66GameMode.cpp` has unrelated companion spawn fallback and boss beacon changes. The RetroFX block at lines 1475-1503 is not modified by that diff.

Conclusion: the root cause is in committed source behavior. Local/staged SaveGame can preserve or replay the bad state, but current uncommitted RetroFX source/config/content changes are not the source.

## Root Cause

The project currently has no single explicit "gameplay RetroFX is off by default" source of truth.

Instead:

1. `FT66RetroFXSettings()` defaults gameplay RetroFX master on.
2. Real low-resolution also defaults on, so default settings lower runtime resolution even when visible PS1/pixelation scalars are zero.
3. `UT66PlayerSettingsSaveGame` duplicates the master flag and also defaults it on.
4. Save migrations, reset-to-defaults, safe-mode, and missing-settings fallback all recreate or copy those on defaults.
5. Gameplay world settings application and async preload callbacks re-apply saved/default settings into global runtime state.

That is why unrelated work appears to turn pixelation back on: the unrelated work does not directly enable pixelation. It causes a normal load/reset/settings/reapply path to run, and that path replays default-on or saved-on RetroFX state.

## Permanent Patch Proposal

Do not fix this by toggling the current setting off. That only changes one local state and leaves all default/reset/migration writers capable of restoring it.

Recommended follow-up patch:

1. Add an explicit disabled gameplay default.
   - Make `FT66RetroFXSettings()` represent current intended gameplay state: `bEnableRetroFXMaster=false`, `bUseRealLowResolution=false`, `TargetResolutionHeightPercent=100.0f` or a neutral value ignored while disabled.
   - Keep all scalar effect strengths zero.
   - Decide separately whether frontend-only `UIFullScreenCRTEnabled` should stay true or default false; do not let it control gameplay world resolution.

2. Consolidate the gameplay master source of truth.
   - Preferred: make `RetroFXSettings.bEnableRetroFXMaster` authoritative and treat `bRetroFXMasterEnabled` as legacy migration-only.
   - If the duplicate flag must stay for compatibility, centralize all reads/writes through one helper so the duplicate cannot diverge from the struct.
   - This consolidation must land before or with the new save migration. The current order resets the struct and then overwrites its master from the duplicate saved flag, so a stale duplicate can preserve the old on state unless the migration explicitly owns both fields.

3. Add a new save schema migration.
   - Convert old default-on signatures to off. A likely default-on signature is `bRetroFXMasterEnabled=true`, `bUseRealLowResolution=true`, default target resolution, and no non-zero gameplay effect strengths.
   - Preserve explicit user opt-ins: if saved settings have non-zero gameplay RetroFX strengths, keep them enabled unless Pablo wants all existing saves forced off.

4. Fix reset and safe-mode semantics.
   - `ResetRetroFXSettingsToDefaults()` must reset to the new off-by-default gameplay state.
   - `ApplySafeModeSettings()` must force gameplay RetroFX and real low-resolution off, not instantiate an enabled default.

5. Guard runtime application.
   - `ApplyCurrentSettings()` should only enable gameplay RetroFX from explicit user/saved opt-in, not from implicit construction defaults.
   - Keep `BuildEffectiveSettings()` disabling real low resolution when master is off; it already does this correctly once the master state is correct.

6. Add a diagnostic smoke check for the patch.
   - Fresh/no-save launch: log or assert effective gameplay RetroFX off, real low-resolution off, and no RetroFX-owned downscale override. Compare against the active project/device-profile baseline rather than hard-coding `r.ScreenPercentage=100` for every profile.
   - Old default-on save migration: verify it becomes off.
   - Explicit non-zero RetroFX save: verify it remains opt-in enabled if that preservation rule is chosen.
   - Settings reset and safe mode: verify both produce off gameplay state.
   - Gameplay map load/settings changed: verify no downscale is applied.

Follow-up product decisions before patch implementation:

- Save migration policy: preserve existing saves with non-zero gameplay RetroFX strengths as explicit opt-in, or force all existing saves off.
- Frontend default: decide whether `UIFullScreenCRTEnabled` remains true or moves to false when gameplay world RetroFX is decoupled.

## Should This Pass Patch It?

No. The root cause is clear, but the permanent fix crosses default values, persisted save migration, duplicate master semantics, reset/safe-mode behavior, and UI fallback semantics. That is low risk in concept but cross-cutting enough that it should be a separate reviewed patch, not silently folded into a diagnostic pass.

## Files Expected In Follow-Up Patch

Likely files:

- `Source/T66/Core/T66RetroFXSettings.h`
- `Source/T66/Core/T66PlayerSettingsSaveGame.h`
- `Source/T66/Core/T66PlayerSettingsSubsystem.cpp`
- `Source/T66/UI/Screens/Settings/T66SettingsScreen_RetroFX.cpp`
- `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_RetroFX.cpp`

Possible verification support:

- Existing staged standalone / settings smoke scripts, if a no-save and migrated-save smoke can be driven through current hooks.

## Verification Performed

- Read root, gameplay, reports, and performance process instructions.
- Ran Claude plan review; review approved diagnostic/report-only scope.
- Enumerated state holders and writers with `rg` and direct source reads.
- Ran narrow `git status`/`git diff` on RetroFX source/config/targeted content paths.
- Searched targeted content/map assets for pixelation/post-process references.
- Searched saved logs for `ApplySettings`, `ApplyCurrentSettings`, and `ApplyResolutionRuntime` evidence.
- Verified local and staged `T66_PlayerSettings.sav` files contain serialized RetroFX field names.

No runtime code/content/config edits were made.
