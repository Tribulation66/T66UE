Codex Approval: APPROVE

## Approved Task

Have Claude produce a corrected fresh Unreal-owned gameplay video capture of the Hero 1 Bounce projectile proof using the original Bounce proof camera/framing, but without the extra preview-spawn targets that `-UseHero1AxePreviewStaging` adds by default.

## Approved Scope

- Use `Scripts/CaptureT66GameplayVideo.ps1` with:
  - `-CaptureMode hero1axebouncevfxbinding`
  - `-UseHero1AxePreviewStaging`
  - `-NoHero1AxeTargets`
  - `-FrameCount 72`
  - `-FrameRate 12`
  - `-EvidenceBundle`
  - `-EvidenceAutoSelectFrames`
  - a fresh output folder under `Saved/VideoCaptures/Hero1BounceOriginalCameraClean_20260529/`
- Do not use `-UseReviewCamera`.
- Confirm the resulting command line includes `-T66Hero1AxeAOECenterPlayer` and does not include `-T66Hero1AxeAOESpawnTargets`.
- Inspect the resulting video/contact sheet and confirm it matches the original Bounce proof framing while preserving the current two-link proof: hero -> primary, then primary -> ChainSecond, with no third projectile/third chain link.
- Update `Reports/AgentReviews/Hero1BounceOriginalCameraRecapture/claude_operator_completion.md` or write `claude_operator_completion_cleanrecapture.md` with the new command, artifacts, validation markers, and note why the prior recapture had `LinkCount=3`.

## Approved Tool Surface

Claude may use FullOperator mode through `Scripts\Invoke-ClaudeDirectRead.ps1` with shell/file access inside `C:\UE\T66` for the approved capture and report work.

## Required Process Rules

- Follow `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `Gameplay/GAMEPLAY_AGENTS.md`, `Reports/AGENTS.md`, and the Combat VFX capture rules.
- Use Unreal-owned capture only.
- Keep Mini/minigame systems out of scope.

## Explicitly Excluded Actions

- No source-code edits.
- No Unreal asset/content edits.
- No CSV/DataTable edits.
- No Git commit, push, reset, checkout, clean, or broad Git/LFS status scans.
- No staged standalone build.
- No deletion or cleanup of prior evidence.
- No changing Bounce runtime behavior or camera occluder behavior.

## Verification Required After Operator Run

- Fresh MP4 exists and is non-empty.
- Evidence bundle exists and includes `manifest.json`, `ffprobe.json`, and `contact_sheet.png`.
- New contact sheet/video matches the original Bounce proof framing and does not show the pale yellow block.
- Logs show `LinkCount=2`, `CombatVFXBounceLinkProjectile LinkIndex=0`, arrival callback/deferred spawn, `LinkIndex=1`, and no `LinkIndex=2`.
- Damage proof pass rows for primary and ChainSecond, with out-of-chain controls unhit.

## Approval Rationale

The first original-camera recapture correctly restored the camera framing but also reproduced extra preview targets because `-UseHero1AxePreviewStaging` added `-T66Hero1AxeAOESpawnTargets`. The corrected capture keeps the original camera and staging but suppresses preview targets so the proof matches the current two-link Bounce requirement.
