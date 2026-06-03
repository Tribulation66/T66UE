Codex Approval: APPROVE

## Approved Task

Have Claude produce a fresh Unreal-owned gameplay video capture of the already-implemented Hero 1 Bounce projectile proof using the standard enemy-visible gameplay angle, not the prior review-camera/top-down angle.

## Approved Scope

- Read live process docs and the current Bounce packet as needed.
- Run `Scripts/CaptureT66GameplayVideo.ps1` with `-CaptureMode hero1axebouncevfxbinding`.
- Do not pass `-UseReviewCamera`.
- Use a fresh output folder under `Saved/VideoCaptures/Hero1BounceStandardAngle_20260529/`.
- Produce an evidence bundle with retained frames, `ffprobe` metadata, contact sheet, and manifest.
- Inspect the resulting video/contact sheet enough to confirm enemies are visible and the camera angle is the standard gameplay-style framing.
- Produce a concise operator completion note under this task folder with capture command, artifact paths, and token/evidence pointers.

## Approved Tool Surface

Claude may use FullOperator mode through `Scripts\Invoke-ClaudeDirectRead.ps1` with shell/file access inside `C:\UE\T66` for the approved capture and report work only.

## Required Process Rules

- Follow `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `Gameplay/GAMEPLAY_AGENTS.md`, `Reports/AGENTS.md`, and the Combat VFX capture rules.
- Use Unreal-owned capture only; desktop screenshots are not valid.
- Current capture freshness is required; do not substitute the prior top-down/review-camera video.
- Keep Mini/minigame systems out of scope.

## Explicitly Excluded Actions

- No source-code edits.
- No Unreal asset/content edits.
- No CSV/DataTable edits.
- No Git commit, push, reset, checkout, clean, or broad Git/LFS status scans.
- No staged standalone build.
- No deletion or cleanup of prior evidence.
- No use of `-UseReviewCamera`.
- No changing the Bounce implementation unless the capture is impossible; if impossible, stop and report the blocker instead of patching code.

## Verification Required After Operator Run

- Fresh MP4 exists and is non-empty.
- Evidence bundle exists and includes `manifest.json`, `ffprobe.json`, and `contact_sheet.png`.
- Capture uses the standard gameplay-style angle with enemies visible.
- Logs still show the sequential Bounce proof: `CombatVFXBounceLinkProjectile LinkIndex=0`, arrival callback/deferred spawn, `LinkIndex=1`, and damage proof pass rows for primary and second target.

## Approval Rationale

The user identified the prior acceptance camera as unreadable and explicitly requested Claude to recapture at the standard angle with enemies visible. This is proof-bearing work, so Claude may run the capture in FullOperator mode after this narrow approval. The approved scope produces evidence only and does not reopen runtime implementation.
