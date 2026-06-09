# Pending Issues - Core

## Staged Readiness Durable Save Integrity Uses Stale Loaded Map

- Severity tag: [Major]
- What's wrong: The 2026-06-08 staged readiness run at `Saved/StagedBuildReadiness/20260608_140504` passed staging, shortcut verification, and frontend smoke, then failed in `RunDurableSaveIntegritySmokeGate.ps1` before lifecycle ran. The queue phase logged `[SaveIntegrity] FAIL` for slot 8 because `MetaMap=T66_SaveIntegrity_DurableGate_20260608_140901` matched the new marker but `LoadedMap=T66_SessionLoadedTravel_SessionLoadedTravel_20260608_031616` remained stale.
- Why it's out of scope now: The active pass changed tower room-size tuning and did not alter durable save queue/load semantics. The gate restored the backed-up slot 8 files after failure.
- What fixing it would entail: Reproduce the durable gate on a clean save slot/root, determine whether the queue shutdown path is failing to persist the loaded map or the verification path is reading a stale root, then update the save integrity harness/runtime path and rerun staged readiness.

## Resolved 2026-05-29 - RetroFX Default-On Recurrence And Low-Resolution Pixelation

- Former severity tag: [Major]
- What was wrong: `FT66RetroFXSettings` and the saved duplicate master flag recreated gameplay RetroFX, real low resolution, and frontend CRT as enabled through settings defaults, migration, reset, safe-mode, UI reset, save load, and world-startup application. The visible pixelation was primarily `r.ScreenPercentage` being reduced by real-low-resolution mode.
- Resolution: RetroFX/CRT now defaults off from the settings struct, the duplicate saved master flag was removed, schema 24 migration forces existing saves off, all named recreation paths were sealed by verification, and the off path restores `r.ScreenPercentage=100`.
- Evidence: `Reports/AgentReviews/20260529_RetroFXOffByDefaultFix/completion_packet.md`

## Resolved 2026-05-28 - Staged Standalone Build Blocked By Undeclared Accuracy Item ID

- Former severity tag: [Blocker]
- What was wrong: `Scripts\StageStandaloneBuild.ps1` failed during the Win64 `T66` target build on 2026-05-28 because `Source/T66/Core/T66GameInstance.cpp` referenced `AccuracyItemID` around lines 774, 775, 777, and 779, but that identifier was undeclared in the compile unit.
- Resolution: The item taxonomy pass retired the old secondary `Accuracy` item, replaced the random-pool fallback with `Execute`, and rebuilt `T66Editor` successfully. The remaining proof gate is the staged standalone refresh for this pass.

## Legacy Lab Unlock IDs In Existing Save Games

- Severity tag: [Minor]
- What's wrong: `UT66ProfileSaveGame::LabUnlockedEnemyIDs` stores raw enemy row names, and existing player saves may still contain legacy IDs such as `Dungeon_Slime` after the roster migration. Runtime source references were moved to the new production IDs, but no save migration remaps old lab unlock IDs to the 50-mob roster.
- Why it's out of scope now: This pass replaces the authored roster/data/assets and verifies the new gallery/spawn path; it does not change persistent player profile migration policy.
- What fixing it would entail: Add a profile migration table from the 25 legacy IDs to appropriate production IDs, run it during profile load, and verify old staged saves still expose expected lab unlocks.

## Skeletal Hero Rows Ignore MeshRelativeScale

- Severity tag: [Minor]
- What's wrong: `UT66CharacterVisualSubsystem::ApplyCharacterVisual` applies `MeshRelativeScale` for static hero visuals, but skeletal `Hero_` visuals are currently forced to `FVector::OneVector`. The accepted Royal Chad/Arthur skeletal asset therefore bakes the former live row scale `1.011123` into `SK_Hero_1_Chad_QuadRetroAnimQA` to preserve the selected character's runtime size.
- Why it's out of scope now: The Arthur pass needed to avoid broad changes to every skeletal hero, companion, and preview alignment path while fixing the live playable visual.
- What fixing it would entail: Add a data-driven skeletal hero scale policy in the character visual subsystem, regression-test preview and gameplay alignment, and remove per-asset baked-scale workarounds only after every affected hero row is revalidated.

## QuadRetro Mob Rows Reference Missing Pixel Textures

