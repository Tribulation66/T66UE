# Scripts

`Scripts` contains project automation that is called by Unreal, import workflows, data-table setup, staging, capture, and validation. Keep live callable scripts in this root until their source/docs callers are updated to a new path.

## Lifecycle Rule

Master scripts are reusable project tools. One-off task scripts should be deleted after the task is proven complete, with any durable lesson folded into an existing master script, a manifest format, a new reusable tool, or a canonical doc.

## Current Master Areas

- Build and package helpers: `StageStandaloneBuild.ps1`, `RunStagedBuildReadinessGate.ps1`, `RunRuntimeHealthGate.ps1`, `GuardT66RuntimeAssetContract.ps1`.
- Agent review helpers: `Invoke-ClaudePlanReview.ps1` is the subscription-backed Claude Validator helper when Codex explicitly engages Claude as Validator. `Invoke-CodexPlanReview.ps1` is the separate local Codex CLI fallback or explicit Codex Validator helper when a separate Codex review artifact is requested. `AGENTS.md` is the canonical source for opt-in validator routing, result semantics, fallback eligibility, and manual-approval rules.
- Claude direct-read helper: `Invoke-ClaudeDirectRead.ps1` runs Claude Code against the live repo in explicit `Review` or `Operator` helper modes. `Operator` is a helper mode name, not a standing T66 process role. The baseline profile is read-only `plan` mode with `--allowedTools Read,Grep,Glob` and `--add-dir C:\UE\T66`. Claude direct-work output requires Codex integration before implementation or completion claims rely on it.
- Review-helper self-tests: `Test-ClaudeReviewVerdictParser.ps1` verifies strict first-line verdict parsing, malformed-output classification, and that the Claude helper remains independent from the Codex fallback helper.
- UI capture/import helpers: `CaptureT66UIScreen.ps1`, `RunFrontendTagClickSmokeMatrix.ps1`, `CaptureT66UIWidget.ps1`, `CaptureT66GameplayVideo.ps1`, `CaptureT66EnemyAnimationPreview.ps1`, `BuildT66VideoEvidenceBundle.py`, UI texture import and repair scripts.
- Pre-release smoke gates: `RunPreReleaseSmokeSuite.ps1` is the default orchestrator for the frontend, durable save, and lifecycle gates. `RunLifecycleTransitionSmokeGate.ps1` wraps the development-only `T66.WorldRuntime.ProofTravel` command and validates the owner-local world-runtime transition manifest. `RunDurableSaveIntegritySmokeGate.ps1` wraps the development-only save integrity shutdown/reload harness and restores protected save files after proof. `RunSaveSlotsLoadClickSmoke.ps1` seeds a protected solo save slot, clicks the matching SaveSlots load button through non-shipping Slate tag automation, asserts the local `TransitionToGameplayLevel` loading markers, and restores protected save/index files. `RunSessionLoadedTravelSmoke.ps1` is the focused session-owned loaded-save travel-plan proof for multiplayer-shaped saves; it is not part of the default suite until release policy explicitly opts it in.
- Runtime health gates: `RunRuntimeHealthGate.ps1` is the packaged startup/runtime diagnostics coordinator. It composes cheap staged-build readiness with a delayed packaged launch, executable provenance check, fresh PerformanceSystem artifact validation, and schema/report/write-queue assertions.
- Data-table setup scripts: `Setup*DataTable.py`, roster/data reload helpers.
- Audio: `SetupAudioEventsDataTable.py` extracts/normalizes Helton pack WAVs, imports SoundWaves, and rebuilds `DT_AudioEvents` from its event spec list. `ComposeT66PlaceholderOSTs.py` (plain Python; numpy/scipy/ffmpeg) deterministically renders the placeholder OST loops/stingers into `Content/Audio/OSTS/<contract folders>`. `ImportGeneratedOSTs.py` (editor pythonscript) imports every OSTS `.ogg` in place — also the replacement path when professional tracks land. Folder contract and event inventory: `AUDIO_SYSTEM.md` at repo root.
- Import core: `ImportStaticMeshes.py` plus active domain-specific import/verification wrappers. The old generic skeletal import and generic import-batch verifier were retired.
- Active batch wrappers: current Quad Retro, combat roster, weapon projectile, coherent theme kit, and world NPC/interactable imports.
- Maintenance: focused audit, repair, and verification scripts that are still used by current docs or source-owned tooling. `Invoke-T66FoundationInventoryScan.ps1` is the reusable Lifecycle/Shutdown foundation scanner used by `LifecycleSystem/FOUNDATION_OWNERSHIP_INVENTORY.md` to produce classified before/after counts for travel, run reset, durable saves, shutdown, direct exits, and world cleanup hooks.

