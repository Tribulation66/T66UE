Codex Approval: APPROVE

## Approved Task

Have Claude produce a fresh Unreal-owned gameplay video capture of the already-implemented Hero 1 Bounce projectile proof using the same camera/framing as the original accepted Bounce proof shown in `Saved/VideoCaptures/hero1axebouncevfxbinding_20260529_080850/`, and investigate the pale yellow block visible in the previous wrong-angle capture.

## Approved Scope

- Read the live process docs and current Bounce/capture artifacts as needed.
- Use `Scripts/CaptureT66GameplayVideo.ps1` with:
  - `-CaptureMode hero1axebouncevfxbinding`
  - `-UseHero1AxePreviewStaging`
  - `-FrameCount 72`
  - `-FrameRate 12`
  - `-EvidenceBundle`
  - `-EvidenceAutoSelectFrames`
  - a fresh output folder under `Saved/VideoCaptures/Hero1BounceOriginalCameraRecapture_20260529/`
- Do not use `-UseReviewCamera`.
- Inspect the new contact sheet/video and confirm it matches the original Bounce proof framing: hero visible from behind, enemies ahead, no large pale yellow occluder block over the hero.
- Investigate the pale yellow block source by reading camera/occluder code and relevant capture command lines/logs.
- Write `Reports/AgentReviews/Hero1BounceOriginalCameraRecapture/claude_operator_completion.md` with:
  - exact capture command,
  - video and evidence paths,
  - comparison against the original `080850` proof framing,
  - whether the yellow block appears in the recapture,
  - likely source of the yellow block with code/log anchors,
  - whether the cleanup should be immediate or tracked as a follow-up.

## Approved Tool Surface

Claude may use FullOperator mode through `Scripts\Invoke-ClaudeDirectRead.ps1` with shell/file access inside `C:\UE\T66` for the approved capture, inspection, and report work.

## Required Process Rules

- Follow `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `Gameplay/GAMEPLAY_AGENTS.md`, `Reports/AGENTS.md`, and the Combat VFX capture rules.
- Use Unreal-owned capture only; desktop screenshots are not valid.
- Current capture freshness is required; do not substitute older evidence as the new proof.
- Keep Mini/minigame systems out of scope.

## Explicitly Excluded Actions

- No source-code edits.
- No Unreal asset/content edits.
- No CSV/DataTable edits.
- No Git commit, push, reset, checkout, clean, or broad Git/LFS status scans.
- No staged standalone build.
- No deletion or cleanup of prior evidence.
- No changing the Bounce implementation.
- No changing the camera occluder system in this phase; identify the cleanup path only.

## Verification Required After Operator Run

- Fresh MP4 exists and is non-empty.
- Evidence bundle exists and includes `manifest.json`, `ffprobe.json`, and `contact_sheet.png`.
- New contact sheet/video matches the original Bounce proof framing from `hero1axebouncevfxbinding_20260529_080850`.
- New capture does not show the pale yellow block over the hero.
- Logs still show the current sequential Bounce proof: `CombatVFXBounceLinkProjectile LinkIndex=0`, arrival callback/deferred spawn, `LinkIndex=1`, and damage proof pass rows for primary and second target.

## Approval Rationale

The user clarified that the desired proof camera is the original Bounce proof framing, not the non-staged default preview camera and not the top-down review camera. This is proof-bearing work, so Claude may run the capture in FullOperator mode after this narrow approval. The yellow-block investigation is read-only because cleanup may touch the camera occluder runtime and should not be mixed into a capture-only pass without a separate accepted fix.