- Severity tag: [Minor]
- What's wrong: The map-transition staged gameplay smoke logged `LogT66CharacterVisuals` warnings from `Source/T66/Core/T66CharacterVisualSubsystem.cpp` for QuadRetro static mob visuals such as `Slime`, `BoneWalker`, `RatPack`, `CaveBat`, `HexSlinger`, `TombSpider`, `StoneSentinel`, `MimicLure`, `BoneConjurer`, and `CryptWraith` because their expected `/Game/Characters/Mobs/.../Textures/T_<Mob>` pixelated textures are missing in the packaged build.
- Why it's out of scope now: The map-transition pass only replaced tower wall/floor/ceiling visuals and did not alter mob visual rows, mob texture assets, or the QuadRetro fallback path.
- What fixing it would entail: Audit the mob visual data rows and packaged texture assets, either restore/import the referenced pixel textures or update the rows to the current production ToonStyle/VAT assets, then add a staged smoke check that `LogT66CharacterVisuals` no longer emits these missing-pixel-texture warnings.

## Headless Packaged Quit Returned Nonzero After Clean Log Exit

- Severity tag: [Major]
- What's wrong: During the 2026-06-07 world-transition consolidation pass, staged packaged `-nullrhi -nosound -unattended -NoSplash -ExecCmds=quit` smokes reached `Closing by request`, `FPlatformMisc::RequestExitWithStatus(0, 0, UGameEngine::HandleExitCommand)`, `LogExit: Exiting`, and `Log file closed`, but the Windows process returned `-1073740791` (`0xC0000409`) after the log closed in the observed failing runs. No new crash directory was produced under `Saved/Crashes`, and no matching Windows Application event was found during the Pass 3.5 follow-up.
- Pass 3.5 evidence: A focused 2026-06-07 reproduction matrix could not reproduce the nonzero return. Current staged packaged runs returned exit code `0` for 11 `-nullrhi` quits, 3 normal D3D quits, one `-nullrhi -nosteam` quit, and one D3D `-nosteam` quit. All current runs logged clean close markers. The current `-nullrhi` logs contain `NullRHI` markers and no D3D RHI teardown, while normal D3D logs contain D3D12 RHI initialization/teardown.
- Why it's still tracked: The historical `0xC0000409` return happened after clean log closure, so it cannot be classified from log evidence alone. The failing command line included `-nullrhi`, but the failing log's window title still reported `PCD3D_SM6`, so the historical failure cannot be cleanly attributed to NullRHI teardown. Without a reproduced faulting module/call stack, a T66 runtime shutdown bug is unproven. Keep this open as a watch/investigation trigger, not a resolved issue.
- What fixing it would entail: If the return code reappears, reproduce it under a debugger or Windows Error Reporting with dump capture and identify the faulting module/call stack. Only make runtime teardown changes if the stack names a T66-owned destructor, shutdown participant, or late callback. For normal staged quit proof, prefer the D3D packaged quit path for authoritative exit-code evidence; treat a one-off `-nullrhi` `0xC0000409` after clean log close as an investigation trigger, not as proof of a gameplay shutdown regression.

## Resolved 2026-06-08 - Session Subsystem Party Restriction Compile Errors Blocked Staged Standalone

- Former severity tag: [Blocker]
- What was wrong: During the 2026-06-07 floor landing-bounce pass, `Scripts\StageStandaloneBuild.ps1` reached the build phase and failed in `Source/T66/Core/T66SessionSubsystem.cpp`. The logged errors included `AT66PlayerController::ClientShowPartyLeaderboardRestrictionWarning` being inaccessible from `UT66SessionSubsystem` because the client RPC was protected, and `LobbyInfo.AccountRestrictionLabel = LexToString(Restriction.Restriction)` lacking a matching `LexToString` overload for `ET66AccountRestrictionKind`.
- Resolution: The 2026-06-08 floor landing-bounce verification rerun explicitly cleaned the `T66Editor Win64 Development` and `T66 Win64 Development` targets, then completed `Scripts\StageStandaloneBuild.ps1` successfully with a fresh 100-action compile/link, cook, stage, and package. It refreshed `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe` and verified a packaged quit smoke with exit code `0`. The final staged smoke log is `Saved/AgentReviews/FloorLandingBounce_20260608/staged_quit_smoke_after_clean_rebuild.log`.