## Pre-Release Smoke Gates

Use the staged build readiness gate as the canonical one-shot release-candidate check. It refreshes the Development staged build through `StageStandaloneBuild.ps1`, verifies `T66 Standalone.lnk` targets the staged executable, then runs the pre-release smoke suite against the executable that the taskbar shortcut launches:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\RunStagedBuildReadinessGate.ps1
```

The readiness gate writes one combined `summary.json` / `summary.md` under `Saved/StagedBuildReadiness/`, while preserving staging stdout/stderr and the smoke suite's child evidence folders. Use `-SkipStage` for a fast rerun against the current staged executable, and use `-SkipSmoke` only when the current task is strictly staging/shortcut readiness. `-PrintOnly` prints the stage command instead of executing staging; if a staged executable already exists, it still launches the smoke suite in the suite's own print-only mode so child command lines and a dry summary are visible. `-ResetSavedGames` is forwarded to staging and clears staged saves before smoke proof, so use it deliberately.

Use the reusable smoke suite directly only when the staged executable is already current and the task is specifically about smoke coverage:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\RunPreReleaseSmokeSuite.ps1 `
  -Exe C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe
```

The suite runs the gates in the default order: frontend tag-click matrix, durable save integrity, then lifecycle transition. It writes one combined `summary.json` / `summary.md` under `Saved/PreReleaseSmokeSuite/`, while preserving each child gate's own evidence folder. It stops on first failure unless `-ContinueOnFailure` is passed. Use `-SkipFrontend`, `-SkipDurable`, or `-SkipLifecycle` only when the omitted gate is intentionally out of scope for the current build.

## Runtime Health Gate

Use the runtime health gate when the task needs packaged startup/runtime diagnostics proof but does not require a full restage and full pre-release smoke pass:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\RunRuntimeHealthGate.ps1
```

The gate writes `summary.json` / `summary.md` under `Saved/RuntimeHealthGate/`. It checks required repo files, schema docs, and runtime ownership inventory links, runs `RunStagedBuildReadinessGate.ps1 -SkipStage -SkipSmoke`, launches the staged executable with a delayed Unreal screenshot, verifies the executable hash did not drift during the run, then requires fresh PerformanceSystem snapshot/session artifacts with the current schema and a clean write queue. Use `PerformanceSystem/RUNTIME_HEALTH_GATE.md` as the owning extension contract and `PerformanceSystem/RUNTIME_OWNERSHIP_INVENTORY.md` to decide whether new proof belongs in this gate or another owner gate.

Focused rerun commands:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\RunFrontendTagClickSmokeMatrix.ps1 `
  -Exe C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe

powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\RunLifecycleTransitionSmokeGate.ps1 `
  -Exe C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe

powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\RunDurableSaveIntegritySmokeGate.ps1 `
  -Exe C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe

powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\RunSessionLoadedTravelSmoke.ps1 `
  -Exe C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe
