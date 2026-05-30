# Scripts

`Scripts` contains project automation that is called by Unreal, import workflows, data-table setup, staging, capture, and validation. Keep live callable scripts in this root until their source/docs callers are updated to a new path.

## Lifecycle Rule

Master scripts are reusable project tools. One-off task scripts should be deleted after the task is proven complete, with any durable lesson folded into an existing master script, a manifest format, a new reusable tool, or a canonical doc.

## Current Master Areas

- Build and package helpers: `StageStandaloneBuild.ps1`, `GuardT66RuntimeAssetContract.ps1`.
- Agent review helpers: `Invoke-ClaudePlanReview.ps1` is the subscription-backed Claude Validator helper for sealed packets when Codex is Operator. `Invoke-CodexPlanReview.ps1` is the separate local Codex CLI fallback or explicit Codex Validator helper when a separate Codex review artifact is needed. `AGENTS.md` is the canonical source for Tier 0/Tier 1 routing, strict first-line verdict semantics, fallback eligibility, and manual-approval rules.
- Claude direct-read helper: `Invoke-ClaudeDirectRead.ps1` runs Claude Code against the live repo in explicit `Review` or `Operator` mode. The baseline profile is read-only `plan` mode with `--allowedTools Read,Grep,Glob` and `--add-dir C:\UE\T66`. Operator output is marked `OperatorArtifactNotGreenlight` and requires Validator integration before implementation or completion claims rely on it.
- Review-helper self-tests: `Test-ClaudeReviewVerdictParser.ps1` verifies strict first-line verdict parsing, malformed-output classification, and that the Claude helper remains independent from the Codex fallback helper.
- UI capture/import helpers: `CaptureT66UIScreen.ps1`, `CaptureT66UIWidget.ps1`, `CaptureT66GameplayVideo.ps1`, `CaptureT66EnemyAnimationPreview.ps1`, `BuildT66VideoEvidenceBundle.py`, UI texture import and repair scripts.
- Data-table setup scripts: `Setup*DataTable.py`, roster/data reload helpers.
- Import core: `ImportStaticMeshes.py` plus active domain-specific import/verification wrappers. The old generic skeletal import and generic import-batch verifier were retired.
- Active batch wrappers: current Quad Retro, combat roster, weapon projectile, coherent theme kit, arcade replacement, and world NPC/interactable imports.
- Maintenance: focused audit, repair, and verification scripts that are still used by current docs or source-owned tooling.

## Operator/Validator Stack Helpers

Use `Invoke-ClaudePlanReview.ps1` when Claude is the Validator for a sealed Codex packet. Use `Invoke-ClaudeDirectRead.ps1` when Claude should inspect files directly as Operator or Validator. Use `Invoke-CodexPlanReview.ps1` only when a separate Codex CLI Validator artifact is required by the active process or fallback path.

The canonical T66 operator state is the local repo file `C:\UE\T66\.t66\operator-state.json`. Agents must read it at task start under the rules in `AGENTS.md`. The usage tray state under `%LOCALAPPDATA%\T66UsageTray\operator-state.json` is a display mirror. The state file is ignored by Git so operator switching does not create normal source-control churn.

### Operator switch commands

- `Make Claude operator`: Operator=Claude for heavier planning, investigation, and proposal work; Validator=Codex for critique and integration.
- `Make Codex operator`: Operator=Codex for heavier planning, investigation, and implementation work; Validator=Claude for review.

Unless the user explicitly names a different validator, the validator is always the other model. The switch changes role routing only; it does not enable Claude editor automation, file writes, Unreal Python, unrestricted shell use, or MCP access beyond the reviewed profile.

Apply operator-switch commands with:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\Set-T66Operator.ps1 -Operator Claude
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\Set-T66Operator.ps1 -Operator Codex
```

The script writes the repo state and mirrors the same values to the tray runtime JSON so the visible widget and future T66 chats stay aligned. Existing chats adopt the switch only after they read the current repo state on a new request.

Direct-read review profile:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\Invoke-ClaudeDirectRead.ps1 `
  -Mode Review `
  -PromptPath C:\UE\T66\Reports\AgentReviews\<Task>\direct_read_review_prompt.md `
  -TaskName <Task> `
  -ReviewedOperatorRun C:\UE\T66\Reports\AgentReviews\ClaudeDirectRead\<OperatorRun>
```

Direct-read operator profile:

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

Default loot UI capture targets exclude Mini/minigame content unless the user explicitly names Mini/minigames.

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