```

The frontend matrix is the packaged frontend interaction gate. The lifecycle transition gate is structural world-runtime leak evidence around gameplay/frontend travel and requires a Development/non-shipping executable because `T66.WorldRuntime.ProofTravel` is compiled out of Shipping builds. The durable save integrity gate proves the save-owned shutdown flush path by running `T66.Save.QueueIntegrityShutdown` and `T66.Save.VerifyIntegritySlot` in separate processes, asserting PASS markers, and restoring the selected `T66_Slot_XX.sav` plus `T66_SaveIndex.sav` files afterward. If the suite reports `BUILD_CONFIG_UNSUPPORTED` for lifecycle or durable-state, verify the target build configuration before changing runtime lifecycle or shutdown code; Shipping builds should skip those gates explicitly rather than treating missing non-shipping commands as success.

## Packaged Quit Smoke Policy

For staged standalone quit proof, use the normal D3D packaged executable as the authoritative exit-code check unless the task specifically requires a headless/NullRHI lane. The accepted basic quit markers are:

- process exit code `0`
- `Closing by request`
- `FPlatformMisc::RequestExitWithStatus(0, 0, UGameEngine::HandleExitCommand)`
- `LogExit: Exiting`

`-nullrhi -ExecCmds=quit` is useful as a faster/headless diagnostic, but it is not the default authority for player-facing quit behavior. On 2026-06-07, a historical staged headless quit command that included `-nullrhi` returned `0xC0000409` after `LogExit: Exiting` and `Log file closed`, with no new `Saved/Crashes` directory and no matching Windows Application event. That failing log still used a `PCD3D_SM6` window title, so the failure was not proven to be NullRHI-specific. A follow-up Pass 3.5 matrix could not reproduce it across repeated current `-nullrhi` runs, and normal D3D quit returned `0`.

If a future headless quit returns `0xC0000409` after clean log close, rerun the exact command, run the normal D3D quit smoke, and collect a dump or Windows faulting-module stack before changing runtime shutdown code. Do not route proof-harness `RequestExitWithStatus` paths through player-facing `QuitGame` unless that proof explicitly requires full shutdown participation.

## Optional Validator Helpers

Use `Invoke-ClaudePlanReview.ps1` when Claude is explicitly engaged as Validator for a Codex packet or draft. Use `Invoke-ClaudeDirectRead.ps1` when Claude should inspect files directly as a requested direct worker or validator. Use `Invoke-CodexPlanReview.ps1` only when a separate Codex CLI Validator artifact is explicitly requested or required by a fallback path.

The canonical T66 validator state is the local repo file `C:\UE\T66\.t66\validator-state.json`. Agents read it under the rules in `AGENTS.md` to learn the named validator and `validatorMode`. The usage tray mirror lives at `%LOCALAPPDATA%\T66UsageTray\validator-state.json`. The state file is ignored by Git so validator setting changes do not create normal source-control churn.

### Validator setting commands

- `turn on validator setting`: stores `validatorMode=on`; future T66 prompts use the named validator until turned off.
- `turn off validator setting`: stores `validatorMode=off`; future T66 prompts return to normal single-agent operation.
- `implement validator Claude` or `implement validator Codex`: per-request only; engages that validator for the current stuck step/problem and does not write persistent state.

When validator mode is engaged and the user does not name a validator, the validator defaults to the value in `.t66\validator-state.json`. The setting changes review routing only; it does not enable Claude editor automation, file writes, Unreal Python, unrestricted shell use, or MCP access beyond the reviewed profile.

Apply validator setting commands with:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\Set-T66ValidatorMode.ps1 -ValidatorMode on
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\Set-T66ValidatorMode.ps1 -ValidatorMode off
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\Set-T66ValidatorMode.ps1 -Validator Claude
```

The script writes the repo validator state and mirrors the same values to the tray runtime JSON. Existing chats adopt persistent validator setting changes only after they read the current repo state on a new request.

Direct-read review profile:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\Invoke-ClaudeDirectRead.ps1 `
  -Mode Review `
  -PromptPath C:\UE\T66\Reports\AgentReviews\<Task>\direct_read_review_prompt.md `
  -TaskName <Task> `
  -ReviewedOperatorRun C:\UE\T66\Reports\AgentReviews\ClaudeDirectRead\<OperatorRun>
```

Direct-work profile:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\Invoke-ClaudeDirectRead.ps1 `
  -Mode Operator `
  -PromptPath C:\UE\T66\Reports\AgentReviews\<Task>\operator_prompt.md `
  -TaskName <Task>
```

Both profiles default to `claude-opus-4-8`, subscription-backed Claude Code auth, `--permission-mode plan`, and `--allowedTools Read,Grep,Glob`. They do not allow `Edit`, `Write`, unrestricted `Bash`, Unreal Python invocation, editor automation, or MCP tools unless a separate reviewed profile explicitly widens access.

Claude Blender access is configured through a user-scoped MCP entry named `blender` that points at the same official Blender MCP executable used by Codex. Verify with `claude mcp list` and `claude mcp get blender` before using it in production. Do not assume concurrent Codex and Claude Blender MCP sessions are safe until a task-specific smoke test proves it.

For Unreal and Niagara work, Claude should inspect repo files and Unreal-owned capture/dump artifacts. Raw GUI observation or desktop screenshots do not replace Unreal-owned proof from the capture scripts below.

## Unreal-Owned Visual Capture

Do not use computer-wide desktop screenshots as visual proof for T66 work unless the user explicitly asks for them. Capture the running game viewport through Unreal automation so the result is independent of what is visible on the user's monitor.

Frontend and screen capture:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 `
  -Exe C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe `
  -Screen MainMenu `
  -ResX 1920 -ResY 1080 `
  -Output C:\UE\T66\Saved\Codex\UI\MainMenu\proof.png `
  -ExtraArgs @("-T66AutoDumpScreen=C:\UE\T66\Saved\Codex\UI\MainMenu\dump.json")
```

Frontend interaction proof can click tagged FlatStyle buttons through Unreal/Slate before capture or exit:

```powershell
$extra = @(
  "-abslog=C:\UE\T66\Saved\StandaloneLogs\T66_QuitButtonProof.log",
  "-forcelogflush"
)

& C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 `
  -Exe C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe `
  -Screen MainMenu `
  -Modal QuitConfirmation `
  -ClickTag "QuitConfirmation.QuitButton" `
  -ClickDelaySeconds 2.5 `
  -WaitForExit `
  -ExtraArgs $extra
```

`-ClickTag` maps to the non-shipping runtime flag `-T66AutoClickTag=<Tag>`, resolves tags with the same Slate/FlatStyle metadata resolver as `T66.UI.DumpWidget`, and simulates the tagged `SButton` through Slate. Use `-WaitForExit` for quit-button proof; a successful quit should exit before a post-click screenshot exists.

Reusable frontend tag-click smoke matrix:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\RunFrontendTagClickSmokeMatrix.ps1 `
  -Exe C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe
```

The matrix is the reusable pre-release frontend interaction smoke gate. It proves high-value packaged frontend interactions through Unreal-owned screenshot and JSON dump artifacts: top-bar power opens `QuitConfirmation`, `QuitConfirmation.StayButton` closes the modal without shutdown, top-bar Settings/PowerUp/Account/Achievements navigation reaches the expected screens, and the Account History sub-tab can be reached through the same tagged-click path. The post-click dump is the primary assertion surface; click resolver/failure markers stay forbidden, but a missing click-completion log line is not a failure when the dump proves the expected target state. Account and Achievements assertions intentionally use screen/top-bar anchors instead of deep Steam/online-populated content so the gate remains valid in packaged offline runs. Keep the dump delay after the click delay and the screenshot delay after the dump delay so capture does not stop the process before the post-click dump is written.

Reusable lifecycle transition smoke gate:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\RunLifecycleTransitionSmokeGate.ps1 `
  -Exe C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe
```

The lifecycle gate wraps `T66.WorldRuntime.ProofTravel` with `Stress=1`, repeated frontend/gameplay travel, `ExitOnComplete=1`, and a manifest assertion pass. A passing run requires process exit code `0`, top-level manifest `status=complete`, completed travel count matching requested travel count, every snapshot reporting `non_current_world_proof_candidate_resource_count=0`, all six expected candidate subsystems present, and active stress resources populated through the existing owner APIs. It is observer-only evidence for world-runtime transition cleanup; it does not route through player-facing quit and it does not approve a new lifecycle coordinator by itself.

Reusable durable save integrity smoke gate:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\RunDurableSaveIntegritySmokeGate.ps1 `
  -Exe C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe
```

The durable gate wraps the non-shipping `T66.Save.QueueIntegrityShutdown` / `T66.Save.VerifyIntegritySlot` harness. It backs up candidate staged save roots for the selected slot file and `T66_SaveIndex.sav`, runs the queue-and-shutdown phase, runs the fresh-process reload verification phase, requires `[SaveIntegrity] PASS` and `[SaveIntegrityReload] PASS`, writes `summary.json` / `summary.md`, and restores or removes proof-created save files in a `finally` path. Slot `8` is the default reserved test slot for this gate; it is still written during proof, then restored from the backup snapshot. Use `-SlotIndex` only when a specific save slot must be tested.

Reusable session loaded-save travel-plan smoke:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\RunSessionLoadedTravelSmoke.ps1 `
  -Exe C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe
```

The session loaded-travel gate wraps the non-shipping `T66.Session.QueueLoadedTravelSeed` / `T66.Session.VerifyLoadedTravelPlan` harness owned by `UT66SessionSubsystem`. It backs up candidate save roots for the selected slot file and `T66_SaveIndex.sav`, seeds a deterministic Duo save with host/guest party metadata, reloads it in a fresh process, applies the loaded run through the session subsystem, verifies the computed gameplay `?listen` travel URL, writes `summary.json` / `summary.md`, and restores or removes proof-created save files in a `finally` path. It intentionally logs `LiveTravelSkipped=1`: this is session-owned plan proof, not a real two-peer `ServerTravel` or remote-client join proof. Slot `8` is the default reserved test slot.

Reusable SaveSlots loaded-save click smoke:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\RunSaveSlotsLoadClickSmoke.ps1 `
  -Exe C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe
```

This focused gate proves the local solo SaveSlots loaded-save resume path. It protects the selected first-page slot plus `T66_SaveIndex.sav`, seeds that slot through the development-only save-integrity harness, opens `SaveSlots`, writes a pre-click widget dump, clicks `SaveSlots.SlotN.LoadButton` through the same Slate tag resolver used by frontend automation, asserts the enabled-click log and `[LOAD] TransitionToGameplayLevel started pre-open asset preload.` / `[LOAD] TransitionToGameplayLevel opening` markers, then restores protected save/index files. It intentionally does not simulate multiplayer lobbies or session travel.

Gameplay HUD, overlay, or widget capture:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIWidget.ps1 `
  -Target "Class=UT66GameplayHUDWidget" `
  -CaptureMode hudreview `
  -ResX 1920 -ResY 1080 `
  -Output C:\UE\T66\Saved\Codex\GameplayHUD\proof.png `
  -Dump C:\UE\T66\Saved\Codex\GameplayHUD\dump.json
```

If the target visual state is not reachable through an existing `-T66FrontendScreen=<name>` or `-T66GameplayAutoCapture=<mode>` route, add a focused Unreal automation hook before claiming visual verification. The expected proof artifact is a PNG written by Unreal's screenshot request path, optionally paired with a JSON dump for UI/layout work.

### Hero Active-Ragdoll Capture

Use the gameplay video master script for Hero 1 active-ragdoll TestRoom proof:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66GameplayVideo.ps1 `
  -CaptureMode heroactiveragdollproof `
  -Output C:\UE\T66\Reports\Proof\Physics\HeroRagdollStage3AuthorityRebuild\heroactiveragdollproof.mp4 `
  -FrameCount 96 `
  -FrameRate 16 `
  -CaptureIntervalSeconds 0.0625
```

`heroactiveragdollproof` automatically adds `-T66AutomationTestRoom` and `-T66AutoCaptureHeroHPOverride=20000`, so it runs against the TestRoom wipeout arm instead of the regular tower layout. The accepted log gate is the arm scheduled plus `Reaction Applied=1 Source=TestRoomWipeoutArm`, `ActiveApplied=1`, and `LegacyApplied=0`.

### Combat VFX Evidence Bundle

For combat VFX reviews, keep the normal Unreal-owned gameplay video path and add the evidence bundle layer:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66GameplayVideo.ps1 `
  -CaptureMode hero1axeaoe `
  -UseHero1AxePreviewStaging `
  -FrameCount 120 `
  -FrameRate 24 `
  -CaptureIntervalSeconds 0.04 `
  -DelaySeconds 7.0 `
  -PostCaptureDelaySeconds 0.25 `
  -EvidenceBundle
```

`-EvidenceBundle` keeps the existing MP4 and PNG frame sequence, then writes `evidence\ffprobe.json`, `evidence\manifest.json`, `evidence\contact_sheet.png`, `evidence\selected_frames.md`, copied selected frames, and `evidence\visibility_checklist.md`. It packages review evidence only; it does not approve visual fidelity. Do not combine it with `-RemoveFrames`, because the selected-frame and contact-sheet evidence comes from the retained Unreal frame sequence. The bundle helper requires Python with Pillow plus ffmpeg/ffprobe available through the script's media-tool resolution paths.

### Combat VFX Editor-Isolation Capture

Use the MRQ isolation wrapper when a VFX needs a same-view black-background frame for target comparison before gameplay presentation is judged:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66NiagaraMRQIsolation.ps1 `
  -OutputDir C:\UE\T66\Saved\VFXResearch\Hero1Axe\AOE_AmericanFlagVisualTarget\EditorIsolation\<timestamp> `
  -ResX 1400 -ResY 1400 `
  -OrthoWidth 1250
```

The wrapper generates temporary `/Game/VFXLab/Temp/MRQ` assets, renders the actual lab Niagara actor through Movie Render Queue, writes `actual.png`, `actual_crop.png`, `contact_sheet.png`, `mismatch_notes.md`, and `manifest.json`, then deletes the temporary MRQ assets by default. `actual.png` is normalized as an opaque black review image while preserving rendered RGB so transparent-alpha display behavior cannot change the review background. It is an isolation/comparison gate only; it does not replace gameplay MP4 evidence or prove temporal mechanisms.

If Unreal returns a null process exit code after a successful MRQ/Python run, this wrapper only treats it as success when the process has exited, stdout contains known Unreal success markers, and the expected output artifacts pass verification. Do not generalize that behavior to other wrappers without equivalent success markers and artifact checks.

### Backrooms QA Capture

Backrooms verification is a non-shipping GameMode route because the target state lives inside the spawned tower stage. Force the entrance, select the QA branch, and let the runtime request the screenshot after the Backrooms entry has activated:

```powershell
C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe `
  -T66Entry=Run:Tower `
  -ExecCmds="T66.Backrooms.ForceSpawn 1" `
  -T66BackroomsAutoQA=Exit `
  -T66BackroomsAutoQAScreenshot=C:\UE\T66\Saved\Codex\Backrooms\backrooms_exit.png `
  -abslog=C:\UE\T66\Saved\StandaloneLogs\T66_BackroomsQA_Exit.log `
  -forcelogflush -nop4 -nosplash
```

Use `Exit` for successful exit and reward restore, `Death` for chaser touch death, and `Consume` for the reward item saving the hero from later lethal damage. Passing runs log `[BackroomsQA]` phases and exit with status `0`; failures log the failed phase at `Error` level and request exit with status `70`.

### Loot UI Animation Capture

Use the gameplay video master script for post-interaction loot UI captures:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66GameplayVideo.ps1 `
  -LootUIAnimationBatch `
  -ResX 1280 -ResY 720
```

Default loot UI capture targets include only the current loot UI targets listed below.

Taxonomy:

- `LiveLootUIAnimation`: target-owned animated UI after interaction. Current default targets: LootCrate, LootChest.
- `RewardCardUI`: post-reward/result card UI after interaction, not an opening/spin animation. Current default target: LootBag pickup card.
- `Gap_NoCurrentUIAnimation`: requested target lacks target-owned live UI animation. Current default gap: LootWheel spin UI; LootBag bag-opening/item-emerge animation if still absent.
- `SelectionCardUI`: pre-commit choice/selection card UI, such as IdolAltar or WeaponAltar. These modes may remain captureable but are not part of the default loot UI animation batch.

### Enemy Animation Preview Capture

Use the enemy animation preview wrapper when judging whether a mob's local VAT motion matches its actual in-map travel speed. This is the standard proof path after a Blender preview has passed body-type, facing, and no-stretch gates.

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66EnemyAnimationPreview.ps1 `
  -EnemyID BoneWalker `
  -FrameCount 75 `
  -FrameRate 15 `
  -StartDistance 3500 `
  -CameraDistance 350 `
  -CameraSideOffset 360 `
  -CameraHeight 165 `
  -PostCaptureDelaySeconds 4.0
```

The wrapper calls `CaptureT66GameplayVideo.ps1` with `-T66GameplayAutoCapture=enemyanimpreview`. The non-shipping capture mode spawns one configured `AT66MobBase`, lets `UT66MobManagerSubsystem` move it toward the hidden hero target, focuses an automation camera on the moving mob, and logs time-sampled location and `StoredVelocity` lines under `[EnemyAnimPreview]`. Use `-PostCaptureDelaySeconds` as the warm-up after the preview has been created; `-DelaySeconds` only delays setup. `-CameraDistance` is clamped to a minimum of `350` in the runtime capture mode, so pass `350` explicitly when reproducing the accepted side-preview lane. If a longer path is needed, use `-TargetForwardOffset` to shift the hidden target, but verify the path stays on visible floor.

### Hero Movement QA Capture

Use the hero movement QA mode when judging whether animated ToonStyle heroes and companions face the correct travel direction and whether walk cadence matches real in-map movement. This is the standard runtime proof path before changing hero/companion facing, speed, or animation play-rate logic.

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66GameplayVideo.ps1 `
  -CaptureMode heromovementqa `
  -FrameCount 120 `
  -FrameRate 15 `
  -DelaySeconds 4.0 `
  -PostCaptureDelaySeconds 0.1 `
  -ExtraArgs "-T66Entry=Tower -T66Hero=Hero_1 -T66HeroVisualOverride=Hero_1_Chad -T66HeroMovementQAVisualID=Hero_1_Chad -T66HeroMovementQACompanionID=Companion_01"
```

The non-shipping capture mode moves the real hero pawn through `CharacterMovement`, triggers jump and roll, spawns one automation-only companion, follows the hero with a camera, and logs root-motion classification plus time-sampled actor speed, `MaxWalkSpeed`, mesh yaw, and skeletal play rate under `[HeroMovementQA]`. The first pass should be a baseline capture before behavior changes.

## Cleanup Boundary

Do not add new task-specific scripts here by default. Prefer a manifest plus an existing master script. If a custom script is unavoidable, delete it after the task is complete unless it has been promoted into a reusable master.
